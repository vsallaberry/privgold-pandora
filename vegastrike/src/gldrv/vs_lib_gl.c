/* GENERATED FILE: vs_lib_gl.c */
#include "vs_lib_syms_gl.h"

#ifndef VS_GL_SO_NAME
int  vs_gl_lib_init() { return 0; }
void vs_gl_lib_destroy() { }
#else

#include <dlfcn.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define fgWarning(...) fprintf(stderr, __VA_ARGS__)

vs_glAlphaFunc_fun                      vs_glAlphaFunc = NULL;
vs_glBegin_fun                          vs_glBegin = NULL;
vs_glBindTexture_fun                    vs_glBindTexture = NULL;
vs_glBlendFunc_fun                      vs_glBlendFunc = NULL;
vs_glCallList_fun                       vs_glCallList = NULL;
vs_glClear_fun                          vs_glClear = NULL;
vs_glClearColor_fun                     vs_glClearColor = NULL;
vs_glClearDepth_fun                     vs_glClearDepth = NULL;
vs_glColor3f_fun                        vs_glColor3f = NULL;
vs_glColor4f_fun                        vs_glColor4f = NULL;
vs_glColor4fv_fun                       vs_glColor4fv = NULL;
vs_glColorMask_fun                      vs_glColorMask = NULL;
vs_glColorMaterial_fun                  vs_glColorMaterial = NULL;
vs_glCullFace_fun                       vs_glCullFace = NULL;
vs_glDeleteLists_fun                    vs_glDeleteLists = NULL;
vs_glDeleteTextures_fun                 vs_glDeleteTextures = NULL;
vs_glDepthFunc_fun                      vs_glDepthFunc = NULL;
vs_glDepthMask_fun                      vs_glDepthMask = NULL;
vs_glDisable_fun                        vs_glDisable = NULL;
vs_glDrawArrays_fun                     vs_glDrawArrays = NULL;
vs_glDrawElements_fun                   vs_glDrawElements = NULL;
vs_glEnable_fun                         vs_glEnable = NULL;
vs_glEnableClientState_fun              vs_glEnableClientState = NULL;
vs_glEnd_fun                            vs_glEnd = NULL;
vs_glEndList_fun                        vs_glEndList = NULL;
vs_glFinish_fun                         vs_glFinish = NULL;
vs_glFlush_fun                          vs_glFlush = NULL;
vs_glFogf_fun                           vs_glFogf = NULL;
vs_glFogfv_fun                          vs_glFogfv = NULL;
vs_glFogi_fun                           vs_glFogi = NULL;
vs_glGenLists_fun                       vs_glGenLists = NULL;
vs_glGenTextures_fun                    vs_glGenTextures = NULL;
vs_glGetError_fun                       vs_glGetError = NULL;
vs_glGetFloatv_fun                      vs_glGetFloatv = NULL;
vs_glGetIntegerv_fun                    vs_glGetIntegerv = NULL;
vs_glGetString_fun                      vs_glGetString = NULL;
vs_glInterleavedArrays_fun              vs_glInterleavedArrays = NULL;
vs_glIsEnabled_fun                      vs_glIsEnabled = NULL;
vs_glIsList_fun                         vs_glIsList = NULL;
vs_glLightf_fun                         vs_glLightf = NULL;
vs_glLightfv_fun                        vs_glLightfv = NULL;
vs_glLightModelfv_fun                   vs_glLightModelfv = NULL;
vs_glLightModeli_fun                    vs_glLightModeli = NULL;
vs_glLineWidth_fun                      vs_glLineWidth = NULL;
vs_glLoadIdentity_fun                   vs_glLoadIdentity = NULL;
vs_glLoadMatrixd_fun                    vs_glLoadMatrixd = NULL;
vs_glLoadMatrixf_fun                    vs_glLoadMatrixf = NULL;
vs_glMaterialfv_fun                     vs_glMaterialfv = NULL;
vs_glMatrixMode_fun                     vs_glMatrixMode = NULL;
vs_glNewList_fun                        vs_glNewList = NULL;
vs_glNormal3f_fun                       vs_glNormal3f = NULL;
vs_glNormal3fv_fun                      vs_glNormal3fv = NULL;
vs_glPixelStorei_fun                    vs_glPixelStorei = NULL;
vs_glPointSize_fun                      vs_glPointSize = NULL;
vs_glPolygonMode_fun                    vs_glPolygonMode = NULL;
vs_glPolygonOffset_fun                  vs_glPolygonOffset = NULL;
vs_glPopMatrix_fun                      vs_glPopMatrix = NULL;
vs_glPrioritizeTextures_fun             vs_glPrioritizeTextures = NULL;
vs_glPushMatrix_fun                     vs_glPushMatrix = NULL;
vs_glRasterPos2f_fun                    vs_glRasterPos2f = NULL;
vs_glReadPixels_fun                     vs_glReadPixels = NULL;
vs_glRectf_fun                          vs_glRectf = NULL;
vs_glScaled_fun                         vs_glScaled = NULL;
vs_glScalef_fun                         vs_glScalef = NULL;
vs_glScissor_fun                        vs_glScissor = NULL;
vs_glShadeModel_fun                     vs_glShadeModel = NULL;
vs_glStencilFunc_fun                    vs_glStencilFunc = NULL;
vs_glStencilMask_fun                    vs_glStencilMask = NULL;
vs_glStencilOp_fun                      vs_glStencilOp = NULL;
vs_glTexCoord2f_fun                     vs_glTexCoord2f = NULL;
vs_glTexCoordPointer_fun                vs_glTexCoordPointer = NULL;
vs_glTexEnvfv_fun                       vs_glTexEnvfv = NULL;
vs_glTexEnvi_fun                        vs_glTexEnvi = NULL;
vs_glTexGenfv_fun                       vs_glTexGenfv = NULL;
vs_glTexGeni_fun                        vs_glTexGeni = NULL;
vs_glTexImage2D_fun                     vs_glTexImage2D = NULL;
vs_glTexParameterf_fun                  vs_glTexParameterf = NULL;
vs_glTexParameterfv_fun                 vs_glTexParameterfv = NULL;
vs_glTexParameteri_fun                  vs_glTexParameteri = NULL;
vs_glTexSubImage2D_fun                  vs_glTexSubImage2D = NULL;
vs_glTranslated_fun                     vs_glTranslated = NULL;
vs_glTranslatef_fun                     vs_glTranslatef = NULL;
vs_gluBuild2DMipmaps_fun                vs_gluBuild2DMipmaps = NULL;
vs_gluErrorString_fun                   vs_gluErrorString = NULL;
vs_glVertex2f_fun                       vs_glVertex2f = NULL;
vs_glVertex3d_fun                       vs_glVertex3d = NULL;
vs_glVertex3f_fun                       vs_glVertex3f = NULL;
vs_glVertex3fv_fun                      vs_glVertex3fv = NULL;
vs_glViewport_fun                       vs_glViewport = NULL;
#ifdef HAVE_GLXGETPROCADDRESSARB
vs_glXGetProcAddressARB_fun             vs_glXGetProcAddressARB = NULL;
#endif

static const char * vs_gl_lib_names[] = {
    VS_GL_SO_NAME, NULL
};
static void * vs_gl_lib[sizeof(vs_gl_lib_names)/sizeof(*vs_gl_lib_names)] = { NULL, };

static void * vs_gl_dlopen(const char * name, int flags) {
    void * lib;
    if ((lib = dlopen(name, flags)) == NULL) {
        size_t len = strlen(name);
        char * path = len ? strdup(name) : NULL;
        const char * ext;
        if (!path) return NULL;
        if (len > 3 && !strcasecmp((ext = name + len - 3), ".so"))
            len -= 3;
        else if (len > 6 && !strcasecmp((ext = name + len - 6), ".dylib"))
            len -= 6;
        else
            ext = NULL;
        while (len > 0 && (isdigit(path[len-1]) || path[len-1] == '.' || (ext && path[len-1] == '-')))
            --len; /* loop */
        while (ext && isdigit(path[len])) ++len;
        path[len] = 0;
        if (ext != NULL)
            strcpy(path + len, ext);
        lib = dlopen(path, flags);
        free(path);
    }
    return lib;
}

void * vs_gl_lib_findsymbol(const char * name) {
    void * sym;
    for (void ** lib = vs_gl_lib; *lib != NULL; ++lib) {
        if ((sym = dlsym(*lib, name)) != NULL)
            return sym;
    }
    return NULL;
}

int vs_gl_lib_init() {
    int ret = 0, i = 0;
    if (*vs_gl_lib != NULL)
        return 0;
    for (const char ** name = vs_gl_lib_names; *name != NULL; ++name, ++i) {
        if (**name == 0) { --i; continue; }
        if ((vs_gl_lib[i] = vs_gl_dlopen(*name, RTLD_LOCAL|RTLD_NOW)) == NULL) {
            return -1;
        }
    }
    vs_gl_lib[i] = NULL;
    if ((vs_glAlphaFunc = vs_gl_lib_findsymbol("glAlphaFunc")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glAlphaFunc");
    if ((vs_glBegin = vs_gl_lib_findsymbol("glBegin")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glBegin");
    if ((vs_glBindTexture = vs_gl_lib_findsymbol("glBindTexture")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glBindTexture");
    if ((vs_glBlendFunc = vs_gl_lib_findsymbol("glBlendFunc")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glBlendFunc");
    if ((vs_glCallList = vs_gl_lib_findsymbol("glCallList")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glCallList");
    if ((vs_glClear = vs_gl_lib_findsymbol("glClear")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glClear");
    if ((vs_glClearColor = vs_gl_lib_findsymbol("glClearColor")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glClearColor");
    if ((vs_glClearDepth = vs_gl_lib_findsymbol("glClearDepth")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glClearDepth");
    if ((vs_glColor3f = vs_gl_lib_findsymbol("glColor3f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glColor3f");
    if ((vs_glColor4f = vs_gl_lib_findsymbol("glColor4f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glColor4f");
    if ((vs_glColor4fv = vs_gl_lib_findsymbol("glColor4fv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glColor4fv");
    if ((vs_glColorMask = vs_gl_lib_findsymbol("glColorMask")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glColorMask");
    if ((vs_glColorMaterial = vs_gl_lib_findsymbol("glColorMaterial")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glColorMaterial");
    if ((vs_glCullFace = vs_gl_lib_findsymbol("glCullFace")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glCullFace");
    if ((vs_glDeleteLists = vs_gl_lib_findsymbol("glDeleteLists")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDeleteLists");
    if ((vs_glDeleteTextures = vs_gl_lib_findsymbol("glDeleteTextures")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDeleteTextures");
    if ((vs_glDepthFunc = vs_gl_lib_findsymbol("glDepthFunc")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDepthFunc");
    if ((vs_glDepthMask = vs_gl_lib_findsymbol("glDepthMask")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDepthMask");
    if ((vs_glDisable = vs_gl_lib_findsymbol("glDisable")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDisable");
    if ((vs_glDrawArrays = vs_gl_lib_findsymbol("glDrawArrays")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDrawArrays");
    if ((vs_glDrawElements = vs_gl_lib_findsymbol("glDrawElements")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glDrawElements");
    if ((vs_glEnable = vs_gl_lib_findsymbol("glEnable")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glEnable");
    if ((vs_glEnableClientState = vs_gl_lib_findsymbol("glEnableClientState")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glEnableClientState");
    if ((vs_glEnd = vs_gl_lib_findsymbol("glEnd")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glEnd");
    if ((vs_glEndList = vs_gl_lib_findsymbol("glEndList")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glEndList");
    if ((vs_glFinish = vs_gl_lib_findsymbol("glFinish")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glFinish");
    if ((vs_glFlush = vs_gl_lib_findsymbol("glFlush")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glFlush");
    if ((vs_glFogf = vs_gl_lib_findsymbol("glFogf")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glFogf");
    if ((vs_glFogfv = vs_gl_lib_findsymbol("glFogfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glFogfv");
    if ((vs_glFogi = vs_gl_lib_findsymbol("glFogi")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glFogi");
    if ((vs_glGenLists = vs_gl_lib_findsymbol("glGenLists")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glGenLists");
    if ((vs_glGenTextures = vs_gl_lib_findsymbol("glGenTextures")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glGenTextures");
    if ((vs_glGetError = vs_gl_lib_findsymbol("glGetError")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glGetError");
    if ((vs_glGetFloatv = vs_gl_lib_findsymbol("glGetFloatv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glGetFloatv");
    if ((vs_glGetIntegerv = vs_gl_lib_findsymbol("glGetIntegerv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glGetIntegerv");
    if ((vs_glGetString = vs_gl_lib_findsymbol("glGetString")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glGetString");
    if ((vs_glInterleavedArrays = vs_gl_lib_findsymbol("glInterleavedArrays")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glInterleavedArrays");
    if ((vs_glIsEnabled = vs_gl_lib_findsymbol("glIsEnabled")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glIsEnabled");
    if ((vs_glIsList = vs_gl_lib_findsymbol("glIsList")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glIsList");
    if ((vs_glLightf = vs_gl_lib_findsymbol("glLightf")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLightf");
    if ((vs_glLightfv = vs_gl_lib_findsymbol("glLightfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLightfv");
    if ((vs_glLightModelfv = vs_gl_lib_findsymbol("glLightModelfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLightModelfv");
    if ((vs_glLightModeli = vs_gl_lib_findsymbol("glLightModeli")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLightModeli");
    if ((vs_glLineWidth = vs_gl_lib_findsymbol("glLineWidth")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLineWidth");
    if ((vs_glLoadIdentity = vs_gl_lib_findsymbol("glLoadIdentity")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLoadIdentity");
    if ((vs_glLoadMatrixd = vs_gl_lib_findsymbol("glLoadMatrixd")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLoadMatrixd");
    if ((vs_glLoadMatrixf = vs_gl_lib_findsymbol("glLoadMatrixf")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glLoadMatrixf");
    if ((vs_glMaterialfv = vs_gl_lib_findsymbol("glMaterialfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glMaterialfv");
    if ((vs_glMatrixMode = vs_gl_lib_findsymbol("glMatrixMode")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glMatrixMode");
    if ((vs_glNewList = vs_gl_lib_findsymbol("glNewList")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glNewList");
    if ((vs_glNormal3f = vs_gl_lib_findsymbol("glNormal3f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glNormal3f");
    if ((vs_glNormal3fv = vs_gl_lib_findsymbol("glNormal3fv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glNormal3fv");
    if ((vs_glPixelStorei = vs_gl_lib_findsymbol("glPixelStorei")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPixelStorei");
    if ((vs_glPointSize = vs_gl_lib_findsymbol("glPointSize")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPointSize");
    if ((vs_glPolygonMode = vs_gl_lib_findsymbol("glPolygonMode")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPolygonMode");
    if ((vs_glPolygonOffset = vs_gl_lib_findsymbol("glPolygonOffset")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPolygonOffset");
    if ((vs_glPopMatrix = vs_gl_lib_findsymbol("glPopMatrix")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPopMatrix");
    if ((vs_glPrioritizeTextures = vs_gl_lib_findsymbol("glPrioritizeTextures")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPrioritizeTextures");
    if ((vs_glPushMatrix = vs_gl_lib_findsymbol("glPushMatrix")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glPushMatrix");
    if ((vs_glRasterPos2f = vs_gl_lib_findsymbol("glRasterPos2f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glRasterPos2f");
    if ((vs_glReadPixels = vs_gl_lib_findsymbol("glReadPixels")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glReadPixels");
    if ((vs_glRectf = vs_gl_lib_findsymbol("glRectf")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glRectf");
    if ((vs_glScaled = vs_gl_lib_findsymbol("glScaled")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glScaled");
    if ((vs_glScalef = vs_gl_lib_findsymbol("glScalef")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glScalef");
    if ((vs_glScissor = vs_gl_lib_findsymbol("glScissor")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glScissor");
    if ((vs_glShadeModel = vs_gl_lib_findsymbol("glShadeModel")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glShadeModel");
    if ((vs_glStencilFunc = vs_gl_lib_findsymbol("glStencilFunc")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glStencilFunc");
    if ((vs_glStencilMask = vs_gl_lib_findsymbol("glStencilMask")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glStencilMask");
    if ((vs_glStencilOp = vs_gl_lib_findsymbol("glStencilOp")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glStencilOp");
    if ((vs_glTexCoord2f = vs_gl_lib_findsymbol("glTexCoord2f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexCoord2f");
    if ((vs_glTexCoordPointer = vs_gl_lib_findsymbol("glTexCoordPointer")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexCoordPointer");
    if ((vs_glTexEnvfv = vs_gl_lib_findsymbol("glTexEnvfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexEnvfv");
    if ((vs_glTexEnvi = vs_gl_lib_findsymbol("glTexEnvi")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexEnvi");
    if ((vs_glTexGenfv = vs_gl_lib_findsymbol("glTexGenfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexGenfv");
    if ((vs_glTexGeni = vs_gl_lib_findsymbol("glTexGeni")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexGeni");
    if ((vs_glTexImage2D = vs_gl_lib_findsymbol("glTexImage2D")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexImage2D");
    if ((vs_glTexParameterf = vs_gl_lib_findsymbol("glTexParameterf")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexParameterf");
    if ((vs_glTexParameterfv = vs_gl_lib_findsymbol("glTexParameterfv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexParameterfv");
    if ((vs_glTexParameteri = vs_gl_lib_findsymbol("glTexParameteri")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexParameteri");
    if ((vs_glTexSubImage2D = vs_gl_lib_findsymbol("glTexSubImage2D")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTexSubImage2D");
    if ((vs_glTranslated = vs_gl_lib_findsymbol("glTranslated")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTranslated");
    if ((vs_glTranslatef = vs_gl_lib_findsymbol("glTranslatef")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glTranslatef");
    if ((vs_gluBuild2DMipmaps = vs_gl_lib_findsymbol("gluBuild2DMipmaps")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "gluBuild2DMipmaps");
    if ((vs_gluErrorString = vs_gl_lib_findsymbol("gluErrorString")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "gluErrorString");
    if ((vs_glVertex2f = vs_gl_lib_findsymbol("glVertex2f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glVertex2f");
    if ((vs_glVertex3d = vs_gl_lib_findsymbol("glVertex3d")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glVertex3d");
    if ((vs_glVertex3f = vs_gl_lib_findsymbol("glVertex3f")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glVertex3f");
    if ((vs_glVertex3fv = vs_gl_lib_findsymbol("glVertex3fv")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glVertex3fv");
    if ((vs_glViewport = vs_gl_lib_findsymbol("glViewport")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glViewport");
#ifdef HAVE_GLXGETPROCADDRESSARB
    if ((vs_glXGetProcAddressARB = vs_gl_lib_findsymbol("glXGetProcAddressARB")) == NULL && ++ret)
        fgWarning("cannot load symbol %s", "glXGetProcAddressARB");
#endif
    return ret;
}

void vs_gl_lib_destroy() {
    if (*vs_gl_lib == NULL)
        return ;
    for (void ** lib = vs_gl_lib; *lib; ++lib) {
        dlclose(*lib);
        *lib = NULL;
    }
    vs_glAlphaFunc = NULL;
    vs_glBegin = NULL;
    vs_glBindTexture = NULL;
    vs_glBlendFunc = NULL;
    vs_glCallList = NULL;
    vs_glClear = NULL;
    vs_glClearColor = NULL;
    vs_glClearDepth = NULL;
    vs_glColor3f = NULL;
    vs_glColor4f = NULL;
    vs_glColor4fv = NULL;
    vs_glColorMask = NULL;
    vs_glColorMaterial = NULL;
    vs_glCullFace = NULL;
    vs_glDeleteLists = NULL;
    vs_glDeleteTextures = NULL;
    vs_glDepthFunc = NULL;
    vs_glDepthMask = NULL;
    vs_glDisable = NULL;
    vs_glDrawArrays = NULL;
    vs_glDrawElements = NULL;
    vs_glEnable = NULL;
    vs_glEnableClientState = NULL;
    vs_glEnd = NULL;
    vs_glEndList = NULL;
    vs_glFinish = NULL;
    vs_glFlush = NULL;
    vs_glFogf = NULL;
    vs_glFogfv = NULL;
    vs_glFogi = NULL;
    vs_glGenLists = NULL;
    vs_glGenTextures = NULL;
    vs_glGetError = NULL;
    vs_glGetFloatv = NULL;
    vs_glGetIntegerv = NULL;
    vs_glGetString = NULL;
    vs_glInterleavedArrays = NULL;
    vs_glIsEnabled = NULL;
    vs_glIsList = NULL;
    vs_glLightf = NULL;
    vs_glLightfv = NULL;
    vs_glLightModelfv = NULL;
    vs_glLightModeli = NULL;
    vs_glLineWidth = NULL;
    vs_glLoadIdentity = NULL;
    vs_glLoadMatrixd = NULL;
    vs_glLoadMatrixf = NULL;
    vs_glMaterialfv = NULL;
    vs_glMatrixMode = NULL;
    vs_glNewList = NULL;
    vs_glNormal3f = NULL;
    vs_glNormal3fv = NULL;
    vs_glPixelStorei = NULL;
    vs_glPointSize = NULL;
    vs_glPolygonMode = NULL;
    vs_glPolygonOffset = NULL;
    vs_glPopMatrix = NULL;
    vs_glPrioritizeTextures = NULL;
    vs_glPushMatrix = NULL;
    vs_glRasterPos2f = NULL;
    vs_glReadPixels = NULL;
    vs_glRectf = NULL;
    vs_glScaled = NULL;
    vs_glScalef = NULL;
    vs_glScissor = NULL;
    vs_glShadeModel = NULL;
    vs_glStencilFunc = NULL;
    vs_glStencilMask = NULL;
    vs_glStencilOp = NULL;
    vs_glTexCoord2f = NULL;
    vs_glTexCoordPointer = NULL;
    vs_glTexEnvfv = NULL;
    vs_glTexEnvi = NULL;
    vs_glTexGenfv = NULL;
    vs_glTexGeni = NULL;
    vs_glTexImage2D = NULL;
    vs_glTexParameterf = NULL;
    vs_glTexParameterfv = NULL;
    vs_glTexParameteri = NULL;
    vs_glTexSubImage2D = NULL;
    vs_glTranslated = NULL;
    vs_glTranslatef = NULL;
    vs_gluBuild2DMipmaps = NULL;
    vs_gluErrorString = NULL;
    vs_glVertex2f = NULL;
    vs_glVertex3d = NULL;
    vs_glVertex3f = NULL;
    vs_glVertex3fv = NULL;
    vs_glViewport = NULL;
#ifdef HAVE_GLXGETPROCADDRESSARB
    vs_glXGetProcAddressARB = NULL;
#endif
    return ;
}

#endif /* ! ifdef VS_GL_SO_NAME */

