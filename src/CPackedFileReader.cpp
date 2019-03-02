//
// Created by monty on 06-12-2017.
//
#include <unordered_map>
#include <vector>

using std::vector;

#include "Common.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"

odb::CPackedFileReader::CPackedFileReader(const char* dataFilePath) : mPackPath(
        const_cast<char *>(dataFilePath)) {
    mDataPack = fopen(dataFilePath, "rb");
    uint16_t entries = 0;
    fread(&entries, 2, 1, mDataPack);

    char buffer[85];

    for ( int c = 0; c < entries; ++c ) {

        uint32_t offset = 0;
        fread(&offset, 4, 1, mDataPack );

        uint8_t stringSize = 0;
        fread(&stringSize, 1, 1, mDataPack );

        fread(&buffer, stringSize + 1, 1, mDataPack );
        std::string name = buffer;



        mOffsets[name] = offset;
    }

    fclose(mDataPack);
}

vector<char> odb::CPackedFileReader::loadBinaryFileFromPath(const char* path) {
    vector<char> toReturn;
    mDataPack = fopen(mPackPath, "rb");
    uint32_t offset = mOffsets[ path ];
    if ( offset == 0 ) {
        printf("failed to load %s from offset %d", path, offset );
        exit(-1);
    }

    auto result = fseek( mDataPack, offset, SEEK_SET );

    uint32_t size = 0;
    char data;
    fread(&size, 4, 1, mDataPack );

    for ( int c = 0; c < size; ++c ) {
        fread(&data, 1, 1, mDataPack );
        toReturn.push_back(data);
    }
    fclose(mDataPack);

    return toReturn;
}

odb::StaticBuffer odb::CPackedFileReader::loadFileFromPath(const char* path) {
    odb::StaticBuffer toReturn;
    mDataPack = fopen(mPackPath, "r");
    uint32_t offset = mOffsets[ path ];

    if ( offset == 0 ) {
        printf("failed to load %s", path);
        exit(-1);
    }
    auto result = fseek( mDataPack, offset, SEEK_SET );

    uint32_t size = 0;
    fread(&size, 4, 1, mDataPack );
    toReturn.data = (uint8_t *)(calloc(size, 1));
    toReturn.size = size;
    fread(toReturn.data, 1, size, mDataPack );
    fclose(mDataPack);

    return toReturn;
}

const char* odb::CPackedFileReader::getFilePathPrefix() {
    return "";
}
