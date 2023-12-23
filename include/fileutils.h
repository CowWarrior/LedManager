#ifndef fileutils_h
#define fileutils_h

#include <Arduino.h>

//uncomment next line to enable debugging
//#define FILEUTILS_DEBUGMODE 1

//Reads the contents of a file as a string, or empty string on error
String FSReadFile(String filePath);

//Writes a string to a file, returns file size or -1 if failed
int FSWriteFile(String filePath, String fileData);

//Deletes a file, returns True if successful
bool FSDeleteFile(String filePath);

//Test if a file exists
bool FSFileExists(String filePath);

#endif