#pragma once

#include <stdio.h>
//This is to get malloc
#include <stdlib.h>
//This is to get memeset
#include <string.h>
//This is to get the edit timestamp of the file
#include <sys/stat.h>

/*
Defines
*/
#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#elif _linux_
#define DEBUG_BREAK() __builtin_debugtrap()
#elif _APPLE_
#define DEBUG_BREAK() __builtin_trap()
#endif
/*
Loggings
*/
enum TextColor
{
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,  
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,  
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT
};

template<typename ...Args>

 void _log(char* prefix, char* msg, TextColor  textcolor, Args... args)
 {
    static char* TextColorTable[TEXT_COLOR_COUNT]=
    {
        "\x1b[30m", //Black
        "\x1b[31m", //Red
        "\x1b[32m", //Green
        "\x1b[33m", //Yellow
        "\x1b[34m", //Blue
        "\x1b[35m", //Magenta
        "\x1b[36m", //Cyan
        "\x1b[37m", //White
        "\x1b[90m", //Bright Black
        "\x1b[91m", //Bright Red
        "\x1b[92m", //Bright Green
        "\x1b[93m", //Bright Yellow
        "\x1b[94m", //Bright Blue
        "\x1b[95m", //Bright Magenta
        "\x1b[96m", //Bright Cyan
        "\x1b[97m", //Bright White

    };
    char formatBuffer[8192]={};
    sprintf(formatBuffer, "%s %s %s" ,TextColorTable[textcolor],prefix, msg);
    

    char textBuffer[8192]={};
    sprintf(textBuffer, formatBuffer, args...);

    puts(textBuffer); 
 }

 #define SM_TRACE(msg, ...) _log("TRACE: ", msg,TEXT_COLOR_GREEN ,##__VA_ARGS__);
 #define SM_WARN(msg, ...) _log("WARN: ", msg,TEXT_COLOR_YELLOW ,##__VA_ARGS__);
 #define SM_ERROR(msg, ...) _log("ERROR: ", msg,TEXT_COLOR_RED ,##__VA_ARGS__);


 #define SM_ASSERT(x, msg, ...)                                 \
 {                                                              \
      if(!(x))                                                  \
      {                                                         \
        SM_ERROR(msg, ##__VA_ARGS__);                           \
        DEBUG_BREAK();                                          \
        SM_ERROR("Assertion Hit!")                              \
      }                                                         \
 }


 /*
 Bump Allocator
 */
struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char* memory;
};

BumpAllocator make_bump_allocator(size_t size)
{
    BumpAllocator ba = {};

    ba.memory= (char*)malloc(size);
    if(ba.memory)
    {
        ba.capacity = size;
        memset(ba.memory, 0, size);
    }
    else
    {
        SM_ASSERT(false,"Failed to allocate memory!");
    }
    return ba;
}

char* bump_alloc(BumpAllocator* bumpAlloc, size_t size)
{
    char* result = nullptr;

    size_t allignedSize = (size+7)& ~7; //Makes sure that the first 4 bits are 0

    if(bumpAlloc->used + allignedSize <= bumpAlloc->capacity)
    {
        result = bumpAlloc->memory+bumpAlloc->used;
        bumpAlloc->used+=allignedSize;
    }
    else
    {
        SM_ASSERT(false, "BumpAllocator is full!");
    }

    return result;
}

/*
File I/O
*/
long long get_timestamp(char* file)
{
    struct stat file_stat = {};  //Makes empty stat struct
    stat(file, &file_stat);      //Fills the stat struct 
    return file_stat.st_mtime;   //Return the most recent timestamp of change of the file(used for checking texture changes)
}

bool file_exists(char* filePath)
{
    SM_ASSERT(filePath, "No file path was supplied");
    auto file = fopen(filePath, "rb");
    if(!file)
    {
        return false;
    }
    fclose(file);

    return true; 
}

long get_file_size(char* filePath)
{
    SM_ASSERT(filePath, "No file path was supplied");
    long file_size = 0;
    auto file = fopen(filePath, "rb");
    if(!file)
    {
        SM_ERROR("Failed to load file at %s",filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);  //go till the end of the file
    file_size = ftell(file);   //tell the size of the file based on the location
    fseek(file, 0, SEEK_SET);  //go back to the beginning of the file before closing it
    fclose(file);

    return file_size;

}

//READING A FILE

//Reads a file into the supplied buffer, we manage our own memory and thus want more control over where it is allocated

char* read_file(char* filePath, int* fileSize, char* Buffer)
{
    SM_ASSERT(filePath, "No file path was supplied");
    SM_ASSERT(fileSize, "No file size was supplied");
    SM_ASSERT(Buffer, "No buffer was supplied");


    *fileSize = 0;
    auto file = fopen(filePath, "rb");
    if(!file)
    {
        SM_ERROR("Failed to load file at %s", filePath);
        return nullptr;
    }
    fseek(file , 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    memset(Buffer, 0, *fileSize + 1);
    fread(Buffer, sizeof(char*), *fileSize, file);
    fclose(file);

    return Buffer;  
}

char* read_file(char* filePath, int* fileSize, BumpAllocator* bumpAlloc)
{
    char* file = nullptr;
    long fileSize2 = get_file_size(filePath);

    if(fileSize2)
    {
        char* buffer = bump_alloc(bumpAlloc, fileSize2 + 1);

        file = read_file(filePath, fileSize, buffer); 
    }  

    return file;
}

void write_file(char* filePath, char* Buffer, int size)
{
    SM_ASSERT(filePath, "No file path was supplied");
    SM_ASSERT(Buffer, "No buffer was supplied");
    SM_ASSERT(size,"No size was specified");

    auto file = fopen(filePath,"wb");
    if(!file)
    {
        SM_ERROR("Failed to load file at %s", filePath);
        return;
    }
    fwrite(Buffer, sizeof(char),size ,file);
    fclose(file);
}

bool copy_file(char* fileName, char* outputName, char* Buffer)
{
    int fileSize = 0;
    char* data = read_file(fileName, &fileSize, Buffer);

    auto outputFile = fopen(outputName, "wb");
    if(!outputFile)
    {
        SM_ERROR("Failed top open file :%s", outputName);
        return false;
    }

    int result = fwrite(data, sizeof(char), fileSize, outputFile);
    if(!result)
    {
        SM_ERROR("Failed to load file %s", result);
        return false;
    }
    fclose(outputFile);
}
bool copy_file(char* fileName, char* outputName, BumpAllocator* bumpAllocator)
{
    char* file = 0;
    long fileSize2 = get_file_size(fileName);

    if(fileSize2)
    {
        char* buffer = bump_alloc(bumpAllocator, fileSize2 + 1);
        return copy_file(fileName, outputName, buffer);
    }
    return false;
}