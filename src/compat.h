// Included first in every engine file (CMake: -include). The engine's game
// code uses strlcpy / strlcat, which the boards' C library has and glibc
// only got in 2.38 - newer than many handheld firmwares'. The port brings
// its own, under its own names, so the binary never depends on glibc's.
#pragma once

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
size_t pm_strlcpy(char* dst, const char* src, size_t cap);
size_t pm_strlcat(char* dst, const char* src, size_t cap);
#ifdef __cplusplus
}
#endif

#define strlcpy pm_strlcpy
#define strlcat pm_strlcat
