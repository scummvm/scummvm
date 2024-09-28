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

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/array.h" // For OSystem::getGlobalKeymaps()
#include "common/list.h" // For OSystem::getSupportedFormats()
#include "common/ustr.h"
#include "common/str-array.h" // For OSystem::updateStartSettings()
#include "common/hash-str.h" // For OSystem::updateStartSettings()
#include "common/path.h"
#include "common/log.h"
#include "graphics/pixelformat.h"
#include "graphics/mode.h"
#include "graphics/opengl/context.h"

namespace Audio {
class Mixer;
}

namespace Graphics {
struct Surface;
}

namespace GUI {
class GuiObject;
class OptionsContainerWidget;
}

namespace DLC {
class Store;
}

namespace Common {
class EventManager;
class MutexInternal;
struct Rect;
class SaveFileManager;
class SearchSet;
class String;
#if defined(USE_TASKBAR)
class TaskbarManager;
#endif
#if defined(USE_UPDATES)
class UpdateManager;
#endif
class TextToSpeechManager;
#if defined(USE_SYSDIALOGS)
class DialogManager;
#endif
class TimerManager;
class SeekableReadStream;
class WriteStream;
class HardwareInputSet;
class Keymap;
class KeymapperDefaultBindings;

typedef Array<Keymap *> KeymapArray;
}

/**
 * @defgroup common_system OSystem
 * @ingroup common
 *
 * @brief API related to OSystem - the main interface for ScummVM backends.
 *
 * @{
 */

class AudioCDManager;
class FilesystemFactory;
class PaletteManager;

/**
 * Structure describing time and date.
 *
 * This is a clone of struct @c tm from time.h.
 * We implement our own since not all systems provide time.h.
 * This is not a one-to-one replacement of the @c tm struct,
 * as only the fields that we need were added.
 *
 * @note For now, the members are named exactly as in struct @c tm to ease
 * the transition.
 */
struct TimeDate {
	int tm_sec;     /**< Seconds (0 - 60). */
	int tm_min;     /**< Minutes (0 - 59). */
	int tm_hour;    /**< Hours (0 - 23). */
	int tm_mday;    /**< Day of month (1 - 31). */
	int tm_mon;     /**< Month of year (0 - 11). */
	int tm_year;    /**< Year - 1900. */
	int tm_wday;    /**< Days since Sunday (0 - 6). */
};

/**
* Pixel mask modes for cursor graphics.
*/
enum CursorMaskValue {
	/** Overlapped pixel is unchanged */
	kCursorMaskTransparent = 0,

	/** Overlapped pixel is replaced with the cursor pixel. */
	kCursorMaskOpaque = 1,

	/** Fully inverts the overlapped pixel regardless of the cursor color data.
	 *  Backend must support kFeatureCursorMaskInvert for this mode. */
	kCursorMaskInvert = 2,

	/** Blends with mode (Destination AND Mask) XOR Color in palette modes, or
	 *  (Destination AND Mask) XOR (NOT Color) in RGB modes, which is equivalent to
	 *  Classic MacOS behavior for pixel colors other than black and white.
	 *  Backend must support kFeatureCursorMaskPaletteXorColorXnor for this mode. */
	kCursorMaskPaletteXorColorXnor = 3,
};

#if defined(USE_IMGUI)
typedef struct ImGuiCallbacks {
	void (*init)() = nullptr;
	void (*render)() = nullptr;
	void (*cleanup)() = nullptr;
} ImGuiCallbacks;
#endif

/**
 * Interface for ScummVM backends.
 *
 * If you want to port ScummVM to a system that is not currently
 * covered by any of our backends, this is the place to start.
 * ScummVM will create an instance of a subclass of this interface
 * and use it to interact with the system.
 *
 * In particular, a backend provides:
 *
 * - A video surface for ScummVM to draw in
 * - Methods to create timers
 * - Methods to handle user input events
 * - Control audio CD playback
 * - Sound output
 */
class OSystem : Common::NonCopyable {
protected:
	OSystem();
	virtual ~OSystem();

protected:
	/**
	 * @defgroup common_system_module Subsystem modules
	 * @ingroup common_system
	 * @{
	 *
	 * For backend authors only, the following pointers (= "slots") to various
	 * subsystem managers / factories / etc. can and should be set to
	 * a suitable instance of the respective type.
	 *
	 * For some of the slots, a default instance is set if your backend
	 * does not do so. For details, refer to the documentation of
	 * each slot.
	 *
	 * A backend may set up slot values in its initBackend() method,
	 * its constructor, or somewhere in between. But it must set a slot's value
	 * no later than in its initBackend() implementation, because
	 * OSystem::initBackend() will create any default instances if
	 * none have been set yet (and for other slots, will verify that
	 * one has been set; if not, an error may be generated).
	 */

	/**
	 * No default value is provided for _audiocdManager by OSystem.
	 * However, BaseBackend::initBackend() does set a default value
	 * if none has been set before.
	 *
	 * @note _audiocdManager is deleted by the OSystem destructor.
	 */
	AudioCDManager *_audiocdManager;

	/**
	 * No default value is provided for _eventManager by OSystem.
	 * However, EventsBaseBackend::initBackend() does set a default value
	 * if none has been set before.
	 *
	 * @note _eventManager is deleted by the OSystem destructor.
	 */
	Common::EventManager *_eventManager;

	/**
	 * No default value is provided for _timerManager by OSystem.
	 *
	 * @note _timerManager is deleted by the OSystem destructor.
	 */
	Common::TimerManager *_timerManager;

	/**
	 * No default value is provided for _savefileManager by OSystem.
	 *
	 * @note _savefileManager is deleted by the OSystem destructor.
	 */
	Common::SaveFileManager *_savefileManager;

#if defined(USE_TASKBAR)
	/**
	 * No default value is provided for _taskbarManager by OSystem.
	 *
	 * @note _taskbarManager is deleted by the OSystem destructor.
	 */
	Common::TaskbarManager *_taskbarManager;
#endif

#if defined(USE_UPDATES)
	/**
	 * No default value is provided for _updateManager by OSystem.
	 *
	 * @note _updateManager is deleted by the OSystem destructor.
	 */
	Common::UpdateManager *_updateManager;
#endif

	/**
	 * No default value is provided for _textToSpeechManager by OSystem.
	 *
	 * @note _textToSpeechManager is deleted by the OSystem destructor.
	 */
	Common::TextToSpeechManager *_textToSpeechManager;

#if defined(USE_SYSDIALOGS)
	/**
	 * No default value is provided for _dialogManager by OSystem.
	 *
	 * @note _dialogManager is deleted by the OSystem destructor.
	 */
	Common::DialogManager *_dialogManager;
#endif

	/**
	 * No default value is provided for _fsFactory by OSystem.
	 *
	 * Note that _fsFactory is typically required very early on,
	 * so it usually should be set in the backends constructor or shortly
	 * thereafter, and before initBackend() is called.
	 *
	 * @note _fsFactory is deleted by the OSystem destructor.
	 */
	FilesystemFactory *_fsFactory;

	/**
	 * Used by the DLC Manager implementation
	 */
	DLC::Store *_dlcStore;

	/**
	 * Used by the default clipboard implementation, for backends that don't
	 * implement clipboard support.
	 */
	Common::U32String _clipboard;

	/** Workaround for a bug in the osx_intel toolchain introduced by
	 * 014bef9eab9fb409cfb3ec66830e033e4aaa29a9. Adding this variable fixes it.
	 */
	bool _dummyUnused;
	 /** @} */
private:
	/**
	 * Indicate if initBackend() has been called.
	 */
	bool _backendInitialized;

	//@}

public:

	/**
	 * Destoy this OSystem instance.
	 */
	void destroy();

	/**
	 * Call this method once, after g_system is created.
	 */
	virtual void init() {}

	/**
	 * The following method is called once, from main.cpp, after all
	 * config data (including command line params etc.) is fully loaded.
	 *
	 * @note Subclasses should always invoke the implementation of their
	 *       parent class. They should do so near the end of their own
	 *       implementation.
	 */
	virtual void initBackend();

	/**
	 * Return false if initBackend() has not yet been called and true otherwise.
	 *
	 * Some functionalities such as mutexes cannot be used until the backend
	 * is initialized.
	 */
	bool backendInitialized() const { return _backendInitialized; }

	/**
	 * Allow the backend to perform engine-specific initialization.
	 *
	 * Called just before the engine is run.
	 */
	virtual void engineInit() { }

	/**
	 * Allow the backend to perform engine-specific deinitialization.
	 *
	 * Called after the engine finishes.
	 */
	virtual void engineDone() { }

	/**
	 * Identify a task that ScummVM can perform.
	 */
	enum Task {
		/**
		 * The local server is running, allowing connections from other devices to transfer files.
		 */
		kLocalServer,

		/**
		 * ScummVM is downloading games or synchronizing savegames from the cloud.
		 */
		kCloudDownload,

		/**
		 * ScummVM is downloading an icons or shaders pack.
		 */
		kDataPackDownload
	};

	/**
	 * Allow the backend to be notified when a task is started.
	 */
	virtual void taskStarted(Task) { }

	/**
	 * Allow the backend to be notified when a task is finished.
	 */
	virtual void taskFinished(Task) { }

	/**
	 * Allow the backend to customize the start settings, such as for example starting
	 * automatically a game under certain circumstances.
	 *
	 * This function is called after the command line parameters have been parsed,
	 * and thus the initial value of command and settings will reflect those.
	 *
	 * The default implementation checks if the executable name is "scummvm-auto"
	 * or if a file named "scummvm-autorun" sits next to it to enable autorun mode.
	 */
	virtual void updateStartSettings(const Common::String &executable, Common::String &command, Common::StringMap &startSettings, Common::StringArray& additionalArgs);

	/**
	 * @defgroup common_system_flags Feature flags
	 * @ingroup common_system
	 * @{
	 */

	/**
	 * A feature in this context means an ability of the backend which can be
	 * either on or off.
	 *
	 * Examples include:
	 * - Fullscreen mode
	 * - Aspect ration correction
	 * - Virtual keyboard for text entry (on PDAs)
	 *
	 * There is a difference between the *availability* of a feature
	 * that can be checked using hasFeature(), and its *state*.
	 * For example, the SDL backend *has* the kFeatureFullscreenMode,
	 * so hasFeature returns true for it. On the other hand,
	 * fullscreen mode may be active or not. This can be determined
	 * by checking the state using getFeatureState(). Finally, to
	 * switch between fullscreen and windowed mode, use setFeatureState().
	 *
	 * Some features, for example kFeatureClipboardSupport and kFeatureOpenUrl
	 * have no state and can only be used to check if the corresponding feature
	 * is available or not. Calling getFeatureState() or setFeatureState()
	 * for them is pointless.
	 */
	enum Feature {
		/**
		 * If supported, this feature flag can be used to switch between
		 * windowed and fullscreen mode.
		 */
		kFeatureFullscreenMode,

		/**
		 * Control aspect ratio correction.
		 *
		 * Aspect ratio correction is used for correcting games running at 320x200
		 * (i.e with an aspect ratio of 8:5), but which on their original hardware
		 * were displayed with the standard 4:3 ratio
		 * (which means that the original graphics used non-square pixels).
		 * When the backend supports this, then games running at
		 * 320x200 pixels should be scaled up to 320x240 pixels.
		 * For all other resolutions, ignore this feature flag.
		 *
		 * @note Backend implementors can find utility functions in common/scaler.h.
		 *       These functions can be used to implement aspect ratio correction.
		 *       You can use stretch200To240() can stretch a rect, including (very fast)
		 *       particular, interpolation, and works in-place.
		 */
		kFeatureAspectRatioCorrection,

		/**
		 * If supported, this flag can be used to switch between unfiltered and
		 * filtered graphics modes.
		 */
		kFeatureFilteringMode,

		/**
		 * Indicate if stretch modes are supported by the backend.
		 */
		kFeatureStretchMode,

		/**
		 * Determine whether a virtual keyboard is to be shown or not.
		 * This would mostly be implemented by backends for handheld devices,
		 * like PocketPC, Palms, Symbian phones like the P800, Zaurus, etc.
		 */
		kFeatureVirtualKeyboard,

		/**
		 * Backends supporting this feature allow specifying a custom palette
		 * for the cursor. The custom palette is used if the feature state
		 * is set to true by the client code using setFeatureState().
		 *
		 * It is currently used only by some Macintosh versions of Humongous
		 * Entertainment games. If the backend doesn't implement this feature
		 * then the engine switches to b/w versions of cursors.
		 * The GUI also relies on this feature for mouse cursors.
		 */
		kFeatureCursorPalette,

		/**
		 * Backends supporting this feature allow cursors to contain an alpha
		 * channel.
		 */
		kFeatureCursorAlpha,

		/**
		 * Backends supporting this feature allow specifying a mask for a
		 * cursor instead of a key color.
		 */
		kFeatureCursorMask,

		/**
		 * Backends supporting this feature allow cursor masks to use mode kCursorMaskInvert in mask values,
		 * which inverts the destination pixel.
		 */
		kFeatureCursorMaskInvert,

		/**
		 * Backends supporting this feature allow cursor masks to use mode kCursorMaskPaletteXorColorXnor in the mask values,
		 * which uses (Color XOR Destination) for CLUT8 blending and (Color XNOR Destination) for RGB blending.  This is
		 * equivalent to Classic MacOS behavior for pixel colors other than black and white.
		 */
		kFeatureCursorMaskPaletteXorColorXnor,

		/**
		 * A backend has this feature if its overlay pixel format has an alpha
		 * channel which offers at least 3-4 bits of accuracy (as opposed to
		 * just a single alpha bit).
		 *
		 * This feature has no associated state.
		 */
		kFeatureOverlaySupportsAlpha,

		/**
		 * Client code can set the state of this feature to true in order to
		 * iconify the application window.
		 */
		kFeatureIconifyWindow,

		/**
		 * This feature flag can be used to check if hardware-accelerated
		 * OpenGL is supported and can be used for 3D game rendering.
		 */
		kFeatureOpenGLForGame,

		/**
		 * This feature flag can be used to check if shaders are supported
		 * and can be used for 3D game rendering.
		 */
		kFeatureShadersForGame,

		/**
		 * If supported, this feature flag can be used to check if
		 * waiting for vertical sync before refreshing the screen to reduce
		 * tearing is enabled.
		 */
		kFeatureVSync,

		/**
		 * When a backend supports this feature, it guarantees the graphics
		 * context is not destroyed when switching to and from fullscreen.
		 *
		 * For OpenGL, that means the context is kept with all of its content:
		 * texture, programs, etc.
		 *
		 * For TinyGL, that means the backbuffer surface is kept.
		 */
		kFeatureFullscreenToggleKeepsContext,

		/**
		 * The presence of this feature indicates whether the displayLogFile()
		 * call is supported.
		 *
		 * This feature has no associated state.
		 */
		kFeatureDisplayLogFile,

		/**
		 * The presence of this feature indicates whether the system clipboard is
		 * available.
		 *
		 * If this feature is not present, the hasTextInClipboard(),
		 * getTextFromClipboard(), and setTextInClipboard() calls can still be used,
		 * however it should not be used in scenarios where the user is expected to
		 * copy data outside of the application.
		 *
		 * This feature has no associated state.
		 */
		kFeatureClipboardSupport,

		/**
		 * The presence of this feature indicates whether the openUrl()
		 * call is supported.
		 *
		 * This feature has no associated state.
		 */
		kFeatureOpenUrl,

		/**
		* Mouse emulation mode.
		*/
		kFeatureTouchpadMode,

		/**
		* Keyboard mouse and joystick mouse speed.
		*/
		kFeatureKbdMouseSpeed,

		/**
		* Change analog joystick deadzone.
		*/
		kFeatureJoystickDeadzone,

		/**
		* Scalers.
		*/
		kFeatureScalers,

		/**
		* Shaders.
		*/
		kFeatureShaders,

		/**
		* Support for downloading DLC packages.
		*/
		kFeatureDLC,

		/**
		* Support for using the native system file browser dialog
		* through the DialogManager.
		*/
		kFeatureSystemBrowserDialog,

		/**
		* For platforms that should not have a Quit button.
		*/
		kFeatureNoQuit,

		/**
		* The presence of this feature indicates that the backend uses a touchscreen.
		*
		* This feature has no associated state.
		*/
		kFeatureTouchscreen,

		/**
		* Arm-v8 requires NEON extensions, but before that, NEON was just
		* optional, so this signifies that the processor can use NEON.
		*/
		kFeatureCpuNEON,

		/**
		* For x86/x86_64 platforms that have SSE2 support
		*/
		kFeatureCpuSSE2,

		/**
		* For x86/x86_64 platforms that have SSE4.1 support
		*/
		kFeatureCpuSSE41,

		/**
		* For x86_64 platforms that have AVX2 support
		*/
		kFeatureCpuAVX2,

		/**
		* For PowerPC platforms that have the altivec standard as of 1999.
		* Covers a wide range of platforms, Apple Macs, XBox 360, PS3, and more
		*/
		kFeatureCpuAltivec,

		/**
		* Graphics code is able to rotate the screen
		*/
		kFeatureRotationMode,
	};

	/**
	 * Determine whether the backend supports the specified feature.
	 */
	virtual bool hasFeature(Feature f) { return false; }

	/**
	 * Enable or disable the specified feature.
	 *
	 * For example, this may be used to enable fullscreen mode
	 * or to deactivate aspect correction, etc.
	 */
	virtual void setFeatureState(Feature f, bool enable) {}

	/**
	 * Query the state of the specified feature.
	 *
	 * For example, test whether fullscreen mode is active or not.
	 */
	virtual bool getFeatureState(Feature f) { return false; }

	/** @} */



	/**
	 * @defgroup common_system_graphics Graphics
	 * @ingroup common_system
	 * @{
	 *
	 * The way graphics work in the OSystem class is meant to make
	 * it possible for game frontends to implement everything they need in
	 * an efficient manner. The downside of this is that it may be
	 * rather complicated for backend authors to fully understand and
	 * implement the semantics of the OSystem interface.
	 *
	 * The graphics visible to the user in the end are actually
	 * composed of three layers: the game graphics, the overlay
	 * graphics, and the mouse.
	 *
	 * First, there are the game graphics. The methods in this section
	 * deal with them exclusively. In particular, the size of the game
	 * graphics is defined by a call to initSize(), and
	 * copyRectToScreen() blits the data in the current pixel format
	 * into the game layer. Let W and H denote the width and height of
	 * the game graphics.
	 *
	 * Before the user sees these graphics, the backend may apply some
	 * transformations to it. For example, they may be scaled to better
	 * fit the visible screen or aspect ratio correction may be
	 * performed (see kFeatureAspectRatioCorrection). As a result of
	 * this, a pixel of the game graphics may occupy a region bigger
	 * than a single pixel on the screen. p_w and p_h are defined to be
	 * the width and, respectively, height of a game pixel on the screen.
	 *
	 * In addition, there is a horizontal and vertical "shake offset" (as
	 * defined by setShakePos) that are used in some games to provide a
	 * shaking effect. Note that shaking is applied to all three layers,
	 * i.e. also to the overlay and the mouse. The shake offsets are
	 * denoted by XS and YS.
	 *
	 * Putting this together, a pixel (x,y) of the game graphics is
	 * transformed to a rectangle of height p_h and width p_w
	 * appearing at position (p_w * (x + XS), p_hw * (y + YS)) on the real
	 * screen. In addition, a backend may choose to offset
	 * everything, e.g. to center the graphics on the screen.
	 *
	 * The next layer is the overlay. It is composed over the game
	 * graphics. Historically, the overlay size had always been a
	 * multiple of the game resolution. For example, if the game
	 * resolution was 320x200 and the user selected a 2x scaler and did
	 * not enable aspect ratio correction, it had a size of 640x400.
	 * An exception was the aspect ratio correction, which did allow
	 * for non multiples of the vertical resolution of the game screen.
	 * Currently, the overlay size does not need to have any relation to
	 * the game resolution though, for example the overlay resolution
	 * might be the same as the physical screen resolution.
	 * The overlay is forced to a 16 bpp mode right now.
	 *
	 * Finally, there is the mouse layer. This layer does not have to
	 * actually exist within the backend -- it all depends on how a
	 * backend chooses to implement mouse cursors. However, in the default
	 * SDL backend, it really is a separate layer. The mouse can
	 * have a palette of its own, if the backend supports it.
	 *
	 * Graphics do not need to be thread-safe and in fact most/all backends
	 * using OpenGL are not. So do *not* try to call any of these functions
	 * from a timer and/or audio callback (like readBuffer of AudioStreams).
	 */

	/**
	 * Description of a graphics mode.
	 */
	struct GraphicsMode {
		/**
		 * The name of the graphics mode.
		 *
		 * This name is matched when selecting a mode using the command line
		 * or using the config file. Examples: "1x", "advmame2x", "hq3x".
		 */
		const char *name;
		/**
		 * Human-readable description of the scaler.
		 *
		 * Examples: "Normal (no scaling)", "AdvMAME2x", "HQ3x".
		 */
		const char *description;
		/**
		 * ID of the graphics mode.
		 *
		 * How to use this is entirely up to the backend. This value is passed
		 * to the setGraphicsMode(int) method by the client code.
		 */
		int id;
	};

	/**
	 * Retrieve a list of all graphics modes supported by this backend.
	 *
	 * This can be both video modes as well as graphic filters/scalers.
	 * It is completely up to the backend maintainer to decide what is
	 * appropriate here and what not.
	 * The list is terminated by an all-zero entry.
	 *
	 * @return List of supported graphics modes.
	 */
	virtual const GraphicsMode *getSupportedGraphicsModes() const {
		static const GraphicsMode noGraphicsModes[] = {{"NONE", "Normal", 0}, {nullptr, nullptr, 0 }};
		return noGraphicsModes;
	}

	/**
	 * Return the ID of the 'default' graphics mode. What exactly this means
	 * is up to the backend. This mode is set by the client code when no user
	 * overrides are present (i.e. if no custom graphics mode is selected using
	 * the command line or a config file).
	 *
	 * @return ID of the 'default' graphics mode.
	 */
	virtual int getDefaultGraphicsMode() const { return 0; }

	enum GfxModeFlags {
		kGfxModeNoFlags = 0,         /**< No flags. */
		kGfxModeRender3d = (1 << 0)  /**< Indicate 3D hardware-accelerated in-game GFX. */
	};

	/**
	 * Switch to the specified graphics mode.
	 *
	 * If switching to the new mode fails, this method returns false.
	 *
	 * The flag 'kGfxModeRender3d' is optional. It allows to switch to 3D-only rendering mode.
	 * In this mode, the game engine is allowed to use OpenGL(ES) directly.
	 *
	 * @param mode	ID of the new graphics mode.
	 * @param flags	Flags for the new graphics mode.
	 *
	 * @return True if the switch was successful, false otherwise.
	 */
	virtual bool setGraphicsMode(int mode, uint flags = kGfxModeNoFlags) { return (mode == 0); }

	/**
	 * Switch to the graphics mode with the given name.
	 *
	 * If @p name is unknown, or if switching to the new mode fails, this method returns false.
	 *
	 * @param name Name of the new graphics mode.
	 *
	 * @return True if the switch was successful, false otherwise.
	 *
	 * @note This is implemented using the setGraphicsMode(int) method, as well
	 *       as getSupportedGraphicsModes() and getDefaultGraphicsMode().
	 *       In particular, backends do not have to overload this!
	 */
	bool setGraphicsMode(const char *name);

	/**
	 * Determine which graphics mode is currently active.
	 *
	 * @return ID of the active graphics mode.
	 */
	virtual int getGraphicsMode() const { return 0; }

#ifdef USE_RGB_COLOR
	/**
	 * Fetch the pixel format currently in use for screen rendering.
	 *
	 * This is not necessarily the native format for the system - if unset
	 * it defaults toCLUT8.  To set a different format, engines should set
	 * their preferred format using ::initGraphics().
	 *
	 * @return the active screen pixel format.
	 *
	 * @see Graphics::PixelFormat
	 */
	virtual Graphics::PixelFormat getScreenFormat() const = 0;

	/**
	 * Return a list of all pixel formats supported by the backend.
	 *
	 * The first item in the list must be directly supported by hardware
	 * and provide the largest color space of those formats with direct
	 * hardware support. It is also strongly recommended that remaining
	 * formats are placed in the order of descending preference for the
	 * backend to use.
	 *
	 * Example: a backend that supports 32-bit ABGR and 16-bit 555 BGR in hardware
	 * and provides conversion from equivalent RGB(A) modes should order its list
	 * in the following way:
	 *    1) Graphics::PixelFormat(4, 0, 0, 0, 0, 0, 8, 16, 24)
	 *    2) Graphics::PixelFormat(2, 3, 3, 3, 8, 0, 5, 10, 0)
	 *    3) Graphics::PixelFormat(4, 0, 0, 0, 0, 24, 16, 8, 0)
	 *    4) Graphics::PixelFormat(2, 3, 3, 3, 8, 10, 5, 0, 0)
	 *    5) Graphics::PixelFormat::createFormatCLUT8()
	 *
	 * @see Graphics::PixelFormat
	 *
	 * @note Backends supporting RGB color should accept game data in RGB color
	 *       order, even if hardware uses BGR or some other color order.
	 */
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const = 0;
#else
	inline Graphics::PixelFormat getScreenFormat() const {
		return Graphics::PixelFormat::createFormatCLUT8();
	}

	inline Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	}
#endif

	/**
	 * Retrieve a list of supported levels of anti-aliasing.
	 *
	 * Anti-aliasing only works when using one of the hardware-accelerated
	 * renderers. An empty list means anti-aliasing is not supported.
	 */
	virtual Common::Array<uint> getSupportedAntiAliasingLevels() const {
		return Common::Array<uint>();
	}

	/**
	 * Return the chosen OpenGL type.
	 *
	 * This function works even when a 2D graphical manager is active and
	 * let to select a proper renderer before changing mode.
	 * Implementation having feature kFeatureOpenGLForGame are expected to
	 * override this function.
	 *
	 * @return the OpenGL type of context which is supported.
	 */
	virtual OpenGL::ContextType getOpenGLType() const {
		return OpenGL::kContextNone;
	}

#if defined(USE_OPENGL) && defined(USE_GLAD)
	/**
	 * Query the address of an OpenGL function by name.
	 *
	 * This can only be used after a context has been created.
	 * Please note that this function can return valid addresses even if the
	 * OpenGL context does not support the function.
	 *
	 * @param name The name of the OpenGL function.
	 * @return A function pointer for the requested OpenGL function or
	 *         nullptr in case of failure.
	 */
	virtual void *getOpenGLProcAddress(const char *name) const { return nullptr; }
#endif

#if defined(USE_IMGUI)
	/**
	 * Set the init/render/cleanup callbacks for ImGui.
	 *
	 * This is only supported on select backends desktop oriented.
	 *
	 * @param callbacks Structure containing init/render/cleanup callbacks called on screen initialization, rendering and when deinitialized.
	 */
	virtual void setImGuiCallbacks(const ImGuiCallbacks &callbacks) {}
	/**
	 * Creates a new ImGui texture from a Graphics::Surface.
	 *
	 * @param image The Surface to convert.
	 * @param palette The palette to use if image is a paletized surface.
	 * @param palCount The number of entries in the palette.
	 *
	 * @return An ImGui texture identifier casted to void *.
	 */
	virtual void *getImGuiTexture(const Graphics::Surface &image, const byte *palette = nullptr, int palCount = 0) { return nullptr; }
	/**
	 * Frees an ImGui texture previously obtained by getImGuiTexture.
	 *
	 * @param texture The texture to free.
	 */
	virtual void freeImGuiTexture(void *texture) {}
#endif

	/**
	 * Load the specified shader.
	 *
	 * If loading the new shader fails, this method returns false.
	 *
	 * @param fileNode File node of the new shader.
	 *
	 * @return True if the switch was successful, false otherwise.
	 */
	virtual bool setShader(const Common::Path &fileName) { return false; }

	/**
	 * Retrieve a list of all stretch modes supported by this backend.
	 *
	 * It is completely up to the backend maintainer to decide what is
	 * appropriate here and what not.
	 * The list is terminated by an all-zero entry.
	 *
	 * @return a list of supported stretch modes
	 */
	virtual const GraphicsMode *getSupportedStretchModes() const {
		static const GraphicsMode noStretchModes[] = {{"NONE", "Normal", 0}, {nullptr, nullptr, 0 }};
		return noStretchModes;
	}

	/**
	 * Return the ID of the 'default' stretch mode.
	 *
	 * What exactly this means is up to the backend. This mode is set
	 * by the client code when no user overrides are present
	 * (i.e. if no custom stretch mode is selected using the command line or a config file).
	 *
	 * @return ID of the 'default' graphics mode.
	 */
	virtual int getDefaultStretchMode() const { return 0; }

	/**
	 * Switch to the specified stretch mode.
	 *
	 * If switching to the new mode fails, this method returns false.
	 *
	 * @param mode ID of the new graphics mode.
	 *
	 * @return True if the switch was successful, false otherwise.
	 */
	virtual bool setStretchMode(int mode) { return false; }

	/**
	 * Switch to the stretch mode with the given name.
	 *
	 * If @p name is unknown, or if switching to the new mode fails,
	 * this method returns false.
	 *
	 * @param name Name of the new stretch mode.
	 *
	 * @return True if the switch was successful, false otherwise.
	 *
	 * @note This is implemented using the setStretchMode(int) method, as well
	 *       as getSupportedStretchModes() and getDefaultStretchMode().
	 *       In particular, backends do not have to overload this!
	 */
	bool setStretchMode(const char *name);

	/**
	 * Determine which stretch mode is currently active.
	 *
	 * @return ID of the active stretch mode.
	 */
	virtual int getStretchMode() const { return 0; }

	/**
	 * Return the ID of the 'default' scaler.
	 *
	 * This mode is set by the client code when no user overrides
	 * are present (i.e. if no custom scaler is selected using the
	 * command line or a config file).
	 *
	 * @return ID of the 'default' scaler.
	 */
	virtual uint getDefaultScaler() const { return 0; }

	/**
	 * Return the 'default' scale factor.
	 *
	 * This mode is set by the client code when no user overrides
	 * are present (i.e. if no custom scaler is selected using the
	 * command line or a config file).
	 *
	 * @return The 'default' scale factor.
	 */
	virtual uint getDefaultScaleFactor() const { return 1; }

	/**
	 * Switch to the specified scaler.
	 *
	 * If switching to the new mode fails, this method returns false.
	 *
	 * @param mode ID of the new scaler.
	 * @param factor The scale factor to use
	 *
	 * @return True if the switch was successful, false otherwise.
	 */
	virtual bool setScaler(uint mode, int factor) { return false; }

	/**
	 * Switch to the scaler with the given name.
	 *
	 * If @p name is unknown, or if switching to the new mode fails,
	 * this method returns false.
	 *
	 * @param name Name of the new scaler.
	 * @param factor The scale factor to use
	 *
	 * @return True if the switch was successful, false otherwise.
	 */
	virtual bool setScaler(const char *name, int factor) { return false; }

	/**
	 * Determine which scaler is currently active.
	 *
	 * @return ID of the active stretch mode.
	 */
	virtual uint getScaler() const { return 0; }

	/**
	 * Determine which scale factor is currently active.
	 *
	 * @return The active scale factor.
	 */
	virtual uint getScaleFactor() const { return 1; }


	/**
	 * Set the size and color format of the virtual screen.
	 *
	 * Typical sizes include:
	 * - 320x200 (e.g. for most SCUMM games, and Simon)
	 * - 320x240 (e.g. for FM-TOWN SCUMM games)
	 * - 640x480 (e.g. for Curse of Monkey Island)
	 *
	 * This is the resolution for which the client code generates data.
	 * This is not necessarily equal to the actual display size. For example,
	 * a backend may magnify the graphics to fit on the screen (see also the
	 * GraphicsMode), stretch the data to perform aspect ratio correction,
	 * or shrink it to fit on small screens (in cell phones).
	 *
	 * Typical formats include:
	 * - CLUT8 (e.g. 256 color, for most games)
	 * - RGB555 (e.g. 16-bit color, for later SCUMM HE games)
	 * - RGB565 (e.g. 16-bit color, for Urban Runner)
	 *
	 * This is the pixel format for which the client code generates data.
	 * It is not necessarily equal to the hardware pixel format. For example,
	 * a backend may perform color lookup of 8-bit graphics before pushing
	 * a screen to hardware, or correct the ARGB color order.
	 *
	 * @param width		New virtual screen width.
	 * @param height	New virtual screen height.
	 * @param format	New virtual screen pixel format.
	 */
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = nullptr) = 0;

	/**
	 * Send a list of graphics modes to the backend so it can make a decision
	 * about the best way to set up the display hardware.
	 *
	 * Engines that switch between different virtual screen sizes during a game
	 * should call this function prior to any call to initSize. Engines that use
	 * only a single screen size do not need to call this function.
	 *
	 * @param modes List of graphics modes the engine will probably use.
	 */
	virtual void initSizeHint(const Graphics::ModeList &modes) {}

	/**
	 * Return an int value that is changed whenever any screen
	 * parameters (like the resolution) change, i.e. whenever
	 * EVENT_SCREEN_CHANGED is sent.
	 *
	 * You can track this value in your code to detect screen changes in case
	 * you do not have full control over the event loop(s)
	 * being used (like the GUI code).
	 *
	 * @return Integer that can be used to track screen changes.
	 *
	 * @note Backends that generate EVENT_SCREEN_CHANGED events must
	 *       overload this method appropriately.
	 */
	virtual int getScreenChangeID() const { return 0; }

	/**
	 * Begin a new GFX transaction, which is a sequence of GFX mode changes.
	 *
	 * The idea behind GFX transactions is to make it possible to activate
	 * several different GFX changes at once as a "batch" operation. For
	 * example, assume we are running in 320x200 with a 2x scaler (thus using
	 * 640x400 pixels in total). Now, we want to switch to 640x400 with the 1x
	 * scaler. Without transactions, we have to choose whether we want to first
	 * switch the scaler mode, or first to 640x400 mode. In either case,
	 * depending on the backend implementation, problems may occur.
	 * For example, the window might briefly switch to 320x200 or 1280x800.
	 * Using transactions, this can be avoided.
	 *
	 * @note Transaction support is optional, and the default implementations
	 *       of the relevant methods simply do nothing.
	 *
	 * @see endGFXTransaction
	 */
	virtual void beginGFXTransaction() {}

	/**
	 * This type can save the different errors which can happen while
	 * changing GFX config values inside GFX transactions.
	 *
	 * endGFXTransaction returns an ORed combination of the '*Failed' values
	 * if any problem occurs. It returns '0' on success.
	 *
	 * @see endGFXTransaction
	 */
	enum TransactionError {
		kTransactionSuccess = 0,                        /**< Everything fine (use EQUAL check for this one!) */
		kTransactionAspectRatioFailed = (1 << 0),       /**< Failed switching aspect ratio correction mode */
		kTransactionFullscreenFailed = (1 << 1),        /**< Failed switching fullscreen mode */
		kTransactionModeSwitchFailed = (1 << 2),        /**< Failed switching the GFX graphics mode (setGraphicsMode) */
		kTransactionSizeChangeFailed = (1 << 3),        /**< Failed switching the screen dimensions (initSize) */
		kTransactionFormatNotSupported = (1 << 4),      /**< Failed setting the color format */
		kTransactionFilteringFailed = (1 << 5),         /**< Failed setting the filtering mode */
		kTransactionStretchModeSwitchFailed = (1 << 6), /**< Failed setting the stretch mode */
		kTransactionShaderChangeFailed = (1 << 7),      /**< Failed setting the shader */
		kTransactionVSyncFailed = (1 << 8),             /**< Failed switching vsync mode */
	};

	/**
	 * End (and thereby commit) the current GFX transaction.
	 *
	 * @see beginGFXTransaction
	 * @see kTransactionError
	 *
	 * @return ORed combination of TransactionError values or 0 on success.
	 */
	virtual TransactionError endGFXTransaction() { return kTransactionSuccess; }

	/**
	 * Return the currently set virtual screen height.
	 *
	 * @see initSize
	 *
	 * @return Currently set virtual screen height.
	 */
	virtual int16 getHeight() = 0;

	/**
	 * Return the currently set virtual screen width.
	 *
	 * @see initSize
	 *
	 * @return Currently set virtual screen width.
	 */
	virtual int16 getWidth() = 0;

	/**
	 * Return the palette manager singleton.
	 *
	 * For more information, see @ref PaletteManager.
	 */
	virtual PaletteManager *getPaletteManager() = 0;

	/**
	 * Return the scale factor for HiDPI screens.
	 * Returns 1 for non-HiDPI screens, or if HiDPI display is not supported by the backend.
	 */
	virtual float getHiDPIScreenFactor() const { return 1.0f; }

	/**
	 * Blit a bitmap to the virtual screen.
	 *
	 * The real screen will not immediately be updated to reflect the changes.
	 * Client code must call updateScreen to ensure any changes are visible
	 * to the user. This can be used to optimize drawing and reduce flicker.
	 *
	 * If the current pixel format has one byte per pixel, the graphics data
	 * uses 8 bits per pixel, using the palette specified via setPalette.
	 * If more than one byte per pixel is in use, the graphics data uses the
	 * pixel format returned by getScreenFormat.
	 *
	 * @param buf    Buffer containing the graphics data source.
	 * @param pitch  Pitch of the buffer (number of bytes in a scanline).
	 * @param x      x coordinate of the destination rectangle.
	 * @param y      y coordinate of the destination rectangle.
	 * @param w      Width of the destination rectangle.
	 * @param h      Height of the destination rectangle.
	 *
	 * @note The specified destination rectangle must be completly contained
	 *       in the visible screen space, and must be non-empty. If not, a
	 *       backend may or may not perform clipping, trigger an assert, or
	 *       silently corrupt memory.
	 *
	 * @see updateScreen
	 * @see getScreenFormat
	 */
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Lock the active screen framebuffer and return a Graphics::Surface
	 * representing it.
	 *
	 * The caller can then perform arbitrary graphics transformations
	 * on the framebuffer (blitting, scrolling, etc.).
	 * Must be followed by a matching call to unlockScreen().
	 * Code that is calling this should make sure to only lock the framebuffer
	 * for the shortest time possible, as the whole system is potentially stalled
	 * while the lock is active.
	 *
	 * @return 0 if an error occurs. Otherwise, a surface with the pixel
	 *         format described by getScreenFormat is returned.
	 *
	 * The returned surface must *not* be deleted by the client code.
	 *
	 * @see getScreenFormat
	 */
	virtual Graphics::Surface *lockScreen() = 0;

	/**
	 * Unlock the screen framebuffer, and mark it as dirty, i.e. during the
	 * next updateScreen() call, the whole screen will be updated.
	 */
	virtual void unlockScreen() = 0;

	/**
	 * Fill the screen with the given color value.
	 */
	virtual void fillScreen(uint32 col) = 0;

	/**
	 * Fill the specified area of the screen with the given color value.
	 */
	virtual void fillScreen(const Common::Rect &r, uint32 col) = 0;

	/**
	 * Flush the whole screen, i.e. render the current content of the screen
	 * framebuffer to the display.
	 *
	 * This method may be called very often by engines. Backends are hence
	 * supposed to only perform any redrawing if it is necessary and otherwise
	 * return immediately. See
	 * <https://wiki.scummvm.org/index.php/HOWTO-Backends#updateScreen.28.29_method>
	 */
	virtual void updateScreen() = 0;

	/**
	 * Set current shake position, a feature needed for screen effects in some
	 * engines.
	 *
	 * The effect causes the displayed graphics to be shifted downwards and
	 * rightwards by the specified offsets (the offsets can be negative to shift
	 * upwards or leftwards). The area at the border of the screen which is
	 * moved into view by this  (for example at the bottom when moving
	 * upward) is filled with black. This does not cause any graphic data to
	 * be lost. To restore the original view, the game engine only has to call
	 * this method again with offset equal to zero. No calls to
	 * copyRectToScreen are necessary.
	 *
	 * @param shakeXOffset	Shake x offset.
	 * @param shakeYOffset	Shake y offset.
	 *
	 * @note This is currently used in the SCUMM, QUEEN, KYRA, SCI, DREAMWEB,
	 * SUPERNOVA, TEENAGENT, TOLTECS, ULTIMA, and PETKA engines.
	 */
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) = 0;

	/**
	 * Set the area of the screen that has the focus.
	 *
	 * For example, when a character is speaking, they will have the focus.
	 * This allows for pan-and-scan style views where the backend
	 * can follow the speaking character or area of interest on the screen.
	 *
	 * The backend is responsible for clipping the rectangle and deciding how best to
	 * zoom the screen to show any shape and size rectangle the engine provides.
	 *
	 * @param rect Rectangle on the screen to be focused on.
	 *
	 * @see clearFocusRectangle
	 */
	virtual void setFocusRectangle(const Common::Rect& rect) {}

	/**
	 * Clear the focus set by a call to setFocusRectangle().
	 *
	 * This allows the engine to clear the focus when no particular area
	 * of the screen has the focus.
	 *
	 * @see setFocusRectangle
	 */
	virtual void clearFocusRectangle() {}

	/**
	 * Instruct the backend to capture a screenshot of the current screen.
	 *
	 * The backend can persist it the way it considers appropriate.
	 */
	virtual void saveScreenshot() {}

	/** @} */


	/**
	 * @defgroup common_system_overlay Overlay
	 * @ingroup common_system
	 * @{
	 *
	 * To display dialogs atop the game graphics, backends
	 * must provide an overlay mode.
	 *
	 * The overlay is currently forced at 16 bpp.
	 *
	 * For 'coolness' we usually want to have an overlay that is blended over
	 * the game graphics. On backends that support alpha blending, this is
	 * no issue but on other systems this needs some trickery.
	 *
	 * Essentially, we fake (alpha) blending on these systems by copying the
	 * current game graphics into the overlay buffer when activating the overlay,
	 * and then manually compose whatever graphics we want to show in the overlay.
	 * This works because we assume the game to be "paused" whenever an overlay
	 * is active.
	 *
	 * @param inGame Whether the overlay is used to display GUI or in game images
	 *
	 */

	/** Activate the overlay mode. */
	virtual void showOverlay(bool inGUI = true) = 0;

	/** Deactivate the overlay mode. */
	virtual void hideOverlay() = 0;

	/** Return true if the overlay mode is activated, false otherwise. */
	virtual bool isOverlayVisible() const = 0;

	/**
	 * Return the pixel format description of the overlay.
	 *
	 * @see Graphics::PixelFormat
	 */
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;

	/**
	 * Reset the overlay.
	 *
	 * After calling this method while the overlay mode is active, the user
	 * should be seeing only the game graphics. How this is achieved depends
	 * on how the backend implements the overlay. It either sets all pixels of
	 * the overlay to be transparent (when alpha blending is used) or,
	 * in case of fake alpha blending, it might just put a copy of the
	 * current game graphics screen into the overlay.
	 */
	virtual void clearOverlay() = 0;

	/**
	 * Copy the content of the overlay into a surface provided by the
	 * caller.
	 *
	 * This is only used to implement fake alpha blending.
	 */
	virtual void grabOverlay(Graphics::Surface &surface) = 0;

	/**
	 * Blit a graphics buffer to the overlay.
	 *
	 * In a sense, this is the reverse of grabOverlay.
	 *
	 * @param buf    Buffer containing the graphics data source.
	 * @param pitch  Pitch of the buffer (number of bytes in a scanline).
	 * @param x      x coordinate of the destination rectangle.
	 * @param y      y coordinate of the destination rectangle.
	 * @param w      Width of the destination rectangle.
	 * @param h      Height of the destination rectangle.
	 *
	 * @see copyRectToScreen
	 * @see grabOverlay
	 */
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Return the height of the overlay.
	 *
	 * @see getHeight
	 */
	virtual int16 getOverlayHeight() = 0;

	/**
	 * Return the width of the overlay.
	 *
	 * @see getWidth
	 */
	virtual int16 getOverlayWidth() = 0;

	/** @} */



	/** @defgroup common_system_mouse Mouse
	 *  @ingroup common_system
	 *  @{
	 *
	 * This is the lower level implementation as provided by the
	 * backends. The engines should use the Graphics::CursorManager
	 * class instead of using this directly.
	 */


	/**
	 * Show or hide the mouse cursor.
	 *
	 * Currently, the backend is not required to immediately draw the
	 * mouse cursor on showMouse(true).
	 *
	 * @todo We might want to reconsider this fact,
	 * check Graphics::CursorManager::showMouse for some details about
	 * this.
	 *
	 * @see Graphics::CursorManager::showMouse
	 */
	virtual bool showMouse(bool visible) = 0;

	/**
	 * Lock or unlock the mouse cursor within the window.
	 *
	 */
	virtual bool lockMouse(bool lock) { return false; }

	/**
	 * Move ("warp") the mouse cursor to the specified position in virtual
	 * screen coordinates.
	 *
	 * @param x		New x position of the mouse.
	 * @param y		New y position of the mouse.
	 */
	virtual void warpMouse(int x, int y) = 0;

	/**
	 * Set the bitmap used for drawing the cursor.
	 *
	 * @param buf       Pixmap data to be used.
	 * @param w         Width of the mouse cursor.
	 * @param h         Height of the mouse cursor.
	 * @param hotspotX  Horizontal offset from the left side to the hotspot.
	 * @param hotspotY  Vertical offset from the top side to the hotspot.
	 * @param keycolor  Transparency color value. This should not exceed the maximum color value of the specified format.
	 *                  In case it does, the behavior is undefined. The backend might just error out or simply ignore the
	 *                  value. (The SDL backend will just assert to prevent abuse of this).
	 *                  This parameter does nothing if a mask is provided.
	 * @param dontScale Whether the cursor should never be scaled. An exception is high ppi displays, where the cursor
	 *                  might be too small to notice otherwise, these are allowed to scale the cursor anyway.
	 * @param format    Pointer to the pixel format that the cursor graphic uses (0 means CLUT8).
	 * @param mask      A mask containing values from the CursorMaskValue enum for each cursor pixel.
	 */
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = nullptr, const byte *mask = nullptr) = 0;

	/**
	 * Replace the specified range of cursor palette with new colors.
	 *
	 * The palette entries from 'start' till (start+num-1) will be replaced - so
	 * a full palette update is accomplished via start=0, num=256.
	 *
	 * Backends which implement this should have the kFeatureCursorPalette flag set.
	 *
	 * @see setPalette
	 * @see kFeatureCursorPalette
	 */
	virtual void setCursorPalette(const byte *colors, uint start, uint num) {}



	/**
	 * Get the system-configured double-click time interval.
	 * If the system doesn't support configuring double-click time, returns 0.
	 */
	virtual uint32 getDoubleClickTime() const { return 0; }

	/** @} */



	/** @defgroup common_system_event_time Events and Time
	 *  @ingroup common_system
	 *  @{
	 */

	/** Get the number of milliseconds since the program was started.
	 *
	 * @param skipRecord  Skip recording of this value by the event recorder.
	 *                    This might be needed particularly when we are in
	 *                    an on-screen GUI loop where the player can pause
	 *                    the recording.
	 */
	virtual uint32 getMillis(bool skipRecord = false) = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	/**
	 * Get the current time and date, in the local timezone.
	 *
	 * On many systems, this corresponds to the combination of time()
	 * and localtime().
	 */
	virtual void getTimeAndDate(TimeDate &td, bool skipRecord = false) const = 0;

	/**
	 * Return the timer manager singleton.
	 *
	 * For more information, see @ref TimerManager.
	 */
	virtual Common::TimerManager *getTimerManager();

	/**
	 * Return the event manager singleton.
	 *
	 * For more information, see @ref EventManager.
	 */
	inline Common::EventManager *getEventManager() {
		return _eventManager;
	}

	/**
	 * Register hardware inputs with keymapper.
	 *
	 * @return HardwareInputSet with all keys and recommended mappings.
	 *
	 * For more information, see @ref keymapper.
	 */
	virtual Common::HardwareInputSet *getHardwareInputSet() { return nullptr; }

	/**
	 * Return a platform-specific global keymap.
	 *
	 * @return Keymap with actions appropriate for the platform.
	 *
	 * The caller will use and delete the return object.
	 *
	 * For more information, see @ref keymapper.
	 */
	virtual Common::KeymapArray getGlobalKeymaps() { return Common::KeymapArray(); }

	/**
	 * Return platform-specific default keybindings.
	 *
	 * @return KeymapperDefaultBindings populated with keybindings.
	 *
	 * For more information, see @ref keymapper.
	 */
	virtual Common::KeymapperDefaultBindings *getKeymapperDefaultBindings() { return nullptr; }

	/** @} */


	/**
	 * @defgroup common_system_mutex Mutex handling
	 * @ingroup common_system
	 * @{
	 *
	 * Historically, the OSystem API used to have a method that allowed
	 * creating threads. Hence, mutex support was needed for thread syncing.
	 * To ease portability, we decided to remove the threading API.
	 * Instead, we now use timers (see setTimerCallback() and Common::Timer).
	 * But since those can be implemented using threads (and in fact, that is
	 * how our primary backend, the SDL one, does it on many systems), we
	 * still must do mutex syncing in our timer callbacks.
	 * In addition, the sound mixer uses a mutex in case the backend runs it
	 * from a dedicated thread (as the SDL backend does).
	 *
	 * Hence, backends that do not use threads to implement the timers can simply
	 * use dummy implementations for these methods.
	 */

	/**
	 * Create a new mutex.
	 *
	 * @return The newly created mutex, or 0 if an error occurred.
	 */
	virtual Common::MutexInternal *createMutex() = 0;

	/** @} */



	/** @defgroup common_system_sound Sound
	 *  @ingroup common_system
	 *  @{
	 */

	/**
	 * Return the audio mixer.
	 *
	 * For more information, see @ref Audio::Mixer.
	 */
	virtual Audio::Mixer *getMixer() = 0;

	/** @} */



	/** @defgroup common_system_audio Audio CD
	 *  @ingroup common_system
	 *  @{
	 */

	/**
	 * Return the audio CD manager.
	 *
	 * For more information, see @ref AudioCDManager.
	 */
	inline AudioCDManager *getAudioCDManager() {
		return _audiocdManager;
	}

	/** @} */



	/** @defgroup common_system_misc Miscellaneous
	 *  @ingroup common_system
	 *  @{
	 */

	/** Quit (exit) the application. */
	virtual void quit() = 0;

	/**
	 * Signal that a fatal error inside the client code has occurred.
	 *
	 * This should quit the application.
	 */
	virtual void fatalError();

	/**
	 * Set a window caption or any other comparable status display to the
	 * given value.
	 *
	 * @param caption The window caption to use.
	 */
	virtual void setWindowCaption(const Common::U32String &caption) {}

	/**
	 * Display a message in an 'on-screen display'.
	 *
	 * Displays a message in such a way that it is visible on or near the screen,
	 * for example in a transparent rectangle over the regular screen content,
	 * or in a message box beneath it.
	 *
	 * The message is expected to be provided in the current TranslationManager
	 * charset.
	 *
	 * @note There is a default implementation in BaseBackend that uses a
	 *       TimedMessageDialog to display the message. Hence, implementing
	 *       this is optional.
	 *
	 * @param msg The message to display on the screen.
	 */
	virtual void displayMessageOnOSD(const Common::U32String &msg) = 0;

	/**
	 * Display an icon that indicates background activity.
	 *
	 * The icon is displayed in an 'on-screen display'. It is visible above
	 * the regular screen content or near it.
	 *
	 * The caller keeps ownership of the icon. It is acceptable to free
	 * the surface just after the call.
	 *
	 * There is no preferred pixel format for the icon. The backend should
	 * convert its copy of the icon to an appropriate format.
	 *
	 * The caller must call this method again with a null pointer
	 * as a parameter to indicate the icon should no longer be displayed.
	 *
	 * @param icon The icon to display on the screen.
	 */
	virtual void displayActivityIconOnOSD(const Graphics::Surface *icon) = 0;
	/** @} */

	/**
	 * @addtogroup common_system_module
	 * @{
	 */

	 /**
	 * Return the SaveFileManager, which is used to store and load savestates
	 * and other modifiable persistent game data.
	 *
	 * For more information, see @ref SaveFileManager.
	 */
	virtual Common::SaveFileManager *getSavefileManager();

#if defined(USE_TASKBAR)
	/**
	 * Return the TaskbarManager, which is used to handle progress bars,
	 * icon overlay, tasks, and recent items list on the taskbar.
	 *
	 * @return The TaskbarManager for the current architecture.
	 */
	virtual Common::TaskbarManager *getTaskbarManager() {
		return _taskbarManager;
	}
#endif

#if defined(USE_UPDATES)
	/**
	 * Return the UpdateManager, which is used to handle auto-updating
	 * and updating of ScummVM in general.
	 *
	 * @return The UpdateManager for the current architecture.
	 */
	virtual Common::UpdateManager *getUpdateManager() {
		return _updateManager;
	}
#endif

	/**
	 * Return the TextToSpeechManager, used to handle text-to-speech features.
	 *
	 * @return The TextToSpeechManager for the current architecture.
	 */
	virtual Common::TextToSpeechManager *getTextToSpeechManager() {
		return _textToSpeechManager;
	}

#if defined(USE_SYSDIALOGS)
	/**
	 * Return the DialogManager, which is used to handle system dialogs.
	 *
	 * @return The DialogManager for the current architecture.
	 */
	virtual Common::DialogManager *getDialogManager() {
		return _dialogManager;
	}
#endif

	/**
	 * Return the DLC Store, used to implement DLC manager functions.
	 *
	 * @return The Store for the current architecture/distribution platform.
	 */
	virtual DLC::Store *getDLCStore() {
		return _dlcStore;
	}

	/**
	 * Return the FilesystemFactory object, depending on the current architecture.
	 *
	 * @return The FSNode factory for the current architecture.
	 */
	virtual FilesystemFactory *getFilesystemFactory();
	/** @} */

	/**
	 * @addtogroup common_system_misc
	 * @{
	 */

	/** Add system-specific Common::Archive objects to the given SearchSet.
	 * For example, on Unix, the directory corresponding to DATA_PATH (if set), or, on
	 * macOS, the 'Resource' dir in the app bundle.
	 *
	 * @todo Come up with a better name.
	 *
	 * @param s         SearchSet to which the system-specific dirs, if any, are added.
	 * @param priority	Priority with which those dirs are added.
	 */
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) {}

	/**
	 * Open the default config file for reading by returning a suitable
	 * ReadStream instance.
	 *
	 * It is the caller's responsibility to delete the stream after use.
	 */
	virtual Common::SeekableReadStream *createConfigReadStream();

	/**
	 * Open the default config file for writing by returning a suitable
	 * WriteStream instance.
	 *
	 * It is the callers responsibility to delete the stream after use.
	 *
	 * May return 0 to indicate that writing to the config file is not possible.
	 */
	virtual Common::WriteStream *createConfigWriteStream();

	/**
	 * Get the default file name (or even path) where the user configuration
	 * of ScummVM will be saved.
	 *
	 * Note that not all ports can use this.
	 */
	virtual Common::Path getDefaultConfigFileName();

	/**
	 * Get the default file name (or even path) where the scummvm.log
	 * will be saved.
	 *
	 * Note that not all ports can use this.
	 */
	virtual Common::Path getDefaultLogFileName() { return Common::Path(); }

	/**
	 * Register the default values for the settings the backend uses into the
	 * configuration manager.
	 *
	 * @param target    name of a config manager target
	 */
	virtual void registerDefaultSettings(const Common::String &target) const {}

	/**
	 * Return a GUI widget container for configuring the specified target options.
	 *
	 * The returned widget is shown in the Backend tab in the options dialog.
	 * Backends can build custom options dialogs.
	 *
	 * Backends that don't want to have a Backend tab in the options dialog
	 * can return nullptr.
	 *
	 * @param boss     the widget / dialog the returned widget is a child of
	 * @param name     the name the returned widget must use
	 * @param target   name of a config manager target
	 */
	virtual GUI::OptionsContainerWidget *buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const { return nullptr; }

	/**
	 * Return list of strings used for building help dialog
	 *
	 * The strings represented in triplets:
	 *   - Name of a tab (will be translated)
	 *   - ZIP pack name with images (optional)
	 *   - Text of the tab with Markdown formatting (also be translated)
	 *
	 * The string list is null-terminated.
	 */
	 virtual const char * const *buildHelpDialogData() { return nullptr; }

	/**
	 * Notify the backend that the settings editable from the game tab in the
	 * options dialog may have changed and that they need to be applied if
	 * necessary.
	 */
	virtual void applyBackendSettings() {}

	/**
	 * Log the given message.
	 *
	 * It is up to the backend where to log the different messages.
	 * The backend should aim at using a non-buffered output for it,
	 * so that no log data is lost in case of a crash.
	 *
	 * The default implementation outputs them on stdout/stderr.
	 *
	 * @param type    Type of the message.
	 * @param message The message itself.
	 */
	virtual void logMessage(LogMessageType::Type type, const char *message) = 0;

	/**
	 * Display a dialog box containing the given message.
	 *
	 * @param type    Type of the message.
	 * @param message The message itself.
	 */
	virtual void messageBox(LogMessageType::Type type, const char *message) {}

	/**
	 * Open the log file in a way that allows the user to review it,
	 * and possibly email it (or parts of it) to the ScummVM team,
	 * for example as part of a bug report.
	 *
	 * On a desktop operating system, this would typically launch
	 * some kind of an (external) text editor / viewer.
	 * On a phone, it can also cause a context switch to another
	 * application. Finally, on some ports, it might not be supported
	 * at all, and do nothing.
	 *
	 * The kFeatureDisplayLogFile feature flag can be used to
	 * test whether this call has been implemented by the active
	 * backend.
	 *
	 * @return True on success, false if an error occurred.
	 *
	 * @note An error might mean that the log file did not exist,
	 * or that the editor could not launch. However, a return value of true does
	 * not guarantee that the user will actually see the log file.
	 *
	 * @note It is up to the backend to ensure that the system is in a state
	 * that allows the user to actually see the displayed log files. This
	 * might for example require leaving fullscreen mode.
	 */
	virtual bool displayLogFile() { return false; }

	/**
	 * Check whether there is text available in the clipboard.
	 *
	 * The kFeatureClipboardSupport feature flag can be used to
	 * test whether this call has been implemented by the active
	 * backend.
	 *
	 * @return True if there is text in the clipboard, false otherwise.
	 */
	virtual bool hasTextInClipboard() { return !_clipboard.empty(); }

	/**
	 * Return clipboard contents as a string.
	 *
	 * The kFeatureClipboardSupport feature flag can be used to
	 * test whether this call has been implemented by the active
	 * backend.
	 *
	 * @return clipboard contents ("" if hasTextInClipboard() == false).
	 */
	virtual Common::U32String getTextFromClipboard() { return _clipboard; }

	/**
	 * Set the content of the clipboard to the given string.
	 *
	 * The kFeatureClipboardSupport feature flag can be used to
	 * test whether this call has been implemented by the active
	 * backend.
	 *
	 * @return True if the text has been properly set in the clipboard, false otherwise.
	 */
	virtual bool setTextInClipboard(const Common::U32String &text) { _clipboard = text; return true; }

	/**
	 * Open the given URL in the default browser (if available on the target
	 * system).
	 *
	 * @return True on success, false otherwise.
	 *
	 * @note It is up to the backend to ensure that the system is in a state
	 * that allows the user to actually see the web page. This might for
	 * example require leaving fullscreen mode.
	 *
	 * @param url The URL to open.
	 */
	virtual bool openUrl(const Common::String &url) {return false; }

	/**
	 * Return the language of the system.
	 *
	 * This returns the currently set language of the system on which
	 * ScummVM is run.
	 *
	 * The format is an ISO 639 language code, optionally followed by an ISO 3166-1 country code
	 * in the form language_country.
	 *
	 * For information about POSIX locales, see the following link:
	 * https://en.wikipedia.org/wiki/ISO_639
	 * https://en.wikipedia.org/wiki/ISO_3166-1
	 *
	 * The default implementation returns "en_US".
	 *
	 * @return Locale of the system.
	 */
	virtual Common::String getSystemLanguage() const;

	/**
	 * Return whether the connection is limited (if available on the target system).
	 *
	 * @return True if the connection is limited.
	 */
	virtual bool isConnectionLimited();

	//@}
};


/** The global OSystem instance. Initialized in main(). */
extern OSystem *g_system;

/** @} */

#endif
