//
// Created by monty on 28/02/16.
//

#ifndef LESSON10_NATIVEBITMAP_H
#define LESSON10_NATIVEBITMAP_H

#include <cstdint>

namespace odb {
    class NativeBitmap {
        int mWidth;
        int mHeight;
        uint8_t *mRawData;

    public:
        NativeBitmap(int aWidth, int aHeight, uint8_t *aRawData);

        int getWidth() const;

        int getHeight() const;

        uint8_t *getPixelData() const;

        ~NativeBitmap();
    };
}

#endif //LESSON10_NATIVEBITMAP_H
