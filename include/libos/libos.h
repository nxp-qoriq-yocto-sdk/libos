#ifndef LIBOS_H
#define LIBOS_H

#include <stddef.h>
#include <stdint.h>
#include <libos/console.h>


#define stopsim() do { \
	asm volatile("mr 22, 22" : : : "memory"); \
	for(;;); \
} while (0)

#define BUG() do { \
	printf("Assertion failure at %s:%d\n", __FILE__, __LINE__); \
	__builtin_trap(); \
} while (0)

#define assert(x) do { if (__builtin_expect(!(x), 0)) BUG(); } while (0)

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define LONG_BITS (sizeof(long) * 8)

#define max(x, y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x > _y ? _x : _y; \
})

#define min(x, y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x < _y ? _x : _y; \
})

#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */

typedef uint64_t physaddr_t;
typedef unsigned long register_t;

void *alloc(unsigned long size, unsigned long align);
void alloc_init(unsigned long heap_start, unsigned long heap_end);

#endif
