//
// Created by monty on 06-12-2017.
//
#include <string>
#include <unordered_map>
#include <vector>

using std::vector;

#include "Common.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"

#include <iostream>

odb::CPackedFileReader::CPackedFileReader(std::string dataFilePath) : mPackPath(dataFilePath) {
    mDataPack = fopen(dataFilePath.c_str(), "rb");
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

vector<char> odb::CPackedFileReader::loadBinaryFileFromPath(const std::string &path) {
    vector<char> toReturn;
    mDataPack = fopen(mPackPath.c_str(), "rb");
    uint32_t offset = mOffsets[ path ];
    if ( offset == 0 ) {
        printf("failed to load %s from offset %d", path.c_str(), offset );
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

std::string odb::CPackedFileReader::loadFileFromPath(const std::string &path) {
    std::string toReturn;
    mDataPack = fopen(mPackPath.c_str(), "r");
    uint32_t offset = mOffsets[ path ];
    if ( offset == 0 ) {
        printf("failed to load %s", path.c_str());
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

std::string odb::CPackedFileReader::getFilePathPrefix() {
    return "";
}
