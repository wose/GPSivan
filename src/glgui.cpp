#include <cstdlib>
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
}

void glgui::init_glprint(int width, int height)
{
}

void glgui::glPrintf(float x, float y, font_t &fnt, const char *fmt, ...)
{
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
                            0 /*transform */ );

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

void glgui::draw_tile(float x, float y, float w, float h, float a, int tex)
{
}

void glgui::swap_buffers()
{
  eglSwapBuffers(_egl_display, _egl_surface);
}

void glgui::init()
{
}

void glgui::stop()
{
  _run = false;
}

void glgui::update()
{
  while(_run)
    {
      
    }
}
