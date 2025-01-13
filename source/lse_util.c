/**
 * Query and Utils
 */
#include <lse.h>


int lse_stage_at(lse_stage_entry_t** out, lse_FILE_t* file, uint8_t stageType, uint8_t worldType, uint8_t worldIndex, uint8_t stageIndex)
{
	lse_world_t* target;
	uint8_t sumWorlds;

	target = file->worlds;
	sumWorlds = file->numWorlds + file->numSpecials;

	if (worldType == LSE_WORLD_TYPE_SPECIAL)
		worldIndex += file->numWorlds;
	else if (worldIndex != LSE_WORLD_TYPE_WORLD) //invalid
		return LSE_INVALID_ARG;

	if (worldIndex >= sumWorlds)
		return LSE_INVALID_SIZE;
	target = &target[worldIndex];
	
	uint8_t index = 0;

	for (uint8_t i = 0; i < target->numStages; i++)
	{
		if (target->stages[i].type == stageType)
		{
			index++;
			if (index == stageIndex)
			{
				*out = &target->stages[index];
				return LSE_OK;
			}
		}
	}

    return LSE_NOT_FOUND;
}

uint32_t lse_stage_get_retry_count(const lse_DATA_t* data, uint8_t i)
{
	if (!data || i > 3)
		return 0;
	
	uint32_t ret = 0;

	for (uint8_t j = 0; j < data->file[i].numWorlds; j++)
	{
		for (uint8_t k = 0; k < data->file[i].worlds[j].numStages; k++)
		{
			if (data->file[i].worlds[j].stages[k].type == LSE_STAGE_TYPE_NORMAL)
				ret += data->file[i].worlds[j].stages[k].normal.misscount;
		}
	}
	
	return ret;
}
