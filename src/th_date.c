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
    date.year = (unsigned int)tm.tm_year;
    date.month = (unsigned int)tm.tm_mon;
    date.day = (unsigned int)tm.tm_mday;
    date.weekday = (unsigned int)tm.tm_wday;
    date.hour = (unsigned int)tm.tm_hour;
    date.minute = (unsigned int)tm.tm_min;
    date.second = (unsigned int)tm.tm_sec;
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
    new_date.year = (unsigned int)tm.tm_year;
    new_date.month = (unsigned int)tm.tm_mon;
    new_date.day = (unsigned int)tm.tm_mday;
    new_date.weekday = (unsigned int)tm.tm_wday;
    new_date.hour = (unsigned int)tm.tm_hour;
    new_date.minute = (unsigned int)tm.tm_min;
    new_date.second = (unsigned int)tm.tm_sec;
    return new_date;
}
