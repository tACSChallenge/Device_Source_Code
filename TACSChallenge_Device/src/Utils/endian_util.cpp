#include "Utils/endian_util.h"

static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order =
    { { 0, 1, 2, 3 } };

ENDIAN getEndian(){
    switch (o32_host_order.value){
        case O32_LITTLE_ENDIAN : return ENDIAN_LITTLE;
        case O32_BIG_ENDIAN : return ENDIAN_BIG;
        case O32_PDP_ENDIAN : return ENDIAN_PDP;
    }
    return ENDIAN_OTHER;
}