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
//              2023-11-13    PP Laplante   Started refoactoring to use new MiniServ
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
void HandleNotFound();
void HandleMainPage();
void RedirectMainPage();
void HandleInfo();

void setup() {
  //initialize pins
  pinMode(BOARD_PIN_LED, OUTPUT);

  //initialize serial port
  InitSerial();

  //Initialize WiFi
  _server.InitWiFi(WIFI_SSIS, WIFI_PWD);

  //Initialize Web Server
  _server.WServer.on("/", HandleMainPage);
  _server.WServer.on("/default.htm", RedirectMainPage);
  _server.WServer.on("/default.html", RedirectMainPage);
  _server.WServer.on("/index.htm", RedirectMainPage);
  _server.WServer.on("/index.html", RedirectMainPage);
  _server.WServer.onNotFound(HandleNotFound);
  _server.WServer.on("/info", HandleInfo);

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
  _server.HandleClientRequests();

  //Handle LED display
  DrawLEDFrame();
}

void HandleWebRequests()
{


    //indicate data received
    BlinkBoardData();

    //read headers
    String path = _server.GetRequestPath();
    path.toLowerCase();

    //DO STUFF HERE
    if (path == "/" || path.startsWith("/index.htm") || path.startsWith("/default.htm"))
    {      
      
    }
    else if (path.startsWith("/info"))
    {      

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

//Serve Main Page
void HandleMainPage()
{
    //indicate data received
    BlinkBoardData();
    
    //Send the main page fille
  _server.PrintFileWebClientResponse("/index.htm");
}

//Redirect to main page
void RedirectMainPage()
{
    //indicate data received
    BlinkBoardData();

    //TODO: for now just serve page, will create proper handler later
    HandleMainPage();
}

//Serve Not Found
void HandleNotFound()
{
    //indicate data received
    BlinkBoardData();
    
    //Send the default Page Not Found file.
  _server.PrintFileWebClientNotFound("/err404.htm");
}

//Serve Info Page
void HandleInfo()
{
  //get headers and convert them to html readable format
  String headers = "Header count: " + String(_server.WServer.headers());
  headers += "<br />";
  headers += _server.GetRequestHeaders();
  headers.replace("/n", "<br />");

  //Build response HTML
  String response = "<!doctype html><html><head><title>ESP32 Info</title></head><h1>ESP32 Info</h1><p><h2>Headers:</h2>" + headers + "</p><p><h2>Hostname</h2>" + WiFi.getHostname() + "</p><p><h2>MAC</h2>" + WiFi.macAddress() + "</p></html>";
  
  //send response
  _server.PrintWebClientResponse(response);
}

