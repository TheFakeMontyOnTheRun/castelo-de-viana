
#ifndef BF_CDOSRenderer_H
#define BF_CDOSRenderer_H

namespace odb {

    std::shared_ptr<NativeBitmap> loadBitmap(std::string path);

    std::vector<char> readToBuffer(FILE *fileDescriptor);

    std::string getResPath();

    std::vector<std::shared_ptr<odb::NativeBitmap>> loadSpriteList(std::string listName);
}
#endif
