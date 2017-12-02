#include <array>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <cmath>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstring>
#include <memory>
#include <iostream>
#include <map>
#include <array>
#include <iostream>



#include "NativeBitmap.h"
#include "LoadImage.h"

#include <memory>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

#include "NativeBitmap.h"


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

    std::string getResPath() {
        return "resSDL/";
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

    std::shared_ptr<NativeBitmap> loadBitmap(std::string path) {

        //std::cout << "loading " << path << std::endl;

        auto buffer = loadBinaryFileFromPath(path);
        int xSize;
        int ySize;
        int components;

        auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components, 4);
        auto rawData = new int[xSize * ySize];

	    std::memcpy( rawData, image, xSize * ySize * 4 );
        stbi_image_free(image);


        for ( int c = 0; c < xSize * ySize; ++c  ) {
            int origin = rawData[ c ];

            int a = (origin & 0xFF000000) >> 24;
            int r = (origin & 0x00FF0000) >> 16;
            int g = (origin & 0x0000FF00) >> 8;
            int b = (origin & 0x000000FF);

            if ( origin == 0 ) {
                origin = 1;
            }

            if ( a < 128 ) {
                origin = 0;
            }

            rawData[ c ] = origin;
        }

        return (std::make_shared<odb::NativeBitmap>( xSize, ySize, rawData));
    }
}
