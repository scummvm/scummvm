/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "common/sys.h"
#include "common/noncopyable.h"
#include "common/rect.h"

#include "graphics/pixelformat.h"

typedef uint16 OverlayColor;

namespace Audio {
	class Mixer;
}

namespace Graphics {
	struct Surface;
}

namespace Common {
	struct Event;
	class EventManager;
	class SaveFileManager;
	class SearchSet;
	class TimerManager;
	class SeekableReadStream;
	class WriteStream;
}

class FilesystemFactory;

/**
 * Interface for Residual backends.
 */
class OSystem : Common::NonCopyable {
protected:
	OSystem();
	virtual ~OSystem();

public:

	/**
	 * The following method is called once, from main.cpp, after all
	 * config data (including command line params etc.) are fully loaded.
	 *
	 * @note Subclasses should always invoke the implementation of their
	 *       parent class. They should do so near the end of their own
	 *       implementation.
	 */
	virtual void initBackend() { }

	/**
	 * Allows the backend to perform engine specific init.
	 * Called just before the engine is run.
	 */
	virtual void engineInit() { }

	/**
	 * Allows the backend to perform engine specific de-init.
	 * Called after the engine finishes.
	 */
	virtual void engineDone() { }

	/** @name Feature flags */
	//@{

	/**
	 * A feature in this context means an ability of the backend which can be
	 * either on or off. Examples include:
	 *  - fullscreen mode
	 */
	enum Feature {
		/**
		 * If your backend supports both a windowed and a fullscreen mode,
		 * then this feature flag can be used to switch between the two.
		 */
		kFeatureFullscreenMode,
		kFeatureFullscreenModeSwitch,
		kFeatureIconifyWindow,
		kFeatureOpenGL
	};

	/**
	 * Determine whether the backend supports the specified feature.
	 */
	virtual bool hasFeature(Feature f) { return false; }

	/**
	 * En-/disable the specified feature. For example, this may be used to
	 * enable fullscreen mode, or to deactivate aspect correction, etc.
	 */
	virtual void setFeatureState(Feature f, bool enable) {}

	/**
	 * Query the state of the specified feature. For example, test whether
	 * fullscreen mode is active or not.
	 */
	virtual bool getFeatureState(Feature f) { return false; }

	//@}



	/**
	 * @name Graphics
	 *
	 * The way graphics work in the class OSystem are meant to make
	 * it possible for game frontends to implement all they need in
	 * an efficient manner. The downside of this is that it may be
	 * rather complicated for backend authors to fully understand and
	 * implement the semantics of the OSystem interface.
	 */

	//@{

	/**
	 * Set the size of the screen.
	 *
	 * @param width			the new screen width
	 * @param height		the new screen height
	 * @param fullscreen	the new screen will be displayed in fullscreeen mode
	 */
	virtual byte *setupScreen(int screenW, int screenH, bool fullscreen, bool accel3d) = 0;

	/**
	 * Flush the whole screen, that is render the current content of the screen
	 * framebuffer to the display.
	 *
	 */
	virtual void updateScreen() = 0;

	//@}



	/**
	 * @name Overlay
	 * In order to be able to display dialogs atop the game graphics, backends
	 * must provide an overlay mode.
	 *
	 */
	//@{

	/** Activate the overlay mode. */
	virtual void showOverlay() = 0;

	/** Deactivate the overlay mode. */
	virtual void hideOverlay() = 0;

	/**
	 * Returns the pixel format description of the overlay.
	 * @see Graphics::PixelFormat
	 */
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;

	/**
	 * Reset the overlay.
	 *
	 * After calling this method while the overlay mode is active, the user
	 * should be seeing only the game graphics. How this is achieved depends
	 * on how the backend implements the overlay. Either it sets all pixels of
	 * the overlay to be transparent (when alpha blending is used).
	 */
	virtual void clearOverlay() = 0;

	/**
	 * Copy the content of the overlay into a buffer provided by the caller.
	 */
	virtual void grabOverlay(OverlayColor *buf, int pitch) = 0;

	/**
	 * Blit a graphics buffer to the overlay.
	 * In a sense, this is the reverse of grabOverlay.
	 *
	 * @note The pitch parameter actually contains the 'pixel pitch', i.e.,
	 * the number of pixels per scanline, and not as usual the number of bytes
	 * per scanline.
	 *
	 * @todo Change 'pitch' to be byte and not pixel based
	 *
	 * @param buf		the buffer containing the graphics data source
	 * @param pitch		the pixel pitch of the buffer (number of pixels in a scanline)
	 * @param x			the x coordinate of the destination rectangle
	 * @param y			the y coordinate of the destination rectangle
	 * @param w			the width of the destination rectangle
	 * @param h			the height of the destination rectangle
	 *
	 * @see copyRectToScreen
	 * @see grabOverlay
	 */
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Return the height of the overlay.
	 * @see getHeight
	 */
	virtual int16 getOverlayHeight() = 0;

	/**
	 * Return the width of the overlay.
	 * @see getWidth
	 */
	virtual int16 getOverlayWidth() = 0;

	//@}



	/** @name Mouse */
	//@{

	/** Show or hide the mouse cursor. */
	virtual bool showMouse(bool visible) = 0;

	/**
	 * Move ("warp") the mouse cursor to the specified position in virtual
	 * screen coordinates.
	 * @param x		the new x position of the mouse
	 * @param y		the new y position of the mouse
	 */
	virtual void warpMouse(int x, int y) = 0;

	//@}



	/** @name Events and Time */
	//@{

	/** Get the number of milliseconds since the program was started. */
	virtual uint32 getMillis() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	/**
	 * Get the current time and date, in the local timezone.
	 * Corresponds on many systems to the combination of time()
	 * and localtime().
	 */
	virtual void getTimeAndDate(struct tm &t) const = 0;

	/**
	 * Return the timer manager singleton. For more information, refer
	 * to the TimerManager documentation.
	 */
	virtual Common::TimerManager *getTimerManager() = 0;

	/**
	 * Return the event manager singleton. For more information, refer
	 * to the EventManager documentation.
	 */
	virtual Common::EventManager *getEventManager() = 0;

	//@}



	/**
	 * @name Mutex handling
	 * Historically, the OSystem API used to have a method which allowed
	 * creating threads. Hence mutex support was needed for thread syncing.
	 * To ease portability, though, we decided to remove the threading API.
	 * Instead, we now use timers (see setTimerCallback() and Common::Timer).
	 * But since those may be implemented using threads (and in fact, that's
	 * how our primary backend, the SDL one, does it on many systems), we
	 * still have to do mutex syncing in our timer callbacks.
	 * In addition, the sound mixer uses a mutex in case the backend runs it
	 * from a dedicated thread (as e.g. the SDL backend does).
	 *
	 * Hence backends which do not use threads to implement the timers simply
	 * can use dummy implementations for these methods.
	 */
	//@{

	typedef struct OpaqueMutex *MutexRef;

	/**
	 * Create a new mutex.
	 * @return the newly created mutex, or 0 if an error occured.
	 */
	virtual MutexRef createMutex() = 0;

	/**
	 * Lock the given mutex.
	 *
	 * @note ScummVM code assumes that the mutex implementation supports
	 * recursive locking. That is, a thread may lock a mutex twice w/o
	 * deadlocking. In case of a multilock, the mutex has to be unlocked
	 * as many times as it was locked befored it really becomes unlocked.
	 *
	 * @param mutex	the mutex to lock.
	 */
	virtual void lockMutex(MutexRef mutex) = 0;

	/**
	 * Unlock the given mutex.
	 * @param mutex	the mutex to unlock.
	 */
	virtual void unlockMutex(MutexRef mutex) = 0;

	/**
	 * Delete the given mutex. Make sure the mutex is unlocked before you delete it.
	 * If you delete a locked mutex, the behavior is undefined, in particular, your
	 * program may crash.
	 * @param mutex	the mutex to delete.
	 */
	virtual void deleteMutex(MutexRef mutex) = 0;

	//@}



	/** @name Sound */
	//@{

	/**
	 * Return the audio mixer. For more information, refer to the
	 * Audio::Mixer documentation.
	 */
	virtual Audio::Mixer *getMixer() = 0;

	//@}



	/**
	 * @name Audio CD
	 * The methods in this group deal with Audio CD playback.
	 * The default implementation simply does nothing.
	 * This is the lower level implementation as provided by the
	 * backends. The engines should use the Audio::AudioCDManager
	 * class instead of using it directly.
	 */
	//@{

	/**
	 * Initialise the specified CD drive for audio playback.
	 * @return true if the CD drive was inited succesfully
	 */
	virtual bool openCD(int drive);

	/**
	 * Poll CD status.
	 * @return true if CD audio is playing
	 */
	virtual bool pollCD();

	/**
	 * Start audio CD playback.
	 * @param track			the track to play.
	 * @param num_loops		how often playback should be repeated (-1 = infinitely often).
	 * @param start_frame	the frame at which playback should start (75 frames = 1 second).
	 * @param duration		the number of frames to play.
	 */
	virtual void playCD(int track, int num_loops, int start_frame, int duration) {}

	/**
	 * Stop audio CD playback.
	 */
	virtual void stopCD() {}

	/**
	 * Update cdrom audio status.
	 */
	virtual void updateCD() {}

	//@}



	/** @name Miscellaneous */
	//@{
	/** Quit (exit) the application. */
	virtual void quit() = 0;

	/**
	 * Set a window caption or any other comparable status display to the
	 * given value. The caption must be a pure ASCII string. Passing a
	 * non-ASCII string may lead to unexpected behavior, even crashes.
	 *
	 * In a future revision of this API, this may be changed to allowing
	 * UTF-8 or UTF-16 encoded data, or maybe ISO LATIN 1.
	 *
	 * @param caption	the window caption to use, as an ASCII string
	 */
	virtual void setWindowCaption(const char *caption) {}

	/**
	 * Return the SaveFileManager, used to store and load savestates
	 * and other modifiable persistent game data. For more information,
	 * refer to the SaveFileManager documentation.
	 */
	virtual Common::SaveFileManager *getSavefileManager() = 0;

	/**
	 * Returns the FilesystemFactory object, depending on the current architecture.
	 *
	 * @return the FSNode factory for the current architecture
	 */
	virtual FilesystemFactory *getFilesystemFactory() = 0;

	/**
	 * Add system specific Common::Archive objects to the given SearchSet.
	 * E.g. on Unix the dir corresponding to DATA_PATH (if set), or on
	 * Mac OS X the 'Resource' dir in the app bundle.
	 *
	 * @todo Come up with a better name. This one sucks.
	 *
	 * @param s		the SearchSet to which the system specific dirs, if any, are added
	 * @param priority	the priority with which those dirs are added
	 */
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) {}

	/**
	 * Open the default config file for reading, by returning a suitable
	 * ReadStream instance. It is the callers responsiblity to delete
	 * the stream after use.
	 */
	virtual Common::SeekableReadStream *createConfigReadStream() = 0;

	/**
	 * Open the default config file for writing, by returning a suitable
	 * WriteStream instance. It is the callers responsiblity to delete
	 * the stream after use.
	 *
	 * May return 0 to indicate that writing to config file is not possible.
	 */
	virtual Common::WriteStream *createConfigWriteStream() = 0;

	//@}
};


/** The global OSystem instance. Initialised in main(). */
extern OSystem *g_system;

#endif
