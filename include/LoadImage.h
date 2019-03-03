
#ifndef BF_CDOSRenderer_H
#define BF_CDOSRenderer_H

namespace odb {

    NativeBitmap* loadBitmap(const char* path, odb::CPackedFileReader* fileLoader, EVideoType videoType);

	odb::ItemVector* loadSpriteList(const char* listName, odb::CPackedFileReader* fileLoader, EVideoType videoType);
}
#endif
