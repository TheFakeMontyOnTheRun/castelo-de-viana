////
//// Created by monty on 28/02/16.
////
#include "NativeBitmap.h"

namespace odb {
    int NativeBitmap::getWidth() const {
        return this->mWidth;
    }

    int NativeBitmap::getHeight() const {
        return this->mHeight;
    }

    int *NativeBitmap::getPixelData() const {
        return this->mRawData;
    }

    NativeBitmap::NativeBitmap(int aWidth, int aHeight, int *aRawData) :
            mWidth(aWidth), mHeight(aHeight), mRawData(aRawData) {
    }

    NativeBitmap::~NativeBitmap() {
        delete[] mRawData;
    }
}