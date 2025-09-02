#include <th.h>

#include <time.h>

#include "th_config.h"
#include "th_string.h"

TH_PUBLIC(th_duration)
th_seconds(int seconds)
{
    return (th_duration){.seconds = seconds};
}

TH_PUBLIC(th_duration)
th_minutes(int minutes)
{
    return th_seconds(minutes * 60);
}

TH_PUBLIC(th_duration)
th_hours(int hours)
{
    return th_minutes(hours * 60);
}

TH_PUBLIC(th_duration)
th_days(int days)
{
    return th_hours(days * 24);
}

TH_PUBLIC(th_date)
th_date_now(void)
{
    time_t t = time(NULL);
    struct tm tm = {0};
    gmtime_r(&t, &tm);
    th_date date = {0};
    date.year = (unsigned int)tm.tm_year & 0xFFFF;
    date.month = (unsigned int)tm.tm_mon & 0xFF;
    date.day = (unsigned int)tm.tm_mday & 0xFF;
    date.weekday = (unsigned int)tm.tm_wday & 0xFF;
    date.hour = (unsigned int)tm.tm_hour & 0xFF;
    date.minute = (unsigned int)tm.tm_min & 0xFF;
    date.second = (unsigned int)tm.tm_sec & 0xFF;
    return date;
}

TH_PUBLIC(th_date)
th_date_add(th_date date, th_duration d)
{
    struct tm tm = {0};
    tm.tm_year = date.year;
    tm.tm_mon = date.month;
    tm.tm_mday = date.day;
    tm.tm_hour = date.hour;
    tm.tm_min = date.minute;
    tm.tm_sec = date.second;
    time_t t = mktime(&tm);
    t += d.seconds;
    gmtime_r(&t, &tm);
    th_date new_date = {0};
    new_date.year = (unsigned int)tm.tm_year & 0xFFFF;
    new_date.month = (unsigned int)tm.tm_mon & 0xFF;
    new_date.day = (unsigned int)tm.tm_mday & 0xFF;
    new_date.weekday = (unsigned int)tm.tm_wday & 0xFF;
    new_date.hour = (unsigned int)tm.tm_hour & 0xFF;
    new_date.minute = (unsigned int)tm.tm_min & 0xFF;
    new_date.second = (unsigned int)tm.tm_sec & 0xFF;
    return new_date;
}
