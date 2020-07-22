#include <dev/cmos.h>
#include <arch/common.h>

uint8_t cmos_read(uint8_t index)
{
	outb(0x70, index);
	return inb(0x71);
}

void cmos_write(uint8_t index, uint8_t data)
{
	outb(0x70, index);
	outb(0x71, data);
}

uint8_t cmos_update_in_progress()
{
	return cmos_read(0x0A) & 0x80;
}

void cmos_get_time(struct cmos_rtc* data)
{
	uint8_t lsecond;
	uint8_t lminute;
	uint8_t lhour;
	uint8_t lday;
	uint8_t lmonth;
	uint8_t lyear;
	uint8_t flags;

	// Wait until the clock is not updating. (Every second or so).
	while(cmos_update_in_progress());

	// Read its values.
	data->second = cmos_read(0x00);
	data->minute = cmos_read(0x02);
	data->hour = cmos_read(0x04);
	data->day = cmos_read(0x07);
	data->month = cmos_read(0x08);
	data->year = cmos_read(0x09);

	do 
	{
		lsecond = data->second;
		lminute = data->minute;
		lhour = data->hour;
		lday = data->day;
		lmonth = data->month;
		lyear = data->year;

		// Wait until the clock is not updating.
		while(cmos_update_in_progress());

		// Repeat the read until two consecutive reads are the same.
		data->second = cmos_read(0x00);
		data->minute = cmos_read(0x02);
		data->hour = cmos_read(0x04);
		data->day = cmos_read(0x07);
		data->month = cmos_read(0x08);
		data->year = cmos_read(0x09);

	}
	while((lsecond != data->second) || (lminute != data->minute) || (lhour != data->hour) || (lday != data->day) || (lmonth != data->month) || (lyear != data->year));

	// Grab the CMOS flags.
	flags = cmos_read(0x0B);

	// Convert BCD to binary.
	if(!(flags & 0x04))
	{
		data->second = (data->second & 0x0F) + ((data->second / 16) * 10);
		data->minute = (data->minute & 0x0F) + ((data->minute / 16) * 10);
		data->hour = ((data->hour & 0x0F) + (((data->hour & 0x70) / 16) * 10)) | (data->hour & 0x80);
		data->day = (data->day & 0x0F) + ((data->day / 16) * 10);
		data->month = (data->month & 0x0F) + ((data->month / 16) * 10);
		data->year = (data->year & 0x0F) + ((data->year / 16) * 10);
	}

	// Convert to 24 hour format.
	if(!(flags & 0x02) && (data->hour & 0x80))
	{
		data->hour = ((data->hour & 0x7F) + 12) % 24;
	}

	data->year += (CURRENT_YEAR / 100) * 100;
	
	if(data->year < CURRENT_YEAR)
		data->year += 100;
}
