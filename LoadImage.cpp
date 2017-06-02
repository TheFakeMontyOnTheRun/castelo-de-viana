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
    auto endPos = ftell( fileDescriptor );
    rewind(fileDescriptor);
    std::vector<char> total(endPos);
    auto writeHead = std::begin( total );
    
    for ( int c = 0; c < endPos; ++c ) {
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

    std::shared_ptr<NativeBitmap> loadBitmap( std::string path ) {

        std::cout << "loading " << path << std::endl;

        auto buffer = loadBinaryFileFromPath(path);
        int xSize;
        int ySize;
        int components;

        auto image = stbi_load_from_memory((const stbi_uc *) buffer.data(), buffer.size(), &xSize, &ySize, &components,
                                           1);
        auto rawData = new int[xSize * ySize];

        for ( int y = 0; y < ySize; ++y ) {
            for ( int x = 0; x < xSize; ++x ) {
                int pixel = image[ ( y * xSize ) + x ];
//                std::cout << pixel << ", ";
                switch (pixel) {
                    case 155:
                        pixel = 6;
                        break;
                    case 77:
                        pixel = 2;
                        break;
                    case 51:
                        pixel = 1;
                        break;
                    case 104:
                        pixel = 5;
                        break;
                    case 0:
                        pixel = 0;
                        break;
                    default:
                        pixel = 0;
                        break;
                }
                rawData[ ( y * xSize ) + x ] = pixel;
                std::cout << (( rawData[ ( y * xSize ) + x ] )) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << components << std::endl;stbi_image_free(image);
//exit(0);
        return (std::make_shared<odb::NativeBitmap>(path, xSize, ySize, rawData));
    }
}
