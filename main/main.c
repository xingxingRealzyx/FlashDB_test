#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"

#include <flashdb.h>

#include "nvs_ops.h"
#include "flashdb_c_api.h"
#include "sta_data.h"

#define FDB_LOG_TAG "[main]"

#ifdef FDB_USING_TIMESTAMP_64BIT
#define __PRITS "ld"
#else
#define __PRITS "d"
#endif


#define TEST_DATA_NUMS 100


static time_t tt;

static bool query_by_time_cb(fdb_tsl_t tsl, void *arg)
{
    struct fdb_blob blob;
    fdb_tsdb_t db = arg;
    char tsdb_data[128];
    fdb_blob_read((fdb_db_t) db, fdb_tsl_to_blob(tsl, fdb_blob_make(&blob, &tsdb_data[0], sizeof(tsdb_data))));
    FDB_INFO("[query_by_time_cb] queried a TSL: time: %" __PRITS " tsdb_data: %s \n", tsl->time, tsdb_data);

    return false;
}

static void print_chip_info()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

static time_t boot_ts;

static void set_time_info()
{
    // 将时区设置为中国标准时间
	setenv("TZ", "CST-8", 1);
	tzset();
    char strftime_buf[64];
	struct tm timeinfo;
    struct tm _tm = { .tm_year = 2023 - 1900, .tm_mon = 9-1, .tm_mday = 19, .tm_hour = 11, .tm_min = 50, .tm_sec = 0 };

    time_t now_time = mktime(&_tm);
    boot_ts = now_time;
    struct timeval now = { .tv_sec = now_time };
    settimeofday(&now, NULL);
	tt = time(NULL);
    printf("tt:%ld\n",tt);
	localtime_r(&tt, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &_tm);
	printf("The current date/time in Shanghai is: %s\n", strftime_buf);
}

void app_main(void)
{
    fdb_err_t result = FDB_NO_ERR;
    struct fdb_blob blob;
    struct sta_data_t sta_data = {
        .reboot_counts = 0,
        .write_error_counts = 0,
        .read_error_counts = 0,
        .full_counts = 0,
        .check_error_counts = 0
    };


    // 打印芯片信息
    print_chip_info();
    // 设置时间信息，虽然没RTC和SNTP，尽量手动输入同步时间
    set_time_info();

    // 初始化 NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // nvs_ops 上电在这里读取挂测异常状态，并打印出来，这样支持离线挂测
    nvs_ops_t *nvs_ops = nvs_ops_get_instance();
    nvs_ops->init(nvs_ops);
    nvs_ops->get_i32(nvs_ops, "reboot_counts", &sta_data.reboot_counts);
    nvs_ops->get_i32(nvs_ops, "write_error_counts", &sta_data.write_error_counts);
    nvs_ops->get_i32(nvs_ops, "read_error_counts", &sta_data.read_error_counts);
    nvs_ops->get_i32(nvs_ops, "full_counts", &sta_data.full_counts);
    nvs_ops->get_i32(nvs_ops, "check_error_counts", &sta_data.check_error_counts);
    
    printf("=========================================\n");

    printf("reboot_counts     : %d\n", sta_data.reboot_counts);
    printf("write_error_counts: %d\n", sta_data.write_error_counts);
    printf("read_error_counts : %d\n", sta_data.read_error_counts);
    printf("full_counts       : %d\n", sta_data.full_counts);
    printf("check_error_counts: %d\n", sta_data.check_error_counts);

    printf("=========================================\n");

    // flashdb ops 
    flashdb_ops_t *flashdb_ops = flashdb_ops_get_instance();
    if (!flashdb_ops)
    {
        printf("flashdb_ops_get_instance error\n");
    }
    // flashdb_ops->test(flashdb_ops);
    
    kvdb_t *kvdb = flashdb_ops->create_kvdb(flashdb_ops);
    tsdb_t *tsdb = flashdb_ops->create_tsdb(flashdb_ops);
    
    kvdb->init(kvdb, "env", "fdb_kvdb1");
    tsdb->init(tsdb, "log", "fdb_tsdb1", 128);


    // 启动之后先清tsdb数据，因为时间戳不是事实的，防止写入失败
    tsdb->delete_all(tsdb);

    char kvdb_key[20] = "kvdbtest0";
    char kvdb_value[20] = "12345";
    char tsdb_data[128] = "hello world!"; 
    int64_t insert_start, select_start;
    int64_t insert_end, select_end;
    
	while (1)
	{

        // 在这里测试kvdb，写入1条数据，读取1条，并把错误计入nvs
        for (int i = 0; i < TEST_DATA_NUMS; i++)
        {
            snprintf(kvdb_key, 20, "kvdbtest%d", i);
            printf("key[%d]:%s\n", i, kvdb_key);
            insert_start = esp_timer_get_time();
            result = kvdb->insert_string(kvdb, (const char*)&kvdb_key[0], "12345");
            insert_end = esp_timer_get_time();

            if (result!=FDB_NO_ERR)
            {
                printf("kvdb insert error!!!\n");
                sta_data.write_error_counts++;
            }
            
            select_start = esp_timer_get_time();
            result = kvdb->select_string(kvdb, (const char*)kvdb_key, kvdb_value);
            select_end = esp_timer_get_time();
            printf("value[%d]:%s\n", i, kvdb_value);

            if (result!=FDB_NO_ERR)
            {
                printf("select error!!!\n");
                sta_data.read_error_counts++;
            }
            
        printf("kvdb insert cost %lld us \n", insert_end - insert_start);
        }

        printf("kvdb select cost %lld us \n", select_end - select_start);

        // 在这里测试tsdb，tsdb数据库默认开启滚动复写功能，把错误计入nvs

            
        insert_start = esp_timer_get_time();
        tsdb->insert_blob(tsdb, fdb_blob_make(&blob, &tsdb_data[0], sizeof(tsdb_data)));
        insert_end = esp_timer_get_time();
        printf("tsdb insert data: %s \n", tsdb_data);
        if (result!=FDB_NO_ERR)
        {
            printf("tsdb insert error!!!\n");
            sta_data.write_error_counts++;
        }
        printf("boot_ts:%ld \n", boot_ts);
        int tsdb_counts = tsdb->query_count(tsdb, boot_ts, time(NULL), FDB_TSL_WRITE);
        printf("tsdb counts:%d \n", tsdb_counts);

        

        
        printf("tsdb insert cost %lld us \n", insert_end - insert_start);

        tsdb->select_by_time(tsdb, boot_ts, time(NULL), query_by_time_cb, tsdb->_fdb_tsdb);
        
    
        // 在这里更新异常状态数据
        printf("=========================================\n");

        printf("reboot_counts     : %d\n", sta_data.reboot_counts);
        printf("write_error_counts: %d\n", sta_data.write_error_counts);
        printf("read_error_counts : %d\n", sta_data.read_error_counts);
        printf("full_counts       : %d\n", sta_data.full_counts);
        printf("check_error_counts: %d\n", sta_data.check_error_counts);

        printf("=========================================\n");
        nvs_ops->set_i32(nvs_ops, "reboot_counts", sta_data.reboot_counts);
        nvs_ops->set_i32(nvs_ops, "write_error_counts", sta_data.write_error_counts);
        nvs_ops->set_i32(nvs_ops, "read_error_counts", sta_data.read_error_counts);
        nvs_ops->set_i32(nvs_ops, "full_counts", sta_data.full_counts);
        int64_t _start = esp_timer_get_time();
 
        nvs_ops->set_i32(nvs_ops, "check_error_counts", sta_data.check_error_counts);
        nvs_ops->commit(nvs_ops);
        int64_t _end = esp_timer_get_time();
        printf("nvs set cost %lld us \n", _end - _start);

	}
    flashdb_ops->drop_kvdb(flashdb_ops, kvdb);
    flashdb_ops->drop_tsdb(flashdb_ops, tsdb);
}
