/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/shared/core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS || AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_LINUX

//include <algorithm>
#include "ags/shared/gfx/ali3dexception.h"
#include "ags/shared/gfx/ali3dogl.h"
#include "ags/shared/gfx/gfxfilter_ogl.h"
#include "ags/shared/gfx/gfxfilter_aaogl.h"
#include "ags/shared/gfx/gfx_util.h"
#include "ags/shared/main/main_allegro.h"
#include "ags/shared/platform/base/agsplatformdriver.h"
#include "ags/shared/util/math.h"
#include "ags/shared/ac/timer.h"

namespace AGS3 {

#if AGS_PLATFORM_OS_ANDROID

#define glOrtho glOrthof
#define GL_CLAMP GL_CLAMP_TO_EDGE

// Defined in Allegro
extern "C"
{
	void android_swap_buffers();
	void android_create_screen(int width, int height, int color_depth);
	void android_mouse_setup(int left, int right, int top, int bottom, float scaling_x, float scaling_y);
}

extern "C" void android_debug_printf(char *format, ...);

extern unsigned int android_screen_physical_width;
extern unsigned int android_screen_physical_height;
extern int android_screen_initialized;

#define device_screen_initialized android_screen_initialized
#define device_mouse_setup android_mouse_setup
#define device_swap_buffers android_swap_buffers

const char *fbo_extension_string = "GL_OES_framebuffer_object";

#define glGenFramebuffersEXT glGenFramebuffersOES
#define glDeleteFramebuffersEXT glDeleteFramebuffersOES
#define glBindFramebufferEXT glBindFramebufferOES
#define glCheckFramebufferStatusEXT glCheckFramebufferStatusOES
#define glGetFramebufferAttachmentParameterivEXT glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapEXT glGenerateMipmapOES
#define glFramebufferTexture2DEXT glFramebufferTexture2DOES
#define glFramebufferRenderbufferEXT glFramebufferRenderbufferOES
// TODO: probably should use EGL and function eglSwapInterval on Android to support setting swap interval
// For now this is a dummy function pointer which is only used to test that function is not supported
const void (*glSwapIntervalEXT)(int) = NULL;

#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES

#elif AGS_PLATFORM_OS_IOS

extern "C"
{
	void ios_swap_buffers();
	void ios_select_buffer();
	void ios_create_screen();
	float get_device_scale();
	void ios_mouse_setup(int left, int right, int top, int bottom, float scaling_x, float scaling_y);
}

#define glOrtho glOrthof
#define GL_CLAMP GL_CLAMP_TO_EDGE

extern unsigned int ios_screen_physical_width;
extern unsigned int ios_screen_physical_height;
extern int ios_screen_initialized;

#define device_screen_initialized ios_screen_initialized
#define device_mouse_setup ios_mouse_setup
#define device_swap_buffers ios_swap_buffers

const char *fbo_extension_string = "GL_OES_framebuffer_object";

#define glGenFramebuffersEXT glGenFramebuffersOES
#define glDeleteFramebuffersEXT glDeleteFramebuffersOES
#define glBindFramebufferEXT glBindFramebufferOES
#define glCheckFramebufferStatusEXT glCheckFramebufferStatusOES
#define glGetFramebufferAttachmentParameterivEXT glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapEXT glGenerateMipmapOES
#define glFramebufferTexture2DEXT glFramebufferTexture2DOES
#define glFramebufferRenderbufferEXT glFramebufferRenderbufferOES
// TODO: find out how to support swap interval setting on iOS
// For now this is a dummy function pointer which is only used to test that function is not supported
const void (*glSwapIntervalEXT)(int) = NULL;

#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0_OES

#endif

// Necessary to update textures from 8-bit bitmaps
extern RGB palette[256];


namespace AGS {
namespace Engine {
namespace OGL {

using namespace AGS::Shared;

void ogl_dummy_vsync() { }

#define GFX_OPENGL  AL_ID('O','G','L',' ')

GFX_DRIVER gfx_opengl = {
	GFX_OPENGL,
	empty_string,
	empty_string,
	"OpenGL",
	nullptr,    // init
	nullptr,   // exit
	nullptr,                        // AL_METHOD(int, scroll, (int x, int y));
	ogl_dummy_vsync,   // vsync
	nullptr,  // setpalette
	nullptr,                        // AL_METHOD(int, request_scroll, (int x, int y));
	nullptr,                        // AL_METHOD(int, poll_scroll, (void));
	nullptr,                        // AL_METHOD(void, enable_triple_buffer, (void));
	nullptr,  //create_video_bitmap
	nullptr,  //destroy_video_bitmap
	nullptr,   //show_video_bitmap
	nullptr,
	nullptr,  //gfx_directx_create_system_bitmap,
	nullptr, //gfx_directx_destroy_system_bitmap,
	nullptr, //gfx_directx_set_mouse_sprite,
	nullptr, //gfx_directx_show_mouse,
	nullptr, //gfx_directx_hide_mouse,
	nullptr, //gfx_directx_move_mouse,
	nullptr,                        // AL_METHOD(void, drawing_mode, (void));
	nullptr,                        // AL_METHOD(void, save_video_state, (void*));
	nullptr,                        // AL_METHOD(void, restore_video_state, (void*));
	nullptr,                        // AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a));
	nullptr,                        // AL_METHOD(int, fetch_mode_list, (void));
	0, 0,                        // int w, h;
	FALSE,                        // int linear;
	0,                           // long bank_size;
	0,                           // long bank_gran;
	0,                           // long vid_mem;
	0,                           // long vid_phys_base;
	TRUE                         // int windowed;
};

void OGLBitmap::Dispose() {
	if (_tiles != nullptr) {
		for (int i = 0; i < _numTiles; i++)
			glDeleteTextures(1, &(_tiles[i].texture));

		free(_tiles);
		_tiles = nullptr;
		_numTiles = 0;
	}
	if (_vertex != nullptr) {
		free(_vertex);
		_vertex = nullptr;
	}
}


OGLGraphicsDriver::ShaderProgram::ShaderProgram() : Program(0), SamplerVar(0), ColorVar(0), AuxVar(0) {}


OGLGraphicsDriver::OGLGraphicsDriver() {
#if AGS_PLATFORM_OS_WINDOWS
	_hDC = NULL;
	_hRC = NULL;
	_hWnd = NULL;
	_hInstance = NULL;
	device_screen_physical_width  = 0;
	device_screen_physical_height = 0;
#elif AGS_PLATFORM_OS_LINUX
	device_screen_physical_width  = 0;
	device_screen_physical_height = 0;
	_glxContext = nullptr;
	_have_window = false;
#elif AGS_PLATFORM_OS_ANDROID
	device_screen_physical_width  = android_screen_physical_width;
	device_screen_physical_height = android_screen_physical_height;
#elif AGS_PLATFORM_OS_IOS
	device_screen_physical_width  = ios_screen_physical_width;
	device_screen_physical_height = ios_screen_physical_height;
#endif

	_firstTimeInit = false;
	_backbuffer = 0;
	_fbo = 0;
	_legacyPixelShader = false;
	_can_render_to_texture = false;
	_do_render_to_texture = false;
	_super_sampling = 1;
	SetupDefaultVertices();

	// Shifts comply to GL_RGBA
	_vmem_r_shift_32 = 0;
	_vmem_g_shift_32 = 8;
	_vmem_b_shift_32 = 16;
	_vmem_a_shift_32 = 24;
}


void OGLGraphicsDriver::SetupDefaultVertices() {
	std::fill(_backbuffer_vertices, _backbuffer_vertices + sizeof(_backbuffer_vertices) / sizeof(GLfloat), 0.0f);
	std::fill(_backbuffer_texture_coordinates, _backbuffer_texture_coordinates + sizeof(_backbuffer_texture_coordinates) / sizeof(GLfloat), 0.0f);

	defaultVertices[0].position.x = 0.0f;
	defaultVertices[0].position.y = 0.0f;
	defaultVertices[0].tu = 0.0;
	defaultVertices[0].tv = 0.0;

	defaultVertices[1].position.x = 1.0f;
	defaultVertices[1].position.y = 0.0f;
	defaultVertices[1].tu = 1.0;
	defaultVertices[1].tv = 0.0;

	defaultVertices[2].position.x = 0.0f;
	defaultVertices[2].position.y = -1.0f;
	defaultVertices[2].tu = 0.0;
	defaultVertices[2].tv = 1.0;

	defaultVertices[3].position.x = 1.0f;
	defaultVertices[3].position.y = -1.0f;
	defaultVertices[3].tu = 1.0;
	defaultVertices[3].tv = 1.0;
}

#if AGS_PLATFORM_OS_WINDOWS || AGS_PLATFORM_OS_LINUX

void OGLGraphicsDriver::CreateDesktopScreen(int width, int height, int depth) {
	device_screen_physical_width = width;
	device_screen_physical_height = height;
}

#elif AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS

void OGLGraphicsDriver::UpdateDeviceScreen() {
#if AGS_PLATFORM_OS_ANDROID
	device_screen_physical_width  = android_screen_physical_width;
	device_screen_physical_height = android_screen_physical_height;
#elif AGS_PLATFORM_OS_IOS
	device_screen_physical_width  = ios_screen_physical_width;
	device_screen_physical_height = ios_screen_physical_height;
#endif

	Debug::Printf("OGL: notified of device screen updated to %d x %d, resizing viewport", device_screen_physical_width, device_screen_physical_height);
	_mode.Width = device_screen_physical_width;
	_mode.Height = device_screen_physical_height;
	InitGlParams(_mode);
	if (_initSurfaceUpdateCallback)
		_initSurfaceUpdateCallback();
}

#endif

void OGLGraphicsDriver::Vsync() {
	// do nothing on OpenGL
}

void OGLGraphicsDriver::RenderSpritesAtScreenResolution(bool enabled, int supersampling) {
	if (_can_render_to_texture) {
		_do_render_to_texture = !enabled;
		_super_sampling = supersampling;
		TestSupersampling();
	}

	if (_do_render_to_texture)
		glDisable(GL_SCISSOR_TEST);
}

bool OGLGraphicsDriver::IsModeSupported(const DisplayMode &mode) {
	if (mode.Width <= 0 || mode.Height <= 0 || mode.ColorDepth <= 0) {
		set_allegro_error("Invalid resolution parameters: %d x %d x %d", mode.Width, mode.Height, mode.ColorDepth);
		return false;
	}
	return true;
}

bool OGLGraphicsDriver::SupportsGammaControl() {
	return false;
}

void OGLGraphicsDriver::SetGamma(int newGamma) {
}

void OGLGraphicsDriver::SetGraphicsFilter(POGLFilter filter) {
	_filter = filter;
	OnSetFilter();
}

void OGLGraphicsDriver::SetTintMethod(TintMethod method) {
	_legacyPixelShader = (method == TintReColourise);
}

void OGLGraphicsDriver::FirstTimeInit() {
	String ogl_v_str;
#ifdef GLAPI
	ogl_v_str.Format("%d.%d", GLVersion.major, GLVersion.minor);
#else
	ogl_v_str = (const char *)glGetString(GL_VERSION);
#endif
	Debug::Printf(kDbgMsg_Info, "Running OpenGL: %s", ogl_v_str.GetCStr());

	// Initialize default sprite batch, it will be used when no other batch was activated
	OGLGraphicsDriver::InitSpriteBatch(0, _spriteBatchDesc[0]);

	TestRenderToTexture();
	CreateShaders();
	_firstTimeInit = true;
}

#if AGS_PLATFORM_OS_LINUX
Atom get_x_atom(const char *atom_name) {
	Atom atom = XInternAtom(_xwin.display, atom_name, False);
	if (atom == None) {
		Debug::Printf(kDbgMsg_Error, "ERROR: X11 atom \"%s\" not found.\n", atom_name);
	}
	return atom;
}
#endif

bool OGLGraphicsDriver::InitGlScreen(const DisplayMode &mode) {
#if AGS_PLATFORM_OS_ANDROID
	android_create_screen(mode.Width, mode.Height, mode.ColorDepth);
#elif AGS_PLATFORM_OS_IOS
	ios_create_screen();
	ios_select_buffer();
#elif AGS_PLATFORM_OS_WINDOWS
	if (mode.Windowed) {
		platform->AdjustWindowStyleForWindowed();
	} else {
		if (platform->EnterFullscreenMode(mode))
			platform->AdjustWindowStyleForFullscreen();
	}

	// NOTE: adjust_window may leave task bar visible, so we do not use it for fullscreen mode
	if (mode.Windowed && adjust_window(mode.Width, mode.Height) != 0) {
		set_allegro_error("Window size not supported");
		return false;
	}

	_hWnd = win_get_window();
	if (!(_hDC = GetDC(_hWnd)))
		return false;

	// First check if we need to recreate GL context, this will only be
	// required if different color depth is requested.
	if (_hRC) {
		GLuint pixel_fmt = GetPixelFormat(_hDC);
		PIXELFORMATDESCRIPTOR pfd;
		DescribePixelFormat(_hDC, pixel_fmt, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		if (pfd.cColorBits != mode.ColorDepth) {
			DeleteGlContext();
		}
	}

	if (!_hRC) {
		if (!CreateGlContext(mode))
			return false;
	}

	if (!gladLoadWGL(_hDC)) {
		Debug::Printf(kDbgMsg_Error, "Failed to load WGL.");
		return false;
	}

	if (!gladLoadGL()) {
		Debug::Printf(kDbgMsg_Error, "Failed to load GL.");
		return false;
	}

	CreateDesktopScreen(mode.Width, mode.Height, mode.ColorDepth);
	win_grab_input();
#elif AGS_PLATFORM_OS_LINUX
	if (!_have_window) {
		// Use Allegro to create our window. We don't care what size Allegro uses
		// here, we will set that ourselves below by manipulating members of
		// Allegro's_xwin structure. We need to use the Allegro routine here (rather
		// than create our own X window) to remain compatible with Allegro's mouse &
		// keyboard handling.
		//
		// Note that although _xwin contains a special "fullscreen" Window member
		// (_xwin.fs_window), we do not use it for going fullscreen. Instead we ask
		// the window manager to take the "managed" Window (_xwin.wm_window)
		// fullscreen for us. All drawing goes to the "real" Window (_xwin.window).
		if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 0, 0, 0, 0) != 0)
			return false;
		_have_window = true;
	}

	if (!gladLoadGLX(_xwin.display, DefaultScreen(_xwin.display))) {
		Debug::Printf(kDbgMsg_Error, "Failed to load GLX.");
		return false;
	}

	if (!_glxContext && !CreateGlContext(mode))
		return false;

	if (!gladLoadGL()) {
		Debug::Printf(kDbgMsg_Error, "Failed to load GL.");
		return false;
	}

	{
		// Set the size of our "managed" window.
		XSizeHints *hints = XAllocSizeHints();

		if (hints) {
			if (mode.Windowed) {
				// Set a fixed-size window. This is copied from Allegro 4's
				// _xwin_private_create_screen().
				hints->flags = PMinSize | PMaxSize | PBaseSize;
				hints->min_width  = hints->max_width  = hints->base_width  = mode.Width;
				hints->min_height = hints->max_height = hints->base_height = mode.Height;
			} else {
				// Clear any previously set demand for a fixed-size window, otherwise
				// the window manager will ignore our request to go full-screen.
				hints->flags = 0;
			}

			XSetWMNormalHints(_xwin.display, _xwin.wm_window, hints);
		}

		XFree(hints);
	}

	// Set the window we are actually drawing into to the desired size.
	XResizeWindow(_xwin.display, _xwin.window, mode.Width, mode.Height);

	// Make Allegro aware of the new window size, otherwise the mouse cursor
	// movement may be erratic.
	_xwin.window_width = mode.Width;
	_xwin.window_height = mode.Height;

	{
		// Ask the window manager to add (or remove) the "fullscreen" property on
		// our top-level window.
		const Atom wm_state = get_x_atom("_NET_WM_STATE");
		const Atom fullscreen = get_x_atom("_NET_WM_STATE_FULLSCREEN");
		const long remove_property = 0;
		const long add_property = 1;

		XEvent xev;
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = _xwin.wm_window;
		xev.xclient.message_type = wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = mode.Windowed ? remove_property : add_property;
		xev.xclient.data.l[1] = fullscreen;
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 1; // Message source is a regular application.
		Status status = XSendEvent(_xwin.display, DefaultRootWindow(_xwin.display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
		if (status == 0) {
			Debug::Printf(kDbgMsg_Error, "ERROR: Failed to encode window state message.\n");
		}
	}

	CreateDesktopScreen(mode.Width, mode.Height, mode.ColorDepth);
#endif

	gfx_driver = &gfx_opengl;
	return true;
}

void OGLGraphicsDriver::InitGlParams(const DisplayMode &mode) {
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, device_screen_physical_width, device_screen_physical_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, device_screen_physical_width, 0, device_screen_physical_height, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	auto interval = mode.Vsync ? 1 : 0;
	bool vsyncEnabled = false;

#if AGS_PLATFORM_OS_WINDOWS
	if (GLAD_WGL_EXT_swap_control) {
		vsyncEnabled = wglSwapIntervalEXT(interval) != FALSE;
	}
#endif

#if AGS_PLATFORM_OS_LINUX
	if (GLAD_GLX_EXT_swap_control) {
		glXSwapIntervalEXT(_xwin.display, _xwin.window, interval);
		// glx requires hooking into XSetErrorHandler to test for BadWindow or BadValue
		vsyncEnabled = true;
	} else if (GLAD_GLX_MESA_swap_control) {
		vsyncEnabled = glXSwapIntervalMESA(interval) == 0;
	} else if (GLAD_GLX_SGI_swap_control) {
		vsyncEnabled = glXSwapIntervalSGI(interval) == 0;
	}
#endif

	// TODO: find out how to implement SwapInterval on other platforms, and how to check if it's supported

	if (mode.Vsync && !vsyncEnabled) {
		Debug::Printf(kDbgMsg_Warn, "WARNING: Vertical sync could not be enabled. Setting will be kept at driver default.");
	}

#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
	// Setup library mouse to have 1:1 coordinate transformation.
	// NOTE: cannot move this call to general mouse handling mode. Unfortunately, much of the setup and rendering
	// is duplicated in the Android/iOS ports' Allegro library patches, and is run when the Software renderer
	// is selected in AGS. This ugly situation causes trouble...
	float device_scale = 1.0f;

#if AGS_PLATFORM_OS_IOS
	device_scale = get_device_scale();
#endif

	device_mouse_setup(0, device_screen_physical_width - 1, 0, device_screen_physical_height - 1, device_scale, device_scale);
#endif
}

bool OGLGraphicsDriver::CreateGlContext(const DisplayMode &mode) {
#if AGS_PLATFORM_OS_WINDOWS
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		(BYTE)mode.ColorDepth,
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

	_oldPixelFormat = GetPixelFormat(_hDC);
	DescribePixelFormat(_hDC, _oldPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &_oldPixelFormatDesc);

	GLuint pixel_fmt;
	if (!(pixel_fmt = ChoosePixelFormat(_hDC, &pfd)))
		return false;

	if (!SetPixelFormat(_hDC, pixel_fmt, &pfd))
		return false;

	if (!(_hRC = wglCreateContext(_hDC)))
		return false;

	if (!wglMakeCurrent(_hDC, _hRC))
		return false;
#endif // AGS_PLATFORM_OS_WINDOWS
#if AGS_PLATFORM_OS_LINUX
	int attrib[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };
	XVisualInfo *vi = glXChooseVisual(_xwin.display, DefaultScreen(_xwin.display), attrib);
	if (!vi) {
		Debug::Printf(kDbgMsg_Error, "ERROR: glXChooseVisual() failed.\n");
		return false;
	}

	if (!(_glxContext = glXCreateContext(_xwin.display, vi, None, True))) {
		Debug::Printf(kDbgMsg_Error, "ERROR: glXCreateContext() failed.\n");
		return false;
	}

	if (!glXMakeCurrent(_xwin.display, _xwin.window, _glxContext)) {
		Debug::Printf(kDbgMsg_Error, "ERROR: glXMakeCurrent() failed.\n");
		return false;
	}
#endif
	return true;
}

void OGLGraphicsDriver::DeleteGlContext() {
#if AGS_PLATFORM_OS_WINDOWS
	if (_hRC) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(_hRC);
		_hRC = NULL;
	}

	if (_oldPixelFormat > 0)
		SetPixelFormat(_hDC, _oldPixelFormat, &_oldPixelFormatDesc);
#elif AGS_PLATFORM_OS_LINUX
	if (_glxContext) {
		glXMakeCurrent(_xwin.display, None, nullptr);
		glXDestroyContext(_xwin.display, _glxContext);
		_glxContext = nullptr;
	}
#endif
}

inline bool CanDoFrameBuffer() {
#ifdef GLAPI
	return GLAD_GL_EXT_framebuffer_object != 0;
#else
#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
	const char *fbo_extension_string = "GL_OES_framebuffer_object";
#else
	const char *fbo_extension_string = "GL_EXT_framebuffer_object";
#endif
	const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
	return extensions && strstr(extensions, fbo_extension_string) != NULL;
#endif
}

void OGLGraphicsDriver::TestRenderToTexture() {
	if (CanDoFrameBuffer()) {
		_can_render_to_texture = true;
		TestSupersampling();
	} else {
		_can_render_to_texture = false;
		Debug::Printf(kDbgMsg_Warn, "WARNING: OpenGL extension 'GL_EXT_framebuffer_object' not supported, rendering to texture mode will be disabled.");
	}

	if (!_can_render_to_texture)
		_do_render_to_texture = false;
}

void OGLGraphicsDriver::TestSupersampling() {
	if (!_can_render_to_texture)
		return;
	// Disable super-sampling if it would cause a too large texture size
	if (_super_sampling > 1) {
		int max = 1024;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if ((max < _srcRect.GetWidth() * _super_sampling) || (max < _srcRect.GetHeight() * _super_sampling))
			_super_sampling = 1;
	}
}

void OGLGraphicsDriver::CreateShaders() {
	if (!GLAD_GL_VERSION_2_0) {
		Debug::Printf(kDbgMsg_Error, "ERROR: Shaders require a minimum of OpenGL 2.0 support.");
		return;
	}
	CreateTintShader();
	CreateLightShader();
}

void OGLGraphicsDriver::CreateTintShader() {
	const char *fragment_shader_src =
	    // NOTE: this shader emulates "historical" AGS software tinting; it is not
	    // necessarily "proper" tinting in modern terms.
	    // The RGB-HSV-RGB conversion found in the Internet (copyright unknown);
	    // Color processing is replicated from Direct3D shader by Chris Jones
	    // (Engine/resource/tintshaderLegacy.fx).
	    // Uniforms:
	    // textID - texture index (usually 0),
	    // tintHSV - tint color in HSV,
	    // tintAmnTrsLum - tint parameters: amount, translucence (alpha), luminance.
	    "\
                                #version 120\n\
                                uniform sampler2D textID;\n\
                                uniform vec3 tintHSV;\n\
                                uniform vec3 tintAmnTrsLum;\n\
                                \
                                vec3 rgb2hsv(vec3 c)\n\
                                {\n\
                                    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n\
                                    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n\
                                    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n\
                                \
                                    float d = q.x - min(q.w, q.y);\n\
                                    const float e = 1.0e-10;\n\
                                    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n\
                                }\n\
                                \
                                vec3 hsv2rgb(vec3 c)\n\
                                {\n\
                                    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n\
                                    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n\
                                    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n\
                                }\n\
                                \
                                float getValue(vec3 color)\n\
                                {\n\
                                    float colorMax = max (color[0], color[1]);\n\
                                    colorMax = max (colorMax, color[2]);\n\
                                    return colorMax;\n\
                                }\n\
                                \
                                void main()\n\
                                {\n\
                                    vec4 src_col = texture2D(textID, gl_TexCoord[0].xy);\n\
                                    float amount = tintAmnTrsLum[0];\n\
                                    float lum = getValue(src_col.xyz);\n\
                                    lum = max(lum - (1.0 - tintAmnTrsLum[2]), 0.0);\n\
                                    vec3 new_col = (hsv2rgb(vec3(tintHSV[0], tintHSV[1], lum)) * amount + src_col.xyz * (1.0 - amount));\n\
                                    gl_FragColor = vec4(new_col, src_col.w * tintAmnTrsLum[1]);\n\
                                }\n\
    ";
	CreateShaderProgram(_tintShader, "Tinting", fragment_shader_src, "textID", "tintHSV", "tintAmnTrsLum");
}

void OGLGraphicsDriver::CreateLightShader() {
	const char *fragment_shader_src =
	    // NOTE: due to how the lighting works in AGS, this is combined MODULATE / ADD shader.
	    // if the light is < 0, then MODULATE operation is used, otherwise ADD is used.
	    // NOTE: it's been said that using branching in shaders produces inefficient code.
	    // If that will ever become a real problem, we can easily split this shader in two.
	    // Uniforms:
	    // textID - texture index (usually 0),
	    // light - light level,
	    // alpha - color alpha value.
	    "\
                                #version 120\n\
                                uniform sampler2D textID;\n\
                                uniform float light;\n\
                                uniform float alpha;\n\
                                \
                                void main()\n\
                                {\n\
                                    vec4 src_col = texture2D(textID, gl_TexCoord[0].xy);\n\
                                    if (light >= 0.0)\n\
                                        gl_FragColor = vec4(src_col.xyz + vec3(light, light, light), src_col.w * alpha);\n\
                                    else\n\
                                        gl_FragColor = vec4(src_col.xyz * abs(light), src_col.w * alpha);\n\
                                }\n\
    ";
	CreateShaderProgram(_lightShader, "Lighting", fragment_shader_src, "textID", "light", "alpha");
}

void OGLGraphicsDriver::CreateShaderProgram(ShaderProgram &prg, const char *name, const char *fragment_shader_src,
        const char *sampler_var, const char *color_var, const char *aux_var) {
	GLint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
	glCompileShader(fragment_shader);
	GLint result;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		OutputShaderError(fragment_shader, String::FromFormat("%s program's fragment shader", name), true);
		glDeleteShader(fragment_shader);
		return;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result == GL_FALSE) {
		OutputShaderError(program, String::FromFormat("%s program", name), false);
		glDeleteProgram(program);
		glDeleteShader(fragment_shader);
		return;
	}
	glDetachShader(program, fragment_shader);
	glDeleteShader(fragment_shader);

	prg.Program = program;
	prg.SamplerVar = glGetUniformLocation(program, sampler_var);
	prg.ColorVar = glGetUniformLocation(program, color_var);
	prg.AuxVar = glGetUniformLocation(program, aux_var);
	Debug::Printf("OGL: %s shader program created successfully", name);
}

void OGLGraphicsDriver::DeleteShaderProgram(ShaderProgram &prg) {
	if (prg.Program)
		glDeleteProgram(prg.Program);
	prg.Program = 0;
}

void OGLGraphicsDriver::OutputShaderError(GLuint obj_id, const String &obj_name, bool is_shader) {
	GLint log_len;
	if (is_shader)
		glGetShaderiv(obj_id, GL_INFO_LOG_LENGTH, &log_len);
	else
		glGetProgramiv(obj_id, GL_INFO_LOG_LENGTH, &log_len);
	std::vector<GLchar> errorLog(log_len);
	if (log_len > 0) {
		if (is_shader)
			glGetShaderInfoLog(obj_id, log_len, &log_len, &errorLog[0]);
		else
			glGetProgramInfoLog(obj_id, log_len, &log_len, &errorLog[0]);
	}

	Debug::Printf(kDbgMsg_Error, "ERROR: OpenGL: %s %s:", obj_name.GetCStr(), is_shader ? "failed to compile" : "failed to link");
	if (errorLog.size() > 0) {
		Debug::Printf(kDbgMsg_Error, "----------------------------------------");
		Debug::Printf(kDbgMsg_Error, "%s", &errorLog[0]);
		Debug::Printf(kDbgMsg_Error, "----------------------------------------");
	} else {
		Debug::Printf(kDbgMsg_Error, "Shader info log was empty.");
	}
}

void OGLGraphicsDriver::SetupBackbufferTexture() {
	// NOTE: ability to render to texture depends on OGL context, which is
	// created in SetDisplayMode, therefore creation of textures require
	// both native size set and context capabilities test passed.
	if (!IsNativeSizeValid() || !_can_render_to_texture)
		return;

	DeleteBackbufferTexture();

	// _backbuffer_texture_coordinates defines translation from wanted texture size to actual supported texture size
	_backRenderSize = _srcRect.GetSize() * _super_sampling;
	_backTextureSize = _backRenderSize;
	AdjustSizeToNearestSupportedByCard(&_backTextureSize.Width, &_backTextureSize.Height);
	const float back_ratio_w = (float)_backRenderSize.Width / (float)_backTextureSize.Width;
	const float back_ratio_h = (float)_backRenderSize.Height / (float)_backTextureSize.Height;
	std::fill(_backbuffer_texture_coordinates, _backbuffer_texture_coordinates + sizeof(_backbuffer_texture_coordinates) / sizeof(GLfloat), 0.0f);
	_backbuffer_texture_coordinates[2] = _backbuffer_texture_coordinates[6] = back_ratio_w;
	_backbuffer_texture_coordinates[5] = _backbuffer_texture_coordinates[7] = back_ratio_h;

	glGenTextures(1, &_backbuffer);
	glBindTexture(GL_TEXTURE_2D, _backbuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _backTextureSize.Width, _backTextureSize.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffersEXT(1, &_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _backbuffer, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// Assign vertices of the backbuffer texture position in the scene
	_backbuffer_vertices[0] = _backbuffer_vertices[4] = 0;
	_backbuffer_vertices[2] = _backbuffer_vertices[6] = _srcRect.GetWidth();
	_backbuffer_vertices[5] = _backbuffer_vertices[7] = _srcRect.GetHeight();
	_backbuffer_vertices[1] = _backbuffer_vertices[3] = 0;
}

void OGLGraphicsDriver::DeleteBackbufferTexture() {
	if (_backbuffer)
		glDeleteTextures(1, &_backbuffer);
	if (_fbo)
		glDeleteFramebuffersEXT(1, &_fbo);
	_backbuffer = 0;
	_fbo = 0;
}

void OGLGraphicsDriver::SetupViewport() {
	if (!IsModeSet() || !IsRenderFrameValid())
		return;

	// Setup viewport rect and scissor
	_viewportRect = ConvertTopDownRect(_dstRect, device_screen_physical_height);
	glScissor(_viewportRect.Left, _viewportRect.Top, _viewportRect.GetWidth(), _viewportRect.GetHeight());
}

Rect OGLGraphicsDriver::ConvertTopDownRect(const Rect &rect, int surface_height) {
	return RectWH(rect.Left, surface_height - 1 - rect.Bottom, rect.GetWidth(), rect.GetHeight());
}

bool OGLGraphicsDriver::SetDisplayMode(const DisplayMode &mode, volatile int *loopTimer) {
	ReleaseDisplayMode();

	if (mode.ColorDepth < 15) {
		set_allegro_error("OpenGL driver does not support 256-color display mode");
		return false;
	}

	try {
		if (!InitGlScreen(mode))
			return false;
		if (!_firstTimeInit)
			FirstTimeInit();
		InitGlParams(mode);
	} catch (Ali3DException exception) {
		if (exception._message != get_allegro_error())
			set_allegro_error(exception._message);
		return false;
	}

	OnInit(loopTimer);

	// On certain platforms OpenGL renderer ignores requested screen sizes
	// and uses values imposed by the operating system (device).
	DisplayMode final_mode = mode;
	final_mode.Width = device_screen_physical_width;
	final_mode.Height = device_screen_physical_height;
	OnModeSet(final_mode);

	// If we already have a native size set, then update virtual screen and setup backbuffer texture immediately
	CreateVirtualScreen();
	SetupBackbufferTexture();
	// If we already have a render frame configured, then setup viewport and backbuffer mappings immediately
	SetupViewport();
	return true;
}

void OGLGraphicsDriver::CreateVirtualScreen() {
	if (!IsModeSet() || !IsNativeSizeValid())
		return;
	// create initial stage screen for plugin raw drawing
	_stageVirtualScreen = CreateStageScreen(0, _srcRect.GetSize());
	// we must set Allegro's screen pointer to **something**
	screen = (BITMAP *)_stageVirtualScreen->GetAllegroBitmap();
}

bool OGLGraphicsDriver::SetNativeSize(const Size &src_size) {
	OnSetNativeSize(src_size);
	SetupBackbufferTexture();
	// If we already have a gfx mode set, then update virtual screen immediately
	CreateVirtualScreen();
	TestSupersampling();
	return !_srcRect.IsEmpty();
}

bool OGLGraphicsDriver::SetRenderFrame(const Rect &dst_rect) {
	if (!IsNativeSizeValid())
		return false;
	OnSetRenderFrame(dst_rect);
	// Also make sure viewport and backbuffer mappings are updated using new native & destination rectangles
	SetupViewport();
	return !_dstRect.IsEmpty();
}

int OGLGraphicsDriver::GetDisplayDepthForNativeDepth(int native_color_depth) const {
	// TODO: check for device caps to know which depth is supported?
	return 32;
}

IGfxModeList *OGLGraphicsDriver::GetSupportedModeList(int color_depth) {
	std::vector<DisplayMode> modes;
	platform->GetSystemDisplayModes(modes);
	return new OGLDisplayModeList(modes);
}

PGfxFilter OGLGraphicsDriver::GetGraphicsFilter() const {
	return _filter;
}

void OGLGraphicsDriver::ReleaseDisplayMode() {
	if (!IsModeSet())
		return;

	OnModeReleased();
	ClearDrawLists();
	ClearDrawBackups();
	DeleteBackbufferTexture();
	DestroyFxPool();
	DestroyAllStageScreens();

	gfx_driver = nullptr;

	platform->ExitFullscreenMode();
}

void OGLGraphicsDriver::UnInit() {
	OnUnInit();
	ReleaseDisplayMode();

	DeleteGlContext();
#if AGS_PLATFORM_OS_WINDOWS
	_hWnd = NULL;
	_hDC = NULL;
#endif

	DeleteShaderProgram(_tintShader);
	DeleteShaderProgram(_lightShader);
}

OGLGraphicsDriver::~OGLGraphicsDriver() {
	OGLGraphicsDriver::UnInit();
}

void OGLGraphicsDriver::ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse) {
	// NOTE: this function is practically useless at the moment, because OGL redraws whole game frame each time
}

bool OGLGraphicsDriver::GetCopyOfScreenIntoBitmap(Bitmap *destination, bool at_native_res, GraphicResolution *want_fmt) {
	(void)at_native_res; // TODO: support this at some point

	// TODO: following implementation currently only reads GL pixels in 32-bit RGBA.
	// this **should** work regardless of actual display mode because OpenGL is
	// responsible to convert and fill pixel buffer correctly.
	// If you like to support writing directly into 16-bit bitmap, please take
	// care of ammending the pixel reading code below.
	const int read_in_colordepth = 32;
	Size need_size = _do_render_to_texture ? _backRenderSize : _dstRect.GetSize();
	if (destination->GetColorDepth() != read_in_colordepth || destination->GetSize() != need_size) {
		if (want_fmt)
			*want_fmt = GraphicResolution(need_size.Width, need_size.Height, read_in_colordepth);
		return false;
	}

	Rect retr_rect;
	if (_do_render_to_texture) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);
		retr_rect = RectWH(0, 0, _backRenderSize.Width, _backRenderSize.Height);
	} else {
#if AGS_PLATFORM_OS_IOS
		ios_select_buffer();
#elif AGS_PLATFORM_OS_WINDOWS || AGS_PLATFORM_OS_LINUX
		glReadBuffer(GL_FRONT);
#endif
		retr_rect = _dstRect;
	}

	int bpp = read_in_colordepth / 8;
	int bufferSize = retr_rect.GetWidth() * retr_rect.GetHeight() * bpp;

	unsigned char *buffer = new unsigned char[bufferSize];
	if (buffer) {
		glReadPixels(retr_rect.Left, retr_rect.Top, retr_rect.GetWidth(), retr_rect.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, buffer);

		unsigned char *sourcePtr = buffer;
		for (int y = destination->GetHeight() - 1; y >= 0; y--) {
			unsigned int *destPtr = reinterpret_cast<unsigned int *>(&destination->GetScanLineForWriting(y)[0]);
			for (int dx = 0, sx = 0; dx < destination->GetWidth(); ++dx, sx = dx * bpp) {
				destPtr[dx] = makeacol32(sourcePtr[sx + 0], sourcePtr[sx + 1], sourcePtr[sx + 2], sourcePtr[sx + 3]);
			}
			sourcePtr += retr_rect.GetWidth() * bpp;
		}

		if (_pollingCallback)
			_pollingCallback();

		delete [] buffer;
	}
	return true;
}

void OGLGraphicsDriver::RenderToBackBuffer() {
	throw Ali3DException("OGL driver does not have a back buffer");
}

void OGLGraphicsDriver::Render() {
	Render(0, 0, kFlip_None);
}

void OGLGraphicsDriver::Render(int /*xoff*/, int /*yoff*/, GlobalFlipType /*flip*/) {
	_render(true);
}

void OGLGraphicsDriver::_reDrawLastFrame() {
	RestoreDrawLists();
}

void OGLGraphicsDriver::_renderSprite(const OGLDrawListEntry *drawListEntry, const GLMATRIX &matGlobal) {
	OGLBitmap *bmpToDraw = drawListEntry->bitmap;

	if (bmpToDraw->_transparency >= 255)
		return;

	const bool do_tint = bmpToDraw->_tintSaturation > 0 && _tintShader.Program > 0;
	const bool do_light = bmpToDraw->_tintSaturation == 0 && bmpToDraw->_lightLevel > 0 && _lightShader.Program > 0;
	if (do_tint) {
		// Use tinting shader
		glUseProgram(_tintShader.Program);
		float rgb[3];
		float sat_trs_lum[3]; // saturation / transparency / luminance
		if (_legacyPixelShader) {
			rgb_to_hsv(bmpToDraw->_red, bmpToDraw->_green, bmpToDraw->_blue, &rgb[0], &rgb[1], &rgb[2]);
			rgb[0] /= 360.0; // In HSV, Hue is 0-360
		} else {
			rgb[0] = (float)bmpToDraw->_red / 255.0;
			rgb[1] = (float)bmpToDraw->_green / 255.0;
			rgb[2] = (float)bmpToDraw->_blue / 255.0;
		}

		sat_trs_lum[0] = (float)bmpToDraw->_tintSaturation / 255.0;

		if (bmpToDraw->_transparency > 0)
			sat_trs_lum[1] = (float)bmpToDraw->_transparency / 255.0;
		else
			sat_trs_lum[1] = 1.0f;

		if (bmpToDraw->_lightLevel > 0)
			sat_trs_lum[2] = (float)bmpToDraw->_lightLevel / 255.0;
		else
			sat_trs_lum[2] = 1.0f;

		glUniform1i(_tintShader.SamplerVar, 0);
		glUniform3f(_tintShader.ColorVar, rgb[0], rgb[1], rgb[2]);
		glUniform3f(_tintShader.AuxVar, sat_trs_lum[0], sat_trs_lum[1], sat_trs_lum[2]);
	} else if (do_light) {
		// Use light shader
		glUseProgram(_lightShader.Program);
		float light_lev = 1.0f;
		float alpha = 1.0f;

		// Light level parameter in DDB is weird, it is measured in units of
		// 1/255 (although effectively 1/250, see draw.cpp), but contains two
		// ranges: 1-255 is darker range and 256-511 is brighter range.
		// (light level of 0 means "default color")
		if ((bmpToDraw->_lightLevel > 0) && (bmpToDraw->_lightLevel < 256)) {
			// darkening the sprite... this stupid calculation is for
			// consistency with the allegro software-mode code that does
			// a trans blend with a (8,8,8) sprite
			light_lev = -((bmpToDraw->_lightLevel * 192) / 256 + 64) / 255.f; // darker, uses MODULATE op
		} else if (bmpToDraw->_lightLevel > 256) {
			light_lev = ((bmpToDraw->_lightLevel - 256) / 2) / 255.f; // brighter, uses ADD op
		}

		if (bmpToDraw->_transparency > 0)
			alpha = bmpToDraw->_transparency / 255.f;

		glUniform1i(_lightShader.SamplerVar, 0);
		glUniform1f(_lightShader.ColorVar, light_lev);
		glUniform1f(_lightShader.AuxVar, alpha);
	} else {
		// Use default processing
		if (bmpToDraw->_transparency == 0)
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		else
			glColor4f(1.0f, 1.0f, 1.0f, bmpToDraw->_transparency / 255.0f);
	}

	float width = bmpToDraw->GetWidthToRender();
	float height = bmpToDraw->GetHeightToRender();
	float xProportion = (float)width / (float)bmpToDraw->_width;
	float yProportion = (float)height / (float)bmpToDraw->_height;
	int drawAtX = drawListEntry->x;
	int drawAtY = drawListEntry->y;

	for (int ti = 0; ti < bmpToDraw->_numTiles; ti++) {
		width = bmpToDraw->_tiles[ti].width * xProportion;
		height = bmpToDraw->_tiles[ti].height * yProportion;
		float xOffs;
		float yOffs = bmpToDraw->_tiles[ti].y * yProportion;
		if (bmpToDraw->_flipped)
			xOffs = (bmpToDraw->_width - (bmpToDraw->_tiles[ti].x + bmpToDraw->_tiles[ti].width)) * xProportion;
		else
			xOffs = bmpToDraw->_tiles[ti].x * xProportion;
		int thisX = drawAtX + xOffs;
		int thisY = drawAtY + yOffs;
		thisX = (-(_srcRect.GetWidth() / 2)) + thisX;
		thisY = (_srcRect.GetHeight() / 2) - thisY;

		//Setup translation and scaling matrices
		float widthToScale = (float)width;
		float heightToScale = (float)height;
		if (bmpToDraw->_flipped) {
			// The usual transform changes 0..1 into 0..width
			// So first negate it (which changes 0..w into -w..0)
			widthToScale = -widthToScale;
			// and now shift it over to make it 0..w again
			thisX += width;
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//
		// IMPORTANT: in OpenGL order of transformation is REVERSE to the order of commands!
		//
		// Origin is at the middle of the surface
		if (_do_render_to_texture)
			glTranslatef(_backRenderSize.Width / 2.0f, _backRenderSize.Height / 2.0f, 0.0f);
		else
			glTranslatef(_srcRect.GetWidth() / 2.0f, _srcRect.GetHeight() / 2.0f, 0.0f);

		// Global batch transform
		glMultMatrixf(matGlobal.m);
		// Self sprite transform (first scale, then rotate and then translate, reversed)
		glTranslatef((float)thisX, (float)thisY, 0.0f);
		glRotatef(0.f, 0.f, 0.f, 1.f);
		glScalef(widthToScale, heightToScale, 1.0f);

		glBindTexture(GL_TEXTURE_2D, bmpToDraw->_tiles[ti].texture);

		if ((_smoothScaling) && bmpToDraw->_useResampler && (bmpToDraw->_stretchToHeight > 0) &&
		        ((bmpToDraw->_stretchToHeight != bmpToDraw->_height) ||
		         (bmpToDraw->_stretchToWidth != bmpToDraw->_width))) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else if (_do_render_to_texture) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
			_filter->SetFilteringForStandardSprite();
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		if (bmpToDraw->_vertex != nullptr) {
			glTexCoordPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &(bmpToDraw->_vertex[ti * 4].tu));
			glVertexPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &(bmpToDraw->_vertex[ti * 4].position));
		} else {
			glTexCoordPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &defaultVertices[0].tu);
			glVertexPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &defaultVertices[0].position);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glUseProgram(0);
}

void OGLGraphicsDriver::_render(bool clearDrawListAfterwards) {
#if AGS_PLATFORM_OS_IOS
	ios_select_buffer();
#endif

#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
	// TODO:
	// For some reason, mobile ports initialize actual display size after a short delay.
	// This is why we update display mode and related parameters (projection, viewport)
	// at the first render pass.
	// Ofcourse this is not a good thing, ideally the display size should be made
	// known before graphic mode is initialized. This would require analysis and rewrite
	// of the platform-specific part of the code (Java app for Android / XCode for iOS).
	if (!device_screen_initialized) {
		UpdateDeviceScreen();
		device_screen_initialized = 1;
	}
#endif

	if (_do_render_to_texture) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fbo);

		glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, _backRenderSize.Width, _backRenderSize.Height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, _backRenderSize.Width, 0, _backRenderSize.Height, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	} else {
		glDisable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_SCISSOR_TEST);

		glViewport(_viewportRect.Left, _viewportRect.Top, _viewportRect.GetWidth(), _viewportRect.GetHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, _srcRect.GetWidth(), 0, _srcRect.GetHeight(), 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	RenderSpriteBatches();

	if (_do_render_to_texture) {
		// Texture is ready, now create rectangle in the world space and draw texture upon it
#if AGS_PLATFORM_OS_IOS
		ios_select_buffer();
#else
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#endif

		glViewport(_viewportRect.Left, _viewportRect.Top, _viewportRect.GetWidth(), _viewportRect.GetHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, _srcRect.GetWidth(), 0, _srcRect.GetHeight(), 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		// use correct sampling method when stretching buffer to the final rect
		_filter->SetFilteringForStandardSprite();

		glBindTexture(GL_TEXTURE_2D, _backbuffer);

		glTexCoordPointer(2, GL_FLOAT, 0, _backbuffer_texture_coordinates);
		glVertexPointer(2, GL_FLOAT, 0, _backbuffer_vertices);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glEnable(GL_BLEND);
	}

	glFinish();

#if AGS_PLATFORM_OS_WINDOWS
	SwapBuffers(_hDC);
#elif AGS_PLATFORM_OS_LINUX
	glXSwapBuffers(_xwin.display, _xwin.window);
#elif AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
	device_swap_buffers();
#endif

	if (clearDrawListAfterwards) {
		BackupDrawLists();
		ClearDrawLists();
	}
	ResetFxPool();
}

void OGLGraphicsDriver::RenderSpriteBatches() {
	// Render all the sprite batches with necessary transformations
	Rect main_viewport = _do_render_to_texture ? _srcRect : _viewportRect;
	int surface_height = _do_render_to_texture ? _srcRect.GetHeight() : device_screen_physical_height;
	// TODO: see if it's possible to refactor and not enable/disable scissor test
	// TODO: also maybe sync scissor code logic with D3D renderer
	if (_do_render_to_texture)
		glEnable(GL_SCISSOR_TEST);

	for (size_t i = 0; i <= _actSpriteBatch; ++i) {
		const Rect &viewport = _spriteBatches[i].Viewport;
		const OGLSpriteBatch &batch = _spriteBatches[i];
		if (!viewport.IsEmpty()) {
			Rect scissor = _do_render_to_texture ? viewport : _scaling.ScaleRange(viewport);
			scissor = ConvertTopDownRect(scissor, surface_height);
			glScissor(scissor.Left, scissor.Top, scissor.GetWidth(), scissor.GetHeight());
		} else {
			glScissor(main_viewport.Left, main_viewport.Top, main_viewport.GetWidth(), main_viewport.GetHeight());
		}
		_stageVirtualScreen = GetStageScreen(i);
		RenderSpriteBatch(batch);
	}

	_stageVirtualScreen = GetStageScreen(0);
	glScissor(main_viewport.Left, main_viewport.Top, main_viewport.GetWidth(), main_viewport.GetHeight());
	if (_do_render_to_texture)
		glDisable(GL_SCISSOR_TEST);
}

void OGLGraphicsDriver::RenderSpriteBatch(const OGLSpriteBatch &batch) {
	OGLDrawListEntry stageEntry; // raw-draw plugin support

	const std::vector<OGLDrawListEntry> &listToDraw = batch.List;
	for (size_t i = 0; i < listToDraw.size(); i++) {
		if (listToDraw[i].skip)
			continue;

		const OGLDrawListEntry *sprite = &listToDraw[i];
		if (listToDraw[i].bitmap == nullptr) {
			if (DoNullSpriteCallback(listToDraw[i].x, listToDraw[i].y))
				stageEntry = OGLDrawListEntry((OGLBitmap *)_stageVirtualScreenDDB);
			else
				continue;
			sprite = &stageEntry;
		}

		this->_renderSprite(sprite, batch.Matrix);
	}
}

void OGLGraphicsDriver::InitSpriteBatch(size_t index, const SpriteBatchDesc &desc) {
	if (_spriteBatches.size() <= index)
		_spriteBatches.resize(index + 1);
	_spriteBatches[index].List.clear();

	Rect orig_viewport = desc.Viewport;
	Rect node_viewport = desc.Viewport;
	// Combine both world transform and viewport transform into one matrix for faster perfomance
	// NOTE: in OpenGL order of transformation is REVERSE to the order of commands!
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Global node transformation (flip and offset)
	int node_tx = desc.Offset.X, node_ty = desc.Offset.Y;
	float node_sx = 1.f, node_sy = 1.f;
	if ((desc.Flip == kFlip_Vertical) || (desc.Flip == kFlip_Both)) {
		int left = _srcRect.GetWidth() - (orig_viewport.Right + 1);
		node_viewport.MoveToX(left);
		node_sx = -1.f;
	}
	if ((desc.Flip == kFlip_Horizontal) || (desc.Flip == kFlip_Both)) {
		int top = _srcRect.GetHeight() - (orig_viewport.Bottom + 1);
		node_viewport.MoveToY(top);
		node_sy = -1.f;
	}
	_spriteBatches[index].Viewport = Rect::MoveBy(node_viewport, node_tx, node_ty);
	glTranslatef(node_tx, -(node_ty), 0.0f);
	glScalef(node_sx, node_sy, 1.f);
	// NOTE: before node, translate to viewport position; remove this if this
	// is changed to a separate operation at some point
	// TODO: find out if this is an optimal way to translate scaled room into Top-Left screen coordinates
	float scaled_offx = (_srcRect.GetWidth() - desc.Transform.ScaleX * (float)_srcRect.GetWidth()) / 2.f;
	float scaled_offy = (_srcRect.GetHeight() - desc.Transform.ScaleY * (float)_srcRect.GetHeight()) / 2.f;
	glTranslatef((float)(orig_viewport.Left - scaled_offx), (float) - (orig_viewport.Top - scaled_offy), 0.0f);
	// IMPORTANT: while the sprites are usually transformed in the order of Scale-Rotate-Translate,
	// the camera's transformation is essentially reverse world transformation. And the operations
	// are inverse: Translate-Rotate-Scale (here they are double inverse because OpenGL).
	glScalef(desc.Transform.ScaleX, desc.Transform.ScaleY, 1.f); // scale camera
	glRotatef(Math::RadiansToDegrees(desc.Transform.Rotate), 0.f, 0.f, 1.f); // rotate camera
	glTranslatef((float)desc.Transform.X, (float) - desc.Transform.Y, 0.0f); // translate camera
	glGetFloatv(GL_MODELVIEW_MATRIX, _spriteBatches[index].Matrix.m);
	glLoadIdentity();

	// create stage screen for plugin raw drawing
	int src_w = orig_viewport.GetWidth() / desc.Transform.ScaleX;
	int src_h = orig_viewport.GetHeight() / desc.Transform.ScaleY;
	CreateStageScreen(index, Size(src_w, src_h));
}

void OGLGraphicsDriver::ResetAllBatches() {
	for (size_t i = 0; i < _spriteBatches.size(); ++i)
		_spriteBatches[i].List.clear();
}

void OGLGraphicsDriver::ClearDrawBackups() {
	_backupBatchDescs.clear();
	_backupBatches.clear();
}

void OGLGraphicsDriver::BackupDrawLists() {
	ClearDrawBackups();
	for (size_t i = 0; i <= _actSpriteBatch; ++i) {
		_backupBatchDescs.push_back(_spriteBatchDesc[i]);
		_backupBatches.push_back(_spriteBatches[i]);
	}
}

void OGLGraphicsDriver::RestoreDrawLists() {
	if (_backupBatchDescs.size() == 0) {
		ClearDrawLists();
		return;
	}
	_spriteBatchDesc = _backupBatchDescs;
	_spriteBatches = _backupBatches;
	_actSpriteBatch = _backupBatchDescs.size() - 1;
}

void OGLGraphicsDriver::DrawSprite(int x, int y, IDriverDependantBitmap *bitmap) {
	_spriteBatches[_actSpriteBatch].List.push_back(OGLDrawListEntry((OGLBitmap *)bitmap, x, y));
}

void OGLGraphicsDriver::DestroyDDB(IDriverDependantBitmap *bitmap) {
	// Remove deleted DDB from backups
	for (OGLSpriteBatches::iterator it = _backupBatches.begin(); it != _backupBatches.end(); ++it) {
		std::vector<OGLDrawListEntry> &drawlist = it->List;
		for (size_t i = 0; i < drawlist.size(); i++) {
			if (drawlist[i].bitmap == bitmap)
				drawlist[i].skip = true;
		}
	}
	delete bitmap;
}


void OGLGraphicsDriver::UpdateTextureRegion(OGLTextureTile *tile, Bitmap *bitmap, OGLBitmap *target, bool hasAlpha) {
	int textureHeight = tile->height;
	int textureWidth = tile->width;

	// TODO: this seem to be tad overcomplicated, these conversions were made
	// when texture is just created. Check later if this operation here may be removed.
	AdjustSizeToNearestSupportedByCard(&textureWidth, &textureHeight);

	int tilex = 0, tiley = 0, tileWidth = tile->width, tileHeight = tile->height;
	if (textureWidth > tile->width) {
		int texxoff = Math::Min(textureWidth - tile->width - 1, 1);
		tilex = texxoff;
		tileWidth += 1 + texxoff;
	}
	if (textureHeight > tile->height) {
		int texyoff = Math::Min(textureHeight - tile->height - 1, 1);
		tiley = texyoff;
		tileHeight += 1 + texyoff;
	}

	const bool usingLinearFiltering = _filter->UseLinearFiltering();
	char *origPtr = (char *)malloc(sizeof(int) * tileWidth * tileHeight);
	const int pitch = tileWidth * sizeof(int);
	char *memPtr = origPtr + pitch * tiley + tilex * sizeof(int);

	TextureTile fixedTile;
	fixedTile.x = tile->x;
	fixedTile.y = tile->y;
	fixedTile.width = Math::Min(tile->width, tileWidth);
	fixedTile.height = Math::Min(tile->height, tileHeight);
	if (target->_opaque)
		BitmapToVideoMemOpaque(bitmap, hasAlpha, &fixedTile, target, memPtr, pitch);
	else
		BitmapToVideoMem(bitmap, hasAlpha, &fixedTile, target, memPtr, pitch, usingLinearFiltering);

	// Mimic the behaviour of GL_CLAMP_EDGE for the tile edges
	// NOTE: on some platforms GL_CLAMP_EDGE does not work with the version of OpenGL we're using.
	if (tile->width < tileWidth) {
		if (tilex > 0) {
			for (int y = 0; y < tileHeight; y++) {
				unsigned int *edge_left_col = (unsigned int *)(origPtr + y * pitch + (tilex - 1) * sizeof(int));
				unsigned int *bm_left_col = (unsigned int *)(origPtr + y * pitch + (tilex) * sizeof(int));
				*edge_left_col = *bm_left_col & 0x00FFFFFF;
			}
		}
		for (int y = 0; y < tileHeight; y++) {
			unsigned int *edge_right_col = (unsigned int *)(origPtr + y * pitch + (tilex + tile->width) * sizeof(int));
			unsigned int *bm_right_col = edge_right_col - 1;
			*edge_right_col = *bm_right_col & 0x00FFFFFF;
		}
	}
	if (tile->height < tileHeight) {
		if (tiley > 0) {
			unsigned int *edge_top_row = (unsigned int *)(origPtr + pitch * (tiley - 1));
			unsigned int *bm_top_row = (unsigned int *)(origPtr + pitch * (tiley));
			for (int x = 0; x < tileWidth; x++) {
				edge_top_row[x] = bm_top_row[x] & 0x00FFFFFF;
			}
		}
		unsigned int *edge_bottom_row = (unsigned int *)(origPtr + pitch * (tiley + tile->height));
		unsigned int *bm_bottom_row = (unsigned int *)(origPtr + pitch * (tiley + tile->height - 1));
		for (int x = 0; x < tileWidth; x++) {
			edge_bottom_row[x] = bm_bottom_row[x] & 0x00FFFFFF;
		}
	}

	glBindTexture(GL_TEXTURE_2D, tile->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, origPtr);

	free(origPtr);
}

void OGLGraphicsDriver::UpdateDDBFromBitmap(IDriverDependantBitmap *bitmapToUpdate, Bitmap *bitmap, bool hasAlpha) {
	OGLBitmap *target = (OGLBitmap *)bitmapToUpdate;
	if (target->_width != bitmap->GetWidth() || target->_height != bitmap->GetHeight())
		throw Ali3DException("UpdateDDBFromBitmap: mismatched bitmap size");
	const int color_depth = bitmap->GetColorDepth();
	if (color_depth != target->_colDepth)
		throw Ali3DException("UpdateDDBFromBitmap: mismatched colour depths");

	target->_hasAlpha = hasAlpha;
	if (color_depth == 8)
		select_palette(palette);

	for (int i = 0; i < target->_numTiles; i++) {
		UpdateTextureRegion(&target->_tiles[i], bitmap, target, hasAlpha);
	}

	if (color_depth == 8)
		unselect_palette();
}

int OGLGraphicsDriver::GetCompatibleBitmapFormat(int color_depth) {
	if (color_depth == 8)
		return 8;
	if (color_depth > 8 && color_depth <= 16)
		return 16;
	return 32;
}

void OGLGraphicsDriver::AdjustSizeToNearestSupportedByCard(int *width, int *height) {
	int allocatedWidth = *width, allocatedHeight = *height;

	bool foundWidth = false, foundHeight = false;
	int tryThis = 2;
	while ((!foundWidth) || (!foundHeight)) {
		if ((tryThis >= allocatedWidth) && (!foundWidth)) {
			allocatedWidth = tryThis;
			foundWidth = true;
		}

		if ((tryThis >= allocatedHeight) && (!foundHeight)) {
			allocatedHeight = tryThis;
			foundHeight = true;
		}

		tryThis = tryThis << 1;
	}

	*width = allocatedWidth;
	*height = allocatedHeight;
}



IDriverDependantBitmap *OGLGraphicsDriver::CreateDDBFromBitmap(Bitmap *bitmap, bool hasAlpha, bool opaque) {
	int allocatedWidth = bitmap->GetWidth();
	int allocatedHeight = bitmap->GetHeight();
	// NOTE: original bitmap object is not modified in this function
	if (bitmap->GetColorDepth() != GetCompatibleBitmapFormat(bitmap->GetColorDepth()))
		throw Ali3DException("CreateDDBFromBitmap: bitmap colour depth not supported");
	int colourDepth = bitmap->GetColorDepth();

	OGLBitmap *ddb = new OGLBitmap(bitmap->GetWidth(), bitmap->GetHeight(), colourDepth, opaque);

	AdjustSizeToNearestSupportedByCard(&allocatedWidth, &allocatedHeight);
	int tilesAcross = 1, tilesDown = 1;

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
	OGLTextureTile *tiles = (OGLTextureTile *)malloc(sizeof(OGLTextureTile) * numTiles);
	memset(tiles, 0, sizeof(OGLTextureTile) * numTiles);

	OGLCUSTOMVERTEX *vertices = nullptr;

	if ((numTiles == 1) &&
	        (allocatedWidth == bitmap->GetWidth()) &&
	        (allocatedHeight == bitmap->GetHeight())) {
		// use default whole-image vertices
	} else {
		// The texture is not the same as the bitmap, so create some custom vertices
		// so that only the relevant portion of the texture is rendered
		int vertexBufferSize = numTiles * 4 * sizeof(OGLCUSTOMVERTEX);

		ddb->_vertex = vertices = (OGLCUSTOMVERTEX *)malloc(vertexBufferSize);
	}

	for (int x = 0; x < tilesAcross; x++) {
		for (int y = 0; y < tilesDown; y++) {
			OGLTextureTile *thisTile = &tiles[y * tilesAcross + x];
			int thisAllocatedWidth = tileAllocatedWidth;
			int thisAllocatedHeight = tileAllocatedHeight;
			thisTile->x = x * tileWidth;
			thisTile->y = y * tileHeight;
			thisTile->width = tileWidth;
			thisTile->height = tileHeight;
			if (x == tilesAcross - 1) {
				thisTile->width += lastTileExtraWidth;
				thisAllocatedWidth = thisTile->width;
				AdjustSizeToNearestSupportedByCard(&thisAllocatedWidth, &thisAllocatedHeight);
			}
			if (y == tilesDown - 1) {
				thisTile->height += lastTileExtraHeight;
				thisAllocatedHeight = thisTile->height;
				AdjustSizeToNearestSupportedByCard(&thisAllocatedWidth, &thisAllocatedHeight);
			}

			if (vertices != nullptr) {
				const int texxoff = (thisAllocatedWidth - thisTile->width) > 1 ? 1 : 0;
				const int texyoff = (thisAllocatedHeight - thisTile->height) > 1 ? 1 : 0;
				for (int vidx = 0; vidx < 4; vidx++) {
					int i = (y * tilesAcross + x) * 4 + vidx;
					vertices[i] = defaultVertices[vidx];
					if (vertices[i].tu > 0.0) {
						vertices[i].tu = (float)(texxoff + thisTile->width) / (float)thisAllocatedWidth;
					} else {
						vertices[i].tu = (float)(texxoff) / (float)thisAllocatedWidth;
					}
					if (vertices[i].tv > 0.0) {
						vertices[i].tv = (float)(texyoff + thisTile->height) / (float)thisAllocatedHeight;
					} else {
						vertices[i].tv = (float)(texyoff) / (float)thisAllocatedHeight;
					}
				}
			}

			glGenTextures(1, &thisTile->texture);
			glBindTexture(GL_TEXTURE_2D, thisTile->texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			// NOTE: pay attention that the texture format depends on the **display mode**'s format,
			// rather than source bitmap's color depth!
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, thisAllocatedWidth, thisAllocatedHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
	}

	ddb->_numTiles = numTiles;
	ddb->_tiles = tiles;

	UpdateDDBFromBitmap(ddb, bitmap, hasAlpha);

	return ddb;
}

void OGLGraphicsDriver::do_fade(bool fadingOut, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	// Construct scene in order: game screen, fade fx, post game overlay
	// NOTE: please keep in mind: redrawing last saved frame here instead of constructing new one
	// is done because of backwards-compatibility issue: originally AGS faded out using frame
	// drawn before the script that triggers blocking fade (e.g. instigated by ChangeRoom).
	// Unfortunately some existing games were changing looks of the screen during same function,
	// but these were not supposed to get on screen until before fade-in.
	if (fadingOut)
		this->_reDrawLastFrame();
	else if (_drawScreenCallback != nullptr)
		_drawScreenCallback();
	Bitmap *blackSquare = BitmapHelper::CreateBitmap(16, 16, 32);
	blackSquare->Clear(makecol32(targetColourRed, targetColourGreen, targetColourBlue));
	IDriverDependantBitmap *d3db = this->CreateDDBFromBitmap(blackSquare, false, true);
	delete blackSquare;
	BeginSpriteBatch(_srcRect, SpriteTransform());
	d3db->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight(), false);
	this->DrawSprite(0, 0, d3db);
	if (_drawPostScreenCallback != NULL)
		_drawPostScreenCallback();

	if (speed <= 0) speed = 16;
	speed *= 2;  // harmonise speeds with software driver which is faster
	for (int a = 1; a < 255; a += speed) {
		d3db->SetTransparency(fadingOut ? a : (255 - a));
		this->_render(false);

		if (_pollingCallback)
			_pollingCallback();
		WaitForNextFrame();
	}

	if (fadingOut) {
		d3db->SetTransparency(0);
		this->_render(false);
	}

	this->DestroyDDB(d3db);
	this->ClearDrawLists();
	ResetFxPool();
}

void OGLGraphicsDriver::FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	do_fade(true, speed, targetColourRed, targetColourGreen, targetColourBlue);
}

void OGLGraphicsDriver::FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue) {
	do_fade(false, speed, targetColourRed, targetColourGreen, targetColourBlue);
}

void OGLGraphicsDriver::BoxOutEffect(bool blackingOut, int speed, int delay) {
	// Construct scene in order: game screen, fade fx, post game overlay
	if (blackingOut)
		this->_reDrawLastFrame();
	else if (_drawScreenCallback != nullptr)
		_drawScreenCallback();
	Bitmap *blackSquare = BitmapHelper::CreateBitmap(16, 16, 32);
	blackSquare->Clear();
	IDriverDependantBitmap *d3db = this->CreateDDBFromBitmap(blackSquare, false, true);
	delete blackSquare;
	BeginSpriteBatch(_srcRect, SpriteTransform());
	size_t fx_batch = _actSpriteBatch;
	d3db->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight(), false);
	this->DrawSprite(0, 0, d3db);
	if (!blackingOut) {
		// when fading in, draw four black boxes, one
		// across each side of the screen
		this->DrawSprite(0, 0, d3db);
		this->DrawSprite(0, 0, d3db);
		this->DrawSprite(0, 0, d3db);
	}
	if (_drawPostScreenCallback != NULL)
		_drawPostScreenCallback();

	int yspeed = _srcRect.GetHeight() / (_srcRect.GetWidth() / speed);
	int boxWidth = speed;
	int boxHeight = yspeed;

	while (boxWidth < _srcRect.GetWidth()) {
		boxWidth += speed;
		boxHeight += yspeed;
		OGLSpriteBatch &batch = _spriteBatches[fx_batch];
		std::vector<OGLDrawListEntry> &drawList = batch.List;
		size_t last = drawList.size() - 1;
		if (blackingOut) {
			drawList[last].x = _srcRect.GetWidth() / 2 - boxWidth / 2;
			drawList[last].y = _srcRect.GetHeight() / 2 - boxHeight / 2;
			d3db->SetStretch(boxWidth, boxHeight, false);
		} else {
			drawList[last - 3].x = _srcRect.GetWidth() / 2 - boxWidth / 2 - _srcRect.GetWidth();
			drawList[last - 2].y = _srcRect.GetHeight() / 2 - boxHeight / 2 - _srcRect.GetHeight();
			drawList[last - 1].x = _srcRect.GetWidth() / 2 + boxWidth / 2;
			drawList[last    ].y = _srcRect.GetHeight() / 2 + boxHeight / 2;
			d3db->SetStretch(_srcRect.GetWidth(), _srcRect.GetHeight(), false);
		}

		this->_render(false);

		if (_pollingCallback)
			_pollingCallback();
		platform->Delay(delay);
	}

	this->DestroyDDB(d3db);
	this->ClearDrawLists();
	ResetFxPool();
}

void OGLGraphicsDriver::SetScreenFade(int red, int green, int blue) {
	OGLBitmap *ddb = static_cast<OGLBitmap *>(MakeFx(red, green, blue));
	ddb->SetStretch(_spriteBatches[_actSpriteBatch].Viewport.GetWidth(),
	                _spriteBatches[_actSpriteBatch].Viewport.GetHeight(), false);
	ddb->SetTransparency(0);
	_spriteBatches[_actSpriteBatch].List.push_back(OGLDrawListEntry(ddb));
}

void OGLGraphicsDriver::SetScreenTint(int red, int green, int blue) {
	if (red == 0 && green == 0 && blue == 0) return;
	OGLBitmap *ddb = static_cast<OGLBitmap *>(MakeFx(red, green, blue));
	ddb->SetStretch(_spriteBatches[_actSpriteBatch].Viewport.GetWidth(),
	                _spriteBatches[_actSpriteBatch].Viewport.GetHeight(), false);
	ddb->SetTransparency(128);
	_spriteBatches[_actSpriteBatch].List.push_back(OGLDrawListEntry(ddb));
}


OGLGraphicsFactory *OGLGraphicsFactory::_factory = nullptr;

OGLGraphicsFactory::~OGLGraphicsFactory() {
	_factory = nullptr;
}

size_t OGLGraphicsFactory::GetFilterCount() const {
	return 2;
}

const GfxFilterInfo *OGLGraphicsFactory::GetFilterInfo(size_t index) const {
	switch (index) {
	case 0:
		return &OGLGfxFilter::FilterInfo;
	case 1:
		return &AAOGLGfxFilter::FilterInfo;
	default:
		return nullptr;
	}
}

String OGLGraphicsFactory::GetDefaultFilterID() const {
	return OGLGfxFilter::FilterInfo.Id;
}

/* static */ OGLGraphicsFactory *OGLGraphicsFactory::GetFactory() {
	if (!_factory)
		_factory = new OGLGraphicsFactory();
	return _factory;
}

OGLGraphicsDriver *OGLGraphicsFactory::EnsureDriverCreated() {
	if (!_driver)
		_driver = new OGLGraphicsDriver();
	return _driver;
}

OGLGfxFilter *OGLGraphicsFactory::CreateFilter(const String &id) {
	if (OGLGfxFilter::FilterInfo.Id.CompareNoCase(id) == 0)
		return new OGLGfxFilter();
	else if (AAOGLGfxFilter::FilterInfo.Id.CompareNoCase(id) == 0)
		return new AAOGLGfxFilter();
	return nullptr;
}

} // namespace OGL
} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif // only on Windows, Android and iOS
