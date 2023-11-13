#ifndef MiniServ_h
#define MiniServ_h

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

class MiniServ
{
public:
    //Constructor
    MiniServ();

    //shortcut to initialize Wifi on instantiate
    MiniServ(String ssid, String password, int timeout_ms=60000);

    //initialize Wifi and attempt to connect
    void InitWiFi(String ssid, String password, int timeout_ms=60000);

    //checks if the WiFi connection has been established
    bool IsWiFiConnected();

    //initializes the webserver
    void InitWebServer(int port=80, int client_timeout=2000);

    //Checks if there is an inbound client request
    bool IsClientRequest();

    //Gets the raw response headers
    String GetRequestHeaders();

    //Gets the verb of the request
    String GetRequestVerb();

    //Gets the path of the client request
    String GetRequestPath();

    //Sends an HTML response to the Web Client
    void PrintWebClientResponse(String htmlBody);

    //Sends a file to the Web Client
    void PrintFileWebClientResponse(const char *filePath);

    //Sends a 404 Not Found to the Web Client, optionally with a body
    void PrintWebClientNotFound(String htmlBody="");

    //Sends a 404 Not Found to the Web Client and the content of a file in body.
    void PrintFileWebClientNotFound(const char *filePath);

    // Close the client connection
    void SendClientResponse();

    // //Handle incoming web request with function
    // void on(const char *uri, THandlerFunction fn);

    // //Handle incoming web request with function, specifying method
    // void on(const char *uri, const char *method, THandlerFunction fn);

    // //Handle incoming web request with function, specifying method and file upload handling function
    // void on(const char *uri, const char *method, THandlerFunction fn, THandlerFunction uploadfn); 

private:
    String _SSID="";
    String _password="";
    int _port=80;
    int _clientTimeout=2000;
    bool _isWiFiConnected = false;
    //WiFiServer _WiFiServer;
    //WiFiClient _client;
    WebServer _WiFiServer; // = {80};
    String _requestRawHeader="";
    String _requestVerb="";
    String _requestPath="";

    //Writes the contents of a file in the response
    void WriteFileWebClient(const char *filePath);
    
    //Reads the contents of a file as a string
    String ReadFileWebClient(const char *filePath);

    //Reads any client requests the server might have received
    void GetWebClientRequest();

    //Reads the client request header
    String ReadRawRequestHeader();

    //Parse raw headers to get path
    String ParseRequestHeaderPath(String headers);

    //Parse raw headers to get verb
    String ParseRequestHeaderVerb(String headers);

    //Write response header
    void WriteResponseHeader(int responseCode=200);
};

#endif