// OpenGL Module

#pragma once

//-----------------------------------------------------------------------------

#pragma comment(lib, "OpenGL32")

#include <Windows.h>
#include <gl/GL.h>

//-----------------------------------------------------------------------------

typedef void (APIENTRY *glBeginFn)(GLenum);
typedef void (APIENTRY *glColor4fFn)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (*V_RenderViewFn)(void);

void InitOpenGLModule();

void ReleaseOpenGLModule();