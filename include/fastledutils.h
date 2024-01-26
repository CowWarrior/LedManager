#ifndef fastledutils_h
#define fastledutils_h

#include <Arduino.h>
#include <FastLED.h>

//uncomment to enable debug mode
#define FASTLEDUTILS_DEBUGMODE  1

//constants
#ifndef FLU_GPIO_PIN
#define FLU_GPIO_PIN 13
#endif

#ifndef FLU_MAX_LEDS
#define FLU_MAX_LEDS 6000
#endif

class FastLEDUtils
{
public:
    //Public Members


    //Constructors
    FastLEDUtils();

    //Public functions
    //Initialize LED display
    void InitLED(int width, int height, bool interlaced=false);

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

    //identifies if it is a matrix
    bool IsMatrix();

private:
    //private members
    int _MATRIX_WIDTH = 60;
    int _MATRIX_HEIGHT = 1;
    int _NUM_LEDS = 60;                             //_MATRIX_WIDTH * _MATRIX_HEIGHT
    int _PX_PER_METER = 60;                         //pixel density
    bool _MATRIX_INTERLACED = false;                //is each row starting in different direction

    CRGB _leds[FLU_MAX_LEDS];                        //Main LED array (max 100m)
    long _ledFramerate = 100;                        //current framerate in milliseconds
    int _ledFrameIndex = 0;                          //Current frame index
    int _ledBrightness = 64;                         //Current LED brightness
    String _ledCurrentEffect = "DEFAULT";            //currently displayed effect
    String _ledCurrentEffectParameters = "";         //current effect params
    unsigned long _ledCurrentTime = millis();        // Current time
    unsigned long _ledPreviousTime = 0;              // Previous time

    //private functions
    void DrawLEDCurrentEffectFrame();
    void DrawLEDBeatEffect();
    void DrawLEDRainbowEffect();
    void DrawLEDSolidEffect();
    void DrawLEDImageEffect();
    void DrawLEDPatternEffect();
    void set_rainbow_pixel(int angle, int pixel);
};

#endif