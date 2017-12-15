//
// Created by monty on 26/09/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_COMMON_H
#define DUNGEONSOFNOUDAR_NDK_COMMON_H
namespace odb {

    int intFrom(std::string str);

    std::string filterComments(std::string input);

    std::string readToString(FILE *fileDescriptor);

    vector<char> readToBuffer(FILE *fileDescriptor);

    std::string fileFromString( const std::string& path );
}
#endif //DUNGEONSOFNOUDAR_NDK_COMMON_H
