
#include <NtpHelper.h>
#include <time.h>
#include <Arduino.h>

//Constructor
NtpHelper::NtpHelper()
{
  
}

void NtpHelper::ConfigNTP(String NTPserver1, String NTPserver2, String NTPserver3)
{

    configTime(-5 * 3600, 3600, "time.nrc.ca", "time1.google.com", "pool.ntp.org");
  
}

String GetFormattedTime(const char *format)
{
    struct tm theTime;
    if (getLocalTime(&theTime))
    {
        char timeoutput[128];
        strftime(timeoutput, 128, format, &theTime);
        
        return timeoutput;
    }
    else
      return "";
}

String NtpHelper::GetTime()
{
    return GetFormattedTime("%H:%M:%S");
}

String NtpHelper::GetDateTime()
{
    return GetFormattedTime("%Y-%m-%d %H:%M:%S");
}

String NtpHelper::GetDate()
{
    return GetFormattedTime("%Y-%m-%d");
}