#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stddef.h>

#ifdef FIBER_DEBUG
#define FIBER_TRACE
#else
#define FIBER_TRACE
#endif

#define OFFSET_OF(type, mem) \
( (size_t) &(\
    ((type*)(NULL))->mem\
    ) \
)

#define CONTAINER_OF(type, mem, ptr) \
( (type*) (\
    (char*)(ptr) - OFFSET_OF(type, mem)\
    ) \
)
#define UNREACHABLE() assert(0 && "Unreachable")

#endif // COMMON_H_INCLUDED
