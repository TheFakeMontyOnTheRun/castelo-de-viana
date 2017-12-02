//
// Created by monty on 28/02/16.
//

#ifndef LESSON10_NATIVEBITMAP_H
#define LESSON10_NATIVEBITMAP_H
namespace odb {
    class NativeBitmap {
        int mWidth;
        int mHeight;
        int *mRawData;

    public:
        NativeBitmap(int aWidth, int aHeight, int *aRawData);

        int getWidth() const;

        int getHeight() const;

        int *getPixelData() const;

        ~NativeBitmap();
    };
}

#endif //LESSON10_NATIVEBITMAP_H
