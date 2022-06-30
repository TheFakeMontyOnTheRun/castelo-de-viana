#ifndef COMMON_H
#define COMMON_H

struct ItemVector {
    void **items;
    size_t capacity;
    size_t used;
};

struct StaticBuffer {
    uint8_t *data;
    size_t size;
};

int pushVector(struct ItemVector *vector, void *item);

int removeFromVector(struct ItemVector *vector, void *item);

void initVector(struct ItemVector *vector, size_t capacity);

void clearVector(struct ItemVector *vector);

size_t countTokens(const char *text, size_t length);

int min(int val1, int val2);

int isBigEndian();

uint32_t toNativeEndianess(uint32_t val);

#define DEFAULT_TOLERANCE 32
#define TRUE 1
#define FALSE 0

#endif
