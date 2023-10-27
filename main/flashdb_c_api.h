#ifndef __FLASHDB_OPS_H__
#define __FLASHDB_OPS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <flashdb.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kvdb_t kvdb_t;

struct kvdb_t
{
    int (*init)         (kvdb_t *, const char *name, const char *path);
    int (*deinit)       (kvdb_t *);
    int (*insert_string)(kvdb_t *, const char *key, const char *value);
    int (*insert_blob)  (kvdb_t *, const char *key, fdb_blob_t blob);
    int (*delete)       (kvdb_t *, const char *key);
    int (*update_string)(kvdb_t *, const char *key, const char *value);
    int (*update_blob)  (kvdb_t *, const char *key, fdb_blob_t blob);
    int (*select_string)(kvdb_t *, const char *key, char *value);
    int (*select_blob)  (kvdb_t *, const char *key, fdb_blob_t blob);
    int (*dump)         (kvdb_t *);

    fdb_kvdb_t _fdb_kvdb;

};

typedef struct tsdb_t tsdb_t;

struct tsdb_t
{
    int (*init)          (tsdb_t *, const char *name, const char *path, int max_len);
    int (*deinit)        (tsdb_t *);
    int (*insert_blob)   (tsdb_t *, fdb_blob_t blob);
    int (*delete_all)    (tsdb_t *);
    int (*select_by_time)(tsdb_t *, fdb_time_t from, fdb_time_t to, fdb_tsl_cb cb, void *cb_arg);
    int (*query_count)   (tsdb_t *, fdb_time_t from, fdb_time_t to, fdb_tsl_status_t status);
    int (*dump)          (tsdb_t *, fdb_tsl_cb cb, void *cb_arg);

    fdb_tsdb_t _fdb_tsdb;
    
};


typedef struct flashdb_ops_t flashdb_ops_t;

struct flashdb_ops_t
{
    kvdb_t* (*create_kvdb)(flashdb_ops_t *);
    int     (*drop_kvdb)  (flashdb_ops_t *, kvdb_t *);
    tsdb_t* (*create_tsdb)(flashdb_ops_t *);
    int     (*drop_tsdb)  (flashdb_ops_t *, tsdb_t *);
    void    (*test)       (flashdb_ops_t *);

};

flashdb_ops_t *flashdb_ops_get_instance();

#ifdef __cplusplus
}
#endif

#endif /* __FLASHDB_OPS_H__ */