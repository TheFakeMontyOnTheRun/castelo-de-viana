#ifndef LOADIMAGE_H
#define LOADIMAGE_H

struct NativeBitmap *loadBitmap(const char *path, enum EVideoType videoType);

struct ItemVector *loadSpriteList(const char *listName, enum EVideoType videoType);

#endif
