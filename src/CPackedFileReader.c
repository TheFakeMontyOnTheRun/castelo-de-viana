#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#include "Common.h"
#include "CPackedFileReader.h"

struct StaticBuffer loadFileFromPath(const char* dataFilePath, const char* path) {
	struct StaticBuffer toReturn;
    FILE* mDataPack = fopen(dataFilePath, "r");
	uint32_t offset = 0;

	uint16_t entries = 0;
	fread(&entries, 2, 1, mDataPack);

	char buffer[85];

	int c = 0;
	for ( c = 0; c < entries; ++c ) {

		fread(&offset, 4, 1, mDataPack );

		uint8_t stringSize = 0;
		fread(&stringSize, 1, 1, mDataPack );
		fread(&buffer, stringSize + 1, 1, mDataPack );

		if (!strcmp( buffer, path ) ) {
			goto found;
		}
	}

	printf("File not found: %s\n", path);
	assert(FALSE);

	found:

    if ( offset == 0 ) {
        printf("failed to load %s", path);
        exit(-1);
    }
    int result = fseek( mDataPack, offset, SEEK_SET );

    uint32_t size = 0;
    fread(&size, 4, 1, mDataPack );
    toReturn.data = (uint8_t *)(calloc(size, 1));
    toReturn.size = size;
    fread(toReturn.data, 1, size, mDataPack );
    fclose(mDataPack);

    return toReturn;
}
