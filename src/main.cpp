#include "memo_lib.h"

#include "platform.h"

#define APIENTRY
#include "glcorearb.h"

/*
Windows Plaform
*/
#ifdef _WIN32
#include "win32_platform.cpp"
#endif

#include "gl_renderer.h" 

int main()
{
    platform_create_window(1200,720,"Hellow");
    while(running)
    {
        //Update
        platform_update_window();
        
    }
    return 0;
}