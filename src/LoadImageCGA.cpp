#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <conio.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <memory>
#include <string>
#include <vector>
#include <array>

#include "../include/NativeBitmap.h"
#include "../include/LoadImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace odb {

    std::vector<char> readToBuffer(FILE *fileDescriptor) {
        const unsigned N = 1024;

        fseek(fileDescriptor, 0, SEEK_END);
        auto endPos = ftell(fileDescriptor);
        rewind(fileDescriptor);
        std::vector<char> total(endPos);
        auto writeHead = std::begin(total);

        for (int c = 0; c < endPos; ++c) {
            char buffer[N];
            size_t read = fread((void *) &buffer[0], 1, N, fileDescriptor);
            if (read) {
                for (int c = 0; c < read; ++c) {
                    *writeHead = (buffer[c]);
                    writeHead = std::next(writeHead);
                }
            }
            if (read < N) {
                break;
            }
        }

        return total;
    }

    std::string getResPath() {
        return "resCGA/";
    }

    std::vector<std::shared_ptr<odb::NativeBitmap>> loadSpriteList(std::string listName) {

        FILE *fd = fopen(listName.c_str(), "r");
        auto buffer = readToBuffer(fd);
        buffer.push_back('\n');
        fclose(fd);
        std::vector<std::shared_ptr<odb::NativeBitmap>> tilesToLoad;
        int lastPoint = 0;
        int since = 0;
        auto bufferBegin = std::begin( buffer );
        for (const auto& c : buffer ) {
            ++since;
            if ( c == '\n' ) {
                auto filename = std::string( bufferBegin + lastPoint, bufferBegin + lastPoint + since - 1 );
                lastPoint += since;
                if ( !filename.empty()) {
                    tilesToLoad.push_back(odb::loadBitmap( getResPath() + filename));
                }
                since = 0;
            }
        }



        return tilesToLoad;
    }


    std::vector<char> loadBinaryFileFromPath(const std::string &path) {
        FILE *fd;

        fd = fopen(path.c_str(), "rb");

        if (fd == nullptr) {
            exit(0);
        }

        std::vector<char> toReturn = readToBuffer(fd);
        fclose(fd);

        return toReturn;
    }

    std::shared_ptr<NativeBitmap> loadBitmap(std::string path) {

        auto buffer = loadBinaryFileFromPath(path);
        int xSize;
        int ySize;
        int components;

        auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components,
                                           1);
        auto rawData = new int[xSize * ySize];

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

        return (std::make_shared<odb::NativeBitmap>(xSize, ySize, rawData));
    }
}
