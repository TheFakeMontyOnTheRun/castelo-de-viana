
#ifndef BF_CDOSRenderer_H
#define BF_CDOSRenderer_H

namespace odb {

    NativeBitmap* loadBitmap(const char* path, EVideoType videoType);

	odb::ItemVector* loadSpriteList(const char* listName, EVideoType videoType);
}
#endif
