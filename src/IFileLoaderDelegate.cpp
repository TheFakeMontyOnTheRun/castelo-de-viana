//
// Created by monty on 09/12/16.
//
#include <string>
#include <vector>

using std::vector;

#include "IFileLoaderDelegate.h"

namespace odb {
    std::string filterLineBreaks(std::string input) {
        auto position = input.find('\n');

        while (position != std::string::npos) {
            input.replace(position, 1, "");
            position = input.find('\n', position + 1);
        }

        return input;
    }
}

