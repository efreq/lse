/**
 * Super Mario 3D Land Save editor
 * 
 * 
 * Copyright (c) 2025 efreq
 * 
 * This file is part of lse
 * lse is Licensed under MIT License
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <time.h>

#include <3ds/mii.h>


#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/**
 * version numbers
 */
#define LSE_VERSION_MAJOR 0
#define LSE_VERSION_MINOR 1
#define LSE_VERSION_REVISION 0

/**
 * results
 * They are not real value
 */
#define LSE_OK				0
#define	LSE_INVALID_ARG		1
#define	LSE_NOT_FOUND		2
#define LSE_INVALID_SIZE	3
#define LSE_FAILED_ALLOCATE	4
#define LSE_CRC_ERROR		5
#define LSE_STREAM_ERROR	6
#define LSE_UNKNOWN_ERR		(-1)

/**
 * @note Signed variables will may resetting(become 0) if (value < 0)
 */

/// lse_DATA_t::filesize
#define LSE_FILE_SIZE_MIN 9
#define LSE_FILE_SIZE_MAX 18444

typedef struct lse_stage_entry_s lse_stage_entry_t;

/// Default rivals (depends on stagelist.byml)
#define LSE_DEFAULT_RIVAL_LENGTH 0x60

#define LSE_DEFAULT_NUM_FILES		3

#define LSE_DEFAULT_NUM_WORLDS		9
#define LSE_DEFAULT_NUM_SPECIALS	8

///stages per a world
#define LSE_DEFAULT_NUM_STAGES		8


//lse_FILE_t
//::powerup
#define LSE_POWERUP_SUPER			0
#define LSE_POWERUP_SMALL			1
#define LSE_POWERUP_FIRE			2
#define LSE_POWERUP_RACOONDOG		3
#define LSE_POWERUP_BOOMERANG		4
#define LSE_POWERUP_TANOOKI			5

//::stockitem
#define LSE_ITEM_NONE				0
#define LSE_ITEM_MUSHROOM			1
#define LSE_ITEM_FIRE_FLOWER		2
#define LSE_ITEM_RACOONDOG_LEAF		3
#define LSE_ITEM_TANOOKI_LEAF		4
#define LSE_ITEM_BOOMERANG_FLOWER	5
#define LSE_ITEM_PWING				6

//lse_normal_stage_t
//::cleared
#define LSE_STAGE_LOCKED			0
#define LSE_STAGE_NOT_CLEARED		1
#define LSE_STAGE_CLEARED			2

//::status
#define LES_STAGE_STATUS_STARCOIN1_BIT		(1<<0)
#define LES_STAGE_STATUS_STARCOIN2_BIT		(1<<1)
#define LSE_STAGE_STATUS_STARCOIN3_BIT		(1<<2)
///@note The next course will unlock automatically if this course was locked (exclude last course of world)
#define LSE_STAGE_STATUS_UNLOCK_NEXT_BIT	(1<<3)
#define LSE_STAGE_STATUS_MARIO_CLEARED_BIT	(1<<4)
#define LSE_STAGE_STATUS_LUIGI_CLEARED_BIT	(1<<5)

//::flagheight
#define LSE_STAGE_MAX_FLAG_HEIGHT	100

typedef struct lse_rival_s
{
	MiiData  mii;
	uint16_t time;

	lse_stage_entry_t* stage; //link
} lse_rival_t;

/* Normal stage data (size 0x0C) */
/* A stage has 0x56 bytes data in global directive, They may be streetpath data. its depends on StageList (in romfs) */
/* TODO: Move to dynamic list, to supporting modded savedata */
typedef struct lse_normal_stage_s
{
	int8_t cleared; //offs 0:locked, 1:not cleared, 2:cleared
	uint8_t status; //offs+0x01	1<<0 star1, 1<<1 star2, 1<<2 star3, 1<<3 unlock_next_course, 1<<4 Mario_cleared, 1<<5 Luigi_cleared, ..., @note The next course will unlock automatically if this course was locked (exclude last course of world)

	uint32_t misscount; //offs+0x05 missed count. The "retry count" is sum of this variables. max of "retry count" is 99999
	int8_t flagheight; //offs+0x09		64:Golden flag
	uint16_t time; //offs+0x0A  clear time
} lse_normal_stage_t;

/* mystery box data (size 0x75) */
/* A mystery box has no globalized data depends on StageList */
typedef struct lse_mystery_box_s
{
	int8_t cleared; //offs 0:locked, 1:not cleared, 2:cleared
	uint8_t status; //offs+0x01		not including starcoin data. it's globalized in a FILE

	uint32_t misscount; //offs+0x05 missed count. The "retry count" is sum of this variables. max of "retry count" is 99999

	int8_t nextbox; //offs+0x6A	index of next mbox, random?

	int8_t isopen; //offs+0x6C 0:true, 1:false
	uint32_t lastopened; //offs+0x6D time	mbox will reopen in 20 hours from lastopened
} lse_mystery_box_t;

/* Toad house data (size 0x13) */
/* This has no globalized data depends on StageList */
typedef struct lse_toad_house_s
{
	int8_t cleared; //offs
	uint8_t status; //offs+0x01

	//uint32_t misscount; //offs+0x05 invalid

	int8_t isopen; //offs+0x0A 0:true, 1:false
	uint32_t lastopened; //offs+0x0B time
} lse_toad_house_t;


/**
 * Stage Types and World Types are not real value
 */
#define LSE_STAGE_TYPE_EMPTY				0
#define LSE_STAGE_TYPE_NORMAL				1	///Castle is same as
#define LSE_STAGE_TYPE_PIPE					2
#define LSE_STAGE_TYPE_MYSTERY_BOX			3
#define LSE_STAGE_TYPE_TOADHOUSE			4
#define LSE_STAGE_TYPE_TOADHOUSE_ALBUM		5	//No mii, but has a normal_stage data on file

//They are same as LSE_STAGE_TYPE_NORMAL
#define LSE_STAGE_TYPE_CHAMPIONSHIP			LSE_STAGE_TYPE_NORMAL	//S8-👑

struct lse_stage_entry_s
{
	uint8_t type;

	union
	{
		lse_normal_stage_t normal;
		lse_mystery_box_t mysteryBox;
		lse_toad_house_t toadHouse;
	};

	//to suppport different mii format
	union
	{
		lse_rival_t* rival;
	};
};

#define LSE_WORLD_TYPE_WORLD	0
#define LSE_WORLD_TYPE_SPECIAL	1

typedef struct lse_world_s
{
	uint8_t type; //LSE_WORLD_TYPE_WORLD or LSE_WORLD_TYPE_SPECIAL

	lse_stage_entry_t stages[LSE_DEFAULT_NUM_STAGES];
	uint32_t numStages;
} lse_world_t;


/* A file data (size 0x983 variable) */
typedef struct lse_FILE_s
{
	//offset 0x2B78, 0x34FB, 0x3E7E
	int8_t coin; //offs

	int8_t powerup; //offs+0x02 0x2B7A Mario's powerUp status, 00:Super 01:Small 02:Fire 03:Raccoon 04:Boomerang 05:Tanooki
	int8_t pos; //offs+0x03  0x2B7B  (1 - ->): 0 (1 - 1): 1		(8*(world-1))+offs
	int8_t stockitem; //offs+0x04  0x2B7C 00:None 01:Mushroom 02:Fire 03:Raccoon 04:Tanooki 05:Boomerang 06:P-wing

	int8_t isLuigi; //offs+0x09  0x2B81 0:Mario, 1:Luigi
	/* These data are sharing in all mistery boxes of FILE */
	int8_t mbox_visitedcount; //offs+0x0A  0x2B82 Count of visited to mbox
	int8_t mbox_lastbox; //offs+0x0B  0x2B83 index of recently visited mbox
	uint16_t mbox_starcoin; //offs+0x0C  0x2B84		1<<0 star1, 1<<1 star2, ...

	uint16_t pictures; // offs+0x12 0x2B8A	1<<0 pic1, ... 1<<11 pic12
	int8_t progress; //offs+0x14 0x2B8C		1<<0 first movie watched, 1<<1 Sworld unlocked, 
	uint16_t world_progress; //offs+0x15 0x2B8D
	//uint8_t unknown; //offs+0x17 0x2B8F

	MiiData mii; //offs+0x18	0x2B90?
	uint32_t lastsaved; //offs+0x78 0x2BF0 u32 time
	//4-byte unknown

	lse_world_t* worlds; //sizeof(worlds) == sizeof(lse_world_t) * (numWorlds+numSpecials)
	uint8_t numWorlds;
	uint8_t numSpecials;

	int16_t lives; //offs+0x967 0x34DF  The max is 0x456(👑👑👑)
	int16_t starcoin; //offs+0x969 0x34E1 max 364  if(<0) then be 0
} lse_FILE_t;

/// @brief Main structure
typedef struct lse_DATA_s
{
	/**
	 * 	@warning
	 *  The 0x0a bytes of header will checking when launched game. 
	 *	If value is invalid, The data will intializing
	
	uint32_t crc; //0x0000 
	//uint32_t 0x21; //0x0004	always 0x21
	uint32_t filesize; //0x0008   including crc32 bytes, use for checksum,	(9 <=  <= 18444) so {0x000a, 0x000b} is always 0 
	 */

	uint32_t lastsaved; //0x0010 u32 time

	//0x0198 ?

	lse_rival_t* rivals; //0x698  only has stages
	uint16_t numRivals;
	//...
	//variable
	//uint16_t rival_time_w1_1; //0x2A98

	lse_FILE_t file[3]; //0x2B78 //0x34FB //0x3E7E
} lse_DATA_t;


/**
 * APIs
 */

/// @brief Read lse_DATA_t from binary file
/// @param[in] filename specify filename
/// @param[out] data 
/// @return 1:success, 0:failed
/// @note Should be call lse_delete_data() to free lse_DATA_t
int lse_read(const char* filename, lse_DATA_t** data);

/// @brief Write lse_DATA to binary file
/// @param[out] fs FILE to write. The mode must be b+
/// @param[in] data 
/// @return 1:success, 0:failed
int lse_write(const char* filename, const lse_DATA_t* data);


/**
 * allocator
 */

lse_DATA_t* lse_init_data_default(void);

void lse_delete_data(lse_DATA_t* data);

/**
 * serializer and deserailizer
 */

/**
 * deserialize raw data to lse_DATA_t
 * @param[in] mem raw data
 * @param size mem size
 * @param[out] out pointer to output data
 * @param validate which validating crc
 * @return LSE_OK (or 0) is no error, otherwise is err occurred
 */
int lse_deserialize(const uint8_t* mem, size_t size, lse_DATA_t** out, bool validate);


int lse_serialize(uint8_t** mem, size_t* size, const lse_DATA_t* data);

/**
 * Query
 */

int lse_stage_at(lse_stage_entry_t** out, lse_FILE_t* file, uint8_t stageType, uint8_t worldType, uint8_t worldIndex, uint8_t stageIndex);

static inline int lse_stage_iswin(const lse_stage_entry_t* s)
{
	if (!s)
		return 0;
	if (!s->rival)
		return 1;
	
	return (s->normal.time >= s->rival->time);
}

uint32_t lse_stage_get_retry_count(const lse_DATA_t* data, uint8_t i);


/**
 * Validation
 */

/** 
 * @brief Calculate crc32 on GameData.bin header (init:0xFFFFFFFF, LSB first, poly:0xEDB88320, xor:0xFFFFFFFF)
 * 
 * Credits: http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html

Permission to use, copy, and distribute this specification for any purpose and without fee or royalty is hereby granted, provided that the full text of this NOTICE appears on ALL copies of the specification or portions thereof, including modifications, that you make.

THIS SPECIFICATION IS PROVIDED "AS IS," AND COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED. BY WAY OF EXAMPLE, BUT NOT LIMITATION, COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE SPECIFICATION WILL NOT INFRINGE ANY THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS. COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE OF THIS SPECIFICATION.

The name and trademarks of copyright holders may NOT be used in advertising or publicity pertaining to the specification without specific, written prior permission. Title to copyright in this specification and any associated documentation will at all times remain with copyright holders.
 
 */
uint32_t lse_crc32(const void* buf, size_t sz);
/// @brief Calculate crc16 on Miidata (init:0, MSB first, poly:0x1021, xor:0, size(fixed):0x5E)
/// https://www.3dbrew.org/wiki/Mii
uint16_t lse_crc16(const void* buf, size_t sz);


static const uint8_t lse_header_magic[4] = {0x21, 0x00, 0x00, 0x00};

/// @return LSE_OK or LSE_INVALID_ARG
int lse_sig_cmp(uint8_t* mem);

#ifdef __cplusplus
}
#endif /* __cplusplus */
