#include "platform.h"
#include "memo_lib.h"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "wglext.h"
#include <glcorearb.h>

/*
Window Globals
*/
static HWND window;
static HDC dc;
/*
Platform Implementations
*/
LRESULT CALLBACK windows_window_callback(HWND window, UINT msg, 
                                         WPARAM wParma, LPARAM lParam)
{
    LRESULT result=0;
    switch(msg)
    {
        case WM_CLOSE:
        {
            running=false;
            break;
        }
        case WM_SIZE:
        {
            RECT rect = {};
            GetClientRect(window, &rect);
            input.screenSizeX = rect.right - rect.left;
            input.screenSizeY = rect.bottom - rect.top;

            break;
        }
        default:
        {
            //Let windows handle the default input for now
            result = DefWindowProcA(window,msg, wParma, lParam);
            break;
        }
    }
    return result;
}

bool platform_create_window(int width, int height,char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA wc ={};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(instance, IDC_ARROW); //Defines the look of the cursor
    wc.lpszClassName = title;// This is an identifier(ID)
    wc.lpfnWndProc = windows_window_callback;//Calls back inputs into the window

    if(!RegisterClassA(&wc))
    {
        return false;
    }

    //WS_CAPTION| WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
    int dwStyle= WS_OVERLAPPEDWINDOW;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
    //Fake Window initializing opengl
    {
        window = CreateWindowExA(0, title, //This is the reference to the lpszClassName
                                        title, //This is the actual title
                                        dwStyle,
                                        100,
                                        100,
                                        width,
                                        height,
                                        NULL, //Parent
                                        NULL, //Menu
                                        instance, 
                                        NULL); //lpParam
        if(window==NULL)
        {
            SM_ASSERT(false, "Failed to create Windows Window")
            return false;
        }

        HDC fakeDC = GetDC(window);
        if(!fakeDC)
        {
            SM_ASSERT(false, "Failed to get HDC");
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(fakeDC,&pfd);
        if(!pixelFormat)
        {
            SM_ASSERT(false, "Failed to choose pixel format");
            return false;
        }

        if(!SetPixelFormat(fakeDC, pixelFormat, &pfd))
        {
            SM_ASSERT(false, "Faile dto set pixel format");
            return false;
        }
        //A handle to a fake opengl rendering context
        HGLRC fakeRC = wglCreateContext(fakeDC);
        if(!fakeRC)
        {
            SM_ASSERT(false, "Failed to create Render context");
            return false;
        }

        if(!wglMakeCurrent(fakeDC,fakeRC))
        {
            SM_ASSERT(false, "Failed to make current context");
            return false;
        }

        wglChoosePixelFormatARB = 
            (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB = 
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");
        if(!wglChoosePixelFormatARB || !wglCreateContextAttribsARB)
        {
            SM_ASSERT(false, "Failed to load opengl functions");
            return false;
        }        


        //Clean up the fake window
        wglMakeCurrent(fakeDC,0);
        wglDeleteContext(fakeRC);
        ReleaseDC(window,fakeDC);

        //Cannot use same device context, due to single call of setpixelformat
        DestroyWindow(window);

        //Actual OpenGl initialization
        {
            //Init the border dimensions
            {
                RECT borderRect = {};
                AdjustWindowRectEx(&borderRect, dwStyle, 0, 0);

                width+=borderRect.right - borderRect.left;
                height+=borderRect.bottom-borderRect.top;
            }
            window = CreateWindowExA(0, title, //This is the reference to the lpszClassName
                                        title, //This is the actual title
                                        dwStyle,
                                        100,
                                        100,
                                        width,
                                        height,
                                        NULL, //Parent
                                        NULL, //Menu
                                        instance, 
                                        NULL); //lpParam
            if(window==NULL)
            {
                SM_ASSERT(false, "Failed to create Windows Window")
                return false;
            }


            dc = GetDC(window);
            if(!dc)
            {
                SM_ASSERT(false, "Failed to get device context");
                return false;
            }

            const int pixelAttribs[]=
            {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
                WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
                WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
                WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
                WGL_COLOR_BITS_ARB,     32,
                WGL_ALPHA_BITS_ARB,     8,
                WGL_DEPTH_BITS_ARB,     24,
                0 //Terminate with 0 to aviod opengl throwing error
            };


            UINT numPixelFormats;
            int pixelFormat = 0;
            if(!wglChoosePixelFormatARB(dc, pixelAttribs, 
                                        0, //Float list
                                        1, //Max Formats
                                        &pixelFormat,
                                        &numPixelFormats))
            {
                SM_ASSERT(0, "Failed to wglChoosePixelFormatARB");
                return false;
            }
            PIXELFORMATDESCRIPTOR pfd = {0};
            DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

            if(!SetPixelFormat(dc, pixelFormat, &pfd))
            {
                SM_ASSERT(0, "Failed to  SetPixelFormat");
                return false;
            }

            const int contextAttribs[] = 
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
                0 //Terminate the array to prevent opengl error
            };

            HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
            if(!rc)
            {
                SM_ASSERT(false, "Failed to create Render context for opengl");
                return false;
            }
            if(!wglMakeCurrent(dc, rc))
            {
                SM_ASSERT(false, "Failed to make current context");
                return false;
            }


        }

    }                
    ShowWindow(window, SW_SHOW);

    return true;                
}
void platform_update_window()
{
    MSG msg;
    while(PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg); // Calls the callback specified when creating the window
    }
}

void* platform_load_gl_function(char* funName)
{
    PROC proc = wglGetProcAddress(funName);
    if(!proc)
    {
        static HMODULE openglDLL = LoadLibraryA("opengl32.dll");
        proc = GetProcAddress(openglDLL,funName);
        if(!proc)
        {
            SM_ASSERT(false, "Failed to load OpenGL function %s", "glCreateProgram");
            return nullptr;
        }
    }
    return (void*)proc;
}

void platform_swap_buffer()
{
    SwapBuffers(dc);
}