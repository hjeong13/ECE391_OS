/* rtc file holds all functions corresponding to the rtc */

#ifndef _RTC_H
#define _RTC_H

#include "lib.h"

void rtc_init(void);

void rtc_handler(void);

void set_frequency(int32_t frequency);

int32_t check_power(int32_t frequency);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

#endif /* _RTC_H */

