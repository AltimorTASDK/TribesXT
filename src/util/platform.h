#pragma once

#include "util/preprocessor.h"

#define PRAGMA(x) _Pragma(#x)

#define PACK_BODY_(...) __VA_ARGS__; PRAGMA(pack(pop)) FORCE_SEMICOLON
#define PACK(n) PRAGMA(pack(push, n)) PACK_BODY_
#define PACKED PACK(1)

constexpr size_t PAGE_SIZE = 0x1000;
