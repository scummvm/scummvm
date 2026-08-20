/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "backends/platform/libretro/include/libretro-gl-context-handoff.h"

#if defined(USE_LIBCO) || !defined(USE_OPENGL)

/* Both halves share an OS thread, or there is no GL at all: nothing to move. */
void retro_gl_context_release(void) {}
void retro_gl_context_acquire(void) {}
bool retro_gl_context_handoff_available(void) {
	return true;
}

#else

#include <stddef.h>

#if defined(_WIN32)
#define CONTEXT_BACKEND_WGL
#include <windows.h>
#elif defined(__APPLE__)
#define CONTEXT_BACKEND_CGL
#include <dlfcn.h>
#elif defined(HAVE_DLFCN) || defined(__linux__) || defined(__ANDROID__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__)
#define CONTEXT_BACKEND_DL
#include <dlfcn.h>
#endif

/* Resolved lazily on first use and then latched, so a platform with no
 * backend costs one failed lookup rather than one per frame. */
enum ContextBackend {
	kBackendUnprobed = 0,
	kBackendNone,
	kBackendEGL,
	kBackendGLX,
	kBackendWGL,
	kBackendCGL
};

static ContextBackend _backend = kBackendUnprobed;
static bool _held = false;

#if defined(CONTEXT_BACKEND_DL)

/* EGL. Declared locally rather than pulling in EGL/egl.h, which is not
 * available on every target that can dlopen libEGL. */
#define LOCAL_EGL_DRAW       0x3059
#define LOCAL_EGL_READ       0x305A

typedef void *(*eglGetCurrentDisplay_t)(void);
typedef void *(*eglGetCurrentContext_t)(void);
typedef void *(*eglGetCurrentSurface_t)(int readdraw);
typedef unsigned (*eglMakeCurrent_t)(void *dpy, void *draw, void *read, void *ctx);

static eglGetCurrentDisplay_t _eglGetCurrentDisplay = NULL;
static eglGetCurrentContext_t _eglGetCurrentContext = NULL;
static eglGetCurrentSurface_t _eglGetCurrentSurface = NULL;
static eglMakeCurrent_t _eglMakeCurrent = NULL;

static void *_egl_dpy = NULL;
static void *_egl_ctx = NULL;
static void *_egl_draw = NULL;
static void *_egl_read = NULL;

/* GLX. Same reasoning; GLXDrawable is an XID, i.e. unsigned long. */
typedef void *(*glXGetCurrentDisplay_t)(void);
typedef void *(*glXGetCurrentContext_t)(void);
typedef unsigned long (*glXGetCurrentDrawable_t)(void);
typedef unsigned long (*glXGetCurrentReadDrawable_t)(void);
typedef int (*glXMakeContextCurrent_t)(void *dpy, unsigned long draw, unsigned long read, void *ctx);

static glXGetCurrentDisplay_t _glXGetCurrentDisplay = NULL;
static glXGetCurrentContext_t _glXGetCurrentContext = NULL;
static glXGetCurrentDrawable_t _glXGetCurrentDrawable = NULL;
static glXGetCurrentReadDrawable_t _glXGetCurrentReadDrawable = NULL;
static glXMakeContextCurrent_t _glXMakeContextCurrent = NULL;

static void *_glx_dpy = NULL;
static void *_glx_ctx = NULL;
static unsigned long _glx_draw = 0;
static unsigned long _glx_read = 0;

static void *openLibrary(const char *const *names) {
	for (int i = 0; names[i]; i++) {
		void *handle = dlopen(names[i], RTLD_LAZY | RTLD_LOCAL);
		if (handle)
			return handle;
	}
	return NULL;
}

static bool probeEGL(void) {
	static const char *const names[] = {"libEGL.so.1", "libEGL.so", NULL};
	void *lib = openLibrary(names);
	if (!lib)
		return false;

	_eglGetCurrentDisplay = (eglGetCurrentDisplay_t)dlsym(lib, "eglGetCurrentDisplay");
	_eglGetCurrentContext = (eglGetCurrentContext_t)dlsym(lib, "eglGetCurrentContext");
	_eglGetCurrentSurface = (eglGetCurrentSurface_t)dlsym(lib, "eglGetCurrentSurface");
	_eglMakeCurrent = (eglMakeCurrent_t)dlsym(lib, "eglMakeCurrent");

	return _eglGetCurrentDisplay && _eglGetCurrentContext && _eglGetCurrentSurface && _eglMakeCurrent;
}

static bool probeGLX(void) {
	static const char *const names[] = {"libGL.so.1", "libGL.so", NULL};
	void *lib = openLibrary(names);
	if (!lib)
		return false;

	_glXGetCurrentDisplay = (glXGetCurrentDisplay_t)dlsym(lib, "glXGetCurrentDisplay");
	_glXGetCurrentContext = (glXGetCurrentContext_t)dlsym(lib, "glXGetCurrentContext");
	_glXGetCurrentDrawable = (glXGetCurrentDrawable_t)dlsym(lib, "glXGetCurrentDrawable");
	_glXGetCurrentReadDrawable = (glXGetCurrentReadDrawable_t)dlsym(lib, "glXGetCurrentReadDrawable");
	_glXMakeContextCurrent = (glXMakeContextCurrent_t)dlsym(lib, "glXMakeContextCurrent");

	return _glXGetCurrentDisplay && _glXGetCurrentContext && _glXGetCurrentDrawable && _glXGetCurrentReadDrawable && _glXMakeContextCurrent;
}

#elif defined(CONTEXT_BACKEND_CGL)

typedef void *(*CGLGetCurrentContext_t)(void);
typedef int (*CGLSetCurrentContext_t)(void *ctx);

static CGLGetCurrentContext_t _CGLGetCurrentContext = NULL;
static CGLSetCurrentContext_t _CGLSetCurrentContext = NULL;
static void *_cgl_ctx = NULL;

static bool probeCGL(void) {
	void *lib = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_LOCAL);
	if (!lib)
		return false;

	_CGLGetCurrentContext = (CGLGetCurrentContext_t)dlsym(lib, "CGLGetCurrentContext");
	_CGLSetCurrentContext = (CGLSetCurrentContext_t)dlsym(lib, "CGLSetCurrentContext");

	return _CGLGetCurrentContext && _CGLSetCurrentContext;
}

#elif defined(CONTEXT_BACKEND_WGL)

/* Resolved from opengl32.dll at runtime, like the dlopen backends above, so
 * the core does not need to be linked against opengl32. */
typedef HDC   (WINAPI *wglGetCurrentDC_t)(void);
typedef HGLRC (WINAPI *wglGetCurrentContext_t)(void);
typedef BOOL  (WINAPI *wglMakeCurrent_t)(HDC, HGLRC);

static wglGetCurrentDC_t      _wglGetCurrentDC      = NULL;
static wglGetCurrentContext_t _wglGetCurrentContext = NULL;
static wglMakeCurrent_t       _wglMakeCurrent       = NULL;

static HDC   _wgl_dc  = NULL;
static HGLRC _wgl_ctx = NULL;

static bool probeWGL(void) {
	HMODULE lib = GetModuleHandleA("opengl32.dll");
	if (!lib)
		lib = LoadLibraryA("opengl32.dll");
	if (!lib)
		return false;

	_wglGetCurrentDC      = (wglGetCurrentDC_t)(void *)GetProcAddress(lib, "wglGetCurrentDC");
	_wglGetCurrentContext = (wglGetCurrentContext_t)(void *)GetProcAddress(lib, "wglGetCurrentContext");
	_wglMakeCurrent       = (wglMakeCurrent_t)(void *)GetProcAddress(lib, "wglMakeCurrent");

	return _wglGetCurrentDC && _wglGetCurrentContext && _wglMakeCurrent;
}

#endif

static ContextBackend probeBackend(void) {
#if defined(CONTEXT_BACKEND_DL)
	if (probeEGL())
		return kBackendEGL;
	if (probeGLX())
		return kBackendGLX;
	return kBackendNone;
#elif defined(CONTEXT_BACKEND_CGL)
	return probeCGL() ? kBackendCGL : kBackendNone;
#elif defined(CONTEXT_BACKEND_WGL)
	return probeWGL() ? kBackendWGL : kBackendNone;
#else
	return kBackendNone;
#endif
}

bool retro_gl_context_handoff_available(void) {
	if (_backend == kBackendUnprobed)
		_backend = probeBackend();

	return _backend != kBackendNone;
}

void retro_gl_context_release(void) {
	if (!retro_gl_context_handoff_available())
		return;

	_held = false;

	switch (_backend) {
#if defined(CONTEXT_BACKEND_DL)
	case kBackendEGL:
		_egl_dpy = _eglGetCurrentDisplay();
		_egl_ctx = _eglGetCurrentContext();
		if (!_egl_dpy || !_egl_ctx)
			return;
		_egl_draw = _eglGetCurrentSurface(LOCAL_EGL_DRAW);
		_egl_read = _eglGetCurrentSurface(LOCAL_EGL_READ);
		_eglMakeCurrent(_egl_dpy, NULL, NULL, NULL);
		_held = true;
		return;

	case kBackendGLX:
		_glx_dpy = _glXGetCurrentDisplay();
		_glx_ctx = _glXGetCurrentContext();
		if (!_glx_dpy || !_glx_ctx)
			return;
		_glx_draw = _glXGetCurrentDrawable();
		_glx_read = _glXGetCurrentReadDrawable();
		_glXMakeContextCurrent(_glx_dpy, 0, 0, NULL);
		_held = true;
		return;
#elif defined(CONTEXT_BACKEND_CGL)
	case kBackendCGL:
		_cgl_ctx = _CGLGetCurrentContext();
		if (!_cgl_ctx)
			return;
		_CGLSetCurrentContext(NULL);
		_held = true;
		return;
#elif defined(CONTEXT_BACKEND_WGL)
	case kBackendWGL:
		_wgl_dc = _wglGetCurrentDC();
		_wgl_ctx = _wglGetCurrentContext();
		if (!_wgl_dc || !_wgl_ctx)
			return;
		_wglMakeCurrent(NULL, NULL);
		_held = true;
		return;
#endif
	default:
		return;
	}
}

void retro_gl_context_acquire(void) {
	if (!_held)
		return;

	_held = false;

	switch (_backend) {
#if defined(CONTEXT_BACKEND_DL)
	case kBackendEGL:
		_eglMakeCurrent(_egl_dpy, _egl_draw, _egl_read, _egl_ctx);
		return;

	case kBackendGLX:
		_glXMakeContextCurrent(_glx_dpy, _glx_draw, _glx_read, _glx_ctx);
		return;
#elif defined(CONTEXT_BACKEND_CGL)
	case kBackendCGL:
		_CGLSetCurrentContext(_cgl_ctx);
		return;
#elif defined(CONTEXT_BACKEND_WGL)
	case kBackendWGL:
		_wglMakeCurrent(_wgl_dc, _wgl_ctx);
		return;
#endif
	default:
		return;
	}
}

#endif /* USE_LIBCO || !USE_OPENGL */
