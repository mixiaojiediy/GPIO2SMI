#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>

typedef uint16_t u16;
typedef uint64_t u64;

static inline uint32_t StrToInt(const char *str) { (void)str; return 0; }
static inline void Raise_Dog(void) {}

#endif
