#include <lse.h>
#include <assert.h>
#include <lse_stream.h>

int lse_read(const char *filename, lse_DATA_t **data)
{
	uint8_t* buf = NULL;

	FILE* fs = fopen(filename, "rb");
	if (!fs)
		return 0;
	
	long size;

	if (fseek(fs, 0, SEEK_END) != 0)
		goto ERR;
	
	size = ftell(fs);

	if (fseek(fs, 0, SEEK_SET) != 0)
		goto ERR;

	if (!(buf = malloc(size)))
		goto ERR;
	
	if (fread(buf, 1, size, fs) != size)
		goto ERR;
	
	if (lse_deserialize(buf, size, data, true) != LSE_OK)
		goto ERR;

	free(buf);
	fclose(fs);
    return 1;
ERR:
	free(buf);
	fclose(fs);
	*data = NULL;
	return 0;
}

int lse_deserialize_mii(MiiData* mii, const uint8_t* miiBuf, bool validate)
{
	if (validate)
	{
		uint16_t crc16 = lse_crc16(miiBuf, 0x5E); //include padding (0x5C, 0x5D)
		uint16_t curCRC = (uint16_t)((miiBuf[0x5E]<<8)|miiBuf[0x5F]); //big endian
		if (crc16 != curCRC)
			return LSE_CRC_ERROR;
	}

#ifdef LSE_PADDING_MII
	static_assert(sizeof(MiiData) == 0x5C, "Can not use memcpy() for MiiData on your env. Please dont define LSE_PADDING_MII.");
	memcpy(mii, miiBuf, 0x5C);
#else

	//mii data
	mii->magic = miiBuf[0]; //usually 3, but empty is 0
	mii->mii_options.allow_copying = miiBuf[1] & 1;
	mii->mii_options.is_private_name = miiBuf[1]>>1 & 1;
	mii->mii_options.region_lock = miiBuf[1]>>2 & 3;
	mii->mii_options.char_set = miiBuf[1]>>4 & 3;
	mii->mii_pos.page_index = miiBuf[2] & 15;
	mii->mii_pos.slot_index = miiBuf[2]>>4 & 15;
	mii->console_identity.unknown0 = miiBuf[3] & 15;
	mii->console_identity.origin_console = miiBuf[3]>>4 & 7;
	mii->system_id = (uint64_t)miiBuf[4] | (uint64_t)miiBuf[5]<<8 | (uint64_t)miiBuf[6]<<16 | (uint64_t)miiBuf[7]<<24 | (uint64_t)miiBuf[8]<<32 | (uint64_t)miiBuf[9]<<40 | (uint64_t)miiBuf[10]<<48 | (uint64_t)miiBuf[11]<<56;
//big endian
	mii->mii_id = (uint32_t)miiBuf[12]<<24 | (uint32_t)miiBuf[13]<<16 | (uint32_t)miiBuf[14]<<8 | (uint32_t)miiBuf[15];
	memcpy(mii->mac, &miiBuf[16], 6);
	mii->pad[0] = miiBuf[22]; 		//padding, unuse
	mii->pad[1] = miiBuf[23]; 
	mii->mii_details.sex = miiBuf[24] & 1;
	mii->mii_details.bday_month = miiBuf[24]>>1 & 15;
	mii->mii_details.bday_day = (miiBuf[24]>>5 & 7) | (miiBuf[25] & 3)<<3; //(bit 5 - 9)
	mii->mii_details.shirt_color = miiBuf[25]>>2 & 15;
	mii->mii_details.favorite = miiBuf[25]>>6 & 1;

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
	mii->eye_details.yposition = eyeBuf>>25 & 31;

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
	mii->mustache_details.mouth_yposition = mouthBuf>>16 & 31;
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

#endif

	return LSE_OK;
}

int lse_read_mii(MiiData* mii, lse_StreamR* stream, bool validate)
{
	uint8_t miiBuf[0x60]; //mii(0x5C) + padding??  + crc16(0x2)
		
	//miibuf -> static warning
	if (!lse_streamR_read(stream, &miiBuf[0], 0x60))
		return LSE_STREAM_ERROR;

	return lse_deserialize_mii(mii, miiBuf, validate);
}

//make cource list
int lse_read_rivals(lse_DATA_t* data, lse_StreamR* stream, bool validate)
{
	for (uint16_t i = 0; i < data->numRivals; i++)
	{
		MiiData* mii = &data->rivals[i].mii;
		int res;
		if ((res = lse_read_mii(mii, stream, validate)) != LSE_OK)
			return res;
	}

	//continue to read time func
	for (uint16_t i = 0; i < data->numRivals; i++)
	{
		//little endian
		uint8_t time[2];
		if (!lse_streamR_read(stream, &time, 2))
			return LSE_STREAM_ERROR;
		
		data->rivals[i].time = (uint16_t)time[0] | (uint16_t)time[1]<<8;
	}

	return LSE_OK;
}

int lse_read_stage_normal(lse_normal_stage_t* stage, lse_StreamR* stream)
{
	uint8_t stage_buf[0x0C];
	if (!lse_streamR_read(stream, stage_buf, sizeof(stage_buf)))
		return LSE_STREAM_ERROR;

	stage->cleared = stage_buf[0];
	stage->status = stage_buf[1];

	stage->misscount = stage_buf[5] | stage_buf[6]<<8 | stage_buf[7]<<16 | stage_buf[8]<<24;
	stage->flagheight = stage_buf[9];
	stage->time = stage_buf[10] | stage_buf[11]<<8;

	return LSE_OK;
}

int lse_read_mystery_box(lse_mystery_box_t* stage, lse_StreamR* stream)
{
	uint8_t stage_buf[0x75];
	if (!lse_streamR_read(stream, stage_buf, sizeof(stage_buf)))
		return LSE_STREAM_ERROR;

	stage->cleared = stage_buf[0];
	stage->status = stage_buf[1];

	stage->misscount = stage_buf[5] | stage_buf[6]<<8 | stage_buf[7]<<16 | stage_buf[8]<<24;
	
	stage->nextbox = stage_buf[0x6A];

	stage->isopen = stage_buf[0x6C];
	stage->lastopened = stage_buf[0x6D] | stage_buf[0x6E]<<8 | stage_buf[0x6F]<<16 | stage_buf[0x70]<<24;

	return LSE_OK;
}

int lse_read_toad_house(lse_toad_house_t* stage, lse_StreamR* stream)
{
	uint8_t stage_buf[0x13];
	if (!lse_streamR_read(stream, stage_buf, sizeof(stage_buf)))
		return LSE_STREAM_ERROR;

	stage->cleared = stage_buf[0];
	stage->status = stage_buf[1];

	stage->isopen = stage_buf[10];
	stage->lastopened = stage_buf[11] | stage_buf[12]<<8 | stage_buf[13]<<16 | stage_buf[14]<<24;

	return LSE_OK;
}

///@param validate Validate mii crc16?
int lse_read_file(lse_FILE_t* file, lse_StreamR* stream, bool validate)
{
	int res; //function results
	uint8_t header[0x80]; //[0 - (Begin Of Stages(variable element))]

	if (!lse_streamR_read(stream, header, sizeof(header)))
		return LSE_STREAM_ERROR;

	file->coin = header[0];
	
	file->powerup = header[2];
	file->pos = header[3];
	file->stockitem = header[4];
	
	file->isLuigi = header[9];
	file->mbox_visitedcount = header[10];
	file->mbox_lastbox = header[11];
	file->mbox_starcoin = header[12] | header[13]<<8;

	file->pictures = header[18] | header[19]<<8;
	file->progress = (uint32_t)header[20] | (uint32_t)header[21]<<8 | (uint32_t)header[22]<<16 | (uint32_t)header[23]<<24;
//mii
	if ((res = lse_deserialize_mii(&file->mii, &header[24], validate)) != LSE_OK)
		return res;
	file->lastsaved = header[120] | header[121]<<8 | header[122]<<16 | header[123]<<24;	

//END OF HEADER

//stages
	for (uint8_t j = 0; j < file->numWorlds + file->numSpecials; j++)
	{
		for (uint8_t k = 0; k < LSE_DEFAULT_NUM_STAGES; k++)
		{
			lse_stage_entry_t* stage = &file->worlds[j].stages[k];
			
			switch (stage->type)
			{
			case LSE_STAGE_TYPE_NORMAL:
			case LSE_STAGE_TYPE_TOADHOUSE_ALBUM: //same as
			{
				if ((res = lse_read_stage_normal(&stage->normal, stream)) != LSE_OK)
					return res;
				break;
			}

			case LSE_STAGE_TYPE_MYSTERY_BOX:
			{
				if ((res = lse_read_mystery_box(&stage->mysteryBox, stream)) != LSE_OK)
					return res;
				break;
			}

			case LSE_STAGE_TYPE_TOADHOUSE:
			{
				if ((res = lse_read_toad_house(&stage->toadHouse, stream)) != LSE_OK)
					return res;
				break;
			}
			
			default:
				break;
			}
		}
	}

	//footer
	uint8_t footer[0x24];

	if (!lse_streamR_read(stream, footer, sizeof(footer)))
		return LSE_STREAM_ERROR;

	file->lives = (uint16_t)footer[8] | (uint16_t)footer[9]<<8;
	file->starcoin = (uint16_t)footer[10] | (uint16_t)footer[11]<<8;

	return LSE_OK;
}

int lse_deserialize(const uint8_t* mem, size_t size, lse_DATA_t** out, bool validate)
{
	*out = NULL;
	int result = LSE_UNKNOWN_ERR;
	int fncRet = LSE_UNKNOWN_ERR; //use for checking func results

	lse_DATA_t* data = lse_init_data_default();
	if (!data)
		return LSE_FAILED_ALLOCATE;

	lse_StreamR stream;
	lse_streamR_init(&stream, mem, size);

	if (validate)
	{
		uint8_t header[0xC];
		uint32_t curCRC; //little endian
		uint32_t crc32;
		uint32_t curSize;

		if (!lse_streamR_read(&stream, header, sizeof(header)))
		{
			result = LSE_STREAM_ERROR;
			goto ERR;
		}

		curCRC = (uint32_t)header[0] | (uint32_t)header[1]<<8 | (uint32_t)header[2]<<16 | (uint32_t)header[3]<<24;
		curSize = (uint32_t)header[8] | (uint32_t)header[9]<<8 | (uint32_t)header[10]<<16 | (uint32_t)header[11]<<24;

		if (lse_sig_cmp(&header[4]) != LSE_OK)
		{
			result = LSE_INVALID_ARG;
			goto ERR;
		}

		if (size < curSize || curSize < LSE_FILE_SIZE_MIN || curSize > LSE_FILE_SIZE_MAX)
		{
			result = LSE_INVALID_SIZE;
			goto ERR;
		}

		crc32 = lse_crc32(&mem[0x04], curSize - 0x04);
		if (crc32 != curCRC)
		{
			result = LSE_CRC_ERROR;
			goto ERR;
		}
	}

	if (!lse_streamR_seek(&stream, 0x10))
	{
		result = LSE_STREAM_ERROR;
		goto ERR;
	}
	if (!lse_streamR_read(&stream, &data->lastsaved, 4))
	{
		result = LSE_STREAM_ERROR;
		goto ERR;
	}

	//mii data
	if (!lse_streamR_seek(&stream, 0x698))
	{
		result = LSE_STREAM_ERROR;
		goto ERR;
	}
	fncRet = lse_read_rivals(data, &stream, true);
	if (fncRet != LSE_OK)
	{
		result = fncRet;
		goto ERR;
	}

	//0x20 byte padding??
	if (!lse_streamR_move(&stream, 0x20))
	{
		result = LSE_STREAM_ERROR;
		goto ERR;
	}

	//read file
	for (uint8_t i = 0; i < LSE_DEFAULT_NUM_FILES; i++)
	{
		fncRet = lse_read_file(&data->file[i], &stream, validate);
		if (fncRet != LSE_OK)
		{
			result = fncRet;
			goto ERR;
		}
	}
	

	*out = data;
    return LSE_OK;
ERR:
	lse_delete_data(data);
	*out = NULL;
	return result;
}
