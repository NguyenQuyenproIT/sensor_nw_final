#include "timer.h"
#include <time.h>

static bool timeReady = false;

void timeInit()
{
    // Việt Nam UTC+7
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 10000))
    {
        timeReady = true;
    }
    else
    {
        timeReady = false;
    }
}

bool getCurrentTimeString(String &timeStr, String &dateStr)
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        return false;
    }

    char timeBuf[16];
    char dateBuf[20];

    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
    strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y", &timeinfo);

    timeStr = String(timeBuf);
    dateStr = String(dateBuf);
    return true;
}

bool isTimeReady()
{
    return timeReady;
}