#ifndef CMOS_H
#define CMOS_H

#include <libk/types.h>

#define CURRENT_YEAR 2020

struct cmos_rtc
{
	// Century is unused until we make sure it exists by polling the ACPI.
	uint8_t century;
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

uint8_t cmos_read(uint8_t index);
void cmos_write(uint8_t index, uint8_t data);
uint8_t cmos_update_in_progress();
void cmos_get_time(struct cmos_rtc*);

#endif
