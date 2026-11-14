#include "compat.h"

// As BSD's: at most cap - 1 characters copied, always ended with 0 (cap >
// 0); the length it tried to make is returned.
size_t pm_strlcpy(char* dst, const char* src, size_t cap)
{
    const size_t n = strlen(src);
    if (cap) {
        const size_t k = n < cap - 1 ? n : cap - 1;
        memcpy(dst, src, k);
        dst[k] = 0;
    }
    return n;
}

size_t pm_strlcat(char* dst, const char* src, size_t cap)
{
    const size_t d = strnlen(dst, cap);
    if (d == cap) return cap + strlen(src);
    return d + pm_strlcpy(dst + d, src, cap - d);
}
