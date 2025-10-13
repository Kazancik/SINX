#include "rtc.h"

#include <iobyte.h>
#define HOUR_12_FLAG 0x02

char *longtostr(long zahl)
{
    static char text[20]; // Make me static, otherwise it's on the stack and will screw up soon, if it's static, it's allocated always, but is not safe for multi-tasking/threading.
    int loc = 19;
    text[19] = 0; // NULL terminate the string
    while (zahl)  // While we have something left, lets add a character to the string
    {
        --loc;
        text[loc] = (zahl % 10) + '0';
        zahl /= 10;
    }
    if (loc == 19) // Nothing, lets at least put a 0 in there!
    {
        --loc;
        text[loc] = '0';
    }
    return &text[loc]; // Start from where loc left off
}
unsigned char isBinary() {
    unsigned char statusB = readCmos(0x0B);
    return statusB & 0x04;
}
unsigned char bcdToBin(unsigned char val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}
unsigned char readCmosStable(unsigned char addr) {
    while (readCmos(0x0A) & 0x80); // wait for update-in-progress flag to clear
    return readCmos(addr);
}
unsigned char readCmos(unsigned char address)
{
    unsigned char data;
    outb(CMOS_ADDREG, address);
    data = inb(CMOS_DATAREG);
    return data;
}

void readTime(struct Time *t)
{
    unsigned char statusB = readCmos(0x0B);
    int isBinary = statusB & 0x04;
    int is24Hour = statusB & 0x02;

    unsigned char hr = readCmosStable(HourIndex);
    unsigned char min = readCmosStable(MinIndex);
    unsigned char sec = readCmosStable(SecIndex);

    // 12h → 24h
    if (!is24Hour) {
        int pm = hr & 0x80;
        hr &= 0x7F;
        if (pm) hr += 12;
        if (hr == 12 && !pm) hr = 0;
    }

    if (!isBinary) {
        hr  = bcdToBin(hr);
        min = bcdToBin(min);
        sec = bcdToBin(sec);
    }

    t->hr  = hr;
    t->min = min;
    t->sec = sec;
}


void readDate(struct Date *d)
{
    int binary = isBinary();
    unsigned char day   = readCmosStable(DayIndex);
    unsigned char month = readCmosStable(MonthIndex);
    unsigned char year  = readCmosStable(YearIndex);

    if (!binary) {
        day   = bcdToBin(day);
        month = bcdToBin(month);
        year  = bcdToBin(year);
    }

    d->day   = day;
    d->month = month;
    d->year  = 2000 + year; // CMOS returns 0–99
}
