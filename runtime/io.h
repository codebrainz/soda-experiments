#ifndef SODA_RUNTIME_IO_H
#define SODA_RUNTIME_IO_H

#include <uchar.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SodaString_ SodaString;

void soda_io_print(SodaString*);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // SODA_RUNTIME_IO_H
