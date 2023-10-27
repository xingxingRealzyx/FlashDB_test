idf_component_register(SRCS "main.c"
                            "nvs_ops.c"
                            "flashdb_c_api.c"
                    INCLUDE_DIRS ""
                    REQUIRES  "FlashDB_port" "spi_flash" "nvs_flash")