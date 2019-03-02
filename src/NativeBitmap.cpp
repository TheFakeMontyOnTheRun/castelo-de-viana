////
//// Created by monty on 28/02/16.
////
#include <stdint.h>

#include "NativeBitmap.h"

namespace odb {
    int NativeBitmap::getWidth() const {
        return this->mWidth;
    }

    int NativeBitmap::getHeight() const {
        return this->mHeight;
    }

    uint8_t *NativeBitmap::getPixelData() const {
        return this->mRawData;
    }

    NativeBitmap::NativeBitmap(int aWidth, int aHeight, uint8_t *aRawData) :
            mWidth(aWidth), mHeight(aHeight), mRawData(aRawData) {
    }

    NativeBitmap::~NativeBitmap() {
        delete[] mRawData;
    }
}