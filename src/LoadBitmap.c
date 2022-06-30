#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "Common.h"
#include "NativeBitmap.h"
#include "CPackedFileReader.h"
#include "Renderer.h"
#include "LoadImage.h"


struct ItemVector *loadSpriteList(const char *listName, enum EVideoType videoType) {
	int lastPoint = 0;
	int since = 0;
	uint8_t *bufferBegin;
	size_t pos;

	struct StaticBuffer buffer = loadFileFromPath(getAssetsPath(), listName);
	struct ItemVector *tilesToLoad = (struct ItemVector *) calloc(sizeof(struct ItemVector), 1);

	size_t items = countTokens((char *) buffer.data, buffer.size) + 1;
	initVector(tilesToLoad, items);


	bufferBegin = buffer.data;

	for (pos = 0; pos < buffer.size; ++pos) {
		uint8_t c = buffer.data[pos];

		++since;
		if (pos == buffer.size - 1 || c == '\n') {
			char *filename;
			size_t diff;

			if (pos == buffer.size - 1) {
				since++;
			}

			filename = (char *) calloc(since - 1 + 1, 1);
			diff = (bufferBegin + lastPoint + since - 1) -
				   (bufferBegin + lastPoint);
			memcpy(&filename[0], bufferBegin + lastPoint, diff);

			lastPoint += since;

			if (strlen(filename) > 0) {
				pushVector(tilesToLoad, loadBitmap(filename, videoType));
			}
			since = 0;
			free(filename);
		}
	}

	free(buffer.data);

	return tilesToLoad;
}

struct NativeBitmap *loadBitmap(const char *path, enum EVideoType videoType) {
	size_t size;
	uint8_t *buffer;

	struct NativeBitmap *toReturn = (struct NativeBitmap *) calloc(1, sizeof(struct NativeBitmap));

	struct StaticBuffer src = loadFileFromPath(getAssetsPath(), path);

	uint16_t tmp = 0;
	uint8_t *ptr = src.data;
	tmp = *ptr++;
	toReturn->mWidth = (tmp & 0xFF) << 8;
	tmp = *ptr++;
	toReturn->mWidth += tmp & 0xFF;

	tmp = *ptr++;
	toReturn->mHeight = (tmp & 0xFF) << 8;
	tmp = *ptr++;
	toReturn->mHeight += tmp & 0xFF;

	size = toReturn->mWidth * toReturn->mHeight;

	buffer = (uint8_t *) calloc(1, size);

	if (videoType == kCGA) {
		/*
		0 black
		1 shaded blue
		2 shaded pink
		3 shaded white
		4 black
		5 mid blue
		6 mid pink
		7 mix between pink and blue
		8 gray
		9 light blue
		10 light pink
		11 white
		*/
		int y = 0;
		for (y = 0; y < toReturn->mHeight; ++y) {
			int x = 0;
			for (x = 0; x < toReturn->mWidth; ++x) {
				int pixel = ptr[(y * toReturn->mWidth) + x];
				switch (pixel) {
					case 82: /* dark white */
						pixel = 3;
						break;
					case 141: /* mid pink */
						pixel = 6;
						break;
					case 66: /* dark pink */
						pixel = 2;
						break;
					case 215: /* light pink */
						pixel = 10;
						break;
					case 173: /* mid white*/
						pixel = 11;
						break;
					case 87: /* light blue */
						pixel = 9;
						break;
					case 0: /* black */
						pixel = 4; /* 0 would be transparent */
						break;
					case 13: /* mid blue */
						pixel = 5;
						break;
					case 2: /* dark blue */
						pixel = 1;
						break;
					case 255: /* bright white */
						pixel = 11;
						break;
					case 199: /* transparent */
						pixel = 0;
						break;
					case 91:
						pixel = 8;
						break;
					case 77:
						pixel = 7;
						break;
					default:
						pixel = 0;
						break;
				}
				buffer[(y * toReturn->mWidth) + x] = pixel;
			}
		}
	} else {
		memcpy(&buffer[0], ptr, size);
	}


	toReturn->mRawData = buffer;

	return toReturn;
}

void releaseBitmap(struct NativeBitmap *ptr) {
	assert(ptr != NULL);

	free(ptr->mRawData);
	free(ptr);
}
