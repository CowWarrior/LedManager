//+--------------------------------------------------------------------------
//
// File:        main.cpp
//
// Project:     LedDriver 
//
// Description: The purpose of this project is to provide a light and 
//              streamlined version of a Manager for the WS2812B LED Strip 
//              hosted on a ESP32 microcontroller development board.
//
// Credits:     This project is inspired from NightDriverStrip
//              https://github.com/PlummersSoftwareLLC/NightDriverStrip
//              https://www.youtube.com/watch?v=UZxY_BLSsGg
//
// History:     2023-10-28    PP Laplante   Created
//
//
//---------------------------------------------------------------------------

// Global Constants
#define LED_MATRIX_WIDTH        60
#define LED_MATRIX_HEIGHT       1
#define LED_NUM_LEDS            (MATRIX_WIDTH*MATRIX_HEIGHT)
#define LED_MATRIX_INTERLACED   0
#define LED_GPIO_PIN            13
#define BOARD_PIN_LED           2
#define WIFI_SSIS               "Maze"
#define WIFI_PWD                "MZZ8zbd9Vv8xtvrG9t38dKRX"
#define WIFIUTILS_SERVERPORT    80

//Libraries
#include <Arduino.h>
#include <Wifi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <arduinoutils.h>
#include <MiniServ.h>
#include <fastledutils.h>
#include <SPIFFS.h>

//Global Variables
MiniServ _server;

//Prototyopes
void HandleWebRequests();

void setup() {
  //initialize pins
  pinMode(BOARD_PIN_LED, OUTPUT);

  //initialize serial port
  InitSerial();

  //Initialize WiFi
  _server.InitWiFi(WIFI_SSIS, WIFI_PWD);

  //Initialize Web Server
  _server.InitWebServer();

  //Initialize LEDs
  InitLED();
  SetLEDCurrentEffect("Default"); 
    
  //blink twice to indicate we are ready
  BlinkBoard(2, 250);

  //Write to serial to indicate we are ready
  PrintlnSerial("Ready!");
}

void loop() {
  //Handle any web requests
  //HandleWebRequests();

  //Handle LED display
  DrawLEDFrame();
}

void HandleWebRequests()
{

  if(_server.IsClientRequest())
  {
    //indicate data received
    BlinkBoardData();

    //read headers
    String path = _server.GetRequestPath();
    path.toLowerCase();

    //DO STUFF HERE
    if (path == "/" || path.startsWith("/index.htm") || path.startsWith("/default.htm"))
    {      
      _server.PrintFileWebClientResponse("/index.htm");
    }
    else if (path.startsWith("/info"))
    {      
      String response = "<!doctype html><html><head><title>ESP32 Info</title></head><h1>ESP32 Info</h1><p><h2>Headers:</h2>" + _server.GetRequestHeaders() + "</p><p><h2>Hostname</h2>" + WiFi.getHostname() + "</p><p><h2>MAC</h2>" + WiFi.macAddress() + "</p></html>";
      _server.PrintWebClientResponse(response);
    }
    else if (path.startsWith("/effect/default"))
    {
      SetLEDCurrentEffect("Default");
      _server.PrintWebClientResponse("Effect set to: Default");
    }
    else if (path.startsWith("/effect/solid"))
    {
      String p = GetParam(path, '/', 3);
      p.toUpperCase();
      SetLEDCurrentEffect("Solid", p);
      _server.PrintWebClientResponse("Effect set to: Solid (" + GetLEDCurrentEffectParameters() + ")");
    }
    else if (path.startsWith("/effect/beat"))
    {
      SetLEDCurrentEffect("Beat");
      _server.PrintWebClientResponse("Effect set to: Beat");
    }
    else if (path.startsWith("/effect/link"))
    {
      SetLEDCurrentEffect("Link");
      _server.PrintWebClientResponse("Effect set to: Link");
    }
    else if (path.startsWith("/effect/rainbow"))
    {
      SetLEDCurrentEffect("Rainbow");
      _server.PrintWebClientResponse("Effect set to: Rainbow");
    }
    else if (path.startsWith("/effect/image"))
    {
      String p = _server.GetRequestHeaders();
      _server.PrintWebClientResponse("Image:" + p);
    }    
    else if (path.startsWith("/brightness/"))
    {
      String p = GetParam(path, '/', 2);
      SetLEDBrightness(HexStrToInt(p));
      _server.PrintWebClientResponse("Brightness set to: " + String(GetLEDBrightness(), HEX));
    }
    else
      _server.PrintFileWebClientNotFound("/err404.htm");

    //close connection
    _server.SendClientResponse();
  }

}

