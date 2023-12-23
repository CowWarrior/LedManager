//+--------------------------------------------------------------------------
//
// File:        fastledutils.cpp
//
// Description: The purpose of this file is to provide utility functions
//              for the FastLED library.
//
// History:     2023-10-28    PP Laplante   Created
//
//
//---------------------------------------------------------------------------
#include <Arduino.h>
#include <arduinoutils.h>
#include <FastLED.h>
#include <fastledutils.h>

//uncomment to enable debug mode
#define FASTLEDUTILS_DEBUGMODE  1

#ifndef LED_MATRIX_WIDTH
#define LED_MATRIX_WIDTH        16
#endif

#ifndef LED_MATRIX_HEIGHT
#define LED_MATRIX_HEIGHT       16
#endif

#ifndef LED_NUM_LEDS
#define LED_NUM_LEDS            (LED_MATRIX_WIDTH*LED_MATRIX_HEIGHT)
#endif

#ifndef LED_MATRIX_INTERLACED
#define LED_MATRIX_INTERLACED   0
#endif

#ifndef LED_GPIO_PIN
#define LED_GPIO_PIN            13
#endif

#ifndef LED_PX_PER_METER
#define LED_PX_PER_METER        60
#endif

//Global variables
CRGB leds[LED_NUM_LEDS];                        //Main LED array
long ledFramerate = 100;                        //current framerate in milliseconds
int ledFrameIndex = 0;                          //Current frame index
int ledBrightness = 64;                         //Current LED brightness
String ledCurrentEffect = "DEFAULT";            //currently displayed effect
String ledCurrentEffectParameters = "";         //current effect params
unsigned long ledCurrentTime = millis();        // Current time
unsigned long ledPreviousTime = 0;              // Previous time

//Local Prototypes
void DrawLEDCurrentEffectFrame();
void DrawLEDBeatEffect();
void DrawLEDRainbowEffect();
void DrawLEDSolidEffect();
void DrawLEDImageEffect();

//Initialize LED display
void InitLED()
{
    //Initialize default values
    SetLEDCurrentEffect("DEFAULT");
    SetLEDTravelSpeed(1.0f);

    //intialize FastLED
    FastLED.addLeds<WS2812, LED_GPIO_PIN, GRB>(leds, LED_NUM_LEDS);

    //set initial brightness
    SetLEDBrightness(ledBrightness);

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.println("FastLED initialized...");
            Serial.print("FastLED GPIO pin set to: ");
            Serial.println(LED_GPIO_PIN);
            Serial.print("Brightness set to: ");
            Serial.println(ledBrightness);
        }
    #endif
}

//Sets which effect should be displayed, optionally choosing parameters
void SetLEDCurrentEffect(String effect, String parameters)
{
    ledCurrentEffect = effect;
    ledCurrentEffect.trim();
    ledCurrentEffect.toUpperCase();
    ledCurrentEffectParameters = parameters;
    ledFrameIndex = 0;
    FastLED.clear();
    FastLED.show();

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.print("Current effect set to: ");
            Serial.println(ledCurrentEffect);
            Serial.print("Current effect parameters: ");
            Serial.println(ledCurrentEffectParameters);
        }
    #endif
}

//Gets which effect is currently displayed
String GetLEDCurrentEffect()
{
    return ledCurrentEffect;    
}

//Gets parameter for current effect displayed
String GetLEDCurrentEffectParameters()
{
    return ledCurrentEffectParameters;    
}

//Sets the speed which leds should travel in meters per second
void SetLEDTravelSpeed(float speed_m_per_s)
{
    ledFramerate = (int) 1000 / speed_m_per_s / LED_PX_PER_METER;

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.print("Travel speed set to: ");
            Serial.print(speed_m_per_s);
            Serial.print("m/s , Framerate: ");
            Serial.print(ledFramerate);
            Serial.println("ms.");
        }
    #endif
}

//Gets the speed which leds should travel in meters per second
float GetLEDTravelSpeed()
{
    return LED_PX_PER_METER / (1000 / ledFramerate);
}

//Gets the speed which leds refresh in milliseconds
long GetLEDFramerate()
{
    return ledFramerate;
}

//Sets the relative brightness of the LED strip (0-255)
void SetLEDBrightness(int brightness)
{
    //set default brightness
    ledBrightness = brightness;

    //clamp
    if (ledBrightness < 0)
        ledBrightness = 0;
    else if (ledBrightness > 255)
        ledBrightness = 255;

    //apply it
    FastLED.setBrightness(ledBrightness);
    FastLED.show();

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.print("Brightness set to: ");
            Serial.println(ledBrightness);
        }
    #endif
}

//Gets the relative brightness of the LED strip (0-255)
int GetLEDBrightness()
{
    return ledBrightness;
}


//Draws the LED effect current frame - to be added to the main loop
void DrawLEDFrame()
{
    //update current time
    ledCurrentTime = millis();

    //check if it is time to draw
    if (ledCurrentTime > ledFramerate + ledPreviousTime)
    {
        DrawLEDCurrentEffectFrame();

        //update previous time the frame was drawn
        ledPreviousTime = ledCurrentTime;
    }
}

//Draws the next frame for the effect
void DrawLEDCurrentEffectFrame()
{
    //choose the right effect
    if (ledCurrentEffect == "RAINBOW")
        DrawLEDRainbowEffect();
    else if (ledCurrentEffect == "SOLID")
        DrawLEDSolidEffect();
    else if (ledCurrentEffect == "IMAGE")
        DrawLEDImageEffect();
    else
        DrawLEDBeatEffect(); //Default to beat effect

}

// BEAT EFFECT
void DrawLEDBeatEffect()
{
    static bool isReverse = false;

    if (!isReverse)
    {
        //FORWARD       
        
        //light new led
        leds[ledFrameIndex] = CRGB(0, 0, 64);

        //cehck if we reached the end
        if (ledFrameIndex >= LED_NUM_LEDS-1)
        {
            //SwitchDirection
            isReverse = true;
            ledFrameIndex = LED_NUM_LEDS - 1;
        }
        else
        {
            //go on
            ledFrameIndex++;
        }
    }
    else
    {
        //BACKWARDS

        //turn-off led
        leds[ledFrameIndex] = CRGB::Black;

        //cehck if we reached the start
        if (ledFrameIndex <= 0)
        {
            //SwitchDirection
            isReverse = false;
            ledFrameIndex = 0;
        }
        else
        {
            //go on
            ledFrameIndex--;
        }
    }

    //update strip
    FastLED.show();
}


void set_rainbow_pixel(int angle, int pixel) {
  //original code: Ontaelio
  //https://www.instructables.com/How-to-Make-Proper-Rainbow-and-Random-Colors-With-/

  byte red, green, blue;

  if (angle<60) {red = 255; green = round(angle*4.25-0.01); blue = 0;} else
  if (angle<120) {red = round((120-angle)*4.25-0.01); green = 255; blue = 0;} else 
  if (angle<180) {red = 0, green = 255; blue = round((angle-120)*4.25-0.01);} else 
  if (angle<240) {red = 0, green = round((240-angle)*4.25-0.01); blue = 255;} else 
  if (angle<300) {red = round((angle-240)*4.25-0.01), green = 0; blue = 255;} else 
                {red = 255, green = 0; blue = round((360-angle)*4.25-0.01);}
    
  leds[pixel] = CRGB( red, green, blue);
}

void DrawLEDRainbowEffect()
{
    static bool isReverse = false;
    static int currAngle = 0;       //TODO: use frameindex instead of a static!

    //set all LEDS
    for (int i = 0; i <= LED_NUM_LEDS-1; i++) {
      //light new pixel
      set_rainbow_pixel(currAngle, i);
    }
    
    //display on LED strip
    FastLED.show();

    if (isReverse)
    {
        //BACKWARDS
        currAngle--;
        
        //check if we need to switch directionLED_NUM_LEDS
        if (currAngle <= 0)
            isReverse = false;
    }
    else
    {
        //FORWARDS
        currAngle++;

        //check if we need to switch directionLED_NUM_LEDS
        if (currAngle >= 360)
            isReverse = true;
    }

}

void DrawLEDSolidEffect()
{
    //only need to do this once really
    if (ledFrameIndex == 0)
    {
        // uint32_t targetColor = ledCurrentEffectParameters.toInt();

        // //Convert HEX parameter to INT
        // char c[7] = "000000";
        // ledCurrentEffectParameters.toCharArray(c, 7);
        // targetColor = HexStrToInt(c);

        uint32_t targetColor = HexStrToInt(ledCurrentEffectParameters);

        #ifdef FASTLEDUTILS_DEBUGMODE
            if (Serial)
            {
                Serial.print("Color set to: ");
                Serial.println(targetColor, HEX);
            }
        #endif

        //set all LEDS
        for (int i = 0; i <= LED_NUM_LEDS-1; i++) {
            //light new pixel
            leds[i] = CRGB(targetColor);
        }

        //update strip
        FastLED.show();

        //change frame
        ledFrameIndex = 1;
    }
}

void DrawLEDImageEffect()
{
    //only need to do this once really
    if (ledFrameIndex == 0)
    {
        //LINK Frame 1
        int frames[1][16][16];

        //load data into matrix
        
        #ifdef FASTLEDUTILS_DEBUGMODE
            if (Serial)
            {
                Serial.println("Load image data...");
            }
        #endif

        int kk=0;
        for (int i = 0; i <= 15; i++) {
            for (int j = 0; j <= 15; j++) {
                //set matrix color value
                String hexVal = ledCurrentEffectParameters.substring(kk, kk+6);
                int intVal = HexStrToInt(hexVal);
                frames[0][i][j] = intVal;

                //too much details
                /*
                #ifdef FASTLEDUTILS_DEBUGMODE
                    if (Serial)
                    {
                        Serial.print("HEX:");
                        Serial.print(hexVal);
                        Serial.print(" INT:");
                        Serial.println(intVal);
                    }
                #endif
                */

                //each color value is six bytes (ex: 0xBBEEFF)
                kk += 6;
            }
        }

        int k=0;

        //set matrix
        //  right to left (interlaced: meaning next row is left to right, and so on...)
        //  top to bottom
        for (int i = 0; i <= 15; i++) {
            for (int j = 0; j <= 15; j++) {
                
                if (i == 0 || i % 2 == 0)
                    leds[k] = CRGB(frames[0][i][15-j]);
                else
                    leds[k] = CRGB(frames[0][i][j]);

                k++;
            }
        }

        //try to tame brightness
        SetLEDBrightness(16);

        //update strip
        FastLED.show();

        //change frame
        ledFrameIndex = 1;
    }
}
