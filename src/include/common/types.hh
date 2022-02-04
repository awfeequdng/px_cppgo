#pragma once
#include <stdlib.h>

typedef uint64_t uint64;
typedef int64_t int64;
typedef int8_t int8;
typedef uint8_t uint8;

typedef unsigned char byte;

typedef unsigned int uint;

#define ARRAY_SZ(a) (sizeof(a)/sizeof(a[0]))
#define panic(a)    { std::cout << (a) << std::endl; exit(0); } while(0)
