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
//              2023-11-13    PP Laplante   Use new MiniServ web server
//              2023-12-22    PP Laplante   Implemented Showcase
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

#define IMAGE_DIR               "/images/"
#define IMAGE_EXT               ".dat"

#define DEBUGMODE              1

//Libraries
#include <Arduino.h>
#include <Wifi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <arduinoutils.h>
#include <MiniServ.h>
#include <fastledutils.h>
#include <fileutils.h>

//Global Variables
MiniServ _server;
bool _showcaseMode = true;
int _showcaseImageIndex  = 0;
unsigned long _showcaseDelayMs = 20000;
unsigned long _showcasePreviousTime = millis() - _showcaseDelayMs - 1;

//Prototyopes
void HandleEffect();
void HandleNotFound();
void HandleMainPage();
void RedirectMainPage();
void HandleInfo();
void HandleFavIcon();
void HandleListImages();
void HandleUploadImage();
void HandleDownloadImage();
void HandleDeleteImage();
void HandleStorageInfo();
void HandleShowcase();

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
    _server.WServer.on("/effect", HandleEffect);
    _server.WServer.on("/favicon.ico", HandleFavIcon);
    _server.WServer.on("/images", HandleListImages);
    _server.WServer.on("/image", HTTP_PUT, HandleUploadImage);
    _server.WServer.on("/image", HTTP_GET, HandleDownloadImage);
    _server.WServer.on("/image", HTTP_DELETE, HandleDeleteImage);
    _server.WServer.on("/storage", HandleStorageInfo);

    //https://techtutorialsx.com/2018/10/12/esp32-http-web-server-handling-body-data/

    _server.InitWebServer();


    //Initialize LEDs
    InitLED();
    //SetLEDCurrentEffect("Default");
    //default to showcase insted
    _showcaseMode=true;
      
    //blink twice to indicate we are ready
    BlinkBoard(2, 250);

    //Write to serial to indicate we are ready
    PrintlnSerial("Ready!");
}

void loop() {
    //Handle any web requests
    _server.HandleClientRequests();

    //Handle showcase
    HandleShowcase();

    //Handle LED display
    DrawLEDFrame();
}

void HandleEffect()
{
    //indicate data received
    BlinkBoardData();

    //read parameters
    String effect = _server.GetQueryStringParameter("name");
    effect.toLowerCase();
    String color = _server.GetQueryStringParameter("color");
    color.toUpperCase();
    String brightness = _server.GetQueryStringParameter("brightness");
    brightness.toUpperCase();
    String imgdata = _server.GetQueryStringParameter("imgdata");
    imgdata.toUpperCase();
    String imgname = _server.GetQueryStringParameter("imgname");
    

    #ifdef DEBUGMODE
        PrintlnSerial("effect:" + effect);
        PrintlnSerial("color:" + color);
        PrintlnSerial("brightness:" + brightness);
    #endif

    //DO STUFF HERE
    if (effect == "default")
    {
        SetLEDCurrentEffect("Default");
        _server.SendResponse("Effect set to: Default");
    }
    else if (effect == "solid")
    {
        _showcaseMode=false;
        SetLEDCurrentEffect("Solid", color);
        _server.SendResponse("Effect set to: Solid (" + GetLEDCurrentEffectParameters() + ")");
    }
    else if (effect == "beat")
    {
        _showcaseMode=false;
        SetLEDCurrentEffect("Beat");
        _server.SendResponse("Effect set to: Beat");
    }
    else if (effect == "rainbow")
    {
        _showcaseMode=false;
        SetLEDCurrentEffect("Rainbow");
        _server.SendResponse("Effect set to: Rainbow");
    }
    else if (effect == "image")
    {
        _showcaseMode=false;
        SetLEDCurrentEffect("Image", FSReadFile(IMAGE_DIR + imgname + IMAGE_EXT));
        _server.SendResponse("Effect set to: Image");
    }
    else if (effect == "showcase")
    {
        _showcaseMode=true;
        _server.SendResponse("OK");
    }
    else
    {
        //sometimes you just want to adjust brightness or color, dont change anything
        _server.SendResponse("OK");
    }

    //adjust brightness if required
    if (brightness != "")
        SetLEDBrightness(HexStrToInt(brightness));

}

//Serve Main Page
void HandleMainPage()
{
    //indicate data received
    BlinkBoardData();
    
    //Send the main page fille
    _server.SendFileResponse("/www/index.htm");
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
    _server.SendFileNotFound("/www/err404.htm");
}

//Serve Info Page
void HandleInfo()
{
    //get headers and convert them to html readable format
    String headers = "Header count: " + String(_server.WServer.headers());
    headers += "<br />";
    headers += _server.GetRequestHeaders();
    headers.replace("/n", "<br />");

    String method = (_server.WServer.method() == HTTP_GET) ? "GET" : "POST";
    
    String args = "Arguments: ";
    args += _server.WServer.args();
    args += "<br />\n";
    for (uint8_t i = 0; i < _server.WServer.args(); i++) {
      args += " " + _server.WServer.argName(i) + ": " + _server.WServer.arg(i) + "<br />\n";
    }

    //Build response HTML
    String response = "<!doctype html><html><head><title>ESP32 Info</title></head><h1>ESP32 Info</h1><p><h2>Headers:</h2>" + headers + 
                      "</p><p><h2>Hostname</h2>" + WiFi.getHostname() + 
                      "</p><p><h2>MAC</h2>" + WiFi.macAddress() + 
                      "</p><p><h2>URI</h2>" + _server.WServer.uri() +
                      "</p><p><h2>Method</h2>" + method +
                      "</p><p><h2>Arguments</h2>" + args +
                      "</p></html>";
    
    //send response
    _server.SendResponse(response);
}

//Favorite icon
void HandleFavIcon()
{
    _server.SendBinaryFileResponse("/www/favicon.ico");
}

void HandleListImages()
{
    String imgListJson = "{\"FilesList\":[";
    bool firstFile = true;

    if(!SPIFFS.begin(true))
    {
        #ifdef DEBUGMODE
            if (Serial)
                Serial.println("An Error has occurred while mounting SPIFFS");
        #endif

        _server.SendResponse("Error listing files.", 500, "text/plain"); 
    }
    else
    {
        //iterate through files      
        File root = SPIFFS.open("/images"); //IT DOES NOT LIKE THE TRAILING "/"
        String fileName = root.getNextFileName();
      
        while(fileName != ""){
            #ifdef DEBUGMODE
                if (Serial)
                {
                  Serial.print("ENUM FILE: ");
                  Serial.println(fileName);
                }
            #endif

            //add to array if proper extension
            if (fileName.endsWith(IMAGE_EXT))
            {
              String cleanFileName = String(fileName);
              cleanFileName.replace(IMAGE_DIR, "");
              cleanFileName.replace(IMAGE_EXT, "");


              if (firstFile)
                imgListJson += "\"" + cleanFileName + "\"";
              else
                imgListJson += ", \"" + cleanFileName + "\"";

              firstFile = false;
            }
      
            fileName = root.getNextFileName();
        }
    }  

    imgListJson += "]}";

    //return JSON list
    _server.SendResponse(imgListJson, 200, "application/json");
}

void HandleUploadImage()
{
    String fileName = _server.GetQueryStringParameter("imgname");
    String fileData = _server.GetQueryStringParameter("imgdata");
    fileData.toUpperCase();

    int fileSize = FSWriteFile(IMAGE_DIR + fileName + IMAGE_EXT, fileData);

    if (fileSize == -1)
    {
        #ifdef DEBUGMODE
            String mess = "Error creating";
            mess += IMAGE_DIR + fileName + IMAGE_EXT;
            PrintlnSerial(mess);
        #endif

        _server.SendResponse("Error creating" + fileName, 500, "text/plain");
    }
    else
    {
        #ifdef DEBUGMODE
            PrintlnSerial("Wrote " + String(fileSize) + "bytes to " + IMAGE_DIR + fileName + IMAGE_EXT);
        #endif

        //return info to client
        _server.SendResponse("Wrote " + String(fileSize) + "bytes to " + fileName + ".");
    }
}

void HandleDownloadImage()
{  
    String fileName = IMAGE_DIR +  _server.GetQueryStringParameter("imgname") + IMAGE_EXT;
    
    if (FSFileExists(fileName))
    {
        //convert to char array
        int l = fileName.length() + 1;
        char fName[l];  
        fileName.toCharArray(fName, l);
        const char* fn = fName;

        _server.SendFileResponse(fName, 200, "text/plain");
    }
    else
    {
        _server.SendResponse("File not found.", 404, "text/plain");
    }
}

void HandleDeleteImage()
{
    String fileName = _server.GetQueryStringParameter("imgname");

    if (FSDeleteFile(IMAGE_DIR + fileName + IMAGE_EXT))
    {
        #ifdef DEBUGMODE
            PrintlnSerial("Deleted " + fileName);
        #endif

        _server.SendResponse("Deleted " + fileName, 200, "text/plain");
    }
    else
    {
        #ifdef DEBUGMODE
            PrintlnSerial("Error deleting " + fileName);
        #endif

        _server.SendResponse("Error deleting " + fileName, 500, "text/plain");
    }

}

void HandleStorageInfo()
{
    if (!SPIFFS.begin(true))
    {
        #ifdef DEBUGMODE
            PrintlnSerial("An Error has occu rred while mounting SPIFFS");
        #endif

        _server.SendResponse("Error getting storage info.", 500, "text/plain"); 
    }
    else
    {
        int total = SPIFFS.totalBytes();
        int used = SPIFFS.usedBytes();

        #ifdef DEBUGMODE
            PrintSerial("Total bytes: ");
            PrintlnSerial(String(total));
            PrintSerial("Used bytes: ");
            PrintlnSerial(String(used));
        #endif

        String info = "{\"TotalBytes\":" + String(total) + ", \"UsedBytes\":" + String(used) + "}" ;
        _server.SendResponse(info, 200, "application/json");
    }   
}

String GetImageById(int imgID)
{
    int currentId = 0;

    //iterate through files
    if (!SPIFFS.begin(true))
    {
        #ifdef DEBUGMODE
            PrintlnSerial("An error occured mounting SPIFFS");
        #endif
    }
    else
    {
        File root = SPIFFS.open("/images"); //IT DOES NOT LIKE THE TRAILING "/"
        String fileName = root.getNextFileName();
        
        while(fileName != ""){
            //check if proper extension
            if (fileName.endsWith(IMAGE_EXT))
            {
                if (imgID == currentId)
                {
                    #ifdef DEBUGMODE
                        PrintlnSerial("Found image " + fileName + " at index " + String(currentId));
                    #endif

                    return fileName;
                }
                currentId++;
            }
        
            fileName = root.getNextFileName();
        }
    }

    //not found
    return "";
}

void HandleShowcase()
{
    //check if we are in showcase mode
    if (_showcaseMode)
    {
        //update current time
        unsigned long showcaseCurrentTime = millis();

        //check if it is time to change image
        if (showcaseCurrentTime > _showcasePreviousTime + _showcaseDelayMs)
        {
            PrintlnSerial("Next image in showcase...");
            String imgName = GetImageById(_showcaseImageIndex);

            //cehck if we went too far
            if (imgName == "")
            {
                //go-back to start
                _showcaseImageIndex=0;
                imgName = GetImageById(_showcaseImageIndex);
            }

            //update previous time
            _showcasePreviousTime = showcaseCurrentTime;
            //increment next image
            _showcaseImageIndex++;

            //display image
            SetLEDCurrentEffect("Image", FSReadFile(imgName));
        }
    }
}