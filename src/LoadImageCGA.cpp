

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <array>

#include <unordered_map>


using std::vector;
#include "NativeBitmap.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"
#include "Renderer.h"
#include "LoadImage.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace odb {

    std::vector<odb::NativeBitmap*> loadSpriteList(std::string listName, odb::IFileLoaderDelegate* fileLoader, EVideoType videoType) {


        auto buffer = fileLoader->loadFileFromPath(listName);
        buffer.push_back('\n');

        std::vector<odb::NativeBitmap*> tilesToLoad;
        int lastPoint = 0;
        int since = 0;
        auto bufferBegin = std::begin( buffer );
        for (const auto& c : buffer ) {
            ++since;
            if ( c == '\n' ) {
                auto filename = std::string( bufferBegin + lastPoint, bufferBegin + lastPoint + since - 1 );
                lastPoint += since;
                if ( !filename.empty()) {
                    tilesToLoad.push_back(odb::loadBitmap( filename, fileLoader, videoType ));
                }
                since = 0;
            }
        }



        return tilesToLoad;
    }

    NativeBitmap* loadBitmap(std::string path, odb::IFileLoaderDelegate* fileLoader, EVideoType videoType) {
        auto buffer = fileLoader->loadBinaryFileFromPath(path);
        int xSize;
        int ySize;
        int components;
        int *rawData;
        uint8_t *data8;

        if (videoType == kVGA ) {
            auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components, 4);
            rawData = new int[xSize * ySize];
            data8 = new uint8_t[ xSize * ySize ];
            std::memcpy( rawData, image, xSize * ySize * sizeof( int ) );

            for ( int c = 0; c < xSize * ySize; ++c  ) {
                int origin = rawData[ c ];

                int a = (origin & 0xFF000000) >> 24;
                int r = (origin & 0x00FF0000) >> 16;
                int g = (origin & 0x0000FF00) >> 8;
                int b = (origin & 0x000000FF);

                if ( a < 128 ) {
                    origin = 0x00FF00;
                }

                rawData[ c ] = origin;
            }
            stbi_image_free(image);
            for ( auto c = 0; c < ( xSize * ySize ); ++c ) {
                data8[ c ] = getPaletteEntry( rawData[ c ] );
            }
        } else {
            auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components, 1);
            rawData = new int[xSize * ySize];
            data8 = new uint8_t[ xSize * ySize ];
            for (int y = 0; y < ySize; ++y) {
                for (int x = 0; x < xSize; ++x) {
                    int pixel = image[(y * xSize) + x];
                    switch (pixel) {
                        case 154:
                            pixel = 10;
                            break;
                        case 103:
                            pixel = 6;
                            break;
                        case 51:
                            pixel = 2;
                            break;
                        case 34:
                            pixel = 1;
                            break;
                        case 108:
                            pixel = 9;
                            break;
                        case 68:
                            pixel = 5;
                            break;
                        case 0:
                        case 102:
                        case 101:
                            pixel = 4;
                            break;
                        case 85:
                            pixel = 3;
                            break;
                        case 255:
                            pixel = 11;
                            break;
                        case 149:
                            pixel = 0;
                            break;
                        default:
                            pixel = 0;
                            break;
                    }
                    rawData[(y * xSize) + x] = pixel;
                }
            }
            stbi_image_free(image);

            for ( auto c = 0; c < ( xSize * ySize ); ++c ) {
                data8[ c ] = ( rawData[ c ] );
            }
        }






        return new odb::NativeBitmap(xSize, ySize, data8);
    }
}
