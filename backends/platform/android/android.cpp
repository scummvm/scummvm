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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/base-backend.h"
#include "base/main.h"
#include "graphics/surface.h"

#include "backends/platform/android/video.h"

#if defined(ANDROID_BACKEND)

#include <jni.h>

#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <android/log.h>

#include "common/archive.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/keymapper/keymapper.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/plugins/posix/posix-provider.h"
#include "sound/mixer_intern.h"

#include "backends/platform/android/asset-archive.h"

#undef LOG_TAG
#define LOG_TAG "ScummVM"

#if 0
#define ENTER(args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, args)
#else
#define ENTER(args...) /**/
#endif

// Fix JNIEXPORT declaration to actually do something useful
#undef JNIEXPORT
#define JNIEXPORT __attribute__ ((visibility("default")))

// This replaces the bionic libc assert message with something that
// actually prints the assertion failure before aborting.
extern "C"
void __assert(const char *file, int line, const char *expr) {
	__android_log_assert(expr, LOG_TAG, "%s:%d: Assertion failure: %s",
						 file, line, expr);
}

static JavaVM *cached_jvm;
static jfieldID FID_Event_type;
static jfieldID FID_Event_synthetic;
static jfieldID FID_Event_kbd_keycode;
static jfieldID FID_Event_kbd_ascii;
static jfieldID FID_Event_kbd_flags;
static jfieldID FID_Event_mouse_x;
static jfieldID FID_Event_mouse_y;
static jfieldID FID_Event_mouse_relative;
static jfieldID FID_ScummVM_nativeScummVM;
static jmethodID MID_Object_wait;

JNIEnv* JNU_GetEnv() {
	JNIEnv* env;
	bool version_unsupported =
		cached_jvm->GetEnv((void**)&env, JNI_VERSION_1_2);
	assert(! version_unsupported);
	return env;
}

static void JNU_ThrowByName(JNIEnv* env, const char* name, const char* msg) {
	jclass cls = env->FindClass(name);
	// if cls is NULL, an exception has already been thrown
	if (cls != NULL)
		env->ThrowNew(cls, msg);
	env->DeleteLocalRef(cls);
}

// floating point.	use sparingly.
template <class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1<<16));
	return (numerator << 16) / denominator;
}

#ifdef DYNAMIC_MODULES
class AndroidPluginProvider : public POSIXPluginProvider {
protected:
	virtual void addCustomDirectories(Common::FSList &dirs) const;
};
#endif


#if 0
#define CHECK_GL_ERROR() checkGlError(__FILE__, __LINE__)
static const char* getGlErrStr(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:		   return "GL_NO_ERROR";
	case GL_INVALID_ENUM:	   return "GL_INVALID_ENUM";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:	   return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:	   return "GL_OUT_OF_MEMORY";
	}

	static char buf[40];
	snprintf(buf, sizeof(buf), "(Unknown GL error code 0x%x)", error);
	return buf;
}
static void checkGlError(const char* file, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		warning("%s:%d: GL error: %s", file, line, getGlErrStr(error));
}
#else
#define CHECK_GL_ERROR() do {} while (false)
#endif

class OSystem_Android : public BaseBackend {
private:
	jobject _back_ptr;	// back pointer to (java) peer instance
	jmethodID MID_displayMessageOnOSD;
	jmethodID MID_setWindowCaption;
	jmethodID MID_initBackend;
	jmethodID MID_audioSampleRate;
	jmethodID MID_showVirtualKeyboard;
	jmethodID MID_getSysArchives;
	jmethodID MID_getPluginDirectories;
	jmethodID MID_setupScummVMSurface;
	jmethodID MID_destroyScummVMSurface;
	jmethodID MID_swapBuffers;

	int _screen_changeid;
	int _egl_surface_width;
	int _egl_surface_height;

	bool _force_redraw;

	// Game layer
	GLESPaletteTexture* _game_texture;
	int _shake_offset;
	Common::Rect _focus_rect;
	bool _full_screen_dirty;

	// Overlay layer
	GLES4444Texture* _overlay_texture;
	bool _show_overlay;

	// Mouse layer
	GLESPaletteATexture* _mouse_texture;
	Common::Point _mouse_hotspot;
	int _mouse_targetscale;
	bool _show_mouse;
	bool _use_mouse_palette;

	Common::Queue<Common::Event> _event_queue;
	MutexRef _event_queue_lock;

	bool _timer_thread_exit;
	pthread_t _timer_thread;
	static void* timerThreadFunc(void* arg);

	bool _enable_zoning;
	bool _virtkeybd_on;

	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;
	Common::TimerManager *_timer;
	FilesystemFactory *_fsFactory;
	Common::Archive *_asset_archive;
	timeval _startTime;

	void setupScummVMSurface();
	void destroyScummVMSurface();
	void setupKeymapper();
	void _setCursorPalette(const byte *colors, uint start, uint num);

public:
	OSystem_Android(jobject am);
	virtual ~OSystem_Android();
	bool initJavaHooks(JNIEnv* env, jobject self);

	static OSystem_Android* fromJavaObject(JNIEnv* env, jobject obj);
	virtual void initBackend();
	void addPluginDirectories(Common::FSList &dirs) const;
	void enableZoning(bool enable) { _enable_zoning = enable; }
	void setSurfaceSize(int width, int height) {
		_egl_surface_width = width;
		_egl_surface_height = height;
	}

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	bool setGraphicsMode(const char *name);
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height,
				  const Graphics::PixelFormat *format);
	virtual int getScreenChangeID() const { return _screen_changeid; }
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);
	virtual void fillScreen(uint32 col);
	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual Graphics::PixelFormat getOverlayFormat() const {
		// RGBA 4444
		Graphics::PixelFormat format;
		format.bytesPerPixel = 2;
		format.rLoss = 8 - 4;
		format.gLoss = 8 - 4;
		format.bLoss = 8 - 4;
		format.aLoss = 8 - 4;
		format.rShift = 3*4;
		format.gShift = 2*4;
		format.bShift = 1*4;
		format.aShift = 0*4;
		return format;
	}

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);
	virtual void disableCursorPalette(bool disable);

	virtual bool pollEvent(Common::Event &event);
	void pushEvent(const Common::Event& event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void quit();

	virtual void setWindowCaption(const char *caption);
	virtual void displayMessageOnOSD(const char *msg);
	virtual void showVirtualKeyboard(bool enable);

	virtual Common::SaveFileManager *getSavefileManager();
	virtual Audio::Mixer *getMixer();
	virtual void getTimeAndDate(TimeDate &t) const;
	virtual Common::TimerManager *getTimerManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
};

OSystem_Android::OSystem_Android(jobject am)
	: _back_ptr(0),
	  _screen_changeid(0),
	  _force_redraw(false),
	  _game_texture(NULL),
	  _overlay_texture(NULL),
	  _mouse_texture(NULL),
	  _use_mouse_palette(false),
	  _show_mouse(false),
	  _show_overlay(false),
	  _enable_zoning(false),
	  _savefile(0),
	  _mixer(0),
	  _timer(0),
	  _fsFactory(new POSIXFilesystemFactory()),
	  _asset_archive(new AndroidAssetArchive(am)),
	  _shake_offset(0),
	  _full_screen_dirty(false),
	  _event_queue_lock(createMutex()) {
}

OSystem_Android::~OSystem_Android() {
	ENTER("~OSystem_Android()");
	delete _game_texture;
	delete _overlay_texture;
	delete _mouse_texture;
	destroyScummVMSurface();
	JNIEnv* env = JNU_GetEnv();
	//env->DeleteWeakGlobalRef(_back_ptr);
	env->DeleteGlobalRef(_back_ptr);
	delete _savefile;
	delete _mixer;
	delete _timer;
	delete _fsFactory;
	delete _asset_archive;
	deleteMutex(_event_queue_lock);
}

OSystem_Android* OSystem_Android::fromJavaObject(JNIEnv* env, jobject obj) {
	jlong peer = env->GetLongField(obj, FID_ScummVM_nativeScummVM);
	return (OSystem_Android*)peer;
}

bool OSystem_Android::initJavaHooks(JNIEnv* env, jobject self) {
	// weak global ref to allow class to be unloaded
	// ... except dalvik doesn't implement NewWeakGlobalRef (yet)
	//_back_ptr = env->NewWeakGlobalRef(self);
	_back_ptr = env->NewGlobalRef(self);

	jclass cls = env->GetObjectClass(_back_ptr);

#define FIND_METHOD(name, signature) do {						\
		MID_ ## name = env->GetMethodID(cls, #name, signature); \
		if (MID_ ## name == NULL)								\
			return false;										\
	} while (0)

	FIND_METHOD(setWindowCaption, "(Ljava/lang/String;)V");
	FIND_METHOD(displayMessageOnOSD, "(Ljava/lang/String;)V");
	FIND_METHOD(initBackend, "()V");
	FIND_METHOD(audioSampleRate, "()I");
	FIND_METHOD(showVirtualKeyboard, "(Z)V");
	FIND_METHOD(getSysArchives, "()[Ljava/lang/String;");
	FIND_METHOD(getPluginDirectories, "()[Ljava/lang/String;");
	FIND_METHOD(setupScummVMSurface, "()V");
	FIND_METHOD(destroyScummVMSurface, "()V");
	FIND_METHOD(swapBuffers, "()Z");

#undef FIND_METHOD

	return true;
}

static void ScummVM_create(JNIEnv* env, jobject self, jobject am) {
	OSystem_Android* cpp_obj = new OSystem_Android(am);
	if (!cpp_obj->initJavaHooks(env, self))
		// Exception already thrown by initJavaHooks
		return;

	env->SetLongField(self, FID_ScummVM_nativeScummVM, (jlong)cpp_obj);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new AndroidPluginProvider());
#endif
}

static void ScummVM_nativeDestroy(JNIEnv* env, jobject self) {
	OSystem_Android* cpp_obj = OSystem_Android::fromJavaObject(env, self);
	delete cpp_obj;
}

static void ScummVM_audioMixCallback(JNIEnv* env, jobject self,
									 jbyteArray jbuf) {
	OSystem_Android* cpp_obj = OSystem_Android::fromJavaObject(env, self);
	jsize len = env->GetArrayLength(jbuf);
	jbyte* buf = env->GetByteArrayElements(jbuf, NULL);
	if (buf == NULL) {
		warning("Unable to get Java audio byte array. Skipping.");
		return;
	}
	Audio::MixerImpl* mixer =
		static_cast<Audio::MixerImpl*>(cpp_obj->getMixer());
	assert(mixer);
	mixer->mixCallback(reinterpret_cast<byte*>(buf), len);
	env->ReleaseByteArrayElements(jbuf, buf, 0);
}

static void ScummVM_setConfManInt(JNIEnv* env, jclass cls,
				  jstring key_obj, jint value) {
	ENTER("setConfManInt(%p, %d)", key_obj, (int)value);
	const char* key = env->GetStringUTFChars(key_obj, NULL);
	if (key == NULL)
		return;
	ConfMan.setInt(key, value);
	env->ReleaseStringUTFChars(key_obj, key);
}

static void ScummVM_setConfManString(JNIEnv* env, jclass cls, jstring key_obj,
					 jstring value_obj) {
	ENTER("setConfManStr(%p, %p)", key_obj, value_obj);
	const char* key = env->GetStringUTFChars(key_obj, NULL);
	if (key == NULL)
		return;
	const char* value = env->GetStringUTFChars(value_obj, NULL);
	if (value == NULL) {
		env->ReleaseStringUTFChars(key_obj, key);
		return;
	}
	ConfMan.set(key, value);
	env->ReleaseStringUTFChars(value_obj, value);
	env->ReleaseStringUTFChars(key_obj, key);
}

void* OSystem_Android::timerThreadFunc(void* arg) {
	OSystem_Android* system = (OSystem_Android*)arg;
	DefaultTimerManager* timer = (DefaultTimerManager*)(system->_timer);

	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = 100 * 1000 * 1000;	// 100ms

	while (!system->_timer_thread_exit) {
		timer->handler();
		nanosleep(&tv, NULL);
	}

	return NULL;
}

void OSystem_Android::initBackend() {
	ENTER("initBackend()");
	JNIEnv* env = JNU_GetEnv();

	ConfMan.setInt("autosave_period", 0);
	ConfMan.setInt("FM_medium_quality", true);

	// must happen before creating TimerManager to avoid race in
	// creating EventManager
	setupKeymapper();

	// BUG: "transient" ConfMan settings get nuked by the options
	// screen.	Passing the savepath in this way makes it stick
	// (via ConfMan.registerDefault)
	_savefile = new DefaultSaveFileManager(ConfMan.get("savepath"));
	_timer = new DefaultTimerManager();

	gettimeofday(&_startTime, NULL);

	jint sample_rate = env->CallIntMethod(_back_ptr, MID_audioSampleRate);
	if (env->ExceptionCheck()) {
		warning("Error finding audio sample rate - assuming 11025HZ");
		env->ExceptionDescribe();
		env->ExceptionClear();
		sample_rate = 11025;
	}
	_mixer = new Audio::MixerImpl(this, sample_rate);
	_mixer->setReady(true);

	env->CallVoidMethod(_back_ptr, MID_initBackend);
	if (env->ExceptionCheck()) {
		error("Error in Java initBackend");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}

	_timer_thread_exit = false;
	pthread_create(&_timer_thread, NULL, timerThreadFunc, this);

	OSystem::initBackend();

	setupScummVMSurface();
}

void OSystem_Android::addPluginDirectories(Common::FSList &dirs) const {
	ENTER("OSystem_Android::addPluginDirectories()");
	JNIEnv* env = JNU_GetEnv();

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_back_ptr, MID_getPluginDirectories);
	if (env->ExceptionCheck()) {
		warning("Error finding plugin directories");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);
		if (path_obj == NULL)
			continue;
		const char* path = env->GetStringUTFChars(path_obj, NULL);
		if (path == NULL) {
			warning("Error getting string characters from plugin directory");
			env->ExceptionClear();
			env->DeleteLocalRef(path_obj);
			continue;
		}
		dirs.push_back(Common::FSNode(path));
		env->ReleaseStringUTFChars(path_obj, path);
		env->DeleteLocalRef(path_obj);
	}
}

bool OSystem_Android::hasFeature(Feature f) {
	return (f == kFeatureCursorHasPalette ||
		f == kFeatureVirtualKeyboard ||
		f == kFeatureOverlaySupportsAlpha);
}

void OSystem_Android::setFeatureState(Feature f, bool enable) {
	ENTER("setFeatureState(%d, %d)", f, enable);
	switch (f) {
	case kFeatureVirtualKeyboard:
		_virtkeybd_on = enable;
		showVirtualKeyboard(enable);
		break;
	default:
		break;
	}
}

bool OSystem_Android::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureVirtualKeyboard:
		return _virtkeybd_on;
	default:
		return false;
	}
}

const OSystem::GraphicsMode* OSystem_Android::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
		{"default", "Default", 1},
		{0, 0, 0},
	};
	return s_supportedGraphicsModes;
}


int OSystem_Android::getDefaultGraphicsMode() const {
	return 1;
}

bool OSystem_Android::setGraphicsMode(const char *mode) {
	ENTER("setGraphicsMode(%s)", mode);
	return true;
}

bool OSystem_Android::setGraphicsMode(int mode) {
	ENTER("setGraphicsMode(%d)", mode);
	return true;
}

int OSystem_Android::getGraphicsMode() const {
	return 1;
}

void OSystem_Android::setupScummVMSurface() {
	ENTER("setupScummVMSurface");
	JNIEnv* env = JNU_GetEnv();
	env->CallVoidMethod(_back_ptr, MID_setupScummVMSurface);
	if (env->ExceptionCheck())
		return;

	// EGL set up with a new surface.  Initialise OpenGLES context.

	GLESTexture::initGLExtensions();

	// Turn off anything that looks like 3D ;)
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DITHER);
	glShadeModel(GL_FLAT);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnable(GL_TEXTURE_2D);

	if (!_game_texture)
		_game_texture = new GLESPaletteTexture();
	else
		_game_texture->reinitGL();

	if (!_overlay_texture)
		_overlay_texture = new GLES4444Texture();
	else
		_overlay_texture->reinitGL();

	if (!_mouse_texture)
		_mouse_texture = new GLESPaletteATexture();
	else
		_mouse_texture->reinitGL();

	glViewport(0, 0, _egl_surface_width, _egl_surface_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0, _egl_surface_width, _egl_surface_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	clearFocusRectangle();
	CHECK_GL_ERROR();
}

void OSystem_Android::destroyScummVMSurface() {
	JNIEnv* env = JNU_GetEnv();
	env->CallVoidMethod(_back_ptr, MID_destroyScummVMSurface);
	// Can't use OpenGLES functions after this
}

void OSystem_Android::initSize(uint width, uint height,
							   const Graphics::PixelFormat *format) {
	ENTER("initSize(%d,%d,%p)", width, height, format);

	_game_texture->allocBuffer(width, height);

	// Cap at 320x200 or the ScummVM themes abort :/
	GLuint overlay_width = MIN(_egl_surface_width, 320);
	GLuint overlay_height = MIN(_egl_surface_height, 200);
	_overlay_texture->allocBuffer(overlay_width, overlay_height);

	// Don't know mouse size yet - it gets reallocated in
	// setMouseCursor.  We need the palette allocated before
	// setMouseCursor however, so just take a guess at the desired
	// size (it's small).
	_mouse_texture->allocBuffer(20, 20);
}

int16 OSystem_Android::getHeight() {
	return _game_texture->height();
}

int16 OSystem_Android::getWidth() {
	return _game_texture->width();
}

void OSystem_Android::setPalette(const byte* colors, uint start, uint num) {
	ENTER("setPalette(%p, %u, %u)", colors, start, num);

	if (!_use_mouse_palette)
		_setCursorPalette(colors, start, num);

	byte* palette = _game_texture->palette() + start*3;
	do {
		for (int i = 0; i < 3; ++i)
			palette[i] = colors[i];
		palette += 3;
		colors += 4;
	} while (--num);
}

void OSystem_Android::grabPalette(byte *colors, uint start, uint num) {
	ENTER("grabPalette(%p, %u, %u)", colors, start, num);
	const byte* palette = _game_texture->palette_const() + start*3;
	do {
		for (int i = 0; i < 3; ++i)
			colors[i] = palette[i];
		colors[3] = 0xff;  // alpha

		palette += 3;
		colors += 4;
	} while (--num);
}

void OSystem_Android::copyRectToScreen(const byte *buf, int pitch,
					   int x, int y, int w, int h) {
	ENTER("copyRectToScreen(%p, %d, %d, %d, %d, %d)",
		  buf, pitch, x, y, w, h);

	_game_texture->updateBuffer(x, y, w, h, buf, pitch);
}

void OSystem_Android::updateScreen() {
	//ENTER("updateScreen()");

	if (!_force_redraw &&
		!_game_texture->dirty() &&
		!_overlay_texture->dirty() &&
		!_mouse_texture->dirty())
		return;

	_force_redraw = false;

	glPushMatrix();

	if (_shake_offset != 0 ||
		(!_focus_rect.isEmpty() &&
		 !Common::Rect(_game_texture->width(),
					   _game_texture->height()).contains(_focus_rect))) {
		// These are the only cases where _game_texture doesn't
		// cover the entire screen.
		glClearColorx(0, 0, 0, 1 << 16);
		glClear(GL_COLOR_BUFFER_BIT);

		// Move everything up by _shake_offset (game) pixels
		glTranslatex(0, -_shake_offset << 16, 0);
	}

	if (_focus_rect.isEmpty()) {
		_game_texture->drawTexture(0, 0,
								   _egl_surface_width, _egl_surface_height);
	} else {
		glPushMatrix();
		glScalex(xdiv(_egl_surface_width, _focus_rect.width()),
				 xdiv(_egl_surface_height, _focus_rect.height()),
				 1 << 16);
		glTranslatex(-_focus_rect.left << 16, -_focus_rect.top << 16, 0);
		glScalex(xdiv(_game_texture->width(), _egl_surface_width),
				 xdiv(_game_texture->height(), _egl_surface_height),
				 1 << 16);
		_game_texture->drawTexture(0, 0,
								   _egl_surface_width, _egl_surface_height);
		glPopMatrix();
	}

	CHECK_GL_ERROR();

	if (_show_overlay) {
		_overlay_texture->drawTexture(0, 0,
									  _egl_surface_width,
									  _egl_surface_height);
		CHECK_GL_ERROR();
	}

	if (_show_mouse) {
		glPushMatrix();

		glTranslatex(-_mouse_hotspot.x << 16,
					 -_mouse_hotspot.y << 16,
					 0);

		// Scale up ScummVM -> OpenGL (pixel) coordinates
		int texwidth, texheight;
		if (_show_overlay) {
			texwidth = getOverlayWidth();
			texheight = getOverlayHeight();
		} else {
			texwidth = getWidth();
			texheight = getHeight();
		}
		glScalex(xdiv(_egl_surface_width, texwidth),
				 xdiv(_egl_surface_height, texheight),
				 1 << 16);

		// Note the extra half texel to position the mouse in
		// the middle of the x,y square:
		const Common::Point& mouse = getEventManager()->getMousePos();
		glTranslatex((mouse.x << 16) | 1 << 15,
					 (mouse.y << 16) | 1 << 15, 0);

		// Mouse targetscale just seems to make the cursor way
		// too big :/
		//glScalex(_mouse_targetscale << 16, _mouse_targetscale << 16,
		//	 1 << 16);

		_mouse_texture->drawTexture();

		glPopMatrix();
	}

	glPopMatrix();

	CHECK_GL_ERROR();

	JNIEnv* env = JNU_GetEnv();
	if (!env->CallBooleanMethod(_back_ptr, MID_swapBuffers)) {
		// Context lost -> need to reinit GL
		destroyScummVMSurface();
		setupScummVMSurface();
	}
}

Graphics::Surface *OSystem_Android::lockScreen() {
	ENTER("lockScreen()");
	Graphics::Surface* surface = _game_texture->surface();
	assert(surface->pixels);
	return surface;
}

void OSystem_Android::unlockScreen() {
	ENTER("unlockScreen()");
	assert(_game_texture->dirty());
}

void OSystem_Android::setShakePos(int shake_offset) {
	ENTER("setShakePos(%d)", shake_offset);
	if (_shake_offset != shake_offset) {
		_shake_offset = shake_offset;
		_force_redraw = true;
	}
}

void OSystem_Android::fillScreen(uint32 col) {
	ENTER("fillScreen(%u)", col);
	assert(col < 256);
	_game_texture->fillBuffer(col);
}

void OSystem_Android::setFocusRectangle(const Common::Rect& rect) {
	ENTER("setFocusRectangle(%d,%d,%d,%d)",
		  rect.left, rect.top, rect.right, rect.bottom);
	if (_enable_zoning) {
		_focus_rect = rect;
		_force_redraw = true;
	}
}

void OSystem_Android::clearFocusRectangle() {
	ENTER("clearFocusRectangle()");
	if (_enable_zoning) {
		_focus_rect = Common::Rect();
		_force_redraw = true;
	}
}

void OSystem_Android::showOverlay() {
	ENTER("showOverlay()");
	_show_overlay = true;
	_force_redraw = true;
}

void OSystem_Android::hideOverlay() {
	ENTER("hideOverlay()");
	_show_overlay = false;
	_force_redraw = true;
}

void OSystem_Android::clearOverlay() {
	ENTER("clearOverlay()");
	_overlay_texture->fillBuffer(0);
}

void OSystem_Android::grabOverlay(OverlayColor *buf, int pitch) {
	ENTER("grabOverlay(%p, %d)", buf, pitch);
	// We support overlay alpha blending, so the pixel data here
	// shouldn't actually be used.	Let's fill it with zeros, I'm sure
	// it will be fine...
	const Graphics::Surface* surface = _overlay_texture->surface_const();
	assert(surface->bytesPerPixel == sizeof(buf[0]));
	int h = surface->h;
	do {
		memset(buf, 0, surface->w * sizeof(buf[0]));
		buf += pitch;  // This 'pitch' is pixels not bytes
	} while (--h);
}

void OSystem_Android::copyRectToOverlay(const OverlayColor *buf, int pitch,
					int x, int y, int w, int h) {
	ENTER("copyRectToOverlay(%p, %d, %d, %d, %d, %d)",
		 buf, pitch, x, y, w, h);
	const Graphics::Surface* surface = _overlay_texture->surface_const();
	assert(surface->bytesPerPixel == sizeof(buf[0]));

	// This 'pitch' is pixels not bytes
	_overlay_texture->updateBuffer(x, y, w, h, buf, pitch * sizeof(buf[0]));
}

int16 OSystem_Android::getOverlayHeight() {
	return _overlay_texture->height();
}

int16 OSystem_Android::getOverlayWidth() {
	return _overlay_texture->width();
}

bool OSystem_Android::showMouse(bool visible) {
	ENTER("showMouse(%d)", visible);
	_show_mouse = visible;
	return true;
}

void OSystem_Android::warpMouse(int x, int y) {
	ENTER("warpMouse(%d, %d)", x, y);
	// We use only the eventmanager's idea of the current mouse
	// position, so there is nothing extra to do here.
}

void OSystem_Android::setMouseCursor(const byte *buf, uint w, uint h,
					 int hotspotX, int hotspotY,
					 uint32 keycolor, int cursorTargetScale,
					 const Graphics::PixelFormat *format) {
	ENTER("setMouseCursor(%p, %u, %u, %d, %d, %d, %d, %p)",
		  buf, w, h, hotspotX, hotspotY, (int)keycolor, cursorTargetScale,
		  format);

	assert(keycolor < 256);

	_mouse_texture->allocBuffer(w, h);

	// Update palette alpha based on keycolor
	byte* palette = _mouse_texture->palette();
	int i = 256;
	do {
		palette[3] = 0xff;
		palette += 4;
	} while (--i);
	palette = _mouse_texture->palette();
	palette[keycolor*4 + 3] = 0x00;
	_mouse_texture->updateBuffer(0, 0, w, h, buf, w);

	_mouse_hotspot = Common::Point(hotspotX, hotspotY);
	_mouse_targetscale = cursorTargetScale;
}

void OSystem_Android::_setCursorPalette(const byte *colors,
					uint start, uint num) {
	byte* palette = _mouse_texture->palette() + start*4;
	do {
		for (int i = 0; i < 3; ++i)
			palette[i] = colors[i];
		// Leave alpha untouched to preserve keycolor

		palette += 4;
		colors += 4;
	} while (--num);
}

void OSystem_Android::setCursorPalette(const byte *colors,
					   uint start, uint num) {
	ENTER("setCursorPalette(%p, %u, %u)", colors, start, num);
	_setCursorPalette(colors, start, num);
	_use_mouse_palette = true;
}

void OSystem_Android::disableCursorPalette(bool disable) {
	ENTER("disableCursorPalette(%d)", disable);
	_use_mouse_palette = !disable;
}

void OSystem_Android::setupKeymapper() {
#ifdef ENABLE_KEYMAPPER
	using namespace Common;

	Keymapper *mapper = getEventManager()->getKeymapper();

	HardwareKeySet *keySet = new HardwareKeySet();
	keySet->addHardwareKey(
		new HardwareKey("n", KeyState(KEYCODE_n), "n (vk)",
				kTriggerLeftKeyType,
				kVirtualKeyboardActionType));
	mapper->registerHardwareKeySet(keySet);

	Keymap *globalMap = new Keymap("global");
	Action *act;

	act = new Action(globalMap, "VIRT", "Display keyboard",
			 kVirtualKeyboardActionType);
	act->addKeyEvent(KeyState(KEYCODE_F7, ASCII_F7, 0));

	mapper->addGlobalKeymap(globalMap);

	mapper->pushKeymap("global");
#endif
}

bool OSystem_Android::pollEvent(Common::Event &event) {
	//ENTER("pollEvent()");
	lockMutex(_event_queue_lock);
	if (_event_queue.empty()) {
		unlockMutex(_event_queue_lock);
		return false;
	}
	event = _event_queue.pop();
	unlockMutex(_event_queue_lock);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		// TODO: only dirty/redraw move bounds
		_force_redraw = true;
		// fallthrough
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP: {
		if (event.kbd.flags == 1) { // relative mouse hack
			// Relative (trackball) mouse hack.
			const Common::Point& mouse_pos =
				getEventManager()->getMousePos();
			event.mouse.x += mouse_pos.x;
			event.mouse.y += mouse_pos.y;
			event.mouse.x = CLIP(event.mouse.x, (int16)0, _show_overlay ?
								 getOverlayWidth() : getWidth());
			event.mouse.y = CLIP(event.mouse.y, (int16)0, _show_overlay ?
								 getOverlayHeight() : getHeight());
		} else {
			// Touchscreen events need to be converted
			// from device to game coords first.
			const GLESTexture* tex = _show_overlay
				? static_cast<GLESTexture*>(_overlay_texture)
				: static_cast<GLESTexture*>(_game_texture);
			event.mouse.x = scalef(event.mouse.x, tex->width(),
								   _egl_surface_width);
			event.mouse.y = scalef(event.mouse.y, tex->height(),
								   _egl_surface_height);
			event.mouse.x -= _shake_offset;
		}
		break;
	}
	case Common::EVENT_SCREEN_CHANGED:
		debug("EVENT_SCREEN_CHANGED");
		_screen_changeid++;
		destroyScummVMSurface();
		setupScummVMSurface();
		break;
	default:
		break;
	}

	return true;
}

void OSystem_Android::pushEvent(const Common::Event& event) {
	lockMutex(_event_queue_lock);

	// Try to combine multiple queued mouse move events
	if (event.type == Common::EVENT_MOUSEMOVE &&
		!_event_queue.empty() &&
		_event_queue.back().type == Common::EVENT_MOUSEMOVE) {
	  Common::Event tail = _event_queue.back();
	  if (event.kbd.flags) {
		// relative movement hack
		tail.mouse.x += event.mouse.x;
		tail.mouse.y += event.mouse.y;
	  } else {
		// absolute position
		tail.kbd.flags = 0;	 // clear relative flag
		tail.mouse.x = event.mouse.x;
		tail.mouse.y = event.mouse.y;
	  }
	}
	else
	  _event_queue.push(event);

	unlockMutex(_event_queue_lock);
}

static void ScummVM_pushEvent(JNIEnv* env, jobject self, jobject java_event) {
	OSystem_Android* cpp_obj = OSystem_Android::fromJavaObject(env, self);

	Common::Event event;
	event.type = (Common::EventType)env->GetIntField(java_event,
							 FID_Event_type);
	event.synthetic =
		env->GetBooleanField(java_event, FID_Event_synthetic);

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP:
		event.kbd.keycode = (Common::KeyCode)env->GetIntField(
			java_event, FID_Event_kbd_keycode);
		event.kbd.ascii = static_cast<int>(env->GetIntField(
			java_event, FID_Event_kbd_ascii));
		event.kbd.flags = static_cast<int>(env->GetIntField(
			java_event, FID_Event_kbd_flags));
		break;
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		event.mouse.x =
			env->GetIntField(java_event, FID_Event_mouse_x);
		event.mouse.y =
			env->GetIntField(java_event, FID_Event_mouse_y);
		// This is a terrible hack.	 We stash "relativeness"
		// in the kbd.flags field until pollEvent() can work
		// it out.
		event.kbd.flags = env->GetBooleanField(
			java_event, FID_Event_mouse_relative) ? 1 : 0;
		break;
	default:
		break;
	}

	cpp_obj->pushEvent(event);
}

uint32 OSystem_Android::getMillis() {
	timeval curTime;
	gettimeofday(&curTime, NULL);
	return (uint32)(((curTime.tv_sec - _startTime.tv_sec) * 1000) + \
			((curTime.tv_usec - _startTime.tv_usec) / 1000));
}

void OSystem_Android::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_Android::createMutex() {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_t *mutex = new pthread_mutex_t;
	if (pthread_mutex_init(mutex, &attr) != 0) {
		warning("pthread_mutex_init() failed!");
		delete mutex;
		return NULL;
	}
	return (MutexRef)mutex;
}

void OSystem_Android::lockMutex(MutexRef mutex) {
	if (pthread_mutex_lock((pthread_mutex_t*)mutex) != 0)
		warning("pthread_mutex_lock() failed!");
}

void OSystem_Android::unlockMutex(MutexRef mutex) {
	if (pthread_mutex_unlock((pthread_mutex_t*)mutex) != 0)
		warning("pthread_mutex_unlock() failed!");
}

void OSystem_Android::deleteMutex(MutexRef mutex) {
	pthread_mutex_t* m = (pthread_mutex_t*)mutex;
	if (pthread_mutex_destroy(m) != 0)
		warning("pthread_mutex_destroy() failed!");
	else
		delete m;
}

void OSystem_Android::quit() {
	ENTER("quit()");

	_timer_thread_exit = true;
	pthread_join(_timer_thread, NULL);
}

void OSystem_Android::setWindowCaption(const char *caption) {
	ENTER("setWindowCaption(%s)", caption);
	JNIEnv* env = JNU_GetEnv();
	jstring java_caption = env->NewStringUTF(caption);
	env->CallVoidMethod(_back_ptr, MID_setWindowCaption, java_caption);
	if (env->ExceptionCheck()) {
		warning("Failed to set window caption");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->DeleteLocalRef(java_caption);
}

void OSystem_Android::displayMessageOnOSD(const char *msg) {
	ENTER("displayMessageOnOSD(%s)", msg);
	JNIEnv* env = JNU_GetEnv();
	jstring java_msg = env->NewStringUTF(msg);
	env->CallVoidMethod(_back_ptr, MID_displayMessageOnOSD, java_msg);
	if (env->ExceptionCheck()) {
		warning("Failed to display OSD message");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
	env->DeleteLocalRef(java_msg);
}

void OSystem_Android::showVirtualKeyboard(bool enable) {
	ENTER("showVirtualKeyboard(%d)", enable);
	JNIEnv* env = JNU_GetEnv();
	env->CallVoidMethod(_back_ptr, MID_showVirtualKeyboard, enable);
	if (env->ExceptionCheck()) {
		error("Error trying to show virtual keyboard");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

Common::SaveFileManager *OSystem_Android::getSavefileManager() {
	assert(_savefile);
	return _savefile;
}

Audio::Mixer *OSystem_Android::getMixer() {
	assert(_mixer);
	return _mixer;
}

Common::TimerManager *OSystem_Android::getTimerManager() {
	assert(_timer);
	return _timer;
}

void OSystem_Android::getTimeAndDate(TimeDate &td) const {
	struct tm tm;
	const time_t curTime = time(NULL);
	localtime_r(&curTime, &tm);
	td.tm_sec = tm.tm_sec;
	td.tm_min = tm.tm_min;
	td.tm_hour = tm.tm_hour;
	td.tm_mday = tm.tm_mday;
	td.tm_mon = tm.tm_mon;
	td.tm_year = tm.tm_year;
}

FilesystemFactory *OSystem_Android::getFilesystemFactory() {
	return _fsFactory;
}

void OSystem_Android::addSysArchivesToSearchSet(Common::SearchSet &s,
						int priority) {
	s.add("ASSET", _asset_archive, priority, false);

	JNIEnv* env = JNU_GetEnv();

	jobjectArray array =
		(jobjectArray)env->CallObjectMethod(_back_ptr, MID_getSysArchives);
	if (env->ExceptionCheck()) {
		warning("Error finding system archive path");
		env->ExceptionDescribe();
		env->ExceptionClear();
		return;
	}

	jsize size = env->GetArrayLength(array);
	for (jsize i = 0; i < size; ++i) {
		jstring path_obj = (jstring)env->GetObjectArrayElement(array, i);
		const char* path = env->GetStringUTFChars(path_obj, NULL);
		if (path != NULL) {
			s.addDirectory(path, path, priority);
			env->ReleaseStringUTFChars(path_obj, path);
		}
		env->DeleteLocalRef(path_obj);
	}
}


static jint ScummVM_scummVMMain(JNIEnv* env, jobject self, jobjectArray args) {
	OSystem_Android* cpp_obj = OSystem_Android::fromJavaObject(env, self);

	const int MAX_NARGS = 32;
	int res = -1;

	int argc = env->GetArrayLength(args);
	if (argc > MAX_NARGS) {
		JNU_ThrowByName(env, "java/lang/IllegalArgumentException",
				"too many arguments");
		return 0;
	}

	char* argv[MAX_NARGS];
	int nargs;	// note use in cleanup loop below
	for (nargs = 0; nargs < argc; ++nargs) {
		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);
		if (arg == NULL) {
			argv[nargs] = NULL;
		} else {
			const char* cstr = env->GetStringUTFChars(arg, NULL);
			argv[nargs] = const_cast<char*>(cstr);
			if (cstr == NULL)
				goto cleanup;  // exception already thrown
		}
		env->DeleteLocalRef(arg);
	}

	g_system = cpp_obj;
	assert(g_system);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
				"Entering scummvm_main with %d args", argc);
	res = scummvm_main(argc, argv);
	__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Exiting scummvm_main");
	g_system->quit();

cleanup:
	nargs--;
	for (int i = 0; i < nargs; ++i) {
		if (argv[i] == NULL)
			continue;
		jstring arg = (jstring)env->GetObjectArrayElement(args, nargs);
		if (arg == NULL)
			// Exception already thrown
			return res;
		env->ReleaseStringUTFChars(arg, argv[i]);
		env->DeleteLocalRef(arg);
	}

	return res;
}

#ifdef DYNAMIC_MODULES
void AndroidPluginProvider::addCustomDirectories(Common::FSList &dirs) const {
	OSystem_Android* g_system_android = (OSystem_Android*)g_system;
	g_system_android->addPluginDirectories(dirs);
}
#endif

static void ScummVM_enableZoning(JNIEnv* env, jobject self, jboolean enable) {
	OSystem_Android* cpp_obj = OSystem_Android::fromJavaObject(env, self);
	cpp_obj->enableZoning(enable);
}

static void ScummVM_setSurfaceSize(JNIEnv* env, jobject self,
								   jint width, jint height) {
	OSystem_Android* cpp_obj = OSystem_Android::fromJavaObject(env, self);
	cpp_obj->setSurfaceSize(width, height);	
}

const static JNINativeMethod gMethods[] = {
	{ "create", "(Landroid/content/res/AssetManager;)V",
	  (void*)ScummVM_create },
	{ "nativeDestroy", "()V", (void*)ScummVM_nativeDestroy },
	{ "scummVMMain", "([Ljava/lang/String;)I",
	  (void*)ScummVM_scummVMMain },
	{ "pushEvent", "(Lorg/inodes/gus/scummvm/Event;)V",
	  (void*)ScummVM_pushEvent },
	{ "audioMixCallback", "([B)V",
	  (void*)ScummVM_audioMixCallback },
	{ "setConfMan", "(Ljava/lang/String;I)V",
	  (void*)ScummVM_setConfManInt },
	{ "setConfMan", "(Ljava/lang/String;Ljava/lang/String;)V",
	  (void*)ScummVM_setConfManString },
	{ "enableZoning", "(Z)V",
	  (void*)ScummVM_enableZoning },
	{ "setSurfaceSize", "(II)V",
	  (void*)ScummVM_setSurfaceSize },
};

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* jvm, void* reserved) {
	cached_jvm = jvm;

	JNIEnv* env;
	if (jvm->GetEnv((void**)&env, JNI_VERSION_1_2))
		return JNI_ERR;

	jclass cls = env->FindClass("org/inodes/gus/scummvm/ScummVM");
	if (cls == NULL)
		return JNI_ERR;
	if (env->RegisterNatives(cls, gMethods, ARRAYSIZE(gMethods)) < 0)
		return JNI_ERR;

	FID_ScummVM_nativeScummVM = env->GetFieldID(cls, "nativeScummVM", "J");
	if (FID_ScummVM_nativeScummVM == NULL)
		return JNI_ERR;

	jclass event = env->FindClass("org/inodes/gus/scummvm/Event");
	if (event == NULL)
		return JNI_ERR;
	FID_Event_type = env->GetFieldID(event, "type", "I");
	if (FID_Event_type == NULL)
		return JNI_ERR;
	FID_Event_synthetic = env->GetFieldID(event, "synthetic", "Z");
	if (FID_Event_synthetic == NULL)
		return JNI_ERR;
	FID_Event_kbd_keycode = env->GetFieldID(event, "kbd_keycode", "I");
	if (FID_Event_kbd_keycode == NULL)
		return JNI_ERR;
	FID_Event_kbd_ascii = env->GetFieldID(event, "kbd_ascii", "I");
	if (FID_Event_kbd_ascii == NULL)
		return JNI_ERR;
	FID_Event_kbd_flags = env->GetFieldID(event, "kbd_flags", "I");
	if (FID_Event_kbd_flags == NULL)
		return JNI_ERR;
	FID_Event_mouse_x = env->GetFieldID(event, "mouse_x", "I");
	if (FID_Event_mouse_x == NULL)
		return JNI_ERR;
	FID_Event_mouse_y = env->GetFieldID(event, "mouse_y", "I");
	if (FID_Event_mouse_y == NULL)
		return JNI_ERR;
	FID_Event_mouse_relative = env->GetFieldID(event, "mouse_relative", "Z");
	if (FID_Event_mouse_relative == NULL)
		return JNI_ERR;

	cls = env->FindClass("java/lang/Object");
	if (cls == NULL)
		return JNI_ERR;
	MID_Object_wait = env->GetMethodID(cls, "wait", "()V");
	if (MID_Object_wait == NULL)
		return JNI_ERR;

	return JNI_VERSION_1_2;
}

#endif
