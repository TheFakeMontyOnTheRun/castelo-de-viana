//
// Created by monty on 06-12-2017.
//

#ifndef DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H
#define DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H

namespace odb {
    class CPackedFileReader {
        FILE *mDataPack;
        char* mPackPath;
    public:
        const char* getFilePathPrefix() ;
        explicit CPackedFileReader( const char* dataFilePath );
        StaticBuffer loadFileFromPath( const char* path );
    };
}

#endif //DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H
