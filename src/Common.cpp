//
// Created by monty on 26/09/16.
//
#include <functional>
#include <string>
#include <iterator>
#include <vector>

using std::vector;

#include "Common.h"

namespace odb {

	std::string fileFromString( const std::string& path ) {
		char * buffer = 0;
		long length;
		FILE * f = fopen (path.c_str(), "r");

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

		std::string toReturn(buffer);

		return toReturn;

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

	std::string readToString(FILE *fileDescriptor) {

		fseek(fileDescriptor, 0, SEEK_END);
		long fileSize = ftell(fileDescriptor);
		rewind(fileDescriptor);
		std::string total;
		total.assign(fileSize, 0);

		for (auto pos = 0; pos < fileSize; ++pos) {
			char buffer[1];
			size_t read = fread((void *) &buffer[0], 1, 1, fileDescriptor);
			if (read) {
				for (unsigned int c = 0; c < read; ++c) {
					total[pos] = buffer[c];
				}
			}
			if (read < 1) {
				return "total not met";
			}
		}

		return total;
	}
}