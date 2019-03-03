//
// Created by monty on 26/09/16.
//

#ifndef DUNGEONSOFNOUDAR_NDK_COMMON_H
#define DUNGEONSOFNOUDAR_NDK_COMMON_H
namespace odb {

	struct ItemVector {
		void** items;
		size_t capacity;
		size_t used;
	};

	struct StaticBuffer {
		uint8_t *data;
		size_t size;
	};

	void pushVector( ItemVector* vector, void* item );

	void initVector(ItemVector* vector, size_t capacity );

	void clearVector(ItemVector* vector);

	size_t countTokens( const char* text, size_t length);

    char* fileFromString( const char* path );

    int min(int val1, int val2);
}
#endif //DUNGEONSOFNOUDAR_NDK_COMMON_H
