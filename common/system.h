/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "common/scummsys.h"
#include "common/savefile.h"


/**
 * Interface for ScummVM backends. If you want to port ScummVM to a system
 * which is not currently covered by any of our backends, this is the place
 * to start. ScummVM will create an instance of a subclass of this interface
 * and use it to interact with the system.
 *
 * In particular, a backend provides a video surface for ScummVM to draw in;
 * methods to create timers, to handle user input events,
 * control audio CD playback, and sound output.
 */
class OSystem {
public:
	/**
	 * Return a pointer to the (singleton) OSystem instance, i.e. the backend.
	 * This is not a proper singleton, since OSystem is an interface, not
	 * a real class.
	 */
	static OSystem *instance();

public:

	/** Virtual destructor */
	virtual ~OSystem() {}

	
	/** @name Graphics */
	//@{
	
	/**
	 * A feature in this context means an ability of the backend which can be
	 * either on or off. Examples include:
	 *  - fullscreen mode
	 *  - aspect ration correction
	 *  - a virtual keyboard for text entry (on PDAs)
	 */
	enum Feature {
		kFeatureFullscreenMode,
		kFeatureAspectRatioCorrection,
		kFeatureVirtualKeyboard,
		kFeatureAutoComputeDirtyRects
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


	/** @name Graphics */
	//@{

	struct GraphicsMode {
		const char *name;
		const char *description;
		int id;
	};

	/**
	 * Retrieve a list of all graphics modes supported by this backend.
	 * This can be both video modes as well as graphic filters/scalers;
	 * it is completely up to the backend maintainer to decide what is
	 * appropriate here and what not.
	 * The list is terminated by an all-zero entry.
	 * @return a list of supported graphics modes
	 */
	virtual const GraphicsMode *getSupportedGraphicsModes() const = 0;
	
	/**
	 * Switch to the specified graphics mode. If switching to the new mode
	 * failed, this method returns false.
	 * @param mode	the ID of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 */
	virtual bool setGraphicsMode(int mode) = 0;

	/**
	 * Switch to the graphics mode with the given name. If 'name' is unknown,
	 * or if switching to the new mode failed, this method returns false.
	 * @param mode	the name of the new graphics mode
	 * @return true if the switch was successful, false otherwise
	 */
	virtual bool setGraphicsMode(const char *name);

	/**
	 * Determine which graphics mode is currently active.
	 * @return the active graphics mode
	 */
	virtual int getGraphicsMode() const = 0;

	/**
	 * Set the size of the video bitmap. Typical sizes include:
	 *  - 320x200 (e.g. for most SCUMM games, and Simon)
	 *  - 320x240 (e.g. for FM-TOWN SCUMM games)
	 *  - 640x480 (e.g. for Curse of Monkey Island)
	 */
	virtual void initSize(uint w, uint h) = 0;

	/**
	 * Returns the currently set screen height.
	 * @see initSize
	 * @return the currently set screen height
	 */
	virtual int16 get_height() = 0;

	/**
	 * Returns the currently set screen width.
	 * @see initSize
	 * @return the currently set screen width
	 */
	virtual int16 get_width() = 0;

	/** Set colors of the palette. */
	virtual void set_palette(const byte *colors, uint start, uint num) = 0;

	/**
	 * Draw a bitmap to screen.
	 * The screen will not be updated to reflect the new bitmap, you have
	 * to call update_screen to do that.
	 * @see update_screen
	 */
	virtual void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) = 0;

	/**
	 * Moves the screen content by the offset specified via dx/dy.
	 * Only the region from x=0 till x=height-1 is affected.
	 * @param dx	the horizontal offset.
	 * @param dy	the vertical offset.
	 * @param height	the number of lines which in which the move will be done.
	 */
	virtual void move_screen(int dx, int dy, int height) = 0;

	/** Update the dirty areas of the screen. */
	virtual void update_screen() = 0;

	/**
	 * Set current shake position, a feature needed for some SCUMM screen effects.
	 * The effect causes the displayed graphics to be shifted upwards by the specified 
	 * (always positive) offset. The area at the bottom of the screen which is moved
	 * into view by this is filled by black. This does not cause any graphic data to
	 * be lost - that is, to restore the original view, the game engine only has to
	 * call this method again with a 0 offset. No calls to copy_rect are necessary.
	 * @param shakeOffset	the shake offset
	 */
	virtual void set_shake_pos(int shakeOffset) = 0;

	/** Convert the given RGB triplet into a NewGuiColor. A NewGuiColor can be
	 * 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation generates a 16 bit color value, in the 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @see colorToRGB
	 */
	virtual NewGuiColor RGBToColor(uint8 r, uint8 g, uint8 b) {
		return ((((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | ((b >> 3) & 0x1F));
	}

	/** Convert the given NewGuiColor into a RGB triplet. A NewGuiColor can be
	 * 8bit, 16bit or 32bit, depending on the target system. The default
	 * implementation takes a 16 bit color value and assumes it to be in 565 format
	 * (that is, 5 bits red, 6 bits green, 5 bits blue).
	 * @see RGBToColor
	 */
	virtual void colorToRGB(NewGuiColor color, uint8 &r, uint8 &g, uint8 &b) {
		r = (((color >> 11) & 0x1F) << 3);
		g = (((color >> 5) & 0x3F) << 2);
		b = ((color&0x1F) << 3);
	}

	//@}



	/** @name Mouse */
	//@{

	/** Show or hide the mouse cursor. */
	virtual bool show_mouse(bool visible) = 0;

	/** 
	 * Move ("warp) the mouse cursor to the specified position.
	 */
	virtual void warp_mouse(int x, int y) = 0;

	/** Set the bitmap used for drawing the cursor. */
	virtual void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) = 0;

	//@}

	/** @name Events and Time */
	//@{

	typedef int (*TimerProc)(int interval);

	/**
	 * The types of events backends can generate.
	 * @see Event
	 */
	enum EventCode {
		EVENT_KEYDOWN = 1,
		EVENT_KEYUP = 2,
		EVENT_MOUSEMOVE = 3,
		EVENT_LBUTTONDOWN = 4,
		EVENT_LBUTTONUP = 5,
		EVENT_RBUTTONDOWN = 6,
		EVENT_RBUTTONUP = 7,
		EVENT_WHEELUP = 8,
		EVENT_WHEELDOWN = 9,

		EVENT_QUIT = 10,
		EVENT_SCREEN_CHANGED = 11
	};

	enum {
		KBD_CTRL = 1,
		KBD_ALT = 2,
		KBD_SHIFT = 4
	};

	/**
	 * Data structure for an event. A pointer to an instance of Event
	 * can be passed to poll_event. 
	 */
	struct Event {
		EventCode event_code;
		struct {
			int keycode;
			uint16 ascii;
			byte flags;
		} kbd;
		struct {
			int x;
			int y;
		} mouse;
	};

	/** Get the number of milliseconds since the program was started. */
	virtual uint32 get_msecs() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delay_msecs(uint msecs) = 0;

	/** Set the timer callback. */
	virtual void set_timer(TimerProc callback, int interval) = 0;

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool poll_event(Event *event) = 0;

	//@}



	/** @name Sound */
	//@{
	typedef void (*SoundProc)(void *param, byte *buf, int len);

	/**
	 * Set the audio callback which is invoked whenever samples need to be generated.
	 * Currently, only the 16-bit signed mode is ever used for Simon & Scumm
	 * @param proc		pointer to the callback.
	 * @param param		an arbitrary parameter which is stored and passed to proc.
	 */
	virtual bool setSoundCallback(SoundProc proc, void *param) = 0;

	/**
	 * Remove any audio callback previously set via setSoundCallback, thus effectively
	 * stopping all audio output immediately.
	 * @see setSoundCallback
	 */
	virtual void clearSoundCallback() = 0;

	/**
	 * Determine the output sample rate. Audio data provided by the sound
	 * callback will be played using this rate.
	 * @return the output sample rate
	 */
	virtual int getOutputSampleRate() const = 0;
	//@}
		


	/**
	 * @name Audio CD
	 * The methods in this group deal with Audio CD playback.
	 */
	//@{

	/**
	 * Initialise the specified CD drive for audio playback.
	 * @return true if the CD drive was inited succesfully
	 */
	virtual bool openCD(int drive) = 0;

	/**
	 * Poll CD status.
	 * @return true if CD audio is playing
	 */
	virtual bool poll_cdrom() = 0;

	/**
	 * Start audio CD playback. 
	 * @param track			the track to play.
	 * @param num_loops		how often playback should be repeated (-1 = infinitely often).
	 * @param start_frame	the frame at which playback should start (75 frames = 1 second).
	 * @param duration		the number of frames to play.
	 */
	virtual void play_cdrom(int track, int num_loops, int start_frame, int duration) = 0;

	/**
	 * Stop audio CD playback.
	 */
	virtual void stop_cdrom() = 0;

	/**
	 * Update cdrom audio status.
	 */
	virtual void update_cdrom() = 0;

	//@} 



	/** @name Mutex handling */
	//@{

	typedef struct Mutex *MutexRef;

	/**
	 * Create a new mutex.
	 * @return the newly created mutex, or 0 if an error occured.
	 */
	virtual MutexRef create_mutex(void) = 0;

	/**
	 * Lock the given mutex.
	 * @param mutex	the mutex to lock.
	 */
	virtual void lock_mutex(MutexRef mutex) = 0;

	/**
	 * Unlock the given mutex.
	 * @param mutex	the mutex to unlock.
	 */
	virtual void unlock_mutex(MutexRef mutex) = 0;

	/**
	 * Delete the given mutex. Make sure the mutex is unlocked before you delete it.
	 * If you delete a locked mutex, the behavior is undefined, in particular, your
	 * program may crash.
	 * @param mutex	the mutex to delete.
	 */
	virtual void delete_mutex(MutexRef mutex) = 0;
	//@} 


	
	/** @name Overlay */
	//@{
	virtual void show_overlay() = 0;
	virtual void hide_overlay() = 0;
	virtual void clear_overlay() = 0;
	virtual void grab_overlay(NewGuiColor *buf, int pitch) = 0;
	virtual void copy_rect_overlay(const NewGuiColor *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual int16 get_overlay_height()	{ return get_height(); }
	virtual int16 get_overlay_width()	{ return get_width(); }
	//@} 



	/** @name Miscellaneous */
	//@{
	/** Quit (exit) the application. */
	virtual void quit() = 0;

	/**
	  * Set a window caption or any other comparable status display to the
	  * given value.
	  * @param caption	the window caption to use from now on
	 */
	virtual void setWindowCaption(const char *caption) {}
	
	/** Savefile management. */
	virtual SaveFileManager *get_savefile_manager() {
		return new SaveFileManager();
	}
	//@}
};

/** The global OSystem instance. Inited in main(). */
#define g_system	(OSystem::instance())


#endif 
