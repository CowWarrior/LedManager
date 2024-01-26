//+--------------------------------------------------------------------------
//
// File:        fastledutils.cpp
//
// Description: The purpose of this file is to provide utility functions
//              for the FastLED library.
//
// History:     2023-10-28      PP Laplante     Created
//              2024-01-20      PP Laplante     Converted library to Class
//
//---------------------------------------------------------------------------
#include <Arduino.h>
#include <arduinoutils.h>
#include <FastLED.h>
#include <fastledutils.h>

//Global variables


//Default constructor
FastLEDUtils::FastLEDUtils()
{
    //nothing to do 
}

//Initialize LED display
void FastLEDUtils::InitLED(int width, int height, bool interlaced)
{
    //Initialize 'constants'
    _MATRIX_WIDTH = width;
    _MATRIX_HEIGHT = height;
    _MATRIX_INTERLACED = interlaced;
    _NUM_LEDS = _MATRIX_WIDTH * _MATRIX_HEIGHT;

    //Initialize default values
    SetLEDCurrentEffect("DEFAULT");
    SetLEDTravelSpeed(1.0f);

    //intialize FastLED
    //FastLED.addLeds<WS2812, _GPIO_PIN, GRB>(_leds, _NUM_LEDS);
    FastLED.addLeds<WS2812, FLU_GPIO_PIN, GRB>(_leds, _NUM_LEDS);

    //set initial brightness
    SetLEDBrightness(_ledBrightness);

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.println("FastLED initialized...");
            Serial.print("FastLED GPIO pin set to: ");
            Serial.println(FLU_GPIO_PIN);
            Serial.print("Brightness set to: ");
            Serial.println(_ledBrightness);
        }
    #endif
}

//Sets which effect should be displayed, optionally choosing parameters
void FastLEDUtils::SetLEDCurrentEffect(String effect, String parameters)
{
    _ledCurrentEffect = effect;
    _ledCurrentEffect.trim();
    _ledCurrentEffect.toUpperCase();
    _ledCurrentEffectParameters = parameters;
    _ledFrameIndex = 0;
    FastLED.clear();
    FastLED.show();

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.print("Current effect set to: ");
            Serial.println(_ledCurrentEffect);
            Serial.print("Current effect parameters: ");
            Serial.println(_ledCurrentEffectParameters);
        }
    #endif
}

//Gets which effect is currently displayed
String FastLEDUtils::GetLEDCurrentEffect()
{
    return _ledCurrentEffect;    
}

//Gets parameter for current effect displayed
String FastLEDUtils::GetLEDCurrentEffectParameters()
{
    return _ledCurrentEffectParameters;    
}

//Sets the speed which leds should travel in meters per second
void FastLEDUtils::SetLEDTravelSpeed(float speed_m_per_s)
{
    _ledFramerate = (int) 1000 / speed_m_per_s / _PX_PER_METER;

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.print("Travel speed set to: ");
            Serial.print(speed_m_per_s);
            Serial.print("m/s , Framerate: ");
            Serial.print(_ledFramerate);
            Serial.println("ms.");
        }
    #endif
}

//Gets the speed which leds should travel in meters per second
float FastLEDUtils::GetLEDTravelSpeed()
{
    return _PX_PER_METER / (1000 / _ledFramerate);
}

//Gets the speed which leds refresh in milliseconds
long FastLEDUtils::GetLEDFramerate()
{
    return _ledFramerate;
}

//Sets the relative brightness of the LED strip (0-255)
void FastLEDUtils::SetLEDBrightness(int brightness)
{
    //set default brightness
    _ledBrightness = brightness;

    //clamp
    if (_ledBrightness < 0)
        _ledBrightness = 0;
    else if (_ledBrightness > 255)
        _ledBrightness = 255;

    //apply it
    FastLED.setBrightness(_ledBrightness);
    FastLED.show();

    #ifdef FASTLEDUTILS_DEBUGMODE
        if (Serial)
        {
            Serial.print("Brightness set to: ");
            Serial.println(_ledBrightness);
        }
    #endif
}

//Gets the relative brightness of the LED strip (0-255)
int FastLEDUtils::GetLEDBrightness()
{
    return _ledBrightness;
}


//Draws the LED effect current frame - to be added to the main loop
void FastLEDUtils::DrawLEDFrame()
{
    //update current time
    _ledCurrentTime = millis();

    //check if it is time to draw
    if (_ledCurrentTime > _ledFramerate + _ledPreviousTime)
    {
        DrawLEDCurrentEffectFrame();

        //update previous time the frame was drawn
        _ledPreviousTime = _ledCurrentTime;
    }
}

//Draws the next frame for the effect
void FastLEDUtils::DrawLEDCurrentEffectFrame()
{
    //choose the right effect
    if (_ledCurrentEffect == "RAINBOW")
        DrawLEDRainbowEffect();
    else if (_ledCurrentEffect == "SOLID")
        DrawLEDSolidEffect();
    else if (_ledCurrentEffect == "IMAGE")
        DrawLEDImageEffect();
    else if (_ledCurrentEffect == "PATTERN")
        DrawLEDPatternEffect();
    else
        DrawLEDBeatEffect(); //Default to beat effect

}

// BEAT EFFECT
void FastLEDUtils::DrawLEDBeatEffect()
{
    static bool isReverse = false;

    //set default beat color
    CRGB targetColor = CRGB(0, 0, 64);

    //override default color if specified
    if (_ledCurrentEffectParameters != "")
        targetColor = CRGB(HexStrToInt(_ledCurrentEffectParameters));

    if (!isReverse)
    {
        //FORWARD       
        
        //light new led
        _leds[_ledFrameIndex] = targetColor;

        //cehck if we reached the end
        if (_ledFrameIndex >= _NUM_LEDS-1)
        {
            //SwitchDirection
            isReverse = true;
            _ledFrameIndex = _NUM_LEDS - 1;
        }
        else
        {
            //go on
            _ledFrameIndex++;
        }
    }
    else
    {
        //BACKWARDS

        //turn-off led
        _leds[_ledFrameIndex] = CRGB::Black;

        //cehck if we reached the start
        if (_ledFrameIndex <= 0)
        {
            //SwitchDirection
            isReverse = false;
            _ledFrameIndex = 0;
        }
        else
        {
            //go on
            _ledFrameIndex--;
        }
    }

    //update strip
    FastLED.show();
}


void FastLEDUtils::set_rainbow_pixel(int angle, int pixel) {
  //original code: Ontaelio
  //https://www.instructables.com/How-to-Make-Proper-Rainbow-and-Random-Colors-With-/

  byte red, green, blue;

  if (angle<60) {red = 255; green = round(angle*4.25-0.01); blue = 0;} else
  if (angle<120) {red = round((120-angle)*4.25-0.01); green = 255; blue = 0;} else 
  if (angle<180) {red = 0, green = 255; blue = round((angle-120)*4.25-0.01);} else 
  if (angle<240) {red = 0, green = round((240-angle)*4.25-0.01); blue = 255;} else 
  if (angle<300) {red = round((angle-240)*4.25-0.01), green = 0; blue = 255;} else 
                {red = 255, green = 0; blue = round((360-angle)*4.25-0.01);}
    
  _leds[pixel] = CRGB( red, green, blue);
}

void FastLEDUtils::DrawLEDRainbowEffect()
{
    static bool isReverse = false;
    static int currAngle = 0;       //TODO: use frameindex instead of a static!

    //set all LEDS
    for (int i = 0; i <= _NUM_LEDS-1; i++) {
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

void FastLEDUtils::DrawLEDSolidEffect()
{
    //only need to do this once really
    if (_ledFrameIndex == 0)
    {
        // uint32_t targetColor = ledCurrentEffectParameters.toInt();

        // //Convert HEX parameter to INT
        // char c[7] = "000000";
        // ledCurrentEffectParameters.toCharArray(c, 7);
        // targetColor = HexStrToInt(c);

        uint32_t targetColor = HexStrToInt(_ledCurrentEffectParameters);

        #ifdef FASTLEDUTILS_DEBUGMODE
            if (Serial)
            {
                Serial.print("Color set to: ");
                Serial.println(targetColor, HEX);
            }
        #endif

        //set all LEDS
        for (int i = 0; i <= _NUM_LEDS-1; i++) {
            //light new pixel
            _leds[i] = CRGB(targetColor);
        }

        //update strip
        FastLED.show();

        //change frame
        _ledFrameIndex = 1;
    }
}

void FastLEDUtils::DrawLEDPatternEffect()
{
    //only need to do this once really
    if (_ledFrameIndex == 0)
    {
        int patternLength = _ledCurrentEffectParameters.length() / 6;
        uint32_t patternArray[patternLength];

        //extract pattern from param
        for (int i=0; i<patternLength; i++)
        {
            int start = i*6;
            int end = start+6;
            String patternPixel = _ledCurrentEffectParameters.substring(start, end);
            uint32_t pixelColor = HexStrToInt(patternPixel);
            patternArray[i] = pixelColor;
        }

        #ifdef FASTLEDUTILS_DEBUGMODE
            if (Serial)
            {
                Serial.print("Pattern \"");
                Serial.print(_ledCurrentEffectParameters);
                Serial.println("\" converted to:");
                for (int i=0; i<patternLength; i++)
                {
                    Serial.print(patternArray[i], HEX);
                    Serial.print(",");
                }
                Serial.println("");
            }
        #endif

        //clear strip
        FastLED.clear();

        //set all LEDS
        for (int i = 0; i <= _NUM_LEDS-1; i+=patternLength) 
        {
            //light segement
            for (int j=0; j<patternLength; j++)
            {
                _leds[i+j] = CRGB(patternArray[j]);
            }
            
        }

        //update strip
        FastLED.show();

        //change frame
        _ledFrameIndex = 1;
    }
}

void FastLEDUtils::DrawLEDImageEffect()
{
    //only need to do this once really
    if (_ledFrameIndex == 0)
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
                String hexVal = _ledCurrentEffectParameters.substring(kk, kk+6);
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
                    _leds[k] = CRGB(frames[0][i][15-j]);
                else
                    _leds[k] = CRGB(frames[0][i][j]);

                k++;
            }
        }

        //try to tame brightness
        SetLEDBrightness(16);

        //update strip
        FastLED.show();

        //change frame
        _ledFrameIndex = 1;
    }
}
 