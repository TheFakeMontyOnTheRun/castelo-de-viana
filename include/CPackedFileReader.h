//
// Created by monty on 06-12-2017.
//

#ifndef DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H
#define DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H

namespace odb {
    class CPackedFileReader : public odb::IFileLoaderDelegate {
        FILE *mDataPack = nullptr;
        char* mPackPath;
    public:
        CPackedFileReader() = delete;
        const char* getFilePathPrefix() override;
        explicit CPackedFileReader( const char* dataFilePath );
        vector<char> loadBinaryFileFromPath( const char* path ) override;

        StaticBuffer loadFileFromPath( const char* path ) override ;
    };
}

#endif //DUNGEONSOFNOUDAR486_CPACKEDFILEREADER_H
