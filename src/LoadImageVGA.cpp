#include <array>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <pc.h>
#include <sys/farptr.h>
#include <bios.h>
#include <sys/nearptr.h>

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

#include <conio.h>

#include <unordered_map>


using std::vector;
#include "NativeBitmap.h"
#include "IFileLoaderDelegate.h"
#include "CPackedFileReader.h"
#include "LoadImage.h"



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace odb {

    std::vector<std::shared_ptr<odb::NativeBitmap>> loadSpriteList(std::string listName, std::shared_ptr<odb::IFileLoaderDelegate> fileLoader) {


        auto buffer = fileLoader->loadFileFromPath(listName);
        buffer.push_back('\n');

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
                    tilesToLoad.push_back(odb::loadBitmap( filename, fileLoader ));
                }
                since = 0;
            }
        }



        return tilesToLoad;
    }

    std::shared_ptr<NativeBitmap> loadBitmap(std::string path, std::shared_ptr<odb::IFileLoaderDelegate> fileLoader) {

        //std::cout << "loading " << path << std::endl;

        auto buffer = fileLoader->loadBinaryFileFromPath(path);
        int xSize;
        int ySize;
        int components;
        auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components, 4);
        auto rawData = new int[xSize * ySize];
        std::memcpy( rawData, image, xSize * ySize * sizeof( int ) );
        stbi_image_free(image);


        for ( int c = 0; c < xSize * ySize; ++c  ) {
            int origin = rawData[ c ];

            int a = (origin & 0xFF000000) >> 24;
            int r = (origin & 0x00FF0000) >> 16;
            int g = (origin & 0x0000FF00) >> 8;
            int b = (origin & 0x000000FF);

            if ( a < 128 ) {
                origin = 0;
            }

            rawData[ c ] = origin;
        }

        return (std::make_shared<odb::NativeBitmap>(xSize, ySize, rawData));
    }
}
