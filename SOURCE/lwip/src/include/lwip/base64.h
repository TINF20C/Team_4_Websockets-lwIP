#ifndef BASE64_H
#define BASE64_H
/*************************** HEADER FILES***************************/
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char base64_table[64];
/*********************** FUNCTION DECLARATIONS **********************/
size_t lwip_base64_encode( char* target, size_t target_len, const char* source, size_t source_len );

#ifdef __cplusplus
}
#endif
#endif // !BASE64_H