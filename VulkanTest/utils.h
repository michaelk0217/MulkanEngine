#pragma once
size_t getAlignedUboSize(size_t originalSize, size_t minAlignment) {
    return (originalSize + minAlignment - 1) & ~(minAlignment - 1);
}