//
// Created by monty on 26/09/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_COMMON_H
#define DUNGEONSOFNOUDAR_NDK_COMMON_H
namespace odb {

    vector<char> readToBuffer(FILE *fileDescriptor);

    char* fileFromString( const char* path );
}
#endif //DUNGEONSOFNOUDAR_NDK_COMMON_H
