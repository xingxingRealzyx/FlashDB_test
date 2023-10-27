#ifndef __NVS_OPS_H__
#define __NVS_OPS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct nvs_ops_t nvs_ops_t;
struct nvs_ops_t
{
    int     (*init)(nvs_ops_t*);
    int     (*set_i32)(nvs_ops_t*, const char *, int32_t);
    int32_t (*get_i32)(nvs_ops_t*, const char *, int32_t*);
    int     (*commit)(nvs_ops_t*);
    int     (*deinit)(nvs_ops_t*);

    nvs_handle_t _handle;
};

nvs_ops_t *nvs_ops_get_instance();

#ifdef __cplusplus
}
#endif


#endif /* __FLASHDB_OPS_H__ */