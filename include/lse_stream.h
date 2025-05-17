#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
	uint8_t* dst;
	uint32_t size;
	int32_t byte;
} lse_StreamW;

static inline int lse_streamW_init(lse_StreamW* stream)
{
	stream->dst = malloc(100);
	memset(stream->dst, 0, 100);
	stream->byte = 0;
	stream->size = 100;
	return stream->dst != NULL;
}

static inline int lse_streamW_realloc(lse_StreamW* stream, uint32_t nsize)
{
	uint8_t* tmp = realloc(stream->dst, nsize);
	if (tmp)
	{
		stream->dst = tmp;
		if ((int32_t)nsize - (int32_t)stream->size > 0)
			memset(&stream->dst[stream->size], 0, nsize - stream->size);
		stream->size = nsize;
		return 1;
	}

	return 0;
}

static inline int lse_streamW_expand(lse_StreamW* stream, int32_t add)
{
	return lse_streamW_realloc(stream, stream->size + add + 100);
}

static inline void lse_streamW_delete(lse_StreamW* stream)
{
	free(stream->dst);
	stream->dst = NULL;
	stream->byte = 0;
	stream->size = 0;
}

static inline int lse_streamW_seek(lse_StreamW* stream, uint32_t addr)
{
	if (addr >= stream->size)
	{
		if (!lse_streamW_realloc(stream, addr + 100))
			return 0;
	}
	stream->byte = addr;
	return 1;
}

static inline int lse_streamW_move(lse_StreamW* stream, int32_t off)
{
	return lse_streamW_seek(stream, (uint32_t)(stream->byte + off));
}

static inline int lse_streamW_write(lse_StreamW* stream, const void* buf, uint32_t size)
{
	if (stream->byte + size >= stream->size)
	{
		if (!lse_streamW_expand(stream, size))
			return 0;
	}

	memcpy(&stream->dst[stream->byte], buf, size);
	(void)lse_streamW_move(stream, size); //not to be err in here
	return 1;
}

static inline uint8_t* lse_streamW_ptr(lse_StreamW* stream)
{
	return &stream->dst[stream->byte];
}

static inline uint32_t lse_streamW_tell(lse_StreamW* stream)
{
	return stream->byte;
}

typedef struct
{
	const uint8_t* src;
	uint32_t size;
	int32_t byte;
} lse_StreamR;

static inline void lse_streamR_init(lse_StreamR* stream, const uint8_t* src, uint32_t size)
{
	stream->src = src;
	stream->byte = 0;
	stream->size = size;
}

static inline int lse_streamR_seek(lse_StreamR* stream, uint32_t addr)
{
	if (addr > stream->size)
		return 0;
	stream->byte = addr;
	return 1;
}

static inline int lse_streamR_move(lse_StreamR* stream, int32_t off)
{
	return lse_streamR_seek(stream, (uint32_t)(stream->byte + off));
}

static inline int lse_streamR_read(lse_StreamR* stream, void* buf, uint32_t size)
{
	if (stream->byte + size > stream->size)
		return 0;
	
	memcpy(buf, &stream->src[stream->byte], size);
	(void)lse_streamR_move(stream, size); //not to be err in here
	return 1;
}
