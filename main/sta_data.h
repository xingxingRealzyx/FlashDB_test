#ifndef __STA_DATA_H__
#define __STA_DATA_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct sta_data_t
{
    int reboot_counts;
    int write_error_counts;
    int read_error_counts;
    int full_counts;
    int check_error_counts;
};



#endif /* __STA_DATA_H__ */