#include <Arduino.h>

//Use the following definitions:
//Built-in LED pin
//      #define BOARD_PIN_LED 2
//
//Default COM port speed
//      #define BOARD_COM_SPEED 115200


// Blinks the onboard LED, with precision parameters
void BlinkBoard(int blink_count=1, int blink_delay_ms=500);

//Blink board standard call
void BlinkBoardPrecise(int blink_count, int blink_interval_ms, int blink_ontime_ms);

//Blink board to tell the user OK
void BlinkBoardOK();

//Blink board to tell the user there is data transfer
void BlinkBoardData();

//Initializes the serial port
void InitSerial();

//Print to serial port
void PrintSerial(String text);

//Print to serial port and adds new line
void PrintlnSerial(String text);

//Splits a string into an array
String GetParam(String str, char separator, int index);

//Converts a hexadecimal character string to Integer
int HexStrToInt(char str[]);

//Converts a hexadecimal String to Integer
int HexStrToInt(String str);