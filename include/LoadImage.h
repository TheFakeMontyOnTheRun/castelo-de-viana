
#ifndef BF_CDOSRenderer_H
#define BF_CDOSRenderer_H

namespace odb {

    NativeBitmap* loadBitmap(std::string path, odb::IFileLoaderDelegate* fileLoader, EVideoType videoType);

    std::vector<odb::NativeBitmap*> loadSpriteList(std::string listName, odb::IFileLoaderDelegate* fileLoader, EVideoType videoType);
}
#endif
