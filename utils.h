//////////////////////////////////////////////////////////////////////////////
//
//  --- utils.h ---
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __UTILS_H__
#define __UTILS_H__

#include "include/GLEW/glew.h"
#include "include/GLFW/glfw3.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

//----------------------------------------------------------------------------
//
//  LoadShaders() takes an array of ShaderFile structures, each of which
//    contains the type of the shader, and a pointer a C-style character
//    string (i.e., a NULL-terminated array of characters) containing the
//    entire shader source.
//
//  The array of structures is terminated by a final Shader with the
//    "type" field set to GL_NONE.
//
//  LoadShaders() returns the shader program value (as returned by
//    glCreateProgram()) on success, or zero on failure. 
//

typedef struct {
    GLenum       type;
    const char*  filename;
    GLuint       shader;
} ShaderInfo;

GLuint LoadShaders( ShaderInfo* );

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  CreateWindow() creates an OpenGL window using GLFW.
//
//  CreateWindow() returns a pointer to a GLFW window with an OpenGL context. 
//

GLFWwindow* CreateWindow( const char *name );

//----------------------------------------------------------------------------


#ifdef __cplusplus
};
#endif // __cplusplus

#endif // __LOAD_SHADERS_H__
