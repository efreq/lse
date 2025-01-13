/**
 * Allocator for structures
 */
#include <lse.h>

extern const uint8_t _lseWorldStageTypes[LSE_DEFAULT_NUM_WORLDS + LSE_DEFAULT_NUM_SPECIALS][LSE_DEFAULT_NUM_STAGES];

//todo
uint16_t lse_count_numMiis(const uint8_t* worlds, uint8_t numWorlds)
{
	uint16_t count = 0;

	for (uint8_t i = 0; i < numWorlds; i++)
	{
		for (uint8_t j = 0; j < LSE_DEFAULT_NUM_STAGES; j++)
		{
			if (worlds[i*LSE_DEFAULT_NUM_STAGES + j] == LSE_STAGE_TYPE_NORMAL)
				count++;
		}
	}

	return count;
}

lse_DATA_t* lse_init_data_default(void)
{
	lse_DATA_t* data = malloc(sizeof(lse_DATA_t));
	if (!data)
		return NULL;

	memset(data, 0, sizeof(lse_DATA_t));

	uint16_t numMiis = lse_count_numMiis((const uint8_t*)_lseWorldStageTypes, 
							LSE_DEFAULT_NUM_WORLDS + LSE_DEFAULT_NUM_SPECIALS);

	//count miis
	data->numRivals = numMiis;
	data->rivals = malloc(sizeof(lse_rival_t) * numMiis);

	uint8_t numWorlds = LSE_DEFAULT_NUM_WORLDS;
	uint8_t numSpecials = LSE_DEFAULT_NUM_SPECIALS;

	//file
	for (uint8_t i = 0; i < 3; i++)
	{
		uint8_t miiCount = 0; //count up miis and link

		if (!(data->file[i].worlds = malloc(sizeof(lse_world_t) * (numWorlds + numSpecials))))
			goto ERR;

		//shortcut
		lse_world_t* worlds = data->file[i].worlds;

		for (uint8_t j = 0; j < (numWorlds + numSpecials); j++)
		{
			//In default, put worlds and specials in succession
			worlds[j].type = j >= numWorlds ? LSE_WORLD_TYPE_SPECIAL : LSE_WORLD_TYPE_WORLD;

			for (uint8_t k = 0; k < LSE_DEFAULT_NUM_STAGES; k++)
			{
				lse_stage_entry_t* stage = &worlds[j].stages[k];
				stage->type = _lseWorldStageTypes[j][k];

				if (stage->type == LSE_STAGE_TYPE_NORMAL)
				{
					//link mii and stage
					if (miiCount >= numMiis)
						goto ERR;
					
					stage->rival = &data->rivals[miiCount];
					data->rivals[miiCount].stage = stage;
					miiCount++;
				}
			}
			worlds[j].numStages = LSE_DEFAULT_NUM_STAGES;
		}

		//assert(miiCount == numMiis);
		
		data->file[i].numWorlds = numWorlds;
		data->file[i].numSpecials = numSpecials;
	}

	return data;

ERR:
	lse_delete_data(data);
	return NULL;
}

void lse_delete_data(lse_DATA_t* data)
{
	if (data)
	{
		//files
		for (uint8_t i = 0; i < 3; i++)
		{
			free(data->file[i].worlds);
		}
		free(data->rivals);
		free(data);
	}
}
