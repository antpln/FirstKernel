#include <string.h>

void* memcpy(void* dstptr, const void* srcptr, size_t size) {
	auto dst = static_cast<unsigned char*>(dstptr);
	auto src = static_cast<const unsigned char*>(srcptr);
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}
