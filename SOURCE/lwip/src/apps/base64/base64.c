#include "lwip/arch.h"
#include "lwip/debug.h"
#include "lwip/base64.h"


static const char base64_table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
  'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '+', '/'
};

/** Base64 encoding */
size_t lwip_base64_encode(char* target, size_t target_len, const char* source, size_t source_len)
{
    size_t i;
    s8_t j;
    size_t target_idx = 0;
    size_t longer = (source_len % 3) ? (3 - (source_len % 3)) : 0;
    size_t source_len_b64 = source_len + longer;
    size_t len = (((source_len_b64) * 4) / 3);
    u8_t x = 5;
    u8_t current = 0;
    LWIP_UNUSED_ARG(target_len);

    LWIP_ASSERT("target_len is too short", target_len >= len);

    for (i = 0; i < source_len_b64; i++) {
        u8_t b = (i < source_len ? (u8_t)source[i] : 0);
        for (j = 7; j >= 0; j--, x--) {
            if ((b & (1 << j)) != 0) {
                current = (u8_t)(current | (1U << x));
            }
            if (x == 0) {
                target[target_idx++] = base64_table[current];
                x = 6;
                current = 0;
            }
        }
    }
    for (i = len - longer; i < len; i++) {
        target[i] = '=';
    }
    return len;
}