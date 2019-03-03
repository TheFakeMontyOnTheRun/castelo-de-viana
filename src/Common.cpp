//
// Created by monty on 26/09/16.
//
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Common.h"

namespace odb {

	void clearVector(ItemVector* vector) {
		memset(vector->items, 0, sizeof(void*) * vector->capacity );
		vector->used = 0;
	}

	void initVector(ItemVector* vector, size_t capacity ) {
		vector->capacity = capacity;
		vector->used = 0;
		vector->items = (void **)(malloc(sizeof(void*) * capacity ));
	}

	void pushVector( ItemVector* vector, void* item ) {

		assert( vector->used + 1 <= vector->capacity );
		vector->items[ vector->used ] = item;
		vector->used++;
	}

	int min(int val1, int val2) {
		return val1 < val2 ? val1 : val2;
	}

	size_t countTokens( const char* text, size_t length ) {

		size_t count = 0;
		char* ptr = (char*)text;

		for (size_t pos = 0; pos < length; ++pos ) {
			if (*ptr == '\n') {
				count++;
			}
			++ptr;
		}

		return count;
	}

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
}