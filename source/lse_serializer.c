#include <lse.h>
#include <lse_stream.h>

int lse_write(const char* filename, const lse_DATA_t* data)
{
	FILE* fs = NULL;
	uint8_t* mem = NULL;
	size_t size;

	if (!(fs = fopen(filename, "wb")))
		return 0;

	if (lse_serialize(&mem, &size, data) != LSE_OK)
		goto ERR;

	if (fwrite(mem, 1, size, fs) != size)
		goto ERR;
	
	fclose(fs);
	free(mem);	
	return 1;

ERR:
	fclose(fs);
	free(mem);
	return 0;
}

//required 0x60 byte buffer
int lse_serialize_mii(const MiiData* mii, uint8_t* miiBuf)
{
#ifdef LSE_PADDING_MII
	static_assert(sizeof(MiiData) == 0x5C, "Can not use memcpy() for MiiData on your env. Please dont define LSE_PADDING_MII.");
	memcpy(miiBuf, mii, 0x5C);
#else

	//mii data
	miiBuf[0] = mii->magic;
	miiBuf[1] = (mii->mii_options.allow_copying & 1) | (mii->mii_options.is_private_name & 1)<<1 | 
		(mii->mii_options.region_lock & 3)<<2 | (mii->mii_options.char_set & 3)<<4;
	miiBuf[2] = (mii->mii_pos.page_index & 15) | (mii->mii_pos.slot_index & 15)<<4;
	miiBuf[3] = (mii->console_identity.unknown0 & 15) | (mii->console_identity.origin_console & 7)<<4;
	for (uint8_t i = 0; i < 8; i++)
		miiBuf[i+4] = (mii->system_id>>(i*8) & 0xFF);
//big endian
	for (uint8_t i = 0; i < 4; i++)
		miiBuf[i+12] = (mii->system_id>>((3-i)*8) & 0xFF);
	memcpy(&miiBuf[16], mii->mac, 6);
	miiBuf[22] = mii->pad[0]; //padding 0
	miiBuf[23] = mii->pad[1];
	miiBuf[24] = (mii->mii_details.sex & 1) | (mii->mii_details.bday_month & 15)<<1 | (mii->mii_details.bday_day & 7)<<5;
	miiBuf[25] = (mii->mii_details.bday_day>>3 & 3) | (mii->mii_details.shirt_color & 15)<<2 | (mii->mii_details.favorite & 1)<<6;
	/*
	//utf16 little
	for (size_t i = 0; i < 10; i++)
		mii->mii_name[i] = (uint16_t)miiBuf[26+(i*2)] | (uint16_t)miiBuf[27+(i*2)]<<8;
	
	mii->height = miiBuf[46];
	mii->width = miiBuf[47];
	mii->face_style.disable_sharing = miiBuf[48] & 1; 
	mii->face_style.shape = miiBuf[48]>>1 & 15;
	mii->face_style.skinColor = miiBuf[48]>>5 & 7;
	mii->face_details.wrinkles = miiBuf[49] & 15;
	mii->face_details.makeup = miiBuf[49]>>4 & 15;
	mii->hair_style = miiBuf[50];
	mii->hair_details.color = miiBuf[51] & 7;
	mii->hair_details.flip = miiBuf[51]>>3 & 1;

	uint32_t eyeBuf = (uint32_t)miiBuf[52] | (uint32_t)miiBuf[53]<<8 | (uint32_t)miiBuf[54]<<16 | (uint32_t)miiBuf[55]<<24;
	mii->eye_details.style = eyeBuf & 63;
	mii->eye_details.color = eyeBuf>>6 & 7;
	mii->eye_details.scale = eyeBuf>>9 & 15;
	mii->eye_details.yscale = eyeBuf>>13 & 7;
	mii->eye_details.rotation = eyeBuf>>16 & 31;
	mii->eye_details.xspacing = eyeBuf>>21 & 15;
	mii->eye_details.yposition = eyeBuf>>25 >> 31;

	uint32_t eyebrowBuf = (uint32_t)miiBuf[56] | (uint32_t)miiBuf[57]<<8 | (uint32_t)miiBuf[58]<<16 | (uint32_t)miiBuf[59]<<24;
	mii->eyebrow_details.style = eyebrowBuf & 31;
	mii->eyebrow_details.color = eyebrowBuf>>5 & 7;
	mii->eyebrow_details.scale = eyebrowBuf>>8 & 15;
	mii->eyebrow_details.yscale = eyebrowBuf>>12 & 7;
	mii->eyebrow_details.pad = eyebrowBuf>>15 & 1; 		//unnecess
	mii->eyebrow_details.rotation = eyebrowBuf>>16 & 31;
	mii->eyebrow_details.xspacing = eyebrowBuf>>21 & 15;
	mii->eyebrow_details.yposition = eyebrowBuf>>25 & 31;

	uint16_t noseBuf = (uint16_t)miiBuf[60] | (uint16_t)miiBuf[61]<<8;
	mii->nose_details.style = noseBuf & 31;
	mii->nose_details.scale = noseBuf>>5 & 15;
	mii->nose_details.yposition = noseBuf>>9 & 31;

	//mouth and mustache
	uint32_t mouthBuf = (uint32_t)miiBuf[62] | (uint32_t)miiBuf[63]<<8 | (uint32_t)miiBuf[64]<<16 | (uint32_t)miiBuf[65]<<24;
	mii->mouth_details.style = mouthBuf & 63;
	mii->mouth_details.color = mouthBuf>>6 & 7;
	mii->mouth_details.scale = mouthBuf>>9 & 15;
	mii->mouth_details.yscale = mouthBuf>>13 & 7;
	mii->mustache_details.mouth_yposition = mouthBuf>>16 & 15;
	mii->mustache_details.mustach_style = mouthBuf>>21 & 7;
	//pad

	uint16_t beardBuf = (uint16_t)miiBuf[66] | (uint16_t)miiBuf[67]<<8;
	mii->beard_details.style = beardBuf & 7;
	mii->beard_details.color = beardBuf>>3 & 7;
	mii->beard_details.scale = beardBuf>>6 & 15;
	mii->beard_details.ypos = beardBuf>>10 & 31;

	uint16_t glassesBuf = (uint16_t)miiBuf[68] | (uint16_t)miiBuf[69]<<8;
	mii->glasses_details.style = glassesBuf & 15;
	mii->glasses_details.color = glassesBuf>>4 & 7;
	mii->glasses_details.scale = glassesBuf>>7 & 15;
	mii->glasses_details.ypos = glassesBuf>>11 & 31;

	uint16_t moleBuf = (uint16_t)miiBuf[70] | (uint16_t)miiBuf[71]<<8;
	mii->mole_details.enable = moleBuf & 1;
	mii->mole_details.scale = moleBuf>>1 & 15; //wrong number of bits on MIiData??
	mii->mole_details.xpos = moleBuf>>5 & 31;
	mii->mole_details.ypos = moleBuf>>10 & 31;
	
	//utf16 little
	for (size_t i = 0; i < 10; i++)
		mii->author_name[i] = (uint16_t)miiBuf[72+(i*2)] | (uint16_t)miiBuf[73+(i*2)]<<8;
	*/
#endif

	//padding, crc16
	miiBuf[0x5C] = miiBuf[0x5D] = 0;
	uint16_t crc16 = lse_crc16(miiBuf, 0x5E);
	miiBuf[0x5E] = crc16>>8 & 0xFF;
	miiBuf[0x5F] = crc16 & 0xFF;

	return LSE_OK;
}

int lse_write_mii(const MiiData* mii, lse_StreamW* stream)
{
	int result;
	uint8_t miiBuf[0x60]; //mii(0x5C) + padding??  + crc16(0x2)

	if ((result = lse_serialize_mii(mii, miiBuf)) != LSE_OK)
		return result;

	return lse_streamW_write(stream, miiBuf, sizeof(miiBuf)) ? LSE_OK : LSE_STREAM_ERROR;
}

int lse_write_rivals(const lse_DATA_t* data, lse_StreamW* stream)
{
	int res;
	for (uint16_t i = 0; i < data->numRivals; i++)
	{
		const MiiData* mii = &data->rivals[i].mii;
		if ((res = lse_write_mii(mii, stream)) != LSE_OK)
			return res;
	}

	//time here
	for (uint16_t i = 0; i < data->numRivals; i++)
	{
		uint8_t time[2] = {data->rivals[i].time & 0xFF, data->rivals[i].time>>8 & 0xFF};
		if (!lse_streamW_write(stream, time, 2))
			return res;
	}
	
	return LSE_OK;
}

int lse_serialize(uint8_t** mem, size_t* size, const lse_DATA_t* data)
{
	int result;
	uint8_t tmp_size[4];
	uint32_t crc32;

	if (!mem || !size)
		return LSE_INVALID_ARG;

	*mem = NULL;
	*size = 0;

	lse_StreamW stream;
	if (!lse_streamW_init(&stream))
		return LSE_STREAM_ERROR;

	if (!lse_streamW_seek(&stream, 0x10))
		return LSE_STREAM_ERROR;

	//todo endian
	if (!lse_streamW_write(&stream, &data->lastsaved, 4))
		return LSE_STREAM_ERROR;
	
	//mii
	if (!lse_streamW_seek(&stream, 0x698))
		return LSE_STREAM_ERROR;
	result = lse_write_rivals(data, &stream);
	if (result != LSE_OK)
		goto ERR;
	
	//padding??
	if (!lse_streamW_move(&stream, 0x20))
	{
		result = LSE_STREAM_ERROR;
		goto ERR;
	}

	*size = lse_streamW_tell(&stream);
	if (*size < LSE_FILE_SIZE_MIN || *size > LSE_FILE_SIZE_MAX)
	{
		result = LSE_INVALID_SIZE;
		goto ERR;
	}
	tmp_size[0] = *size & 0xFF;
	tmp_size[1] = *size>>8 & 0xFF;
	tmp_size[2] = *size>>16 & 0xFF;
	tmp_size[3] = *size>>24 & 0xFF;
	
	(void)lse_streamW_seek(&stream, 4);
	(void)lse_streamW_write(&stream, lse_header_magic, sizeof(lse_header_magic));
	(void)lse_streamW_write(&stream, tmp_size, 4);

	(void)lse_streamW_seek(&stream, 4);
	crc32 = lse_crc32(lse_streamW_ptr(&stream), *size - 4);
	(void)lse_streamW_seek(&stream, 0);

	*mem = lse_streamW_ptr(&stream); //first address
	(void)lse_streamW_write(&stream, &crc32, 4); //todo: endian

	return LSE_OK;

ERR:
	lse_streamW_delete(&stream);
	return result;
}
