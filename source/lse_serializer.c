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
	miiBuf[0] = mii->magic; //3: valid mii, 0: empty mii
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
	
	//utf16 little
	for (size_t i = 0; i < 10; i++)
	{
		miiBuf[26+(i*2)] = mii->mii_name[i] & 0xFF;
		miiBuf[27+(i*2)] = mii->mii_name[i]>>8 & 0xFF;
	}
	
	miiBuf[46] = mii->height;
	miiBuf[47] = mii->width;
	miiBuf[48] = (mii->face_style.disable_sharing & 1) |  (mii->face_style.shape & 15)<<1 | (mii->face_style.skinColor & 7)<<5;
	miiBuf[49] = (mii->face_details.wrinkles & 15) | (mii->face_details.makeup & 15)<<4;
	miiBuf[50] = mii->hair_style;
	miiBuf[51] = (mii->hair_details.color & 7) | (mii->hair_details.flip & 1)<<3;

	uint32_t eyeBuf = (mii->eye_details.style & 63) 
		| (mii->eye_details.color & 7)<<6
		| (mii->eye_details.scale & 15)<<9
		| (mii->eye_details.yscale & 7)<<13
		| (mii->eye_details.rotation & 31)<<16
		| (mii->eye_details.xspacing & 15)<<21
		| (mii->eye_details.yposition & 31)<<25; 
	miiBuf[52] = eyeBuf & 0xFF;
	miiBuf[53] = eyeBuf>>8 & 0xFF;
	miiBuf[54] = eyeBuf>>16 & 0xFF;
	miiBuf[55] = eyeBuf>>24 & 0xFF;

	uint32_t eyebrowBuf = (mii->eyebrow_details.style & 31) 
		| (mii->eyebrow_details.color & 7)<<5
		| (mii->eyebrow_details.scale & 15)<<8
		| (mii->eyebrow_details.yscale & 7)<<12
		| (mii->eyebrow_details.pad & 1)<<15		//unnecess
		| (mii->eyebrow_details.rotation & 31)<<16	//mask is 15?(4 bit)
		| (mii->eyebrow_details.xspacing & 15)<<21
		| (mii->eyebrow_details.yposition & 31)<<25; 
	miiBuf[56] = eyebrowBuf & 0xFF;
	miiBuf[57] = eyebrowBuf>>8 & 0xFF;
	miiBuf[58] = eyebrowBuf>>16 & 0xFF;
	miiBuf[59] = eyebrowBuf>>24 & 0xFF;

	uint16_t noseBuf = (mii->nose_details.style & 31) 
		| (mii->nose_details.scale & 15)<<5
		| (mii->nose_details.yposition & 31)<<9; 
	miiBuf[60] = noseBuf & 0xFF;
	miiBuf[61] = noseBuf>>8 & 0xFF;

	uint32_t mouthBuf = (mii->mouth_details.style & 63) 
		| (mii->mouth_details.color & 7)<<6
		| (mii->mouth_details.scale & 15)<<9
		| (mii->mouth_details.yscale & 7)<<13
		| (mii->mustache_details.mouth_yposition & 31)<<16
		| (mii->mustache_details.mustach_style & 15)<<21; 
		//padding unnecess for integer
	miiBuf[62] = mouthBuf & 0xFF;
	miiBuf[63] = mouthBuf>>8 & 0xFF;
	miiBuf[64] = mouthBuf>>16 & 0xFF;
	miiBuf[65] = mouthBuf>>24 & 0xFF;

	uint16_t beardBuf = (mii->beard_details.style & 7) 
		| (mii->beard_details.color & 7)<<3
		| (mii->beard_details.scale & 15)<<6 
		| (mii->beard_details.ypos & 31)<<10;
	miiBuf[66] = beardBuf & 0xFF;
	miiBuf[67] = beardBuf>>8 & 0xFF;

	uint16_t glassesBuf = (mii->glasses_details.style & 15) 
		| (mii->glasses_details.color & 7)<<4
		| (mii->glasses_details.scale & 15)<<7 
		| (mii->glasses_details.ypos & 31)<<11;
	miiBuf[68] = glassesBuf & 0xFF;
	miiBuf[69] = glassesBuf>>8 & 0xFF;

	uint16_t moleBuf = (mii->mole_details.enable & 1) 
		| (mii->mole_details.scale & 15)<<1
		| (mii->mole_details.xpos & 31)<<5 			//wrong number of bits on MiiData??
		| (mii->mole_details.ypos & 31)<<10;
	miiBuf[70] = moleBuf & 0xFF;
	miiBuf[71] = moleBuf>>8 & 0xFF;
	
	//utf16 little
	for (size_t i = 0; i < 10; i++)
	{
		miiBuf[72+(i*2)] = mii->author_name[i] & 0xFF;
		miiBuf[73+(i*2)] = mii->author_name[i]>>8 & 0xFF;
	}

#endif

	//padding, crc16(big endian)
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
