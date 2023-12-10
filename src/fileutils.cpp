//+--------------------------------------------------------------------------
//
// File:        fileutils.cpp
//
// Description: The purpose of this file is to provide utility functions
//              for various filesystem support.
//
// History:     2023-12-03    PP Laplante   Created
//
//
//---------------------------------------------------------------------------

#include <fileutils.h>
#include <SPIFFS.h>
#include <arduinoutils.h>

//Reads the contents of a file as a string, or empty string on error
String FSReadFile(String filePath)
{
    String ret = "";

    if(!SPIFFS.begin(true)){
            #ifdef FILEUTILS_DEBUGMODE
                PrintlnSerial("An error occured mounting SPIFFS");
            #endif      
    }
    else
    {    
        //open file
        File file = SPIFFS.open(filePath);

        if(!file)
        {
            #ifdef FILEUTILS_DEBUGMODE
                PrintSerial("Unable to open file: ");
                PrintlnSerial(filePath);
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

            #ifdef FILEUTILS_DEBUGMODE
                PrintSerial("Read file: ");
                PrintlnSerial(filePath);
            #endif
        }
    }

    return ret;
}

//Writes a string to a file, returns file size or -1 if failed
int FSWriteFile(String filePath, String fileData)
{
    //default to fail
    int ret = -1;

    if(!SPIFFS.begin(true))
    {
        #ifdef FILEUTILS_DEBUGMODE
            PrintlnSerial("An error occured mounting SPIFFS");
        #endif      
    }
    else
    {  
        //delete file if already exists
        if (SPIFFS.exists(filePath))
        {
            SPIFFS.remove(filePath);

            #ifdef FILEUTILS_DEBUGMODE
                PrintSerial("Deleted file: ");
                PrintlnSerial(filePath);
            #endif
        }

        //create file
        File file = SPIFFS.open(filePath, FILE_WRITE);

        if (!file)
        {
            #ifdef FILEUTILS_DEBUGMODE
                PrintSerial("Unable to open file: ");
                PrintlnSerial(filePath);
            #endif
        }
        else
        {
            #ifdef FILEUTILS_DEBUGMODE
                PrintSerial("Created file: ");
                PrintlnSerial(filePath);
            #endif

            //Write to file
            file.print(fileData);
            file.flush();
            
            //note number of bytes written
            ret = file.size();
            file.close();
        }
    }

    //return status
    return ret;
}

//Deletes a file, returns True if successful
bool FSDeleteFile(String filePath)
{
    //assume operation will fail.
    bool ret = false;

    if(!SPIFFS.begin(true))
    {
        #ifdef FILEUTILS_DEBUGMODE
            PrintlnSerial("An Error has occurred while mounting SPIFFS");
        #endif
    }
    else
    {
        //delete file if it exists
        if (SPIFFS.exists(filePath))
        {
            ret = SPIFFS.remove(filePath);

            #ifdef FILEUTILS_DEBUGMODE
                if (ret)
                    PrintlnSerial("File deleted: " + filePath);
                else
                    PrintlnSerial("ERROR deleting file: " + filePath);
            #endif
        }
        else
        {
            #ifdef FILEUTILS_DEBUGMODE
                PrintlnSerial("File not deleted, file not found: " + filePath);
            #endif

            //Consider a success even if the file did not exist initially
            ret = true;
        }
    }

    //return result
    return ret;
}

//Test if a file exists
bool FSFileExists(String filePath)
{
    //assume operation will fail.
    bool ret = false;

    if(!SPIFFS.begin(true))
    {
        #ifdef FILEUTILS_DEBUGMODE
            PrintlnSerial("An Error has occurred while mounting SPIFFS");
        #endif
        ret = false;
    }
    else
    {
        ret = SPIFFS.exists(filePath);
    }

    return ret;
}

