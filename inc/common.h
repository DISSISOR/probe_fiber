#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <stddef.h>

#define CONTAINER_OF(type, mem, ptr) \
( (type*) (\
    (char*)(ptr) - offsetof(type, mem)\
    ) \
)

#endif // COMMON_H_INCLUDED
