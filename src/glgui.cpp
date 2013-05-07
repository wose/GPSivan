#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <unistd.h>

#include <bcm_host.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <png.h>
#include <kazmath/kazmath.h>

#include "glgui.h"

glgui::glgui()
{
}

glgui::~glgui()
{
  eglDestroyContext(_egl_display, _egl_context);
  eglDestroySurface(_egl_display, _egl_surface);
  eglTerminate(_egl_display);
}

void glgui::print_log(GLuint object)
{
  GLint log_length = 0;
  if (glIsShader(object))
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    fprintf(stderr, "printlog: Not a shader or a program\n");
    return;
  }

  char *log = (char *)malloc(log_length);

  if (glIsShader(object))
    glGetShaderInfoLog(object, log_length, NULL, log);
  else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);

  fprintf(stderr, "%s", log);
  free(log);
}

void glgui::init_glprint(int width, int height)
{
  kmMat4OrthographicProjection(&_glp.opm, 0, width, height, 0, -10, 10);

  GLuint vs, fs;
  vs = create_shader("resources/shaders/glprint.vert", GL_VERTEX_SHADER);
  fs = create_shader("resources/shaders/glprint.frag", GL_FRAGMENT_SHADER);

  _glp.printProg = glCreateProgram();
  glAttachShader(_glp.printProg, vs);
  glAttachShader(_glp.printProg, fs);
  glLinkProgram(_glp.printProg);
  int link_ok;
  glGetProgramiv(_glp.printProg, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    printf("glLinkProgram:");
    print_log(_glp.printProg);
    printf("\n");
  }

  _glp.cx_uniform = get_shader_location(shaderUniform, _glp.printProg, "cx");
  _glp.cy_uniform = get_shader_location(shaderUniform, _glp.printProg, "cy");
  _glp.opm_uniform =
    get_shader_location(shaderUniform, _glp.printProg, "opm_uniform");
  _glp.texture_uniform =
    get_shader_location(shaderUniform, _glp.printProg, "texture_uniform");

  _glp.vert_attrib = get_shader_location(shaderAttrib, _glp.printProg, "vert_attrib");
  _glp.uv_attrib = get_shader_location(shaderAttrib, _glp.printProg, "uv_attrib");
}

void glgui::glPrintf(float x, float y, font_t &fnt, const char *fmt, ...)
{
  char text[256];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);

  glUseProgram(_glp.printProg);
  kmMat4Assign(&_glp.otm, &_glp.opm);
  kmMat4Translation(&_glp.t, x, y, -1);
  kmMat4Multiply(&_glp.otm, &_glp.otm, &_glp.t);

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

    //glBindTexture(GL_TEXTURE_2D, __glp.fonttex);
  glBindTexture(GL_TEXTURE_2D, fnt.tex);

  glEnableVertexAttribArray(_glp.vert_attrib);
// glBindBuffer(GL_ARRAY_BUFFER, __glp.quadvbo);
  glBindBuffer(GL_ARRAY_BUFFER, fnt.vertBuf);
  glVertexAttribPointer(_glp.vert_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(_glp.uv_attrib);
// glBindBuffer(GL_ARRAY_BUFFER, __glp.texvbo);
  glBindBuffer(GL_ARRAY_BUFFER, fnt.texBuf);
  glVertexAttribPointer(_glp.uv_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glUniform1i(_glp.texture_uniform, 0);

  for (int n = 0; n < strlen(text); n++) {
    int c = (int)text[n] - fnt.base;
// float cx = c % 16;
// float cy = (int)(c / 16.0);
// cy = cy * (1. / 16);
// cx = cx * (1. / 16);
    float cx = c % 16;
    float cy = (int)(c/16.0);
    cy = cy * (1. / (fnt.tLines));
    cx = cx * (1. / 16);

    glUniformMatrix4fv(_glp.opm_uniform, 1, GL_FALSE, (GLfloat *) & _glp.otm);
    glUniform1f(_glp.cx_uniform, cx);
    glUniform1f(_glp.cy_uniform, cy);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    kmMat4Translation(&_glp.t, fnt.fWidth, 0, 0);
    kmMat4Multiply(&_glp.otm, &_glp.otm, &_glp.t);
  }

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDisableVertexAttribArray(_glp.uv_attrib);
  glDisableVertexAttribArray(_glp.vert_attrib);
}

void glgui::make_native_window()
{
  bcm_host_init();

  int32_t success = 0;

  success = graphics_get_display_size(0 /* LCD */ , &_display_width,
                                      &_display_height);
  if (success < 0) {
    printf("unable to get display size\n");
    //return EGL_FALSE;
  }

  static EGL_DISPMANX_WINDOW_T nativewindow;

  DISPMANX_ELEMENT_HANDLE_T dispman_element;
  DISPMANX_DISPLAY_HANDLE_T dispman_display;
  DISPMANX_UPDATE_HANDLE_T dispman_update;
  VC_RECT_T dst_rect;
  VC_RECT_T src_rect;

  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = _display_width;
  dst_rect.height = _display_height;

  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = _display_width << 16;
  src_rect.height = _display_height << 16;

  dispman_display = vc_dispmanx_display_open(0 /* LCD */ );
  dispman_update = vc_dispmanx_update_start(0);

  VC_DISPMANX_ALPHA_T alpha = { DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS,255,0 };
  dispman_element =
    vc_dispmanx_element_add(dispman_update, dispman_display,
                            0 /*layer */ , &dst_rect, 0 /*src */ ,
                            &src_rect, DISPMANX_PROTECTION_NONE,
                            &alpha /*alpha */ , 0 /*clamp */ ,
                            (DISPMANX_TRANSFORM_T)0 /*transform */ );

  nativewindow.element = dispman_element;
  nativewindow.width = _display_width;
  nativewindow.height =_display_height;
  vc_dispmanx_update_submit_sync(dispman_update);

  _win = &nativewindow;

  _egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (_egl_display == EGL_NO_DISPLAY) {
    printf("Got no EGL display.\n");
  }
}

font_t glgui::create_font(const char* tpath, unsigned int cbase, float txt_height, float txt_lines, int fnt_width, int fnt_height)
{
  font_t t;

  t.tex = loadpng(tpath);
  t.base=cbase;
  t.tHeight=txt_height;
  t.tLines=txt_lines;
  t.fWidth=fnt_width;
  t.fHeight=fnt_height;

  float *vb = (float*)malloc(sizeof(float) * 3 * 6);
    
  vb[0]=vb[1]=vb[2]=vb[5]=vb[7]=vb[8]=vb[11]=vb[12]=vb[14]=vb[15]=vb[17]=0;
  vb[3]=vb[6]=vb[9]=fnt_width;
  vb[4]=vb[10]=vb[16]=fnt_height;

  glGenBuffers(1, &t.vertBuf);
  glBindBuffer(GL_ARRAY_BUFFER, t.vertBuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 6, vb, GL_STATIC_DRAW);

  free(vb);

  float *tc = (float*)malloc(sizeof(float) * 2 * 6);
  tc[0]=tc[1]=tc[5]=tc[8]=tc[9]=tc[10]=0;
  tc[2]=tc[4]=tc[6]=1./16;
  tc[3]=tc[7]=tc[11]=1./txt_lines;

  glGenBuffers(1, &t.texBuf);
  glBindBuffer(GL_ARRAY_BUFFER, t.texBuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, tc, GL_STATIC_DRAW);

  free(tc);

  return t;
}

int glgui::loadpng(const char* filename)
{
  GLuint texture;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_bytep *row_pointers = NULL;
  int bitDepth, colourType;

  FILE *pngFile = fopen(filename, "rb");

  if (!pngFile)
    return 0;

  png_byte sig[8];

  fread(&sig, 8, sizeof(png_byte), pngFile);
  rewind(pngFile);
  if (!png_check_sig(sig, 8)) {
    printf("png sig failure\n");
    return 0;
  }

  png_ptr =
    png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr) {
    printf("png ptr not created\n");
    return 0;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    printf("set jmp failed\n");
    return 0;
  }

  info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr) {
    printf("cant get png info ptr\n");
    return 0;
  }

  png_init_io(png_ptr, pngFile);
  png_read_info(png_ptr, info_ptr);
  bitDepth = png_get_bit_depth(png_ptr, info_ptr);
  colourType = png_get_color_type(png_ptr, info_ptr);

  if (colourType == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr);
  if (colourType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    //png_set_gray_1_2_4_to_8(png_ptr);
    png_set_expand_gray_1_2_4_to_8(png_ptr); // thanks to Jesse Jaara for bug fix for newer libpng...
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);

  if (bitDepth == 16)
    png_set_strip_16(png_ptr);
  else if (bitDepth < 8)
    png_set_packing(png_ptr);

  png_read_update_info(png_ptr, info_ptr);

  png_uint_32 width, height;
  png_get_IHDR(png_ptr, info_ptr, &width, &height,
               &bitDepth, &colourType, NULL, NULL, NULL);

  int components; // = GetTextureInfo(colourType);
  switch (colourType) {
  case PNG_COLOR_TYPE_GRAY:
    components = 1;
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    components = 2;
    break;
  case PNG_COLOR_TYPE_RGB:
    components = 3;
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    components = 4;
    break;
  default:
    components = -1;
  }

  if (components == -1) {
    if (png_ptr)
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    printf("%s broken?\n", filename);
    return 0;
  }

  GLubyte *pixels =
    (GLubyte *) malloc(sizeof(GLubyte) * (width * height * components));
  row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);

  for (int i = 0; i < height; ++i)
    row_pointers[i] =
      (png_bytep) (pixels + (i * width * components));

  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, NULL);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  GLuint glcolours;
  (components == 4) ? (glcolours = GL_RGBA) : (0);
  (components == 3) ? (glcolours = GL_RGB) : (0);
  (components == 2) ? (glcolours = GL_LUMINANCE_ALPHA) : (0);
  (components == 1) ? (glcolours = GL_LUMINANCE) : (0);

  //printf("%s has %i colour components\n",filename,components);
  //glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, glcolours, GL_UNSIGNED_BYTE, pixels);
  glTexImage2D(GL_TEXTURE_2D, 0, glcolours, width, height, 0, glcolours,
               GL_UNSIGNED_BYTE, pixels);

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  fclose(pngFile);
  free(row_pointers);
  free(pixels);

  return texture;
}

char* glgui::file_read(const char *filename)
{
  FILE *in = fopen(filename, "rb");
  if (in == NULL)
    return NULL;

  int res_size = BUFSIZ;
  char *res = (char *)malloc(res_size);
  int nb_read_total = 0;

  while (!feof(in) && !ferror(in)) {
    if (nb_read_total + BUFSIZ > res_size) {
      if (res_size > 10 * 1024 * 1024)
        break;
      res_size = res_size * 2;
      res = (char *)realloc(res, res_size);
    }
    char *p_res = res + nb_read_total;
    nb_read_total += fread(p_res, 1, BUFSIZ, in);
  }

  fclose(in);
  res = (char *)realloc(res, nb_read_total + 1);
  res[nb_read_total] = '\0';
  return res;
}

GLuint glgui::get_shader_location(int type, GLuint prog, const char *name)
{
  GLuint ret;
  if (type == shaderAttrib)
    ret = glGetAttribLocation(prog, name);
  if (type == shaderUniform)
    ret = glGetUniformLocation(prog, name);
  if (ret == -1) {
    printf("Cound not bind shader location %s\n", name);
  }
  return ret;
}

GLuint glgui::create_shader(const char *filename, GLenum type)
{
  const GLchar *source = file_read(filename);
  if (source == NULL) {
    fprintf(stderr, "Error opening %s: ", filename);
    perror("");
    return 0;
  }
  GLuint res = glCreateShader(type);
  const GLchar *sources[] = {
    // Define GLSL version
#define GL_ES_VERSION_2_0
#ifdef GL_ES_VERSION_2_0
    "#version 100\n"
#else
    "#version 120\n"
#endif
        ,
    // GLES2 precision specifiers
#ifdef GL_ES_VERSION_2_0
    // Define default float precision for fragment shaders:
    (type == GL_FRAGMENT_SHADER) ?
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "precision highp float; \n"
    "#else \n"
    "precision mediump float; \n"
    "#endif \n" : ""
    // Note: OpenGL ES automatically defines this:
    // #define GL_ES
#else
    // Ignore GLES 2 precision specifiers:
    "#define lowp \n" "#define mediump\n" "#define highp \n"
#endif
    ,
    source
  };
  glShaderSource(res, 3, sources, NULL);
  free((void *)source);

  glCompileShader(res);
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "%s:", filename);
    print_log(res);
    glDeleteShader(res);
    return 0;
  }

  return res;
}

void glgui::draw_tile(float x, float y, float w, float h, float a, int tex)
{
}

void glgui::swap_buffers()
{
  eglSwapBuffers(_egl_display, _egl_surface);
}

bool glgui::init()
{
  make_native_window();

  EGLint majorVersion;
  EGLint minorVersion;

  // most egl you can sends NULLS for maj/min but not RPi
  if (!eglInitialize(_egl_display, &majorVersion, &minorVersion)) {
    printf("Unable to initialize EGL\n");
    return false;
  }

  EGLint attr[] = { // some attributes to set up our egl-interface
    EGL_BUFFER_SIZE, 16,
    EGL_DEPTH_SIZE, 16,
    EGL_RENDERABLE_TYPE,
    EGL_OPENGL_ES2_BIT,
    EGL_NONE
  };

  EGLConfig ecfg;
  EGLint num_config;
  if (!eglChooseConfig(_egl_display, attr, &ecfg, 1, &num_config)) {
    //cerr << "Failed to choose config (eglError: " << eglGetError() << ")" << endl;
    printf("failed to choose config eglerror:%i\n", eglGetError()); // todo change error number to text error
    return false;
  }

  if (num_config != 1) {
    printf("Didn't get exactly one config, but %i\n", num_config);
    return false;
  }

  _egl_surface = eglCreateWindowSurface(_egl_display, ecfg, _win, NULL);

  if (_egl_surface == EGL_NO_SURFACE) {
    //cerr << "Unable to create EGL surface (eglError: " << eglGetError() << ")" << endl;
    printf("failed create egl surface eglerror:%i\n",
           eglGetError());
    return false;
  }
  //// egl-contexts collect all state descriptions needed required for operation
  EGLint ctxattr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  _egl_context =
    eglCreateContext(_egl_display, ecfg, EGL_NO_CONTEXT, ctxattr);
  if (_egl_context == EGL_NO_CONTEXT) {
    //cerr << "Unable to create EGL context (eglError: " << eglGetError() << ")" << endl;
    printf("unable to create EGL context eglerror:%i\n",
           eglGetError());
    return false;
  }
  //// associate the egl-context with the egl-surface
  eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, _egl_context);

  return true;
}

void glgui::stop()
{
  _run = false;
}

void glgui::update()
{
  unsigned long frame_counter = 0;
  glActiveTexture(GL_TEXTURE0);

  glViewport(0, 0, _display_width, _display_height);
  init_glprint(_display_width, _display_height);
  font_t basic_font = create_font("resources/textures/font.png", 0, 256, 16, 16, 16);

  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.2, 0.2, 0.2, 1);

  while(_run)
    {
      ++frame_counter;
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glPrintf(0, 0, basic_font, "frame=%i", frame_counter);
      swap_buffers();
      usleep(16000);
    }
}
