/**
 * CRC32, CRC16 and signature 
 */
#include <lse.h>

uint32_t lse_crc32(const void* buf, size_t sz)
{
	uint32_t crc=0xFFFFFFFF;
	if (!buf)
		return crc;

	for (size_t i = 0; i < sz; i++)
	{
		crc ^= ((uint8_t*)buf)[i];
		for (int j = 0; j < 8; j++)
			crc = crc&1 ? crc>>1^0xEDB88320 : crc>>1;
	}
	return crc^0xFFFFFFFF;
}

uint16_t lse_crc16(const void* buf, size_t sz)
{
	uint16_t crc=0;
	if (!buf)
		return crc;

	for (size_t i = 0; i < sz; i++)
	{
		crc ^= ((uint8_t*)buf)[i]<<8;
		for (int j = 0; j < 8; j++)
			crc = crc&0x8000 ? crc<<1^0x1021 : crc<<1;
	}
	return crc;
}

int lse_sig_cmp(uint8_t *mem)
{
	for (uint8_t i = 0; i < sizeof(lse_header_magic); i++)
	{
		if (mem[i] != lse_header_magic[i])
			return LSE_INVALID_ARG;
	}
	
    return LSE_OK;
}
