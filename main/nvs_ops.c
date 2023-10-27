#include "nvs_ops.h"

static const char *TAG = "NVS_OPS";

static nvs_ops_t *nvs_ops = NULL;
static nvs_ops_t s_nvs_ops;

static int nvs_ops_init(nvs_ops_t *t)
{
    int err = 0;
    
    err = nvs_open("storage", NVS_READWRITE, &t->_handle);

    return err;
}

static int nvs_ops_set_i32(nvs_ops_t *t,char *key, int32_t value)
{
    int err = 0;

    err = nvs_set_i32(t->_handle, key, value);

    return err;
}

static int nvs_ops_get_i32(nvs_ops_t *t,const char *key, int32_t *value)
{

    int err = 0;

    err = nvs_get_i32(t->_handle, key, value);

    return err;
}

static int nvs_ops_commit(nvs_ops_t *t)
{
    int err = 0;

    err = nvs_commit(t->_handle);

    return err;
}

static int nvs_ops_deinit(nvs_ops_t *t)
{
    int err = 0;

    nvs_close(t->_handle);

    return err;
}

nvs_ops_t *nvs_ops_get_instance()
{
    if (nvs_ops == NULL)
    {
        nvs_ops = &s_nvs_ops;
        nvs_ops->init    = nvs_ops_init;
        nvs_ops->set_i32 = nvs_ops_set_i32;
        nvs_ops->get_i32 = nvs_ops_get_i32;
        nvs_ops->commit  = nvs_ops_commit;
        nvs_ops->deinit  = nvs_ops_deinit;
    }
    return nvs_ops;
    
}
