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
#include <WebServer.h>
#include <MiniServ.h>

//References:
//https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
//https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
//https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/src/WebServer.h

//Constructor
MiniServ::MiniServ()
{
    //nothing to do
}

//Constructor with shortcut to init wifi
MiniServ::MiniServ(String ssid, String password, int timeout_ms)
{
    InitWiFi(ssid, password, timeout_ms);
}


//initialize Wifi and attempt to connect
void MiniServ::InitWiFi(String ssid, String password, int timeout_ms)
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

        if (Serial)
        {
            Serial.println(" Connected!");

            Serial.print("signal strength (RSSI): ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");

            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        }
    }
}

//checks if the WiFi connection is established
bool MiniServ::IsWiFiConnected()
{
    return WiFi.status() == WL_CONNECTED;
}


//initializes the webserver
void MiniServ::InitWebServer(int port, int client_timeout)
{
    if(IsWiFiConnected())
    {
        _port = port;
        _clientTimeout = client_timeout;
        //_WiFiServer = WiFiServer(_port);
        //_WiFiServer = new WebServer(_port);
        _WiFiServer.begin();

        

        #ifdef MINISERV_DEBUGMODE
        if (Serial)
            Serial.print("Webserver ready on port ");
            Serial.print(_port);
            Serial.println("!");
        #endif
    }
    else
    {
        if(Serial)
            Serial.println("Cannot instantiate WebServer, no WiFi available.");
    }
}

// //Handle incoming web request with function
// void MiniServ::on(const char *uri, THandlerFunction fn)
// {
//     _WiFiServer.on(uri, fn);
// }

// //Handle incoming web request with function, specifying method
// void MiniServ::on(const char *uri, const char *method, THandlerFunction fn)
// {
//     _WiFiServer.on(uri, method, fn);
// }

// //Handle incoming web request with function, specifying method and file upload handling function
// void MiniServ::on(const char *uri, const char *method, THandlerFunction fn, THandlerFunction uploadfn)
// {
//     _WiFiServer.on(uri, method, fn, uploadfn);
// } 



//Checks if there is an inbound client request
bool MiniServ::IsClientRequest()
{
    _WiFiServer.handleClient();
    return true;
    
    //GetWebClientRequest();
    
    //if (_client && _client.connected())
    // {
    //     #ifdef MINISERV_DEBUGMODE
    //         if (Serial)
    //             Serial.println("New web client connection.");
    //     #endif

    //     return true;
    // }
    // else
    //     return false;
}

//Reads any client requests the server might have received
void MiniServ::GetWebClientRequest()
{
    // _client = _WiFiServer.available();

    // //parse header
    // if (_client)
    // {
    //     _requestRawHeader = ReadRawRequestHeader();
    //     _requestPath = ParseRequestHeaderPath(_requestRawHeader);
    //     _requestVerb = ParseRequestHeaderVerb(_requestRawHeader);
    // }
}

//Reads the client request header
String MiniServ::ReadRawRequestHeader()
{
    //header string
    String header_data = "";

    header_data = _WiFiServer.hostHeader();


    // // Current time
    // unsigned long currentTime = millis();
    // // Previous time
    // unsigned long previousTime = 0; 
    
    // if (_client)
    // {
    //     currentTime = millis();
    //     previousTime = currentTime;
    //     bool readComplete = false; //indicate if the header was entirely read (exit condition)
    //     String currentLine = "";

    //     // loop while the client's connected
    //     while (_client.connected() && !readComplete && currentTime - previousTime <= _clientTimeout) 
    //     {
    //         if (_client.available())
    //         {
    //             char c = _client.read();  // read a byte from the client
    //             header_data += c;

    //             //Check for exit conditions if the byte is a newline character
    //             if (c == '\n') 
    //             {                    
    //                 // if the current line is blank, you got two newline characters in a row.
    //                 // that's the end of the client HTTP request, so send a response:
    //                 if (currentLine.length() == 0) 
    //                 {
    //                     // Break out of the while loop
    //                     readComplete = true;
    //                     break;
    //                 } 
    //                 else 
    //                 { 
    //                     // if you got a newline, then clear currentLine
    //                     currentLine = "";
    //                 }
    //             } 
    //             else if (c != '\r') 
    //             {
    //                 // if you got anything else but a carriage return character,
    //                 // add it to the end of the currentLine
    //                 currentLine += c;      
    //             }

    //         }
    //     }

    //     #ifdef MINISERV_DEBUGMODE
    //         if (Serial)
    //         {
    //             //check if timeout occured
    //             if (currentTime - previousTime > _clientTimeout)
    //             {
    //                 Serial.print("Client Timeout! (");
    //                 Serial.print(_clientTimeout);
    //                 Serial.println("ms)");
    //             }

    //             //check if client disconnected
    //             if (_client.connected())
    //             {
    //                 Serial.println("Client Disconnected!");
    //             }

    //             //header data
    //             Serial.print("Web client header: '");
    //             Serial.print(header_data);
    //             Serial.println("'.");
    //         }
    //     #endif
    // }
    
    return header_data;
}

//Gets the raw RequestHeaders
String MiniServ::GetRequestHeaders()
{
    return _requestRawHeader;
}

//Gets the verb of the request
String MiniServ::GetRequestVerb()
{
    return _requestVerb;
}

//Gets the path of the client request
String MiniServ::GetRequestPath()
{
    return _requestPath;
}

//Writes the response header based on the response code
void MiniServ::WriteResponseHeader(int responseCode)
{
    // switch (responseCode)
    // {
    //     case 404:
    //         //Not found
    //         _client.println("HTTP/1.1 404 Not Found");
    //         break;
    //     default:
    //         //default to 200 OK
    //         _client.println("HTTP/1.1 200 OK");
    //         break;
    // }

    // _client.println("Content-type:text/html");
    // _client.println("Connection: close");
    // _client.println();
}

//Sends an HTML response to the Web Client
void MiniServ::PrintWebClientResponse(String htmlBody)
{
    _WiFiServer.send(200, "text/html", htmlBody);

    // WriteResponseHeader();

    // //Write body
    // _client.println(htmlBody);
    // _client.println();
}

//Sends a file to the Web Client
void MiniServ::PrintFileWebClientResponse(const char *filePath)
{
    //WriteResponseHeader();

    //Write body
    WriteFileWebClient(filePath);

    //flush
    //_client.println();
}

//Sends a 404 Not Found to the Web Client, optionally with a body
void MiniServ::PrintWebClientNotFound(String htmlBody)
{
    _WiFiServer.send(404, "text/html", htmlBody);

    // WriteResponseHeader(404);

    // //Write body (optional)
    // _client.println(htmlBody);
    // _client.println();
}

//Sends a 404 Not Found to the Web Client and the content of a file in body.
void MiniServ::PrintFileWebClientNotFound(const char *filePath)
{
    //WriteResponseHeader(404);

    //Write file
    WriteFileWebClient(filePath);
    //_client.println();
}

// Close the client connection
void MiniServ::SendClientResponse()
{
    //_client.stop();

    #ifdef MINISERV_DEBUGMODE
        if (Serial)
            Serial.println("Closed client connection.");
    #endif
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

//Reads the contents of a file as a string
String MiniServ::ReadFileWebClient(const char *filePath)
{
    String ret = "";

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
                    Serial.println("Unable to to open file " + filePath);
            #endif    
        }
        else
        {
            //write content
            if(file.available())
            {
                ret = file.readString();
            }

            //close file
            file.close();

            #ifdef MINISERV_DEBUGMODE
                if (Serial)
                    Serial.println("File read: " + filePath);
            #endif                
        }
    }

    return ret;
}

//Writes a file to the client
void MiniServ::WriteFileWebClient(const char *filePath)
{
    PrintWebClientResponse(ReadFileWebClient(filePath));

    #ifdef MINISERV_DEBUGMODE
        if (Serial)
            Serial.println("File sent to client: " + filePath);
    #endif                
}
