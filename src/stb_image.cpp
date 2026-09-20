#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244 4996 4456)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif