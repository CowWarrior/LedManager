//+--------------------------------------------------------------------------
//
// File:        arduinoutils.cpp
//
// Description: The purpose of this file is to provide utility functions
//              for arduino projects.
//
// History:     2023-10-28      PP Laplante     Created
//              2023-10-29      PP Laplante     Made more generic and 
//                                              renamed to arduinoutils
//
//
//---------------------------------------------------------------------------
#include <Arduino.h>

#ifndef BOARD_PIN_LED
#define BOARD_PIN_LED 2
#endif

#ifndef BOARD_COM_SPEED
#define BOARD_COM_SPEED 115200
#endif

// Blinks the onboard LED, with precision parameters
void BlinkBoardPrecise(int blink_count, int blink_interval_ms, int blink_ontime_ms)
{
    for (int i=0; i<blink_count; i++)
    {
        digitalWrite(BOARD_PIN_LED, HIGH);
        delay(blink_interval_ms);
        digitalWrite(BOARD_PIN_LED, LOW);
        delay(blink_interval_ms);
    }
}

//Blink board standard call
void BlinkBoard(int blink_count=1, int blink_interval_ms=500)
{
    BlinkBoardPrecise(blink_count, blink_interval_ms, blink_interval_ms);
}

//Blink board to tell the user OK
void BlinkBoardOK()
{
    BlinkBoard(2, 500);
}

//Blink board to tell the user there is data transfer
void BlinkBoardData()
{
    BlinkBoardPrecise(3, 50, 50);
}

//Initializes the serial port
void InitSerial()
{
    if (!Serial)
        Serial.begin(BOARD_COM_SPEED);
}

//Print to serial port
void PrintSerial(String text)
{
    if (Serial)
        Serial.print(text);
}

//Print to serial port and adds new line
void PrintlnSerial(String text)
{
    if (Serial)
        Serial.println(text);
}

//Splits a string into an array
String GetParam(String str, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = str.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) 
    {
        if (str.charAt(i) == separator || i == maxIndex) 
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? str.substring(strIndex[0], strIndex[1]) : "";
}

//Converts a hexadecimal character string to Integer
int HexStrToInt(char str[])
{
    return (int) strtol(str, 0, 16);
}

//Converts a hexadecimal String to Integer
int HexStrToInt(String str)
{
    char c[7] = "000000";
    str.toCharArray(c, 7);

    return (int) HexStrToInt(c);
}