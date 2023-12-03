#ifndef fastledutils_h
#define fastledutils_h

#include <Arduino.h>

//Use the following definitions:
//LED Matrix width - width of your strip(s)
//      #define LED_MATRIX_WIDTH        60
//
//LED Matrix Height - set to 1 if it's a single strip
//      #define LED_MATRIX_HEIGHT       1
//
//Total number of LEDs - should be calculated unless this is an exception case
//      #define LED_NUM_LEDS            (LED_MATRIX_WIDTH*LED_MATRIX_HEIGHT)
//
//Is LED Matrix interlaced? - Set to 1 the first row goes one way and the next the other way
//      #define LED_MATRIX_INTERLACED   0
//
//LED GPIO pin to use for data
//      #define LED_GPIO_PIN            13
//
//LED Strip resolution in Pixels per Meter - how  many LEDs you have per meter - essential for speed calculation
//      #define LED_PX_PER_METER        60
//



//Initialize LED display
void InitLED();

//Sets which effect should be displayed, optionally choosing parameters
void SetLEDCurrentEffect(String effect, String parameters="");

//Gets which effect is currently displayed
String GetLEDCurrentEffect();

//Gets parameter for current effect displayed
String GetLEDCurrentEffectParameters();

//Sets the speed which leds should travel in meters per second
void SetLEDTravelSpeed(float speed_m_per_s);

//Gets the speed which leds should travel in meters per second
float GetLEDTravelSpeed();

//Gets the speed which leds refresh in milliseconds
long GetLEDFramerate();

//Sets the relative brightness of the LED strip (0-255)
void SetLEDBrightness(int brightness);

//Gets the relative brightness of the LED strip (0-255)
int GetLEDBrightness();

//Draws the LED effect current frame - to be added to the main loop
void DrawLEDFrame();

#endif