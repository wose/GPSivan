/*
 * A huge part of this is stolen from https://github.com/chriscamacho/gles2framework 
 */

#ifndef _GL_GUI_H_
#define _GL_GUI_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <kazmath/kazmath.h>

#include <atomic>

typedef struct {
  unsigned int tex;
    unsigned int base;
  unsigned int vertBuf;
  unsigned int texBuf;
  float tHeight;
  float tLines;
  int fWidth;
  int fHeight;
} font_t;

class glgui
{
 public:
  glgui();
  ~glgui();

 private:
  bool _run = true;
  std::atomic<bool> _menu_visible;
  std::atomic<int> _submenu;
  std::atomic<int> _menu_item;

  EGLDisplay _egl_display;
  EGLContext _egl_context;
  EGLSurface _egl_surface;
  EGLNativeWindowType _win;

  unsigned int _display_width;
  unsigned int _display_height;

  enum shaderLocationType {
    shaderAttrib,
    shaderUniform
  };


  struct {
    kmMat4 opm, otm, t;
    GLuint printProg, opm_uniform;
    GLuint fonttex, texture_uniform, cx_uniform, cy_uniform;
    GLuint vert_attrib, uv_attrib;
    GLuint quadvbo, texvbo;
  } _glp;

 private:
  void init_glprint(int width, int height);
  void glPrintf(float x, float y, font_t &fnt, const char *fmt, ...);
  font_t create_font(const char* tpath, unsigned int cbase, float txt_height, float txt_lines, int fnt_width, int fnt_height);
  int loadpng(const char* filename);
  void draw_tile(float x, float y, float w, float h, float a, int tex);
  void swap_buffers();
  void make_native_window();
  char *file_read(const char *filename);
  GLuint create_shader(const char *filename, GLenum type);

 public:
  int get_display_width() { return _display_width; }
  int get_display_height() { return _display_height; }
  bool init();
  void update();
  void stop();
};

#endif
