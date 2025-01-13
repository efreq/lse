#include <lse.h>

int main(int argc, char** argv)
{
	int result = EXIT_FAILURE;
	
	FILE* ifs = NULL, *ofs = NULL;
    char* in = NULL, *out = NULL;

	lse_DATA_t* data = NULL;

	if (!lse_read("GameData.bin", &data))
		goto EXIT;

	if (!lse_write("GameDataOut.bin", data))
		goto EXIT;
	

	result = EXIT_SUCCESS;
EXIT:
    fclose(ifs); fclose(ofs);
	lse_delete_data(data);
    return result;
}