#include "flashdb_c_api.h"

static flashdb_ops_t *flashdb_ops = NULL;
static flashdb_ops_t s_flashdb_ops;
static SemaphoreHandle_t s_lock = NULL;

static void lock(fdb_db_t db)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
}

static void unlock(fdb_db_t db)
{
    xSemaphoreGive(s_lock);
}

static struct fdb_default_kv_node default_kv_table[] = {
    {"username", "zyx",    0},                       /* string KV */
    {"password", "123456", 0},                       /* string KV */
};

static int kvdb_init(kvdb_t *t, const char *name, const char *path)
{
    int err = 0;
    struct fdb_default_kv default_kv;

    default_kv.kvs = default_kv_table;
    default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);

    fdb_kvdb_control(t->_fdb_kvdb, FDB_KVDB_CTRL_SET_LOCK, lock);
    fdb_kvdb_control(t->_fdb_kvdb, FDB_KVDB_CTRL_SET_UNLOCK, unlock);
    err = fdb_kvdb_init(t->_fdb_kvdb, name, path, &default_kv, NULL);

    return err;
}

static int kvdb_deinit(kvdb_t *t)
{
    int err = 0;
    err = fdb_kvdb_deinit(t->_fdb_kvdb);

    return err;    
}

static int kvdb_insert_string(kvdb_t *t, const char *key, const char *value)
{
    int err = 0;
    err = fdb_kv_set(t->_fdb_kvdb, key, value);

    return err;
}

static int kvdb_insert_blob(kvdb_t *t, const char *key, fdb_blob_t blob)
{
    int err = 0;
    err = fdb_kv_set_blob(t->_fdb_kvdb, key, blob);

    return err;
}

static int kvdb_delete(kvdb_t *t, const char *key)
{
    int err = 0;
    err = fdb_kv_del(t->_fdb_kvdb, key);

    return err;
}

static int kvdb_update_string(kvdb_t *t, const char *key, const char *value)
{
    int err = 0;
    err = fdb_kv_set(t->_fdb_kvdb, key, value);

    return err;
}

static int kvdb_update_blob(kvdb_t *t, const char *key, fdb_blob_t blob)
{
    int err = 0;
    err = fdb_kv_set_blob(t->_fdb_kvdb, key, blob);

    return err;
}

static int kvdb_select_string(kvdb_t *t, const char *key, char *value)
{
    int err = 0;
    value = fdb_kv_get(t->_fdb_kvdb, key);
    if (value == NULL)
    {
        return -1;   
    }
    return err;
}

static int kvdb_select_blob(kvdb_t *t, const char *key, fdb_blob_t blob)
{
    int err = 0;
    fdb_kv_get_blob(t->_fdb_kvdb, key, blob);

    return err;
}

static int kvdb_dump(kvdb_t *t)
{
    int err = 0;
    fdb_kv_print(t->_fdb_kvdb);
    return err;
}

static kvdb_t *flashdb_ops_create_kvdb(flashdb_ops_t *fops)
{
    kvdb_t *kvdb = (kvdb_t *)malloc(sizeof(kvdb_t));

    if (kvdb != NULL)
    {
        kvdb->init          = kvdb_init;
        kvdb->deinit        = kvdb_deinit;
        kvdb->insert_string = kvdb_insert_string;
        kvdb->insert_blob   = kvdb_insert_blob;
        kvdb->delete        = kvdb_delete;
        kvdb->update_string = kvdb_update_string;
        kvdb->update_blob   = kvdb_update_blob;
        kvdb->select_string = kvdb_select_string;
        kvdb->select_blob   = kvdb_select_blob;
        kvdb->dump          = kvdb_dump;

        kvdb->_fdb_kvdb     = (fdb_kvdb_t)malloc(sizeof(struct fdb_kvdb));
        if (kvdb->_fdb_kvdb == NULL)
        {
            free(kvdb);
            kvdb = NULL;
            return NULL;
        }
        memset(kvdb->_fdb_kvdb, 0, sizeof(struct fdb_kvdb)); // 一定要清内存

    }

    return kvdb;
    
}

static int flashdb_ops_drop_kvdb(flashdb_ops_t *fops, kvdb_t *kvdb)
{
    int err = 0;
    if (kvdb)
    {
        err = kvdb->deinit(kvdb);
        free(kvdb);
        kvdb = NULL;
    } else err = -1;

    return err;
}

static fdb_time_t get_time(void)
{
    return time(NULL);
}

static int tsdb_init(tsdb_t *t, const char *name, const char *path, int max_len)
{
    int err = 0;
    fdb_tsdb_control(t->_fdb_tsdb, FDB_TSDB_CTRL_SET_LOCK, lock);
    fdb_tsdb_control(t->_fdb_tsdb, FDB_TSDB_CTRL_SET_UNLOCK, unlock);
    err = fdb_tsdb_init(t->_fdb_tsdb, name, path, get_time, max_len, NULL);

    return err;
}

static int tsdb_deinit(tsdb_t *t)
{
    int err = 0;
    err = fdb_tsdb_deinit(t->_fdb_tsdb);

    return err;
}

static int tsdb_insert_blob(tsdb_t *t, fdb_blob_t blob)
{
    int err = 0;
    fdb_tsl_append(t->_fdb_tsdb, blob);

    return err;
}

static int tsdb_delete_all(tsdb_t *t)
{
    int err = 0;
    fdb_tsl_clean(t->_fdb_tsdb);
    return err;
}

static int tsdb_select_by_time(tsdb_t *t, fdb_time_t from, fdb_time_t to, fdb_tsl_cb cb, void *cb_arg)
{
    int err = 0;
    fdb_tsl_iter_by_time(t->_fdb_tsdb, from, to, cb, cb_arg);

    return err;
}

static int tsdb_query_count(tsdb_t *t, fdb_time_t from, fdb_time_t to, fdb_tsl_status_t status)
{
    return fdb_tsl_query_count(t->_fdb_tsdb, from, to, status);

}

static int tsdb_dump(tsdb_t *t, fdb_tsl_cb cb, void *cb_arg)
{
    int err = 0;
    fdb_tsl_iter(t->_fdb_tsdb, cb, cb_arg);
    return err;
}

static tsdb_t *flashdb_ops_create_tsdb(flashdb_ops_t *fops)
{
    tsdb_t *tsdb = (tsdb_t *)malloc(sizeof(tsdb_t));
    if (tsdb != NULL)
    {
        tsdb->init           = tsdb_init;
        tsdb->deinit         = tsdb_deinit;
        tsdb->insert_blob    = tsdb_insert_blob;
        tsdb->delete_all     = tsdb_delete_all;
        tsdb->select_by_time = tsdb_select_by_time;
        tsdb->query_count    = tsdb_query_count;
        tsdb->dump           = tsdb_dump;
        tsdb->_fdb_tsdb      = (fdb_tsdb_t)malloc(sizeof(struct fdb_tsdb));

        if (tsdb->_fdb_tsdb == NULL)
        {
            free(tsdb);
            tsdb = NULL;
            return NULL;
        }
        memset(tsdb->_fdb_tsdb, 0, sizeof(struct fdb_tsdb)); // 一定要清内存
        
    }
    return tsdb;
}

static int flashdb_ops_drop_tsdb(flashdb_ops_t *fops, tsdb_t *tsdb)
{
    int err = 0;
    if (tsdb)
    {
        err = tsdb->deinit(tsdb);
        free(tsdb);
        tsdb = NULL;
    } else err = -1;
    
    return err;
}

static void flashdb_ops_test()
{
    fdb_kvdb_t kvdb = (fdb_kvdb_t)malloc(sizeof(struct fdb_kvdb));
    memset(kvdb, 0, sizeof(struct fdb_kvdb));
    struct fdb_default_kv default_kv;

    default_kv.kvs = default_kv_table;
    default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);

    fdb_kvdb_control(kvdb, FDB_KVDB_CTRL_SET_LOCK, lock);
    fdb_kvdb_control(kvdb, FDB_KVDB_CTRL_SET_UNLOCK, unlock);
    
    fdb_kvdb_init(kvdb, "env", "fdb_kvdb1", &default_kv, NULL);

}

flashdb_ops_t *flashdb_ops_get_instance()
{

    if (flashdb_ops == NULL)
    {
        flashdb_ops              = &s_flashdb_ops;
        flashdb_ops->create_kvdb = flashdb_ops_create_kvdb;
        flashdb_ops->drop_kvdb   = flashdb_ops_drop_kvdb;
        flashdb_ops->create_tsdb = flashdb_ops_create_tsdb;
        flashdb_ops->drop_tsdb   = flashdb_ops_drop_tsdb;
        flashdb_ops->test        = flashdb_ops_test;
        if (s_lock == NULL)
        {
            s_lock = xSemaphoreCreateCounting(1, 1);
            assert(flashdb_ops->_lock != NULL);
        }
        
    }
    
    return flashdb_ops;
}

