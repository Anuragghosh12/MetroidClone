#include "gl_renderer.h"
#include <glcorearb.h>

/*
OpenGL Structs
*/
struct GLContext
{
    GLuint programID;
};

/*
OpenGL globals
*/
static GLContext glContext;

/*
OpenGL Functions
*/
static void APIENTRY gl_debug_callback(GLenum source, GLenum type,GLuint id, GLenum severity, GLsizei length,
                                        const GLchar* message, const void* user)
{
    if( severity == GL_DEBUG_SEVERITY_LOW ||
        severity == GL_DEBUG_SEVERITY_MEDIUM ||
        severity == GL_DEBUG_SEVERITY_HIGH)
    {
        SM_ASSERT(false, "OpenGL Error: %s", message);
    }
    else
    {
        SM_TRACE((char*)message);
    }
}



bool gl_inti(BumpAllocator* transientStorage)
{
    load_gl_functions();

    glDebugMessageCallback(&gl_debug_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);

    GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    int fileSize = 0;
    char* vertexShader = read_file("assets/shaders/quad.vert", &fileSize, transientStorage);
    char* fragmentShader = read_file("assets/shaders/quad.frag", &fileSize, transientStorage);

    if(!vertexShader || !fragmentShader)
    {
        SM_ASSERT(false, "Failed to load shaders");
        return false;
    }
    glShaderSource(vertShaderID, 1, &vertexShader, 0);
    glShaderSource(fragShaderID, 1, &fragmentShader, 0);
    

    glCompileShader(vertShaderID);
    glCompileShader(fragShaderID);

    //Test to check if the vertex shader compiled successfully
    {
        int success;
        char shaderLog[2048] = {};
        glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(vertShaderID, 2048, 0, shaderLog);
            SM_ASSERT(false, "Failed to compile vertex shaders: %s", shaderLog);
        }
    }
    //Test to check if the fragment shader compiled successfully
    {
        int success;
        char shaderLog[2048] = {};
        glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(fragShaderID, 2048, 0, shaderLog);
            SM_ASSERT(false, "Failed to compile fragment shaders: %s", shaderLog);
        }
    }

    glContext.programID = glCreateProgram();
    glAttachShader(glContext.programID, vertShaderID);
    glAttachShader(glContext.programID, fragShaderID);
    glLinkProgram(glContext.programID);


    glDetachShader(glContext.programID, vertShaderID);
    glDetachShader(glContext.programID, fragShaderID);
    glDeleteShader(vertShaderID);
    glDeleteShader(fragShaderID);


    //Compulsory for displaying using opengl
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);    

    //GL Depth Testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(glContext.programID);


    return true;
}


void gl_render()
{
    glClearColor(119.0f/ 255.0f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, input.screenSizeX, input.screenSizeY);


    glDrawArrays(GL_TRIANGLES, 0, 6);
}