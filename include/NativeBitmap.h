//
// Created by monty on 28/02/16.
//

#ifndef LESSON10_NATIVEBITMAP_H
#define LESSON10_NATIVEBITMAP_H

namespace odb {
    struct NativeBitmap {
        int mWidth;
        int mHeight;
        uint8_t *mRawData;
    };
}

#endif //LESSON10_NATIVEBITMAP_H
