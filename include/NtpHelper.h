#ifndef ntphelper_h
#define ntphelper_h

#include <Arduino.h>

class NtpHelper
{
public:
    //Public Members


    //Constructors
    NtpHelper();

    //Public functions
    void ConfigNTP(String NTPserver1="pool.ntp.org", String NTPserver2="time.google.com", String NTPserver3="time.cloudflare.com");
    String GetTime();
    String GetDateTime();
    String GetDate();
    String GetTimeFormat(String format);

private:
    //private members
    
};

#endif