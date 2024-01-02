//+-----------------------------------------------------------------------------------------
//
// File:        miniserv.cpp
//
// Description: The purpose of this file is to provide utility class for
//              WiFi functionnality asn basic web server.
//
// History:     2023-10-28      PP Laplante     Created
//              2023-11-05      PP Laplante     Converted to a class
//              2023-11-11      PP Laplante     Leverage WebServer.h functionnality
//
//------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <fileutils.h>
#include <WebServer.h>
#include <MiniServ.h>

//References:
//https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/
//https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
//https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
//https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/src/WebServer.h
//
//https://techtutorialsx.com/2017/03/26/esp8266-webserver-accessing-the-body-of-a-http-request/
//https://www.dfrobot.com/blog-1105.html
//https://esp32io.com/tutorials/esp32-web-server-multiple-pages
//to handle WiFi re-connect: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/

void OnLostConnection(WiFiEvent_t event, WiFiEventInfo_t info);


//Constructor
MiniServ::MiniServ()
{
    //nothing to do
}

//Constructor with shortcut to init wifi
MiniServ::MiniServ(String ssid, String password, int timeout_ms, String hostname)
{
    InitWiFi(ssid, password, timeout_ms, hostname);
}


//initialize Wifi and attempt to connect
void MiniServ::InitWiFi(String ssid, String password, int timeout_ms, String hostname)
{
    _SSID = ssid;
    _password = password;

    int wait_time_ms = 0;
    
    if (Serial)
    {
        Serial.print("Connecting to ");
        Serial.print(_SSID);
        Serial.print("... ");
    }

    if (hostname!="")
        WiFi.setHostname(hostname.c_str());

    WiFi.begin(_SSID, _password);

    while (wait_time_ms < timeout_ms && WiFi.status() != WL_CONNECTED)
    {
        if (Serial)
            Serial.print(".");

        delay(500);
        wait_time_ms += 500;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        _isWiFiConnected = false;

        if (Serial)
            Serial.println(" Timeout!");
    }
    else
    {
        _isWiFiConnected = true;

        if (AutoReconnect)
        {
            WiFi.onEvent(OnLostConnection, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        }

        if (Serial)
        {
            Serial.print(hostname);
            Serial.println(" Connected!");

            Serial.print("signal strength (RSSI): ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");

            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        }
    }
}

//Handles lost connections
void OnLostConnection(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (Serial)
    {
        Serial.println("Disconnected from WiFi access point");
        Serial.print("WiFi lost connection. Reason: ");
        Serial.println(info.wifi_sta_disconnected.reason);
        Serial.println("Trying to Reconnect...");
    }

    //avoid endless loops during reconnection process
    WiFi.removeEvent(WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.disconnect();
    delay(10000);
   
    int wait_time_ms = 0;
    WiFi.begin();
    
    while (wait_time_ms < 60000 && WiFi.status() != WL_CONNECTED)
    {
        if (Serial)
            Serial.print(".");

        delay(500);
        wait_time_ms += 500;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        if (Serial)
            Serial.println(" Timeout!");
    }
    else
    {
        //re-attach event
        WiFi.onEvent(OnLostConnection, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

        if (Serial)
        {
            Serial.println("Re-connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        }
    }
    
}

//initialize Access Point
void MiniServ::InitAP(String ssid, String password)
{
    _SSID = ssid;
    _password = password;

    _isAPConnected = WiFi.softAP(_SSID, _password);

    if (Serial)
    {
        Serial.println("Access Point Started!");
        Serial.print("Ip Address:");
        Serial.println(WiFi.softAPIP());
    }
}

//checks if the WiFi connection is established
bool MiniServ::IsWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

//Checks if the AP is active
bool MiniServ::IsAPConnected()
{
    return _isAPConnected;
}

//initializes the webserver
void MiniServ::InitWebServer(int port, int client_timeout)
{
    if(IsWiFiConnected() || IsAPConnected())
    {
        _port = port;
        _clientTimeout = client_timeout;

        WServer.begin(_port);

        if (Serial)
        {
            Serial.print("Webserver ready on port ");
            Serial.print(_port);
            Serial.println("!");
        }
    }
    else
    {
        if(Serial)
            Serial.println("Cannot instantiate WebServer, no WiFi available.");
    }
}

//Checks if there is an inbound client request
void MiniServ::HandleClientRequests()
{
    WServer.handleClient();

    //allow the cpu to switch to other tasks
    delay(2);
}

//Gets the raw RequestHeaders
String MiniServ::GetRequestHeaders()
{
    String ret = "";
    
    //ret = ReadRawRequestHeader();
    
    int headCount = WServer.headers();

    for (int i=0; i<headCount; i++)
    {
        ret += WServer.headerName(i) + ":" + WServer.header(i) + "/n";
    }

    

    return ret;
}

//Gets the verb of the request
String MiniServ::GetRequestVerb()
{
    return WServer.header("VERB");
}

//Gets the path of the client request
String MiniServ::GetRequestPath()
{
    return WServer.uri();
}

//Sends an HTML response to the Web Client
void MiniServ::SendResponse(String htmlBody)
{
    WServer.send(200, "text/html", htmlBody);
}

//Sends an HTML response to the Web Client, with specific response code and content type
void MiniServ::SendResponse(String htmlBody, int responseCode, String contentType)
{
    WServer.send(responseCode, contentType, htmlBody);
}

//Sends a file to the Web Client
void MiniServ::SendFileResponse(const char *filePath)
{
    //Write body
    SendFileResponse(filePath, 200, GetContentType(filePath));
}

//sends a file to the Web Client, with specific response code and content type
void MiniServ::SendFileResponse(const char *filePath, int responseCode, String contentType)
{
    WServer.send(responseCode, contentType, FSReadFile(filePath));
}

//Sends a 404 Not Found to the Web Client, optionally with a body
void MiniServ::SendNotFound(String htmlBody)
{
    //Write string
    WServer.send(404, "text/html", htmlBody);
}

//Sends a binary file to the Web Client
void MiniServ::SendBinaryFileResponse(const char *filePath)
{
    SendBinaryFileResponse(filePath, 200, GetContentType(filePath));
}

//Sends a binary file to the Web Client
void MiniServ::SendBinaryFileResponse(const char *filePath, int responseCode, String contentType)
{
    if(!SPIFFS.begin(true)){
        #ifdef MINISERV_DEBUGMODE
            if (Serial)
                Serial.println("An Error has occurred while mounting SPIFFS");
        #endif        
    }
    else
    {    
        //open file
        File file = SPIFFS.open(filePath);

        if(!file){
            #ifdef MINISERV_DEBUGMODE
                if (Serial)
                {
                    Serial.print("Unable to to open file "); 
                    Serial.println(filePath);
                }
            #endif    
        }
        else
        {
            //write content
            if(file.available())
            {                
                WServer.streamFile(file, contentType);
            }

            //close file
            file.close();

            #ifdef MINISERV_DEBUGMODE
                if (Serial)
                {
                    Serial.print("File read: ");
                    Serial.println(filePath);
                }
            #endif                
        }
    }
}


//Gets the value of a uery string parameter by name
String MiniServ::GetQueryStringParameter(String paramName)
{
    return WServer.arg(paramName);
}

//Gets the value of a uery string parameter by index
String MiniServ::GetQueryStringParameter(int paramIndex)
{
    return WServer.arg(paramIndex);
}

//Sends a 404 Not Found to the Web Client and the content of a file in body.
void MiniServ::SendFileNotFound(const char *filePath)
{
    //Write body
    WServer.send(404, "text/html", FSReadFile(filePath));
}

//Parse raw headers to get path
String MiniServ::ParseRequestHeaderPath(String headers)
{
    //path should be second space delimited string
    int startIndex = headers.indexOf(" ");
    int endIndex = headers.indexOf(" ", startIndex + 1);
    
    return headers.substring(startIndex + 1, endIndex);
}

//Parse raw headers to get verb
String MiniServ::ParseRequestHeaderVerb(String headers)
{
    //verb should be first characters before space
    return headers.substring(0, headers.indexOf(" "));
}

void MiniServ::SaveFileUpload()
{
    SaveFileUploadAs("");
}

//Saves an uploaded file to the file system
void MiniServ::SaveFileUploadAs(String filePath)
{
  HTTPUpload& upload = WServer.upload();

  String fileName = "";

  if (filePath == "")
  {
    fileName = "/" + upload.filename;
  }
  else
  {
    fileName = filePath;
  }

  if (upload.status == UPLOAD_FILE_START) {
    //Start of the upload process
    if(!SPIFFS.begin(true))
    {
        #ifdef MINISERV_DEBUGMODE
            if (Serial)
                Serial.println("An Error has occurred while mounting SPIFFS");
        #endif        
    }
    else
    {
        //delete file if already exists
        if (SPIFFS.exists(fileName))
        {
            SPIFFS.remove(fileName);

            #ifdef MINISERV_DEBUGMODE
                if (Serial)
                {
                    Serial.print("Deleted file: ");
                    Serial.println(fileName);
                }
            #endif
        }

        //create file
        uploadFile = SPIFFS.open(fileName, FILE_WRITE);
        #ifdef MINISERV_DEBUGMODE
            if (Serial)
            {
                Serial.print("Created file: ");
                Serial.println(fileName);
            }
        #endif        
    }
    
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) 
    {
        uploadFile.write(upload.buf, upload.currentSize);

        #ifdef MINISERV_DEBUGMODE
            if (Serial)
            {
                Serial.print("Wrote ");
                Serial.print(upload.currentSize);
                Serial.println("bytes to file.");                
            }
        #endif
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) 
    {
        uploadFile.close();

        if (Serial)
        {
            Serial.print("Upload complete!");
            Serial.print(fileName);
            Serial.print(", ");
            Serial.print(upload.totalSize);
            Serial.println("bytes.");                
        }
    }
  }    
}

//Get content type based on file extension
String MiniServ::GetContentType(String filePath) {
  if (filePath.endsWith(".htm")) {
    return "text/html";
  } else if (filePath.endsWith(".html")) {
    return "text/html";
  } else if (filePath.endsWith(".css")) {
    return "text/css";
  } else if (filePath.endsWith(".js")) {
    return "application/javascript";
  } else if (filePath.endsWith(".png")) {
    return "image/png";
  } else if (filePath.endsWith(".gif")) {
    return "image/gif";
  } else if (filePath.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filePath.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filePath.endsWith(".xml")) {
    return "text/xml";
  } else if (filePath.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filePath.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filePath.endsWith(".gz")) {
    return "application/x-gzip";
  } else if (filePath.endsWith(".txt")) {
    return "text/plain";
  } else
    return "application/octetstream";
}

//Read request headers from client
// String MiniServ::ReadRawRequestHeader()
// {
//     //header string
//     String header_data = "";

//     WiFiClient _client = WServer.client();


//     // Current time
//     unsigned long currentTime = millis();
//     // Previous time
//     unsigned long previousTime = 0; 
    
//     if (_client)
//     {
//         currentTime = millis();
//         previousTime = currentTime;
//         bool readComplete = false; //indicate if the header was entirely read (exit condition)
//         String currentLine = "";

//         // loop while the client's connected
//         while (_client.connected() && !readComplete && currentTime - previousTime <= _clientTimeout) 
//         {
//             if (_client.available())
//             {
//                 char c = _client.read();  // read a byte from the client
//                 header_data += c;

//                 //Check for exit conditions if the byte is a newline character
//                 if (c == '\n') 
//                 {                    
//                     // if the current line is blank, you got two newline characters in a row.
//                     // that's the end of the client HTTP request, so send a response:
//                     if (currentLine.length() == 0) 
//                     {
//                         // Break out of the while loop
//                         readComplete = true;
//                         break;
//                     } 
//                     else 
//                     { 
//                         // if you got a newline, then clear currentLine
//                         currentLine = "";
//                     }
//                 } 
//                 else if (c != '\r') 
//                 {
//                     // if you got anything else but a carriage return character,
//                     // add it to the end of the currentLine
//                     currentLine += c;      
//                 }

//             }
//         }

//         #ifdef MINISERV_DEBUGMODE
//             if (Serial)
//             {
//                 //check if timeout occured
//                 if (currentTime - previousTime > _clientTimeout)
//                 {
//                     Serial.print("Client Timeout! (");
//                     Serial.print(_clientTimeout);
//                     Serial.println("ms)");
//                 }

//                 //check if client disconnected
//                 if (_client.connected())
//                 {
//                     Serial.println("Client Disconnected!");
//                 }

//                 //header data
//                 Serial.print("Web client header: '");
//                 Serial.print(header_data);
//                 Serial.println("'.");
//             }
//         #endif
//     }
    
//     return header_data;
// }


