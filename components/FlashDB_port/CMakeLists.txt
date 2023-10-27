set (SOURCES)

list (APPEND SOURCES "src/fal_flash_esp32_port.c")

list (APPEND SOURCES "../../FlashDB/src/fdb.c" "../../FlashDB/src/fdb_file.c" "../../FlashDB/src/fdb_kvdb.c" "../../FlashDB/src/fdb_tsdb.c" "../../FlashDB/src/fdb_utils.c")

list (APPEND SOURCES "../../FlashDB/port/fal/src/fal_flash.c" "../../FlashDB/port/fal/src/fal_partition.c" "../../FlashDB/port/fal/src/fal_rtt.c" "../../FlashDB/port/fal/src/fal.c")

idf_component_register(SRCS "${SOURCES}"
                    INCLUDE_DIRS "include" "../../FlashDB/port/fal/inc" "../../FlashDB/inc"
                    REQUIRES "spi_flash")
