
#ifndef BF_CDOSRenderer_H
#define BF_CDOSRenderer_H

namespace odb {

    std::shared_ptr<NativeBitmap> loadBitmap(std::string path, std::shared_ptr<odb::IFileLoaderDelegate> fileLoader, EVideoType videoType);

    std::vector<std::shared_ptr<odb::NativeBitmap>> loadSpriteList(std::string listName, std::shared_ptr<odb::IFileLoaderDelegate> fileLoader, EVideoType videoType);
}
#endif
