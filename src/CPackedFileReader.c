#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef AMIGA
#include "AmigaInt.h"

#else

#ifdef CPC

#include "CPCInt.h"

#else

#include <stdint.h>
#include <unistd.h>

#endif

#endif

#include "Common.h"
#include "CPackedFileReader.h"

int status;

#ifdef ANDROID
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include <android/asset_manager.h>

extern AAssetManager *defaultAssetManager;

int android_read(void *cookie, char *buf, int size) {
    return AAsset_read((AAsset *) cookie, buf, size);
}

int android_write(void *cookie, const char *buf, int size) {
    return EACCES;
}

fpos_t android_seek(void *cookie, fpos_t offset, int whence) {
    return AAsset_seek((AAsset *) cookie, offset, whence);
}

int android_close(void *cookie) {
    AAsset_close((AAsset *) cookie);
    return 0;
}


FILE *android_fopen(const char* filename) {

    AAsset *asset = AAssetManager_open(defaultAssetManager, filename, 0);
    if (!asset) {
        return NULL;
    }

    return funopen(asset, android_read, android_write, android_seek, android_close);

}
#endif

struct StaticBuffer loadFileFromPath(const char *dataFilePath, const char *path) {
    struct StaticBuffer toReturn;
#ifndef ANDROID
    FILE *mDataPack = fopen(dataFilePath, "rb");
#else
    FILE *mDataPack = android_fopen(dataFilePath);
#endif

    uint32_t offset = 0;
    char buffer[85];
    uint16_t entries = 0;
    size_t lenA;
    int c;
    int toRead = 0;
    uint32_t pos = 0;
    uint32_t until = 0;
    uint32_t size;

    assert(fread(&entries, 2, 1, mDataPack));
    pos += 2;
    lenA = strlen(path);

    for (c = 0; c < entries; ++c) {
        uint8_t stringSize = 0;
        size_t lenB;

        assert(fread(&offset, 4, 1, mDataPack));
        offset = toNativeEndianess(offset);
        pos += 4;

        stringSize = 0;
        assert(fread(&stringSize, 1, 1, mDataPack));
        pos += 1;
        assert(fread(&buffer, stringSize + 1, 1, mDataPack));
        pos += stringSize + 1;

        /* printf("Loading %s at %lu\n", buffer, offset ); */
        lenB = strlen(buffer);


        if (!strcmp(path + (strlen(path) - 4), ".png") &&
            !strcmp(buffer + (strlen(buffer) - 4), ".img") &&
            lenA == lenB) {

            size_t d = 0;
            for (d = 0; d < lenA; ++d) {
                if (path[d] != buffer[d]) {
                    goto nextfile;
                }

                if (path[d] == '.') {
                    goto found;
                }
            }
        }

        nextfile:

        if (!strcmp(buffer, path)) {
            goto found;
        }
    }

    printf("File not found: %s\n", path);
    assert(FALSE);

    found:

    if (offset == 0) {
        printf("failed to load %s", path);
        exit(-1);
    }

    fseek(mDataPack, offset, SEEK_SET);
    status = fread(&size, 4, 1, mDataPack);
    size = toNativeEndianess(size);
    /* printf("status: %d, errno: %d ferror: %d feof: %d\n", status, errno, ferror(mDataPack), feof(mDataPack)); */
    assert(status != 0);
    toReturn.data = (uint8_t *) (calloc(size, 1));
    toReturn.size = size;
    assert(fread(toReturn.data, 1, size, mDataPack));
    fclose(mDataPack);

    return toReturn;
}
