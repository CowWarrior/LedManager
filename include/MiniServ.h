#ifndef MiniServ_h
#define MiniServ_h

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

// Toggle Debug Mode here
#define MINISERV_DEBUGMODE 1

class MiniServ
{
public:
    //Public Members
    WebServer WServer;

    //Constructor
    MiniServ();

    //shortcut to initialize Wifi on instantiate
    MiniServ(String ssid, String password, int timeout_ms=60000, String hostname="");

    //initialize Wifi and attempt to connect
    void InitWiFi(String ssid, String password, int timeout_ms=60000, String hostname="");

    //checks if the WiFi connection has been established
    bool IsWiFiConnected();

    //initializes the webserver
    void InitWebServer(int port=80, int client_timeout=2000);

    //Checks if there is an inbound client request
    void HandleClientRequests();

    //Gets the raw response headers
    String GetRequestHeaders();

    //Gets the verb of the request
    String GetRequestVerb();

    //Gets the path of the client request
    String GetRequestPath();

    //Gets the value of a uery string parameter by name
    String GetQueryStringParameter(String paramName);

    //Gets the value of a uery string parameter by index
    String GetQueryStringParameter(int paramIndex);

    //Sends an HTML response to the Web Client
    void SendResponse(String htmlBody);

    //Sends an HTML response to the Web Client
    void SendResponse(String htmlBody, int responseCode, String contentType);

    //Sends a file to the Web Client
    void SendFileResponse(const char *filePath);

    //Sends a file to the Web Client
    void SendFileResponse(const char *filePath, int responseCode, String contentType);

    //Sends a binary file to the Web Client
    void SendBinaryFileResponse(const char *filePath);

    //Sends a binary file to the Web Client
    void SendBinaryFileResponse(const char *filePath, int responseCode, String contentType);

    //Sends a 404 Not Found to the Web Client, optionally with a body
    void SendNotFound(String htmlBody="");

    //Sends a 404 Not Found to the Web Client and the content of a file in body.
    void SendFileNotFound(const char *filePath);

    //Saves an uploaded file to the file system
    void SaveFileUpload();

    //Saves an uploaded file to the file system as a specific name
    void SaveFileUploadAs(String filePath);

private:
    String _SSID="";
    String _password="";
    int _port=80;
    int _clientTimeout=2000;
    bool _isWiFiConnected = false;
    File uploadFile;
    
    //Parse raw headers to get path
    String ParseRequestHeaderPath(String headers);

    //Parse raw headers to get verb
    String ParseRequestHeaderVerb(String headers);

    //Get content type based on file extension
    String GetContentType(String filePath);

    //Read request headers from client
    //String ReadRawRequestHeader();
};

#endif