#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "Common.h"
#include "NativeBitmap.h"
#include "CPackedFileReader.h"
#include "Renderer.h"
#include "LoadImage.h"


#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"


struct ItemVector *loadSpriteList(const char *listName, enum EVideoType videoType) {

	struct StaticBuffer buffer = loadFileFromPath(getAssetsPath(), listName);
	struct ItemVector *tilesToLoad = (struct ItemVector *) calloc(sizeof(struct ItemVector), 1);

	size_t items = countTokens((char *) buffer.data, buffer.size) + 1;
	initVector(tilesToLoad, items);


	int lastPoint = 0;
	int since = 0;

	uint8_t *bufferBegin = buffer.data;
	size_t pos = 0;
	for (pos = 0; pos < buffer.size; ++pos) {
		const char c = buffer.data[pos];

		++since;
		if (pos == buffer.size - 1 || c == '\n') {

			if (pos == buffer.size - 1) {
				since++;
			}

			char *filename = (char *) calloc(since - 1 + 1, 1);
			size_t diff = (bufferBegin + lastPoint + since - 1) -
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
	struct StaticBuffer buffer = loadFileFromPath(getAssetsPath(), path);
	int xSize;
	int ySize;
	int components;
	int *rawData;
	uint8_t *data8;

	if (videoType == kVGA) {
		stbi_uc *image = stbi_load_from_memory((const stbi_uc *) buffer.data,
											   buffer.size, &xSize, &ySize,
											   &components, 4);
		rawData = (int *) malloc(sizeof(int) * xSize * ySize);
		data8 = (uint8_t *) malloc(sizeof(uint8_t) * xSize * ySize);
		memcpy(rawData, image, xSize * ySize * sizeof(int));
		int c = 0;
		for (c = 0; c < xSize * ySize; ++c) {
			int origin = rawData[c];

			int a = (origin & 0xFF000000) >> 24;
			int r = (origin & 0x00FF0000) >> 16;
			int g = (origin & 0x0000FF00) >> 8;
			int b = (origin & 0x000000FF);

			if (a < 128) {
				origin = 0x00FF00;
			}

			rawData[c] = origin;
		}
		stbi_image_free(image);
		for (c = 0; c < (xSize * ySize); ++c) {
			data8[c] = getPaletteEntry(rawData[c]);
		}

		free(rawData);

	} else {
		stbi_uc *image = stbi_load_from_memory((const stbi_uc *) buffer.data,
											   buffer.size, &xSize, &ySize,
											   &components, 1);
		rawData = (int *) malloc(sizeof(int) * xSize * ySize);
		data8 = (uint8_t *) malloc(sizeof(uint8_t) * xSize * ySize);

		int y = 0;
		for (y = 0; y < ySize; ++y) {
			int x = 0;
			for (x = 0; x < xSize; ++x) {
				int pixel = image[(y * xSize) + x];
				switch (pixel) {
					case 154:
						pixel = 10;
						break;
					case 103:
						pixel = 6;
						break;
					case 51:
						pixel = 2;
						break;
					case 34:
						pixel = 1;
						break;
					case 108:
						pixel = 9;
						break;
					case 68:
						pixel = 5;
						break;
					case 0:
					case 102:
					case 101:
						pixel = 4;
						break;
					case 85:
						pixel = 3;
						break;
					case 255:
						pixel = 11;
						break;
					case 149:
						pixel = 0;
						break;
					default:
						pixel = 0;
						break;
				}
				rawData[(y * xSize) + x] = pixel;
			}
		}
		stbi_image_free(image);

		int c = 0;
		for (c = 0; c < (xSize * ySize); ++c) {
			data8[c] = (rawData[c]);
		}

		free(rawData);
	}


	struct NativeBitmap *toReturn = (struct NativeBitmap *) calloc(sizeof(struct NativeBitmap), 1);
	toReturn->mWidth = xSize;
	toReturn->mHeight = ySize;
	toReturn->mRawData = data8;

	return toReturn;
}

