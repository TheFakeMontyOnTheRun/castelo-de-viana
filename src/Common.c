#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef AMIGA
#include "AmigaInt.h"

#else

#ifdef CPC

#include "CPCInt.h"

#else

#include <stdint.h>
#include <unistd.h>

#endif

#endif

#include "Common.h"

void clearVector(struct ItemVector *vector) {
	memset(vector->items, 0, sizeof(void *) * vector->capacity);
	vector->used = 0;
}

void initVector(struct ItemVector *vector, size_t capacity) {
	vector->capacity = capacity;
	vector->used = 0;
	vector->items = (void **) (malloc(sizeof(void *) * capacity));
}

void pushVector(struct ItemVector *vector, void *item) {

	assert(vector->used + 1 <= vector->capacity);
	vector->items[vector->used] = item;
	vector->used++;
}

int min(int val1, int val2) {
	return val1 < val2 ? val1 : val2;
}

int isBigEndian() {
	union {
		uint32_t i;
		char c[4];
	} e = { 0x01000000 };

	return e.c[0];
}

uint32_t toNativeEndianess(uint32_t val) {
	uint32_t  val2 = val;

	if (isBigEndian()) {
		uint32_t b0,b1,b2,b3;

		b0 = (val & 0x000000ff) << 24u;
		b1 = (val & 0x0000ff00) << 8u;
		b2 = (val & 0x00ff0000) >> 8u;
		b3 = (val & 0xff000000) >> 24u;

		val2 = b0 | b1 | b2 | b3;
	}

	return val2;
}

size_t countTokens(const char *text, size_t length) {

	size_t count = 0;
	char *ptr = (char *) text;

	size_t pos = 0;
	for (pos = 0; pos < length; ++pos) {
		if (*ptr == '\n') {
			count++;
		}
		++ptr;
	}

	return count;
}
