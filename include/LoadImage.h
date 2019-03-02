
#ifndef BF_CDOSRenderer_H
#define BF_CDOSRenderer_H

namespace odb {

    NativeBitmap* loadBitmap(const char* path, odb::IFileLoaderDelegate* fileLoader, EVideoType videoType);

    std::vector<odb::NativeBitmap*> loadSpriteList(const char* listName, odb::IFileLoaderDelegate* fileLoader, EVideoType videoType);
}
#endif
