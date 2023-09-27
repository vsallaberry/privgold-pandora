/* GENERATED FILE: vs_lib_syms_gl.h */
#ifndef FREEGLUT_LIB_SYMS_GL_H
#define FREEGLUT_LIB_SYMS_GL_H

#include "config.h"
#ifndef VS_GL_SO_NAME
int vs_gl_lib_init();
void vs_gl_lib_destroy();
#else

#if defined(__APPLE__) || defined(MACOSX)
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#else
//#define __glext_h_
    #include <GL/gl.h>
    #include <GL/glext.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
# if !defined(WIN32)
    # define GLX_GLXEXT_PROTOTYPES 1
    # define GLX_GLXEXT_LEGACY 1
    # include <GL/glx.h>
    # include <GL/glext.h>
# endif
//#undef __glext_h_
#endif

//!! NOT FOUND: glActiveTextureARB_p
typedef    void          (*vs_glAlphaFunc_fun)( GLenum func, GLclampf ref );
//!! NOT FOUND: glAttachShader_p
typedef         void     (*vs_glBegin_fun)( GLenum mode );
//!! NOT FOUND: glBindBufferARB_p
typedef    void          (*vs_glBindTexture_fun)( GLenum target, GLuint texture );
typedef    void          (*vs_glBlendFunc_fun)( GLenum sfactor, GLenum dfactor );
//!! NOT FOUND: glBufferDataARB_p
typedef    void          (*vs_glCallList_fun)( GLuint list );
typedef    void          (*vs_glClear_fun)( GLbitfield mask );
typedef    void          (*vs_glClearColor_fun)( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
typedef         void     (*vs_glClearDepth_fun)( GLclampd depth );
//!! NOT FOUND: glClientActiveTextureARB_p
typedef   void           (*vs_glColor3f_fun)( GLfloat red, GLfloat green, GLfloat blue );
typedef   void           (*vs_glColor4f_fun)( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
typedef   void           (*vs_glColor4fv_fun)( const GLfloat *v );
typedef    void          (*vs_glColorMask_fun)( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha );
typedef    void          (*vs_glColorMaterial_fun)( GLenum face, GLenum mode );
//!! NOT FOUND: glColorTable_p
//!! NOT FOUND: glCompileShader_p
//!! NOT FOUND: glCompressedTexImage2D_p
//!! NOT FOUND: glCreateProgram_p
//!! NOT FOUND: glCreateShader_p
typedef    void          (*vs_glCullFace_fun)( GLenum mode );
//!! NOT FOUND: glDeleteBuffersARB_p
typedef    void          (*vs_glDeleteLists_fun)( GLuint list, GLsizei range );
//!! NOT FOUND: glDeleteProgram_p
//!! NOT FOUND: glDeleteShader_p
typedef    void          (*vs_glDeleteTextures_fun)( GLsizei n, const GLuint *textures);
typedef    void          (*vs_glDepthFunc_fun)( GLenum func );
typedef    void          (*vs_glDepthMask_fun)( GLboolean flag );
typedef    void          (*vs_glDisable_fun)( GLenum cap );
typedef    void          (*vs_glDrawArrays_fun)( GLenum mode, GLint first, GLsizei count );
typedef    void          (*vs_glDrawElements_fun)( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );
typedef    void          (*vs_glEnable_fun)( GLenum cap );
typedef     void         (*vs_glEnableClientState_fun)( GLenum cap );
typedef    void          (*vs_glEnd_fun)( void );
typedef    void          (*vs_glEndList_fun)( void );
typedef    void          (*vs_glFinish_fun)( void );
typedef    void          (*vs_glFlush_fun)( void );
typedef         void     (*vs_glFogf_fun)( GLenum pname, GLfloat param );
typedef    void          (*vs_glFogfv_fun)( GLenum pname, const GLfloat *params );
typedef    void          (*vs_glFogi_fun)( GLenum pname, GLint param );
//!! NOT FOUND: glGenBuffersARB_p
typedef    GLuint        (*vs_glGenLists_fun)( GLsizei range );
typedef       void       (*vs_glGenTextures_fun)( GLsizei n, GLuint *textures );
typedef    GLenum        (*vs_glGetError_fun)( void );
typedef    void          (*vs_glGetFloatv_fun)( GLenum pname, GLfloat *params );
typedef    void          (*vs_glGetIntegerv_fun)( GLenum pname, GLint *params );
//!! NOT FOUND: glGetProgramInfoLog_p
//!! NOT FOUND: glGetProgramiv_p
//!! NOT FOUND: glGetShaderInfoLog_p
//!! NOT FOUND: glGetShaderiv_p
typedef    const GLubyte * (*vs_glGetString_fun)( GLenum name );
//!! NOT FOUND: glGetUniformLocation_p
typedef    void          (*vs_glInterleavedArrays_fun)( GLenum format, GLsizei stride, const GLvoid *pointer );
typedef    GLboolean     (*vs_glIsEnabled_fun)( GLenum cap );
typedef         GLboolean (*vs_glIsList_fun)( GLuint list );
typedef    void          (*vs_glLightf_fun)( GLenum light, GLenum pname, GLfloat param );
typedef   void           (*vs_glLightfv_fun)( GLenum light, GLenum pname, const GLfloat *params );
typedef   void           (*vs_glLightModelfv_fun)( GLenum pname, const GLfloat *params );
typedef   void           (*vs_glLightModeli_fun)( GLenum pname, GLint param );
typedef    void          (*vs_glLineWidth_fun)( GLfloat width );
//!! NOT FOUND: glLinkProgram_p
typedef    void          (*vs_glLoadIdentity_fun)( void );
typedef    void          (*vs_glLoadMatrixd_fun)( const GLdouble *m );
typedef   void           (*vs_glLoadMatrixf_fun)( const GLfloat *m );
//!! NOT FOUND: glLockArraysEXT_p
//!! NOT FOUND: glMapBufferARB_p
typedef   void           (*vs_glMaterialfv_fun)( GLenum face, GLenum pname, const GLfloat *params );
typedef         void     (*vs_glMatrixMode_fun)( GLenum mode );
//!! NOT FOUND: glMultiDrawArrays_p
//!! NOT FOUND: glMultiDrawElements_p
//!! NOT FOUND: glMultiTexCoord2fARB_p
//!! NOT FOUND: glMultiTexCoord4fARB_p
typedef    void          (*vs_glNewList_fun)( GLuint list, GLenum mode );
typedef   void           (*vs_glNormal3f_fun)( GLfloat nx, GLfloat ny, GLfloat nz );
typedef   void           (*vs_glNormal3fv_fun)( const GLfloat *v );
//!! NOT FOUND: globalWindowManagerPtr
//!! NOT FOUND: gl_options
typedef   void           (*vs_glPixelStorei_fun)( GLenum pname, GLint param );
typedef    void          (*vs_glPointSize_fun)( GLfloat size );
typedef    void          (*vs_glPolygonMode_fun)( GLenum face, GLenum mode );
typedef    void          (*vs_glPolygonOffset_fun)( GLfloat factor, GLfloat units );
typedef    void          (*vs_glPopMatrix_fun)( void );
typedef    void          (*vs_glPrioritizeTextures_fun)( GLsizei n, const GLuint *textures, const GLclampf *priorities );
typedef    void          (*vs_glPushMatrix_fun)( void );
typedef   void           (*vs_glRasterPos2f_fun)( GLfloat x, GLfloat y );
typedef    void          (*vs_glReadPixels_fun)( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );
typedef   void           (*vs_glRectf_fun)( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
typedef    void          (*vs_glScaled_fun)( GLdouble x, GLdouble y, GLdouble z );
typedef   void           (*vs_glScalef_fun)( GLfloat x, GLfloat y, GLfloat z );
typedef    void          (*vs_glScissor_fun)( GLint x, GLint y, GLsizei width, GLsizei height);
typedef        void      (*vs_glShadeModel_fun)( GLenum mode );
//!! NOT FOUND: glShaderSource_p
typedef        void      (*vs_glStencilFunc_fun)( GLenum func, GLint ref, GLuint mask );
typedef    void          (*vs_glStencilMask_fun)( GLuint mask );
typedef    void          (*vs_glStencilOp_fun)( GLenum fail, GLenum zfail, GLenum zpass );
//!! NOT FOUND: glswap_count
typedef   void           (*vs_glTexCoord2f_fun)( GLfloat s, GLfloat t );
typedef    void          (*vs_glTexCoordPointer_fun)( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr );
typedef    void          (*vs_glTexEnvfv_fun)( GLenum target, GLenum pname, const GLfloat *params );
typedef   void           (*vs_glTexEnvi_fun)( GLenum target, GLenum pname, GLint param );
typedef   void           (*vs_glTexGenfv_fun)( GLenum coord, GLenum pname, const GLfloat *params );
typedef   void           (*vs_glTexGeni_fun)( GLenum coord, GLenum pname, GLint param );
typedef    void          (*vs_glTexImage2D_fun)( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
typedef     void         (*vs_glTexParameterf_fun)( GLenum target, GLenum pname, GLfloat param );
typedef    void          (*vs_glTexParameterfv_fun)( GLenum target, GLenum pname, const GLfloat *params );
typedef   void           (*vs_glTexParameteri_fun)( GLenum target, GLenum pname, GLint param );
typedef     void         (*vs_glTexSubImage2D_fun)( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
typedef    void          (*vs_glTranslated_fun)( GLdouble x, GLdouble y, GLdouble z );
typedef   void           (*vs_glTranslatef_fun)( GLfloat x, GLfloat y, GLfloat z );
typedef   GLint          (*vs_gluBuild2DMipmaps_fun) (GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data);
typedef   const GLubyte * (*vs_gluErrorString_fun) (GLenum error);
//!! NOT FOUND: glUniform1f_p
//!! NOT FOUND: glUniform1fv_p
//!! NOT FOUND: glUniform1i_p
//!! NOT FOUND: glUniform1iv_p
//!! NOT FOUND: glUniform2f_p
//!! NOT FOUND: glUniform2fv_p
//!! NOT FOUND: glUniform2i_p
//!! NOT FOUND: glUniform2iv_p
//!! NOT FOUND: glUniform3f_p
//!! NOT FOUND: glUniform3fv_p
//!! NOT FOUND: glUniform3i_p
//!! NOT FOUND: glUniform3iv_p
//!! NOT FOUND: glUniform4f_p
//!! NOT FOUND: glUniform4fv_p
//!! NOT FOUND: glUniform4i_p
//!! NOT FOUND: glUniform4iv_p
//!! NOT FOUND: glUnlockArraysEXT_p
//!! NOT FOUND: glUnmapBufferARB_p
//!! NOT FOUND: glUseProgram_p
typedef   void           (*vs_glVertex2f_fun)( GLfloat x, GLfloat y );
typedef    void          (*vs_glVertex3d_fun)( GLdouble x, GLdouble y, GLdouble z );
typedef   void           (*vs_glVertex3f_fun)( GLfloat x, GLfloat y, GLfloat z );
typedef   void           (*vs_glVertex3fv_fun)( const GLfloat *v );
typedef    void          (*vs_glViewport_fun)( GLint x, GLint y, GLsizei width, GLsizei height );
#ifdef HAVE_GLXGETPROCADDRESSARB
typedef  __GLXextFuncPtr (*vs_glXGetProcAddressARB_fun) (const GLubyte *);
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern vs_glAlphaFunc_fun                   vs_glAlphaFunc;
extern vs_glBegin_fun                       vs_glBegin;
extern vs_glBindTexture_fun                 vs_glBindTexture;
extern vs_glBlendFunc_fun                   vs_glBlendFunc;
extern vs_glCallList_fun                    vs_glCallList;
extern vs_glClear_fun                       vs_glClear;
extern vs_glClearColor_fun                  vs_glClearColor;
extern vs_glClearDepth_fun                  vs_glClearDepth;
extern vs_glColor3f_fun                     vs_glColor3f;
extern vs_glColor4f_fun                     vs_glColor4f;
extern vs_glColor4fv_fun                    vs_glColor4fv;
extern vs_glColorMask_fun                   vs_glColorMask;
extern vs_glColorMaterial_fun               vs_glColorMaterial;
extern vs_glCullFace_fun                    vs_glCullFace;
extern vs_glDeleteLists_fun                 vs_glDeleteLists;
extern vs_glDeleteTextures_fun              vs_glDeleteTextures;
extern vs_glDepthFunc_fun                   vs_glDepthFunc;
extern vs_glDepthMask_fun                   vs_glDepthMask;
extern vs_glDisable_fun                     vs_glDisable;
extern vs_glDrawArrays_fun                  vs_glDrawArrays;
extern vs_glDrawElements_fun                vs_glDrawElements;
extern vs_glEnable_fun                      vs_glEnable;
extern vs_glEnableClientState_fun           vs_glEnableClientState;
extern vs_glEnd_fun                         vs_glEnd;
extern vs_glEndList_fun                     vs_glEndList;
extern vs_glFinish_fun                      vs_glFinish;
extern vs_glFlush_fun                       vs_glFlush;
extern vs_glFogf_fun                        vs_glFogf;
extern vs_glFogfv_fun                       vs_glFogfv;
extern vs_glFogi_fun                        vs_glFogi;
extern vs_glGenLists_fun                    vs_glGenLists;
extern vs_glGenTextures_fun                 vs_glGenTextures;
extern vs_glGetError_fun                    vs_glGetError;
extern vs_glGetFloatv_fun                   vs_glGetFloatv;
extern vs_glGetIntegerv_fun                 vs_glGetIntegerv;
extern vs_glGetString_fun                   vs_glGetString;
extern vs_glInterleavedArrays_fun           vs_glInterleavedArrays;
extern vs_glIsEnabled_fun                   vs_glIsEnabled;
extern vs_glIsList_fun                      vs_glIsList;
extern vs_glLightf_fun                      vs_glLightf;
extern vs_glLightfv_fun                     vs_glLightfv;
extern vs_glLightModelfv_fun                vs_glLightModelfv;
extern vs_glLightModeli_fun                 vs_glLightModeli;
extern vs_glLineWidth_fun                   vs_glLineWidth;
extern vs_glLoadIdentity_fun                vs_glLoadIdentity;
extern vs_glLoadMatrixd_fun                 vs_glLoadMatrixd;
extern vs_glLoadMatrixf_fun                 vs_glLoadMatrixf;
extern vs_glMaterialfv_fun                  vs_glMaterialfv;
extern vs_glMatrixMode_fun                  vs_glMatrixMode;
extern vs_glNewList_fun                     vs_glNewList;
extern vs_glNormal3f_fun                    vs_glNormal3f;
extern vs_glNormal3fv_fun                   vs_glNormal3fv;
extern vs_glPixelStorei_fun                 vs_glPixelStorei;
extern vs_glPointSize_fun                   vs_glPointSize;
extern vs_glPolygonMode_fun                 vs_glPolygonMode;
extern vs_glPolygonOffset_fun               vs_glPolygonOffset;
extern vs_glPopMatrix_fun                   vs_glPopMatrix;
extern vs_glPrioritizeTextures_fun          vs_glPrioritizeTextures;
extern vs_glPushMatrix_fun                  vs_glPushMatrix;
extern vs_glRasterPos2f_fun                 vs_glRasterPos2f;
extern vs_glReadPixels_fun                  vs_glReadPixels;
extern vs_glRectf_fun                       vs_glRectf;
extern vs_glScaled_fun                      vs_glScaled;
extern vs_glScalef_fun                      vs_glScalef;
extern vs_glScissor_fun                     vs_glScissor;
extern vs_glShadeModel_fun                  vs_glShadeModel;
extern vs_glStencilFunc_fun                 vs_glStencilFunc;
extern vs_glStencilMask_fun                 vs_glStencilMask;
extern vs_glStencilOp_fun                   vs_glStencilOp;
extern vs_glTexCoord2f_fun                  vs_glTexCoord2f;
extern vs_glTexCoordPointer_fun             vs_glTexCoordPointer;
extern vs_glTexEnvfv_fun                    vs_glTexEnvfv;
extern vs_glTexEnvi_fun                     vs_glTexEnvi;
extern vs_glTexGenfv_fun                    vs_glTexGenfv;
extern vs_glTexGeni_fun                     vs_glTexGeni;
extern vs_glTexImage2D_fun                  vs_glTexImage2D;
extern vs_glTexParameterf_fun               vs_glTexParameterf;
extern vs_glTexParameterfv_fun              vs_glTexParameterfv;
extern vs_glTexParameteri_fun               vs_glTexParameteri;
extern vs_glTexSubImage2D_fun               vs_glTexSubImage2D;
extern vs_glTranslated_fun                  vs_glTranslated;
extern vs_glTranslatef_fun                  vs_glTranslatef;
extern vs_gluBuild2DMipmaps_fun             vs_gluBuild2DMipmaps;
extern vs_gluErrorString_fun                vs_gluErrorString;
extern vs_glVertex2f_fun                    vs_glVertex2f;
extern vs_glVertex3d_fun                    vs_glVertex3d;
extern vs_glVertex3f_fun                    vs_glVertex3f;
extern vs_glVertex3fv_fun                   vs_glVertex3fv;
extern vs_glViewport_fun                    vs_glViewport;
#ifdef HAVE_GLXGETPROCADDRESSARB
extern vs_glXGetProcAddressARB_fun          vs_glXGetProcAddressARB;
#endif

int     vs_gl_lib_init();
void    vs_gl_lib_destroy();
void *  vs_gl_lib_findsymbol(const char * name);

#ifdef __cplusplus
}
#endif

#endif /* ! ifdef VS_GL_SO_NAME */
#endif /* ! ifdef FREEGLUT_LIB_SYMS_GL_H */

