//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#if defined(WINDOWS_VERSION) || defined(ANDROID_VERSION) || defined(IOS_VERSION)

#include "gfx/ali3dexception.h"
#include "gfx/ali3dogl.h"
#include "gfx/gfxfilter_ogl.h"
#include "main/main_allegro.h"
#include "platform/base/agsplatformdriver.h"

#if defined(WINDOWS_VERSION)

int psp_gfx_smoothing = 1;
int psp_gfx_scaling = 1;
int psp_gfx_renderer = 0;
int psp_gfx_super_sampling = 0;

unsigned int device_screen_physical_width = 1000;
unsigned int device_screen_physical_height = 500;
int device_screen_initialized = 1;
int device_mouse_clip_left = 0;
int device_mouse_clip_right = device_screen_physical_width;
int device_mouse_clip_top = 0;
int device_mouse_clip_bottom = device_screen_physical_height;

const char* fbo_extension_string = "GL_EXT_framebuffer_object";

PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = 0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = 0;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0;

#elif defined(ANDROID_VERSION)

#define glOrtho glOrthof
#define GL_CLAMP GL_CLAMP_TO_EDGE

// Defined in Allegro
extern "C" 
{
  void android_swap_buffers();
  void android_create_screen(int width, int height, int color_depth);
  void android_mouse_setup(int left, int right, int top, int bottom, float scaling_x, float scaling_y);
}

extern "C" void android_debug_printf(char* format, ...);

extern int psp_gfx_smoothing;
extern int psp_gfx_scaling;
extern int psp_gfx_renderer;
extern int psp_gfx_super_sampling;

extern unsigned int android_screen_physical_width;
extern unsigned int android_screen_physical_height;
extern int android_screen_initialized;
extern int android_mouse_clip_left;
extern int android_mouse_clip_right;
extern int android_mouse_clip_top;
extern int android_mouse_clip_bottom;

#define device_screen_physical_width android_screen_physical_width
#define device_screen_physical_height android_screen_physical_height
#define device_screen_initialized android_screen_initialized
#define device_mouse_setup android_mouse_setup
#define device_swap_buffers android_swap_buffers
#define device_mouse_clip_left android_mouse_clip_left
#define device_mouse_clip_right android_mouse_clip_right
#define device_mouse_clip_top android_mouse_clip_top
#define device_mouse_clip_bottom android_mouse_clip_bottom

const char* fbo_extension_string = "GL_OES_framebuffer_object";

#define glGenFramebuffersEXT glGenFramebuffersOES
#define glDeleteFramebuffersEXT glDeleteFramebuffersOES
#define glBindFramebufferEXT glBindFramebufferOES
#define glCheckFramebufferStatusEXT glCheckFramebufferStatusOES
#define glGetFramebufferAttachmentParameterivEXT glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapEXT glGenerateMipmapOES
#define glFramebufferTexture2DEXT glFramebufferTexture2DOES
#define glFramebufferRenderbufferEXT glFramebufferRenderbufferOES

#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES

#elif defined(IOS_VERSION)

extern "C" 
{
  void ios_swap_buffers();
  void ios_select_buffer();
  void ios_create_screen();
  void ios_mouse_setup(int left, int right, int top, int bottom, float scaling_x, float scaling_y);  
}

#define glOrtho glOrthof
#define GL_CLAMP GL_CLAMP_TO_EDGE

extern int psp_gfx_smoothing;
extern int psp_gfx_scaling;
extern int psp_gfx_renderer;
extern int psp_gfx_super_sampling;

extern unsigned int ios_screen_physical_width;
extern unsigned int ios_screen_physical_height;
extern int ios_screen_initialized;
extern int ios_mouse_clip_left;
extern int ios_mouse_clip_right;
extern int ios_mouse_clip_top;
extern int ios_mouse_clip_bottom;

#define device_screen_physical_width ios_screen_physical_width
#define device_screen_physical_height ios_screen_physical_height
#define device_screen_initialized ios_screen_initialized
#define device_mouse_setup ios_mouse_setup
#define device_swap_buffers ios_swap_buffers
#define device_mouse_clip_left ios_mouse_clip_left
#define device_mouse_clip_right ios_mouse_clip_right
#define device_mouse_clip_top ios_mouse_clip_top
#define device_mouse_clip_bottom ios_mouse_clip_bottom

const char* fbo_extension_string = "GL_OES_framebuffer_object";

#define glGenFramebuffersEXT glGenFramebuffersOES
#define glDeleteFramebuffersEXT glDeleteFramebuffersOES
#define glBindFramebufferEXT glBindFramebufferOES
#define glCheckFramebufferStatusEXT glCheckFramebufferStatusOES
#define glGetFramebufferAttachmentParameterivEXT glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapEXT glGenerateMipmapOES
#define glFramebufferTexture2DEXT glFramebufferTexture2DOES
#define glFramebufferRenderbufferEXT glFramebufferRenderbufferOES

#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES

#endif

namespace AGS
{
namespace Engine
{
namespace OGL
{

using namespace AGS::Common;

GLfloat backbuffer_vertices[] =
{
   0, 0,
   320, 0,
   0, 200,
   320, 200
};

GLfloat backbuffer_texture_coordinates[] =
{
   0, 0,
   320.0f / 512.0f, 0,
   0, 200.0f / 256.0f,
   320.0f / 512.0f, 200.0f / 256.0f
};

void ogl_dummy_vsync() { }

#define algetr32(xx) ((xx >> _rgb_r_shift_32) & 0xFF)
#define algetg32(xx) ((xx >> _rgb_g_shift_32) & 0xFF)
#define algetb32(xx) ((xx >> _rgb_b_shift_32) & 0xFF)
#define algeta32(xx) ((xx >> _rgb_a_shift_32) & 0xFF)
#define algetr15(xx) ((xx >> _rgb_r_shift_15) & 0x1F)
#define algetg15(xx) ((xx >> _rgb_g_shift_15) & 0x1F)
#define algetb15(xx) ((xx >> _rgb_b_shift_15) & 0x1F)

#define GFX_OPENGL  AL_ID('O','G','L',' ')

GFX_DRIVER gfx_opengl =
{
   GFX_OPENGL,
   empty_string,
   empty_string,
   "OpenGL",
   NULL,    // init
   NULL,   // exit
   NULL,                        // AL_METHOD(int, scroll, (int x, int y)); 
   ogl_dummy_vsync,   // vsync
   NULL,  // setpalette
   NULL,                        // AL_METHOD(int, request_scroll, (int x, int y));
   NULL,                        // AL_METHOD(int, poll_scroll, (void));
   NULL,                        // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,  //create_video_bitmap
   NULL,  //destroy_video_bitmap
   NULL,   //show_video_bitmap
   NULL,
   NULL,  //gfx_directx_create_system_bitmap,
   NULL, //gfx_directx_destroy_system_bitmap,
   NULL, //gfx_directx_set_mouse_sprite,
   NULL, //gfx_directx_show_mouse,
   NULL, //gfx_directx_hide_mouse,
   NULL, //gfx_directx_move_mouse,
   NULL,                        // AL_METHOD(void, drawing_mode, (void));
   NULL,                        // AL_METHOD(void, save_video_state, (void*));
   NULL,                        // AL_METHOD(void, restore_video_state, (void*));
   NULL,                        // AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a));
   NULL,                        // AL_METHOD(int, fetch_mode_list, (void));
   0, 0,                        // int w, h;
   FALSE,                        // int linear;
   0,                           // long bank_size;
   0,                           // long bank_gran;
   0,                           // long vid_mem;
   0,                           // long vid_phys_base;
   TRUE                         // int windowed;
};

void OGLBitmap::Dispose()
{
    if (_tiles != NULL)
    {
        for (int i = 0; i < _numTiles; i++)
            glDeleteTextures(1, &(_tiles[i].texture));

        free(_tiles);
        _tiles = NULL;
        _numTiles = 0;
    }
    if (_vertex != NULL)
    {
        free(_vertex);
        _vertex = NULL;
    }
}

OGLGraphicsDriver::OGLGraphicsDriver() 
{
  numToDraw = 0;
  numToDrawLastTime = 0;
  _pollingCallback = NULL;
  _drawScreenCallback = NULL;
  _initGfxCallback = NULL;
  _tint_red = 0;
  _tint_green = 0;
  _tint_blue = 0;
  _filter = NULL;
  _screenTintLayer = NULL;
  _screenTintLayerDDB = NULL;
  _screenTintSprite.skip = true;
  _screenTintSprite.x = 0;
  _screenTintSprite.y = 0;
  _legacyPixelShader = false;
  _scale_width = 1.0f;
  _scale_height = 1.0f;
  set_up_default_vertices();
}


void OGLGraphicsDriver::set_up_default_vertices()
{
  defaultVertices[0].position.x = 0.0f;
  defaultVertices[0].position.y = 0.0f;
  defaultVertices[0].tu=0.0;
  defaultVertices[0].tv=0.0;

  defaultVertices[1].position.x = 1.0f;
  defaultVertices[1].position.y = 0.0f;
  defaultVertices[1].tu=1.0;
  defaultVertices[1].tv=0.0;

  defaultVertices[2].position.x = 0.0f;
  defaultVertices[2].position.y = -1.0f;
  defaultVertices[2].tu=0.0;
  defaultVertices[2].tv=1.0;

  defaultVertices[3].position.x = 1.0f;
  defaultVertices[3].position.y = -1.0f;
  defaultVertices[3].tu=1.0;
  defaultVertices[3].tv=1.0;
}


void OGLGraphicsDriver::Vsync() 
{
  // do nothing on D3D
}

bool OGLGraphicsDriver::IsModeSupported(const DisplayMode &mode)
{
  if (mode.Width <= 0 || mode.Height <= 0 || mode.ColorDepth <= 0)
  {
    set_allegro_error("Invalid resolution parameters: %d x %d x %d", mode.Width, mode.Height, mode.ColorDepth);
    return false;
  }
  return true;
}

bool OGLGraphicsDriver::SupportsGammaControl() 
{
  return false;
}

void OGLGraphicsDriver::SetGamma(int newGamma)
{
}

void OGLGraphicsDriver::SetGraphicsFilter(OGLGfxFilter *filter)
{
  _filter = filter;
}

void OGLGraphicsDriver::SetTintMethod(TintMethod method) 
{
  _legacyPixelShader = (method == TintReColourise);
}

void OGLGraphicsDriver::create_backbuffer_arrays()
{
  float android_screen_ar = (float)_srcRect.GetWidth() / (float)_srcRect.GetHeight();
  float android_device_ar = (float)device_screen_physical_width / (float)device_screen_physical_height;

  if (psp_gfx_scaling == 1)
  {
    // Preserve aspect ratio
    if (android_device_ar <= android_screen_ar)
    {
      backbuffer_vertices[2] = backbuffer_vertices[6] = device_screen_physical_width - 1;
      backbuffer_vertices[5] = backbuffer_vertices[7] = device_screen_physical_width * ((float)_srcRect.GetHeight() / (float)_srcRect.GetWidth());

#if defined(ANDROID_VERSION) || defined(IOS_VERSION)
      device_mouse_setup(
        0, 
        device_screen_physical_width - 1, 
        (device_screen_physical_height - backbuffer_vertices[5]) / 2, 
        device_screen_physical_height - ((device_screen_physical_height - backbuffer_vertices[5]) / 2), 
        (float)_srcRect.GetWidth() / (float)device_screen_physical_width, 
        (float)_srcRect.GetHeight() / backbuffer_vertices[5]);
#endif
    }
    else
    {
      backbuffer_vertices[2] = backbuffer_vertices[6] = device_screen_physical_height * ((float)_srcRect.GetWidth() / (float)_srcRect.GetHeight());
      backbuffer_vertices[5] = backbuffer_vertices[7] = device_screen_physical_height - 1;

#if defined(ANDROID_VERSION) || defined(IOS_VERSION)
      device_mouse_setup(
        (device_screen_physical_width - backbuffer_vertices[2]) / 2,
        device_screen_physical_width - ((device_screen_physical_width - backbuffer_vertices[2]) / 2),
        0,
        device_screen_physical_height - 1,
        (float)_srcRect.GetWidth() / backbuffer_vertices[2], 
        (float)_srcRect.GetHeight() / (float)device_screen_physical_height);
#endif
    }
  }
  else if (psp_gfx_scaling == 2)
  {
    // Stretch to whole screen
    backbuffer_vertices[2] = backbuffer_vertices[6] = device_screen_physical_width - 1;
    backbuffer_vertices[5] = backbuffer_vertices[7] = device_screen_physical_height - 1;

#if defined(ANDROID_VERSION) || defined(IOS_VERSION)
    device_mouse_setup(
      0, 
      device_screen_physical_width - 1, 
      0, 
      device_screen_physical_width - 1, 
      (float)_srcRect.GetWidth() / (float)device_screen_physical_width, 
      (float)_srcRect.GetHeight() / (float)device_screen_physical_height);
#endif
  }
  else
  {
    // No scaling
    backbuffer_vertices[0] = backbuffer_vertices[4] = _srcRect.GetWidth() * (-0.5f);
    backbuffer_vertices[2] = backbuffer_vertices[6] = _srcRect.GetWidth() * 0.5f;
    backbuffer_vertices[5] = backbuffer_vertices[7] = _srcRect.GetHeight() * 0.5f;
    backbuffer_vertices[1] = backbuffer_vertices[3] = _srcRect.GetHeight() * (-0.5f);

#if defined(ANDROID_VERSION) || defined(IOS_VERSION)
    device_mouse_setup(
      (device_screen_physical_width - _srcRect.GetWidth()) / 2,
      device_screen_physical_width - ((device_screen_physical_width - _srcRect.GetWidth()) / 2),
      (device_screen_physical_height - _srcRect.GetHeight()) / 2, 
      device_screen_physical_height - ((device_screen_physical_height - _srcRect.GetHeight()) / 2), 
      1.0f,
      1.0f);
#endif
  }

   backbuffer_texture_coordinates[5] = backbuffer_texture_coordinates[7] = (float)_srcRect.GetHeight() * _super_sampling / (float)_backbuffer_texture_height;
   backbuffer_texture_coordinates[2] = backbuffer_texture_coordinates[6] = (float)_srcRect.GetWidth() * _super_sampling / (float)_backbuffer_texture_width;
}

void OGLGraphicsDriver::InitOpenGl()
{
#if defined(IOS_VERSION)
  ios_select_buffer();
#endif

  if (_render_to_texture)
  {
    char* extensions = (char*)glGetString(GL_EXTENSIONS);

    if (strstr(extensions, fbo_extension_string) != NULL)
    {
#if defined(WINDOWS_VERSION)
      glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
      glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
      glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
      glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
      glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)wglGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
      glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");
      glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
      glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
#endif

      // Disable super-sampling if it would cause a too large texture size
      if (_super_sampling > 1)
      {
        int max = 1024;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
        if ((max < _srcRect.GetWidth() * 2) || (max < _srcRect.GetHeight() * 2))
          _super_sampling = 1;
      }
    }
    else
    {
      _render_to_texture = false;
    }
  }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glShadeModel(GL_FLAT);

  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, device_screen_physical_width, device_screen_physical_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, device_screen_physical_width - 1, 0, device_screen_physical_height - 1, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  if (psp_gfx_scaling && !_render_to_texture)
  {
    if (psp_gfx_scaling == 1)
    {
      float android_screen_ar = (float)_srcRect.GetWidth() / (float)_srcRect.GetHeight();
      float android_device_ar = (float)device_screen_physical_width / (float)device_screen_physical_height;

      if (android_device_ar <= android_screen_ar)
      {
        _scale_width = (float)device_screen_physical_width / (float)_srcRect.GetWidth();
        _scale_height = _scale_width;
      }
      else
      {
        _scale_height = (float)device_screen_physical_height / (float)_srcRect.GetHeight();
        _scale_width = _scale_height;
      }
    }
    else
    {
      _scale_width = (float)device_screen_physical_width / (float)_srcRect.GetWidth();
      _scale_height = (float)device_screen_physical_height / (float)_srcRect.GetHeight();
    }
  }
  else
  {
    _scale_width = _scale_height = 1.0f * _super_sampling;
  }

  if (_render_to_texture)
  {
    _backbuffer_texture_width = _srcRect.GetWidth() * _super_sampling;
    _backbuffer_texture_height = _srcRect.GetHeight() * _super_sampling;
    AdjustSizeToNearestSupportedByCard(&_backbuffer_texture_width, &_backbuffer_texture_height);

    glGenTextures(1, &_backbuffer);
    glBindTexture(GL_TEXTURE_2D, _backbuffer);
    
    if (psp_gfx_smoothing)
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _backbuffer_texture_width, _backbuffer_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffersEXT(1, &_fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _backbuffer, 0);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }
  else
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)(((float)device_screen_physical_width - _scale_width * (float)_srcRect.GetWidth()) / 2.0f + 1.0f), (int)(((float)device_screen_physical_height - _scale_height * (float)_srcRect.GetHeight()) / 2.0f), (int)(_scale_width * (float)_srcRect.GetWidth()), (int)(_scale_height * (float)_srcRect.GetHeight()));
  }

  create_backbuffer_arrays();
}

bool OGLGraphicsDriver::Init(const DisplayMode &mode, const Size src_size, const Rect dst_rect, volatile int *loopTimer)
{
#if defined(ANDROID_VERSION)
  android_create_screen(mode.Width, mode.Height, mode.ColorDepth);
#elif defined(IOS_VERSION)
  ios_create_screen();
#endif

  if (mode.ColorDepth < 15)
  {
    set_allegro_error("OpenGL driver does not support 256-colour games");
    return false;
  }

  _Init(mode, src_size, dst_rect, loopTimer);
  _mode.Windowed = true;

  if (psp_gfx_renderer == 2)
  {
    _super_sampling = ((psp_gfx_super_sampling > 0) ? 2 : 1);
    _render_to_texture = true;
  }
  else
  {
    _super_sampling = 1;
    _render_to_texture = false;
  }

  try
  {
    int i = 0;

#if defined(WINDOWS_VERSION)
    HWND allegro_wnd = _hWnd = win_get_window();

    if (!mode.Windowed)
    {
      // Remove the border in full-screen mode, otherwise if the player
      // clicks near the edge of the screen it goes back to Windows
      LONG windowStyle = WS_POPUP;
      SetWindowLong(allegro_wnd, GWL_STYLE, windowStyle);
    }
#endif

    gfx_driver = &gfx_opengl;

#if defined(WINDOWS_VERSION)
    if (_mode.Windowed)
    {
      if (adjust_window(device_screen_physical_width, device_screen_physical_height) != 0) 
      {
        set_allegro_error("Window size not supported");
        return -1;
      }
    }

    win_grab_input();


    GLuint PixelFormat;

    static PIXELFORMATDESCRIPTOR pfd =
    {
       sizeof(PIXELFORMATDESCRIPTOR),
       1,
       PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
       PFD_TYPE_RGBA,
       mode.ColorDepth,
       0, 0, 0, 0, 0, 0,
       0,
       0,
       0,
       0, 0, 0, 0,
       0,
       0,
       0,
       PFD_MAIN_PLANE,
       0,
       0, 0, 0
    };

    if (!(_hDC = GetDC(_hWnd)))
      return false;

    if (!(PixelFormat = ChoosePixelFormat(_hDC, &pfd)))
      return false;

    if (!SetPixelFormat(_hDC, PixelFormat, &pfd))
      return false;

    if (!(_hRC = wglCreateContext(_hDC)))
      return false;

    if(!wglMakeCurrent(_hDC, _hRC))
      return false;
#endif

    InitOpenGl();

    this->create_screen_tint_bitmap();

  }
  catch (Ali3DException exception)
  {
    if (exception._message != get_allegro_error())
      set_allegro_error(exception._message);
    return false;
  }
  // create dummy screen bitmap
  BitmapHelper::SetScreenBitmap( ConvertBitmapToSupportedColourDepth(BitmapHelper::CreateBitmap(src_size.Width, src_size.Height, mode.ColorDepth)) );

  return true;
}

IGfxModeList *OGLGraphicsDriver::GetSupportedModeList(int color_depth)
{
    // TODO!!
    return NULL;
}

IGfxFilter *OGLGraphicsDriver::GetGraphicsFilter() const
{
    return _filter;
}

void OGLGraphicsDriver::UnInit() 
{
  _UnInit();

  if (_screenTintLayerDDB != NULL) 
  {
    this->DestroyDDB(_screenTintLayerDDB);
    _screenTintLayerDDB = NULL;
    _screenTintSprite.bitmap = NULL;
  }
  delete _screenTintLayer;
  _screenTintLayer = NULL;

  gfx_driver = NULL;
}

OGLGraphicsDriver::~OGLGraphicsDriver()
{
  UnInit();
}

void OGLGraphicsDriver::ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse)
{

}

void OGLGraphicsDriver::GetCopyOfScreenIntoBitmap(Bitmap *destination)
{
#if defined(IOS_VERSION)
  ios_select_buffer();
#endif

  int retrieve_width = device_mouse_clip_right - device_mouse_clip_left;
  int retrieve_height = device_mouse_clip_bottom - device_mouse_clip_top;

  int bufferSize = retrieve_width * retrieve_height * 4;

  unsigned char* buffer = (unsigned char*)malloc(bufferSize);
  if (buffer)
  {
    glReadPixels(device_mouse_clip_left, device_mouse_clip_top, retrieve_width, retrieve_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    unsigned char* surfaceData = buffer;
    unsigned char* sourcePtr;
    unsigned char* destPtr;
    
	Bitmap* retrieveInto = BitmapHelper::CreateBitmap(retrieve_width, retrieve_height, 32);

    if (retrieveInto)
    {
      for (int y = retrieveInto->GetHeight() - 1; y >= 0; y--)
      {
        sourcePtr = surfaceData;
        destPtr = &retrieveInto->GetScanLineForWriting(y)[0];
        for (int x = 0; x < retrieveInto->GetWidth() * 4; x += 4)
        {
          destPtr[x] = sourcePtr[x + 2];
          destPtr[x + 2] = sourcePtr[x];
          destPtr[x + 1] = sourcePtr[x + 1];
          destPtr[x + 3] = sourcePtr[x + 3];
        }
        surfaceData += retrieve_width * 4;
      }

      destination->StretchBlt(retrieveInto, RectWH(0, 0, retrieveInto->GetWidth(), retrieveInto->GetHeight()),
		  RectWH(0, 0, destination->GetWidth(), destination->GetHeight()));
      delete retrieveInto;
    }

    free(buffer);
  }
}

void OGLGraphicsDriver::RenderToBackBuffer()
{
  throw Ali3DException("D3D driver does not have a back buffer");
}

void OGLGraphicsDriver::Render()
{
  Render(kFlip_None);
}

void OGLGraphicsDriver::Render(GlobalFlipType flip)
{
  _render(flip, true);
}

void OGLGraphicsDriver::_reDrawLastFrame()
{
  memcpy(&drawList[0], &drawListLastTime[0], sizeof(SpriteDrawListEntry) * numToDrawLastTime);
  numToDraw = numToDrawLastTime;
}

void OGLGraphicsDriver::_renderSprite(SpriteDrawListEntry *drawListEntry, bool globalLeftRightFlip, bool globalTopBottomFlip)
{
  OGLBitmap *bmpToDraw = drawListEntry->bitmap;

  if (bmpToDraw->_transparency >= 255)
    return;

  float width = bmpToDraw->GetWidthToRender();
  float height = bmpToDraw->GetHeightToRender();
  float xProportion = (float)width / (float)bmpToDraw->_width;
  float yProportion = (float)height / (float)bmpToDraw->_height;

  bool flipLeftToRight = globalLeftRightFlip ^ bmpToDraw->_flipped;
  int drawAtX = drawListEntry->x + _global_x_offset;
  int drawAtY = drawListEntry->y + _global_y_offset;

  for (int ti = 0; ti < bmpToDraw->_numTiles; ti++)
  {
    width = bmpToDraw->_tiles[ti].width * xProportion;
    height = bmpToDraw->_tiles[ti].height * yProportion;
    float xOffs;
    float yOffs = bmpToDraw->_tiles[ti].y * yProportion;
    if (flipLeftToRight != globalLeftRightFlip)
    {
      xOffs = (bmpToDraw->_width - (bmpToDraw->_tiles[ti].x + bmpToDraw->_tiles[ti].width)) * xProportion;
    }
    else
    {
      xOffs = bmpToDraw->_tiles[ti].x * xProportion;
    }
    int thisX = drawAtX + xOffs;
    int thisY = drawAtY + yOffs;

    if (globalLeftRightFlip)
    {
      thisX = (_srcRect.GetWidth() - thisX) - width;
    }
    if (globalTopBottomFlip) 
    {
      thisY = (_srcRect.GetHeight() - thisY) - height;
    }

    thisX = (-(_srcRect.GetWidth() / 2)) + thisX;
    thisY = (_srcRect.GetHeight() / 2) - thisY;


    //Setup translation and scaling matrices
    float widthToScale = (float)width;
    float heightToScale = (float)height;
    if (flipLeftToRight)
    {
      // The usual transform changes 0..1 into 0..width
      // So first negate it (which changes 0..w into -w..0)
      widthToScale = -widthToScale;
      // and now shift it over to make it 0..w again
      thisX += width;
    }
    if (globalTopBottomFlip) 
    {
      heightToScale = -heightToScale;
      thisY -= height;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (bmpToDraw->_transparency == 0)
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    else
      glColor4f(1.0f, 1.0f, 1.0f, ((float)bmpToDraw->_transparency / 255.0f));

    if (_render_to_texture)
      glTranslatef(_srcRect.GetWidth() * _super_sampling / 2.0f, _srcRect.GetHeight() * _super_sampling / 2.0f, 0.0f);
    else
      glTranslatef(device_screen_physical_width / 2.0f, device_screen_physical_height / 2.0f, 0.0f);

    glTranslatef((float)thisX * _scale_width, (float)thisY * _scale_height, 0.0f);
    glScalef(widthToScale * _scale_width, heightToScale * _scale_height, 1.0f);

    glBindTexture(GL_TEXTURE_2D, bmpToDraw->_tiles[ti].texture);

    if ((psp_gfx_smoothing  && !_render_to_texture) || (_smoothScaling) && (bmpToDraw->_stretchToHeight > 0) &&
        ((bmpToDraw->_stretchToHeight != bmpToDraw->_height) ||
         (bmpToDraw->_stretchToWidth != bmpToDraw->_width)))
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else 
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    if (bmpToDraw->_vertex != NULL)
    {
      glTexCoordPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &(bmpToDraw->_vertex[ti * 4].tu));
      glVertexPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &(bmpToDraw->_vertex[ti * 4].position));  
    }
    else
    {
      glTexCoordPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &defaultVertices[0].tu);
      glVertexPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &defaultVertices[0].position);  
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}

void OGLGraphicsDriver::_render(GlobalFlipType flip, bool clearDrawListAfterwards)
{
#if defined(IOS_VERSION)
  ios_select_buffer();
#endif

  if (!device_screen_initialized)
  {
    InitOpenGl();
    device_screen_initialized = 1;
  }

  SpriteDrawListEntry *listToDraw = drawList;
  int listSize = numToDraw;

  bool globalLeftRightFlip = (flip == kFlip_Vertical) || (flip == kFlip_Both);
  bool globalTopBottomFlip = (flip == kFlip_Horizontal) || (flip == kFlip_Both);

  if (_render_to_texture)
  {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);

    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, _srcRect.GetWidth() * _super_sampling, _srcRect.GetHeight() * _super_sampling);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, _srcRect.GetWidth() * _super_sampling - 1, 0, _srcRect.GetHeight() * _super_sampling - 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
  else
  {
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);

    glViewport(0, 0, device_screen_physical_width, device_screen_physical_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, device_screen_physical_width - 1, 0, device_screen_physical_height - 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }

  for (int i = 0; i < listSize; i++)
  {
    if (listToDraw[i].skip)
      continue;

    if (listToDraw[i].bitmap == NULL)
    {
      if (_nullSpriteCallback)
        _nullSpriteCallback(listToDraw[i].x, listToDraw[i].y);
      else
        throw Ali3DException("Unhandled attempt to draw null sprite");

      continue;
    }

    this->_renderSprite(&listToDraw[i], globalLeftRightFlip, globalTopBottomFlip);
  }

  if (!_screenTintSprite.skip)
  {
    this->_renderSprite(&_screenTintSprite, false, false);
  }

  if (_render_to_texture)
  {
#if defined(IOS_VERSION)
    ios_select_buffer();
#else
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#endif

    glViewport(0, 0, device_screen_physical_width, device_screen_physical_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, device_screen_physical_width - 1, 0, device_screen_physical_height - 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (psp_gfx_scaling)
       glTranslatef((device_screen_physical_width - backbuffer_vertices[2] - 1) / 2, (device_screen_physical_height - backbuffer_vertices[5] - 1) / 2, 0);
    else
      glTranslatef(device_screen_physical_width / 2.0f, device_screen_physical_height / 2.0f, 0);

    glBindTexture(GL_TEXTURE_2D, _backbuffer);

    glTexCoordPointer(2, GL_FLOAT, 0, backbuffer_texture_coordinates);
    glVertexPointer(2, GL_FLOAT, 0, backbuffer_vertices);  
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glEnable(GL_BLEND);
  }

  glFinish();

#if defined(WINDOWS_VERSION)
  SwapBuffers(_hDC);
#elif defined(ANDROID_VERSION) || defined(IOS_VERSION)
  device_swap_buffers();
#endif

  if (clearDrawListAfterwards)
  {
    numToDrawLastTime = numToDraw;
    memcpy(&drawListLastTime[0], &drawList[0], sizeof(SpriteDrawListEntry) * listSize);
    flipTypeLastTime = flip;
    ClearDrawList();
  }
}

void OGLGraphicsDriver::ClearDrawList()
{
  numToDraw = 0;
}

void OGLGraphicsDriver::DrawSprite(int x, int y, IDriverDependantBitmap* bitmap)
{
  if (numToDraw >= MAX_DRAW_LIST_SIZE)
  {
    throw Ali3DException("Too many sprites to draw in one frame");
  }

  drawList[numToDraw].bitmap = (OGLBitmap*)bitmap;
  drawList[numToDraw].x = x;
  drawList[numToDraw].y = y;
  drawList[numToDraw].skip = false;
  numToDraw++;
}

void OGLGraphicsDriver::DestroyDDB(IDriverDependantBitmap* bitmap)
{
  for (int i = 0; i < numToDrawLastTime; i++)
  {
    if (drawListLastTime[i].bitmap == bitmap)
    {
      drawListLastTime[i].skip = true;
    }
  }
  delete ((OGLBitmap*)bitmap);
}

__inline void get_pixel_if_not_transparent15(unsigned short *pixel, unsigned short *red, unsigned short *green, unsigned short *blue, unsigned short *divisor)
{
  if (pixel[0] != MASK_COLOR_15)
  {
    *red += algetr15(pixel[0]);
    *green += algetg15(pixel[0]);
    *blue += algetb15(pixel[0]);
    divisor[0]++;
  }
}

__inline void get_pixel_if_not_transparent32(unsigned int *pixel, unsigned int *red, unsigned int *green, unsigned int *blue, unsigned int *divisor)
{
  if (pixel[0] != MASK_COLOR_32)
  {
    *red += algetr32(pixel[0]);
    *green += algetg32(pixel[0]);
    *blue += algetb32(pixel[0]);
    divisor[0]++;
  }
}

#define D3DCOLOR_RGBA(r,g,b,a) \
  (((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))


void OGLGraphicsDriver::UpdateTextureRegion(TextureTile *tile, Bitmap *bitmap, OGLBitmap *target, bool hasAlpha)
{
  int textureHeight = tile->height;
  int textureWidth = tile->width;

  AdjustSizeToNearestSupportedByCard(&textureWidth, &textureHeight);

  int tileWidth = (textureWidth > tile->width) ? tile->width + 1 : tile->width;
  int tileHeight = (textureHeight > tile->height) ? tile->height + 1 : tile->height;

  bool usingLinearFiltering = (psp_gfx_smoothing == 1); //_filter->NeedToColourEdgeLines();
  bool lastPixelWasTransparent = false;
  char *origPtr = (char*)malloc(4 * tileWidth * tileHeight);
  char *memPtr = origPtr;
  for (int y = 0; y < tileHeight; y++)
  {
    // Mimic the behaviour of GL_CLAMP_EDGE for the bottom line
    if (y == tile->height)
    {
      unsigned int* memPtrLong = (unsigned int*)memPtr;
      unsigned int* memPtrLong_previous = (unsigned int*)(memPtr - tileWidth * 4);

      for (int x = 0; x < tileWidth; x++)
        memPtrLong[x] = memPtrLong_previous[x] & 0x00FFFFFF;

      continue;
    }

    lastPixelWasTransparent = false;
    const uint8_t *scanline_before = bitmap->GetScanLine(y + tile->y - 1);
    const uint8_t *scanline_at     = bitmap->GetScanLine(y + tile->y);
    const uint8_t *scanline_after  = bitmap->GetScanLine(y + tile->y + 1);
    for (int x = 0; x < tileWidth; x++)
    {

/*    if (target->_colDepth == 15)
      {
        unsigned short* memPtrShort = (unsigned short*)memPtr;
        unsigned short* srcData = (unsigned short*)&bitmap->GetScanLine[y + tile->y][(x + tile->x) * 2];
        if (*srcData == MASK_COLOR_15) 
        {
          if (target->_opaque)  // set to black if opaque
            memPtrShort[x] = 0x8000;
          else if (!usingLinearFiltering)
            memPtrShort[x] = 0;
          // set to transparent, but use the colour from the neighbouring 
          // pixel to stop the linear filter doing black outlines
          else
          {
            unsigned short red = 0, green = 0, blue = 0, divisor = 0;
            if (x > 0)
              get_pixel_if_not_transparent15(&srcData[-1], &red, &green, &blue, &divisor);
            if (x < tile->width - 1)
              get_pixel_if_not_transparent15(&srcData[1], &red, &green, &blue, &divisor);
            if (y > 0)
              get_pixel_if_not_transparent15((unsigned short*)&bitmap->GetScanLine[y + tile->y - 1][(x + tile->x) * 2], &red, &green, &blue, &divisor);
            if (y < tile->height - 1)
              get_pixel_if_not_transparent15((unsigned short*)&bitmap->GetScanLine[y + tile->y + 1][(x + tile->x) * 2], &red, &green, &blue, &divisor);
            if (divisor > 0)
              memPtrShort[x] = ((red / divisor) << 10) | ((green / divisor) << 5) | (blue / divisor);
            else
              memPtrShort[x] = 0;
          }
          lastPixelWasTransparent = true;
        }
        else
        {
          memPtrShort[x] = 0x8000 | (algetr15(*srcData) << 10) | (algetg15(*srcData) << 5) | algetb15(*srcData);
          if (lastPixelWasTransparent)
          {
            // update the colour of the previous tranparent pixel, to
            // stop black outlines when linear filtering
            memPtrShort[x - 1] = memPtrShort[x] & 0x7FFF;
            lastPixelWasTransparent = false;
          }
        }
      }
      else if (target->_colDepth == 32)
*/
      {
        unsigned int* memPtrLong = (unsigned int*)memPtr;

        if (x == tile->width)
        {
          memPtrLong[x] = memPtrLong[x - 1] & 0x00FFFFFF;
          continue;
        }

        unsigned int* srcData = (unsigned int*)&scanline_at[(x + tile->x) << 2];
        if (*srcData == MASK_COLOR_32)
        {
          if (target->_opaque)  // set to black if opaque
            memPtrLong[x] = 0xFF000000;
          else if (!usingLinearFiltering)
            memPtrLong[x] = 0;
          // set to transparent, but use the colour from the neighbouring 
          // pixel to stop the linear filter doing black outlines
          else
          {
            unsigned int red = 0, green = 0, blue = 0, divisor = 0;
            if (x > 0)
              get_pixel_if_not_transparent32(&srcData[-1], &red, &green, &blue, &divisor);
            if (x < tile->width - 1)
              get_pixel_if_not_transparent32(&srcData[1], &red, &green, &blue, &divisor);
            if (y > 0)
              get_pixel_if_not_transparent32((unsigned int*)&scanline_before[(x + tile->x) << 2], &red, &green, &blue, &divisor);
            if (y < tile->height - 1)
              get_pixel_if_not_transparent32((unsigned int*)&scanline_after[(x + tile->x) << 2], &red, &green, &blue, &divisor);
            if (divisor > 0)
              memPtrLong[x] = ((red / divisor) << 16) | ((green / divisor) << 8) | (blue / divisor);
            else
              memPtrLong[x] = 0;
          }
          lastPixelWasTransparent = true;
        }
        else if (hasAlpha)
        {
          memPtrLong[x] = D3DCOLOR_RGBA(algetr32(*srcData), algetg32(*srcData), algetb32(*srcData), algeta32(*srcData));
        }
        else
        {
          memPtrLong[x] = D3DCOLOR_RGBA(algetr32(*srcData), algetg32(*srcData), algetb32(*srcData), 0xff);
          if (lastPixelWasTransparent)
          {
            // update the colour of the previous tranparent pixel, to
            // stop black outlines when linear filtering
            memPtrLong[x - 1] = memPtrLong[x] & 0x00FFFFFF;
            lastPixelWasTransparent = false;
          }
        }
      }
    }

    memPtr += tileWidth * 4;
  }

  unsigned int newTexture = tile->texture;

  glBindTexture(GL_TEXTURE_2D, tile->texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, origPtr);

  free(origPtr);
}

void OGLGraphicsDriver::UpdateDDBFromBitmap(IDriverDependantBitmap* bitmapToUpdate, Bitmap *bitmap, bool hasAlpha)
{
  OGLBitmap *target = (OGLBitmap*)bitmapToUpdate;
  Bitmap *source = bitmap;
  if ((target->_width == bitmap->GetWidth()) &&
     (target->_height == bitmap->GetHeight()))
  {
    if (bitmap->GetColorDepth() != target->_colDepth)
    {
      //throw Ali3DException("Mismatched colour depths");
      source = BitmapHelper::CreateBitmapCopy(bitmap, 32);
    }

    target->_hasAlpha = hasAlpha;

    for (int i = 0; i < target->_numTiles; i++)
    {
      UpdateTextureRegion(&target->_tiles[i], source, target, hasAlpha);
    }

    if (source != bitmap)
      delete source;
  }
}

Bitmap *OGLGraphicsDriver::ConvertBitmapToSupportedColourDepth(Bitmap *bitmap)
{
   int colorConv = get_color_conversion();
   set_color_conversion(COLORCONV_KEEP_TRANS | COLORCONV_TOTAL);

   int colourDepth = bitmap->GetColorDepth();
/*   if ((colourDepth == 8) || (colourDepth == 16))
   {
     // Most 3D cards don't support 8-bit; and we need 15-bit colour
     Bitmap* tempBmp = BitmapHelper::CreateBitmap_(15, bitmap->GetWidth(), bitmap->GetHeight());
     Blit(bitmap, tempBmp, 0, 0, 0, 0, tempBmp->GetWidth(), tempBmp->GetHeight());
     destroy_bitmap(bitmap);
     set_color_conversion(colorConv);
     return tempBmp;
   }
*/   if (colourDepth != 32)
   {
     // we need 32-bit colour
     Bitmap *tempBmp = BitmapHelper::CreateBitmapCopy(bitmap, 32);
     delete bitmap;
     set_color_conversion(colorConv);
     return tempBmp;
   }
   return bitmap;
}

void OGLGraphicsDriver::AdjustSizeToNearestSupportedByCard(int *width, int *height)
{
  int allocatedWidth = *width, allocatedHeight = *height;

    bool foundWidth = false, foundHeight = false;
    int tryThis = 2;
    while ((!foundWidth) || (!foundHeight))
    {
      if ((tryThis >= allocatedWidth) && (!foundWidth))
      {
        allocatedWidth = tryThis;
        foundWidth = true;
      }

      if ((tryThis >= allocatedHeight) && (!foundHeight))
      {
        allocatedHeight = tryThis;
        foundHeight = true;
      }

      tryThis = tryThis << 1;
    }

  *width = allocatedWidth;
  *height = allocatedHeight;
}



IDriverDependantBitmap* OGLGraphicsDriver::CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque)
{
  int allocatedWidth = bitmap->GetWidth();
  int allocatedHeight = bitmap->GetHeight();
  Bitmap *tempBmp = NULL;
  int colourDepth = bitmap->GetColorDepth();
/*  if ((colourDepth == 8) || (colourDepth == 16))
  {
    // Most 3D cards don't support 8-bit; and we need 15-bit colour
    tempBmp = BitmapHelper::CreateBitmap_(15, bitmap->GetWidth(), bitmap->GetHeight());
    Blit(bitmap, tempBmp, 0, 0, 0, 0, tempBmp->GetWidth(), tempBmp->GetHeight());
    bitmap = tempBmp;
    colourDepth = 15;
  }
*/  if (colourDepth != 32)
  {
    // we need 32-bit colour
	tempBmp = BitmapHelper::CreateBitmapCopy(bitmap, 32);
    bitmap = tempBmp;
    colourDepth = 32;
  }

  OGLBitmap *ddb = new OGLBitmap(bitmap->GetWidth(), bitmap->GetHeight(), colourDepth, opaque);

  AdjustSizeToNearestSupportedByCard(&allocatedWidth, &allocatedHeight);
  int tilesAcross = 1, tilesDown = 1;

  // *************** REMOVE THESE LINES *************
  //direct3ddevicecaps.MaxTextureWidth = 64;
  //direct3ddevicecaps.MaxTextureHeight = 256;
  // *************** END REMOVE THESE LINES *************

  // Calculate how many textures will be necessary to
  // store this image

  int MaxTextureWidth = 512;
  int MaxTextureHeight = 512;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTextureWidth);
  MaxTextureHeight = MaxTextureWidth;

  tilesAcross = (allocatedWidth + MaxTextureWidth - 1) / MaxTextureWidth;
  tilesDown = (allocatedHeight + MaxTextureHeight - 1) / MaxTextureHeight;
  int tileWidth = bitmap->GetWidth() / tilesAcross;
  int lastTileExtraWidth = bitmap->GetWidth() % tilesAcross;
  int tileHeight = bitmap->GetHeight() / tilesDown;
  int lastTileExtraHeight = bitmap->GetHeight() % tilesDown;
  int tileAllocatedWidth = tileWidth;
  int tileAllocatedHeight = tileHeight;

  AdjustSizeToNearestSupportedByCard(&tileAllocatedWidth, &tileAllocatedHeight);

  int numTiles = tilesAcross * tilesDown;
  TextureTile *tiles = (TextureTile*)malloc(sizeof(TextureTile) * numTiles);
  memset(tiles, 0, sizeof(TextureTile) * numTiles);

  OGLCUSTOMVERTEX *vertices = NULL;

  if ((numTiles == 1) &&
      (allocatedWidth == bitmap->GetWidth()) &&
      (allocatedHeight == bitmap->GetHeight()))
  {
    // use default whole-image vertices
  }
  else
  {
     // The texture is not the same as the bitmap, so create some custom vertices
     // so that only the relevant portion of the texture is rendered
     int vertexBufferSize = numTiles * 4 * sizeof(OGLCUSTOMVERTEX);

     ddb->_vertex = vertices = (OGLCUSTOMVERTEX*)malloc(vertexBufferSize);
  }

  for (int x = 0; x < tilesAcross; x++)
  {
    for (int y = 0; y < tilesDown; y++)
    {
      TextureTile *thisTile = &tiles[y * tilesAcross + x];
      int thisAllocatedWidth = tileAllocatedWidth;
      int thisAllocatedHeight = tileAllocatedHeight;
      thisTile->x = x * tileWidth;
      thisTile->y = y * tileHeight;
      thisTile->width = tileWidth;
      thisTile->height = tileHeight;
      if (x == tilesAcross - 1) 
      {
        thisTile->width += lastTileExtraWidth;
        thisAllocatedWidth = thisTile->width;
        AdjustSizeToNearestSupportedByCard(&thisAllocatedWidth, &thisAllocatedHeight);
      }
      if (y == tilesDown - 1) 
      {
        thisTile->height += lastTileExtraHeight;
        thisAllocatedHeight = thisTile->height;
        AdjustSizeToNearestSupportedByCard(&thisAllocatedWidth, &thisAllocatedHeight);
      }

      if (vertices != NULL)
      {
        for (int vidx = 0; vidx < 4; vidx++)
        {
          int i = (y * tilesAcross + x) * 4 + vidx;
          vertices[i] = defaultVertices[vidx];
          if (vertices[i].tu > 0.0)
          {
            vertices[i].tu = (float)thisTile->width / (float)thisAllocatedWidth;
          }
          if (vertices[i].tv > 0.0)
          {
            vertices[i].tv = (float)thisTile->height / (float)thisAllocatedHeight;
          }
        }
      }

      glGenTextures(1, &thisTile->texture);
      glBindTexture(GL_TEXTURE_2D, thisTile->texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, thisAllocatedWidth, thisAllocatedHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
  }

  ddb->_numTiles = numTiles;
  ddb->_tiles = tiles;

  UpdateDDBFromBitmap(ddb, bitmap, hasAlpha);

  delete tempBmp;

  return ddb;
}

void OGLGraphicsDriver::do_fade(bool fadingOut, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue)
{
  if (fadingOut)
  {
    this->_reDrawLastFrame();
  }
  else if (_drawScreenCallback != NULL)
    _drawScreenCallback();
  
  Bitmap *blackSquare = BitmapHelper::CreateBitmap(16, 16, 32);
  blackSquare->Clear(makecol32(targetColourRed, targetColourGreen, targetColourBlue));
  IDriverDependantBitmap *d3db = this->CreateDDBFromBitmap(blackSquare, false, false);
  delete blackSquare;

  d3db->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight());
  this->DrawSprite(-_global_x_offset, -_global_y_offset, d3db);

  if (speed <= 0) speed = 16;
  speed *= 2;  // harmonise speeds with software driver which is faster
  for (int a = 1; a < 255; a += speed)
  {
    int timerValue = *_loopTimer;
    d3db->SetTransparency(fadingOut ? a : (255 - a));
    this->_render(flipTypeLastTime, false);

    do
    {
      if (_pollingCallback)
        _pollingCallback();
      platform->YieldCPU();
    }
    while (timerValue == *_loopTimer);

  }

  if (fadingOut)
  {
    d3db->SetTransparency(0);
    this->_render(flipTypeLastTime, false);
  }

  this->DestroyDDB(d3db);
  this->ClearDrawList();
}

void OGLGraphicsDriver::FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) 
{
  do_fade(true, speed, targetColourRed, targetColourGreen, targetColourBlue);
}

void OGLGraphicsDriver::FadeIn(int speed, PALLETE p, int targetColourRed, int targetColourGreen, int targetColourBlue) 
{
  do_fade(false, speed, targetColourRed, targetColourGreen, targetColourBlue);
}

void OGLGraphicsDriver::BoxOutEffect(bool blackingOut, int speed, int delay)
{
  if (blackingOut)
  {
    this->_reDrawLastFrame();
  }
  else if (_drawScreenCallback != NULL)
    _drawScreenCallback();
  
  Bitmap *blackSquare = BitmapHelper::CreateBitmap(16, 16, 32);
  blackSquare->Clear();
  IDriverDependantBitmap *d3db = this->CreateDDBFromBitmap(blackSquare, false, false);
  delete blackSquare;

  d3db->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight());
  this->DrawSprite(0, 0, d3db);
  if (!blackingOut)
  {
    // when fading in, draw four black boxes, one
    // across each side of the screen
    this->DrawSprite(0, 0, d3db);
    this->DrawSprite(0, 0, d3db);
    this->DrawSprite(0, 0, d3db);
  }

  int yspeed = _srcRect.GetHeight() / (_srcRect.GetWidth() / speed);
  int boxWidth = speed;
  int boxHeight = yspeed;

  while (boxWidth < _srcRect.GetWidth())
  {
    boxWidth += speed;
    boxHeight += yspeed;
    if (blackingOut)
    {
      this->drawList[this->numToDraw - 1].x = _srcRect.GetWidth() / 2- boxWidth / 2;
      this->drawList[this->numToDraw - 1].y = _srcRect.GetHeight() / 2 - boxHeight / 2;
      d3db->SetStretch(boxWidth, boxHeight);
    }
    else
    {
      this->drawList[this->numToDraw - 4].x = _srcRect.GetWidth() / 2 - boxWidth / 2 - _srcRect.GetWidth();
      this->drawList[this->numToDraw - 3].y = _srcRect.GetHeight() / 2 - boxHeight / 2 - _srcRect.GetHeight();
      this->drawList[this->numToDraw - 2].x = _srcRect.GetWidth() / 2 + boxWidth / 2;
      this->drawList[this->numToDraw - 1].y = _srcRect.GetHeight() / 2 + boxHeight / 2;
      d3db->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight());
    }
    
    this->_render(flipTypeLastTime, false);

    if (_pollingCallback)
      _pollingCallback();
    platform->Delay(delay);
  }

  this->DestroyDDB(d3db);
  this->ClearDrawList();
}

bool OGLGraphicsDriver::PlayVideo(const char *filename, bool useAVISound, VideoSkipType skipType, bool stretchToFullScreen)
{
 return true;
}

void OGLGraphicsDriver::create_screen_tint_bitmap() 
{
	_screenTintLayer = BitmapHelper::CreateBitmap(16, 16, _mode.ColorDepth);
  _screenTintLayer = this->ConvertBitmapToSupportedColourDepth(_screenTintLayer);
  _screenTintLayerDDB = (OGLBitmap*)this->CreateDDBFromBitmap(_screenTintLayer, false, false);
  _screenTintSprite.bitmap = _screenTintLayerDDB;
}

void OGLGraphicsDriver::SetScreenTint(int red, int green, int blue)
{ 
  if ((red != _tint_red) || (green != _tint_green) || (blue != _tint_blue))
  {
    _tint_red = red; 
    _tint_green = green; 
    _tint_blue = blue;

    _screenTintLayer->Clear(makecol_depth(_screenTintLayer->GetColorDepth(), red, green, blue));
    this->UpdateDDBFromBitmap(_screenTintLayerDDB, _screenTintLayer, false);
    _screenTintLayerDDB->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight());
    _screenTintLayerDDB->SetTransparency(128);

    _screenTintSprite.skip = ((red == 0) && (green == 0) && (blue == 0));
  }
}


OGLGraphicsFactory *OGLGraphicsFactory::_factory = NULL;

OGLGraphicsFactory::~OGLGraphicsFactory()
{
    _factory = NULL;
}

size_t OGLGraphicsFactory::GetFilterCount() const
{
    return 1;
}

const GfxFilterInfo *OGLGraphicsFactory::GetFilterInfo(size_t index) const
{
    return index == 0 ? &OGLGfxFilter::FilterInfo : NULL;
}

/* static */ OGLGraphicsFactory *OGLGraphicsFactory::GetFactory()
{
    if (!_factory)
        _factory = new OGLGraphicsFactory();
    return _factory;
}

OGLGraphicsDriver *OGLGraphicsFactory::EnsureDriverCreated()
{
    if (!_driver)
        _driver = new OGLGraphicsDriver();
    return _driver;
}

OGLGfxFilter *OGLGraphicsFactory::CreateFilter(const String &id)
{
    if (OGLGfxFilter::FilterInfo.Id.CompareNoCase(id) == 0)
        return new OGLGfxFilter();
    return NULL;
}

} // namespace OGL
} // namespace Engine
} // namespace AGS

#endif // only on Windows, Android and iOS
