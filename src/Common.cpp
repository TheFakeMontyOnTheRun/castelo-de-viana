//
// Created by monty on 26/09/16.
//
#include <string.h>

#include <functional>
#include <iterator>
#include <vector>

using std::vector;

#include "Common.h"

namespace odb {

	char* fileFromString( const char* path ) {
		char * buffer = 0;
		long length;
		FILE * f = fopen (path, "r");

		if (f)
		{
			fseek (f, 0, SEEK_END);
			length = ftell (f);
			fseek (f, 0, SEEK_SET);
			buffer = static_cast<char *>(malloc (length));
			if (buffer)
			{
				fread (buffer, 1, length, f);
			}
			fclose (f);
		}

		return buffer;
	}

	vector<char> readToBuffer(FILE *fileDescriptor) {
		const unsigned N = 1024;

		fseek(fileDescriptor, 0, SEEK_END);
		auto endPos = ftell( fileDescriptor );
		rewind(fileDescriptor);
		vector<char> total(endPos);
		auto writeHead = std::begin( total );

		for ( int c = 0; c < endPos; ++c ) {
			char buffer[N];
			size_t read = fread((void *) &buffer[0], 1, N, fileDescriptor);
			if (read) {
				for (unsigned int c = 0; c < read; ++c) {
					*writeHead = (buffer[c]);
					writeHead = std::next(writeHead);
				}
			}
			if (read < N) {
				break;
			}
		}

		return total;
	}
}