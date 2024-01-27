//----------------------------------------------------------------------------------------------
//
// File:            main.cpp
//
// Project:         LedDriver 
//
// Description:     The purpose of this project is to provide a light and 
//                  streamlined version of a Manager for the WS2812B LED Strip 
//                  hosted on a ESP32 microcontroller development board.
//
// Credits:         This project is inspired from NightDriverStrip
//                  https://github.com/PlummersSoftwareLLC/NightDriverStrip
//                  https://www.youtube.com/watch?v=UZxY_BLSsGg
//
// History:         2023-10-28    PP Laplante   Created
//                  2023-11-13    PP Laplante   Use new MiniServ web server
//                  2023-12-22    PP Laplante   Implemented Showcase
//                  2024-01-07    PP Laplante   Implemented NTP real time clock sync
//                  2024-01-09    PP Laplante   Set default persistance
//
// Known Issues:    - All effects are now set as default regardless if checkbox is set or not
//                  - When getting current effect, string is mangled when received by client.
//
//----------------------------------------------------------------------------------------------

// Global Constants
#define LED_MATRIX_WIDTH        16
#define LED_MATRIX_HEIGHT       16
#define LED_NUM_LEDS            (MATRIX_WIDTH*MATRIX_HEIGHT)
#define LED_MATRIX_INTERLACED   1
#define LED_GPIO_PIN            13
#define BOARD_PIN_LED           2
#define WIFIUTILS_SERVERPORT    80
#define LED_DEFAULT_EFFECT      "SHOWCASE"
#define LED_DEFAULT_SHOWCASE    false

#define CONFIG_FILE             "/config.json"
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
#include <ArduinoJson.h>
#include <NtpHelper.h>
#include <version.h>

struct LedManagerConfiguration
{
    String  wifiSSID = "MyWiFi";
    String  wifiPassword = "MyPassword";
    int     wifiTimeout = 60000;
    String  wifiHostname = "PIXELART";
    String  effectDefault = "DEFAULT";
};

struct DeviceInformation
{
    char    deviceMAC[18] = "00:00:00:00:00:00";
    char    deviceIP[16] = "0.0.0.0";
    char    deviceSSID[33] = "MyWiFi";
    char    deviceHostname[33] = "PIXELART";
    int     deviceSignal = 0;
    char    firmwareVersion[33] = VERSION_SHORT;
};


//Global Variables
MiniServ _server;
bool _showcaseMode = LED_DEFAULT_SHOWCASE;
bool _configMode = false;
int _showcaseImageIndex  = 0;
unsigned long _showcaseDelayMs = 20000;
unsigned long _showcasePreviousTime = millis() - _showcaseDelayMs - 1;
LedManagerConfiguration _config;
NtpHelper _timeLord = NtpHelper();
String _currentEffect = "";
DeviceInformation _deviceInfo;

//Prototyopes
bool ReadConfig();
bool SaveConfig();
void HandleSetEffect();
void HandleGetEffect();
void HandleNotFound();
void HandleGetMainPage();
void RedirectMainPage();
void HandleGetInfoPage();
void HandleGetFavIcon();
void HandleListImages();
void HandleSetImage();
void HandleGetImage();
void HandleDeleteImage();
void HandleGetStorageInfo();
void HandleShowcaseMode();
void HandleSetConfig();
void HandleGetConfig();
void HandleConfigPage();
void ActivateEffect(String effect, String color="", String brightness="", String imgname="");
void HandleReboot();
void HandleGetInfo();
void UpdateDeviceInfo();
String SerializeDeviceInfo();

void setup() {
    //initialize pins
    pinMode(BOARD_PIN_LED, OUTPUT);

    //initialize serial port
    InitSerial();

    //Read configuration
    if (!ReadConfig())
    {
        #ifdef DEBUGMODE
            PrintlnSerial("Unable to read configuration file!");
        #endif
        _configMode = true;
    }
    else
    {
        //Initialize WiFi
        _server.InitWiFi(_config.wifiSSID, _config.wifiPassword, _config.wifiTimeout, _config.wifiHostname);
    }

    //check if we are successfully connected to the WiFi (and hopefully internet)
    if (_server.IsWiFiConnected())
    {
        _timeLord.ConfigNTP("time.nrc.ca");
        PrintSerial("Current time is: ");
        PrintlnSerial(_timeLord.GetDateTime());
    }
    else
    {
        //fallback in AP mode if WIFI failed or WiFi not yet configured
        _server.InitAP("PIXELART-AP", "");
    }

    //Initialize Web Server
    if (_server.IsAPConnected())
    {
        //force configuration mode
        _server.WServer.on("/", HandleConfigPage);
        _server.WServer.on("/default.htm", HandleConfigPage);
        _server.WServer.on("/default.html", HandleConfigPage);
        _server.WServer.on("/index.htm", HandleConfigPage);
        _server.WServer.on("/index.html", HandleConfigPage);
    }
    else
    {
        //regular main page handling
        _server.WServer.on("/", HandleGetMainPage);
        _server.WServer.on("/default.htm", RedirectMainPage);
        _server.WServer.on("/default.html", RedirectMainPage);
        _server.WServer.on("/index.htm", RedirectMainPage);
        _server.WServer.on("/index.html", RedirectMainPage);
    }

    //Standard pages request handling
    _server.WServer.onNotFound(HandleNotFound);
    _server.WServer.on("/favicon.ico", HandleGetFavIcon);
    _server.WServer.on("/config.htm", HandleConfigPage);
    _server.WServer.on("/config.html", HandleConfigPage);
    _server.WServer.on("/info.htm", HandleGetInfoPage);
    _server.WServer.on("/info.html", HandleGetInfoPage);

    //API requests
    _server.WServer.on("/api/effect", HTTP_PUT, HandleSetEffect);
    _server.WServer.on("/api/effect", HTTP_POST, HandleSetEffect);
    _server.WServer.on("/api/effect", HTTP_GET, HandleGetEffect);
    _server.WServer.on("/api/images", HandleListImages);
    _server.WServer.on("/api/image", HTTP_PUT, HandleSetImage);
    _server.WServer.on("/api/image", HTTP_GET, HandleGetImage);
    _server.WServer.on("/api/image", HTTP_DELETE, HandleDeleteImage);
    _server.WServer.on("/api/storage", HandleGetStorageInfo);
    _server.WServer.on("/api/config", HTTP_PUT, HandleSetConfig);
    _server.WServer.on("/api/config", HTTP_POST, HandleSetConfig);
    _server.WServer.on("/api/config", HTTP_GET, HandleGetConfig);
    _server.WServer.on("/api/reboot", HandleReboot);
    _server.WServer.on("/api/info", HandleGetInfo);


    //https://techtutorialsx.com/2018/10/12/esp32-http-web-server-handling-body-data/

    _server.InitWebServer();


    //Initialize LEDs
    InitLED();

    //Apply initial effect (from config if possible)
    if (_config.effectDefault != "")
        ActivateEffect(_config.effectDefault);
    else
        ActivateEffect(LED_DEFAULT_EFFECT);

    //blink twice to indicate we are ready
    BlinkBoard(2, 250);

    //Write to serial to indicate we are ready
    PrintlnSerial("Ready!");
}

void loop() {
    //Handle any web requests
    _server.HandleClientRequests();

    //Handle showcase
    HandleShowcaseMode();

    //Handle LED display
    DrawLEDFrame();
}

void HandleGetEffect()
{
    //indicate data received
    BlinkBoardData();

    //build information
    String effectInfo = "{\"effect\": \"" + _currentEffect + "\", \"brightness\": " + GetLEDBrightness() + "}";

    //Return current effect
    _server.SendResponse(effectInfo, 200, "application/json");
}

void HandleSetEffect()
{
    //indicate data received
    BlinkBoardData();

    //read parameters
    String p_effect = _server.GetQueryStringParameter("name");
    p_effect.toLowerCase();
    String p_color = _server.GetQueryStringParameter("color");
    p_color.toUpperCase();
    String p_brightness = _server.GetQueryStringParameter("brightness");
    p_brightness.toUpperCase();
    String p_imgname = _server.GetQueryStringParameter("imgname");
    String p_setdefault = _server.GetQueryStringParameter("setdefault");

    PrintSerial("Query String: ");
    PrintlnSerial(_server.GetRequestPath());

    #ifdef DEBUGMODE
        PrintlnSerial("effect:" + p_effect);
        PrintlnSerial("color:" + p_color);
        PrintlnSerial("brightness:" +p_brightness);
        PrintlnSerial("imgname:" + p_imgname);
        PrintlnSerial("setdefault:" + p_setdefault);
    #endif

    //Activate the effect
    ActivateEffect(p_effect, p_color, p_brightness, p_imgname);

    //Respond to client request
    String responseMesage = "Effect set to: " + p_effect;
    if (p_color != "")
        responseMesage += " (" + p_color + ")";
    if (p_imgname != "")
        responseMesage += " (" + p_imgname + ")";
    _server.SendResponse(responseMesage);

    //adjust brightness if required
    if (p_brightness != "")
        SetLEDBrightness(HexStrToInt(p_brightness));

    //change default if required
    if (p_setdefault == "1")
    {
        _config.effectDefault = _currentEffect;
        SaveConfig();

        #ifdef DEBUGMODE
            PrintlnSerial("Default changed to: " + _currentEffect);
        #endif        
    }
}

void ActivateEffect(String effect, String color, String brightness, String imgname)
{
    if (effect == "default")
    {
        _currentEffect = "default";
        SetLEDCurrentEffect("Default");   
    }
    else if (effect == "solid")
    {
        _showcaseMode=false;
        _currentEffect = "solid";
        SetLEDCurrentEffect("Solid", color);
    }
    else if (effect == "off")
    {
        _showcaseMode=false;
        _currentEffect = "off";
        SetLEDCurrentEffect("Solid", "000000");
    }    
    else if (effect == "beat")
    {
        _showcaseMode=false;
        _currentEffect = "beat";
        SetLEDCurrentEffect("Beat", color);
    }
    else if (effect == "rainbow")
    {
        _showcaseMode=false;
        _currentEffect = "rainbow";
        SetLEDCurrentEffect("Rainbow");
    }
    else if (effect == "northpole")
    {
        _showcaseMode=false;
        _currentEffect = "northpole";
        SetLEDCurrentEffect("Pattern", "FF0000000000000000FFFFFF000000000000");
    }
    else if (effect == "quebec")
    {
        _showcaseMode=false;
        _currentEffect = "quebec";
        SetLEDCurrentEffect("Pattern", "0000FF000000000000FFFFFF000000000000");
    }    
    else if (effect == "festive")
    {
        //bleu orange vert roughe jaune
        _showcaseMode=false;
        _currentEffect = "fastive";
        SetLEDCurrentEffect("Pattern", "0000FF00000000000000FF000000000000000000FF000000000000F3E220000000000000FF0000000000000000");
    }
    else if (effect == "image")
    {
        _showcaseMode=false;
        _currentEffect = "image";
        SetLEDCurrentEffect("Image", FSReadFile(IMAGE_DIR + imgname + IMAGE_EXT));
    }
    else if (effect == "showcase")
    {
        _showcaseMode=true;
        _currentEffect = "showcase";
    }
    else
    {
        //sometimes you just want to adjust brightness or color, dont change anything
    }
}

//Serve Main Page
void HandleGetMainPage()
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
    HandleGetMainPage();
}

//Serve Not Found
void HandleNotFound()
{
    //indicate data received
    BlinkBoardData();
    
    //Send the default Page Not Found file.
    _server.SendFileNotFound("/www/err404.htm");
}

void UpdateDeviceInfo()
{
    //update device info
    strcpy(_deviceInfo.deviceHostname, WiFi.getHostname());
    strcpy(_deviceInfo.deviceIP, WiFi.localIP().toString().c_str());
    strcpy(_deviceInfo.deviceMAC, WiFi.macAddress().c_str());
    _deviceInfo.deviceSignal = WiFi.RSSI();
    strcpy(_deviceInfo.deviceSSID, WiFi.SSID().c_str());

    /*  RSSI signal strength chart
        https://www.securedgenetworks.com/blog/wifi-signal-strength
        
    Greater than
    > -30 dBm : Excellent signal
    > -67 dBm : Good signal
    > -70 dBm : Okay signal
    > -80 dBm : Poor signal
    > -90 dBm : Unsusable signal    
    */
}

String SerializeDeviceInfo()
{
    String info = "";
    StaticJsonDocument<512> doc;

    //create JSON document form global variable
    doc["device"]["hostname"] = _deviceInfo.deviceHostname;
    doc["device"]["ip"] = _deviceInfo.deviceIP;
    doc["device"]["ssid"] = _deviceInfo.deviceSSID;
    doc["device"]["mac"] = _deviceInfo.deviceMAC;
    doc["device"]["signal"] = _deviceInfo.deviceSignal;
    doc["device"]["firmware"] = _deviceInfo.firmwareVersion;

    //serialize data
    serializeJson(doc, info);

    return info;
}


//Serve Raw Info
void HandleGetInfo()
{
    UpdateDeviceInfo();
    _server.SendResponse(SerializeDeviceInfo(), 200, "application/json");
}

//Serve Info Page
void HandleGetInfoPage()
{
    UpdateDeviceInfo();

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
    String response = "<!doctype html><html><head><title>ESP32 Info</title></head><h1>ESP32 Info</h1><p><h2>Headers</h2>" + headers + 
                      "</p><p><h2>HTTP Request</h2>URI: " + _server.WServer.uri() +
                      "<br />Method: " + method +
                      "<br />Arguments: " + args +
                      "</p><p><h2>WiFi</h2>" + 
                      "MAC address: " + _deviceInfo.deviceMAC + 
                      "<br />IP address: " + _deviceInfo.deviceIP + 
                      "<br />Hostname: " + _deviceInfo.deviceHostname + 
                      "<br />SSID: " + _deviceInfo.deviceSSID + 
                      "<br />Signal strength: " + _deviceInfo.deviceSignal + "dBm" + 
                      "</p></html>";
    
    //send response
    _server.SendResponse(response);
}

//Favorite icon
void HandleGetFavIcon()
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

void HandleSetImage()
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

void HandleGetImage()
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

void HandleGetStorageInfo()
{
    if (!SPIFFS.begin(true))
    {
        #ifdef DEBUGMODE
            PrintlnSerial("An Error has occured while mounting SPIFFS");
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

void HandleShowcaseMode()
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


bool DeserializeConfig(String conf)
{
    StaticJsonDocument<512> doc;
    deserializeJson(doc, conf);

    #ifdef DEBUGMODE
        PrintlnSerial("Deserialized config:");
        serializeJsonPretty(doc, Serial);
    #endif

    //load config data into global variable
    _config.wifiHostname = doc["wifi"]["hostname"].as<String>();
    _config.wifiPassword = doc["wifi"]["pwd"].as<String>();
    _config.wifiSSID = doc["wifi"]["ssid"].as<String>();
    _config.wifiTimeout = doc["wifi"]["timeout"];
    _config.effectDefault = doc["effect"]["default"].as<String>();

    return true; //success
}

String SerializeConfig(bool maskPassword=false)
{
    String conf = "";
    StaticJsonDocument<512> doc;

    //create JSON document form global variable
    doc["wifi"]["hostname"] = _config.wifiHostname;
    doc["wifi"]["pwd"] = _config.wifiPassword;
    doc["wifi"]["ssid"] = _config.wifiSSID;
    doc["wifi"]["timeout"] = _config.wifiTimeout;
    doc["effect"]["default"] = _config.effectDefault;

    if (maskPassword)
        doc["wifi"]["pwd"]="";

    //serialize data
    serializeJson(doc, conf);

    return conf;
}

bool ReadConfig()
{
    //read config file
    String conf = FSReadFile(CONFIG_FILE);

    #ifdef DEBUGMODE
        PrintlnSerial("Read config from file:");
        PrintlnSerial(conf);
    #endif

    if (conf =="")
        return false;
    else
    {
        //deserialize config data, and return if the operation was succesful
        return DeserializeConfig(conf);
    }
}

bool SaveConfig()
{
    String conf = SerializeConfig();

    //write config to file
    int ret = FSWriteFile(CONFIG_FILE, conf);

    #ifdef DEBUGMODE
        PrintlnSerial("Wrote config to file:");
        PrintlnSerial(conf);
    #endif

    return (ret > 0);
}

//Configuration Webpage
void HandleConfigPage()
{
    //indicate data received
    BlinkBoardData();
    
    //Send the main page fille
    _server.SendFileResponse("/www/config.htm");
}

//Handle config API PUT
void HandleSetConfig()
{
    String configData = _server.GetQueryStringParameter("configdata");

    #ifdef DEBUGMODE
        PrintlnSerial("received configuration string:");
        PrintlnSerial(configData);
    #endif

    if (DeserializeConfig(configData))
    {
        //default timout to 60s
        _config.wifiTimeout = 60000;

        //Read OK, save the config
        SaveConfig();
        _server.SendResponse("OK", 200, "text/plain");
    }
    else
        _server.SendResponse("Unable to update config!", 500, "text/plain");
}

//Handle config API GET
void HandleGetConfig()
{
    String configData = SerializeConfig(true);

    _server.SendResponse(configData, 200, "application/json");
}

void HandleReboot()
{
    ESP.restart();
}