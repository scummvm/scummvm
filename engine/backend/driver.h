/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BACKEND_DRIVER_H
#define BACKEND_DRIVER_H

#include "common/sys.h"
#include "common/vector3d.h"
#include "common/mutex.h"

#include "engine/color.h"
#include "engine/model.h"
#include "engine/scene.h"
#include "engine/colormap.h"
#include "engine/font.h"
#include "engine/primitives.h"
#include "engine/actor.h"
#include "engine/backend/default-timer.h"

class Material;
class Bitmap;
class Timer;

namespace Audio {
	class MixerImpl;
	class Mixer;
}

class Driver {
public:
	Driver() { ; }
	virtual ~Driver() { ; }

	struct TextObjectHandle {
		uint16 *bitmapData;
		void *surface;
		int numTex;
		void *texIds;
		int width;
		int height;
	};

	virtual void init() = 0;

	virtual void toggleFullscreenMode() = 0;

	virtual bool isHardwareAccelerated() = 0;

	virtual void setupIcon() = 0;

	virtual void setupCamera(float fov, float nclip, float fclip, float roll) = 0;
	virtual void positionCamera(Vector3d pos, Vector3d interest) = 0;

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	virtual void startActorDraw(Vector3d pos, float yaw, float pitch, float roll) = 0;
	virtual void finishActorDraw() = 0;
	virtual void setShadow(Shadow *shadow) = 0;
	virtual void drawShadowPlanes() = 0;
	virtual void setShadowMode() = 0;
	virtual void clearShadowMode() = 0;
	virtual void setShadowColor(byte r, byte g, byte b) = 0;

	virtual void set3DMode() = 0;

	virtual void translateViewpoint(Vector3d pos, float pitch, float yaw, float roll) = 0;
	virtual void translateViewpoint() = 0;

	virtual void drawHierachyNode(const Model::HierNode *node) = 0;
	virtual void drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts) = 0;

	virtual void disableLights() = 0;
	virtual void setupLight(Scene::Light *light, int lightId) = 0;

	virtual void createMaterial(Material *material, const char *data, const CMap *cmap) = 0;
	virtual void selectMaterial(const Material *material) = 0;
	virtual void destroyMaterial(Material *material) = 0;

	virtual void createBitmap(Bitmap *bitmap) = 0;
	virtual void drawBitmap(const Bitmap *bitmap) = 0;
	virtual void destroyBitmap(Bitmap *bitmap) = 0;

	virtual void drawDepthBitmap(int x, int y, int w, int h, char *data) = 0;

	virtual Bitmap *getScreenshot(int w, int h) = 0;
	virtual void storeDisplay() = 0;
	virtual void copyStoredToDisplay() = 0;
	virtual void dimScreen() = 0;
	virtual void dimRegion(int x, int y, int w, int h, float level) = 0;

	virtual void drawEmergString(int x, int y, const char *text, const Color &fgColor) = 0;
	virtual void loadEmergFont() = 0;
	virtual TextObjectHandle *createTextBitmap(uint8 *bitmap, int width, int height, const Color &fgColor) = 0;
	virtual void drawTextBitmap(int x, int y, TextObjectHandle *handle) = 0;
	virtual void destroyTextBitmap(TextObjectHandle *handle) = 0;

	virtual void drawRectangle(PrimitiveObject *primitive) = 0;
	virtual void drawLine(PrimitiveObject *primitive) = 0;
	virtual void drawPolygon(PrimitiveObject *primitive) = 0;

	virtual void prepareSmushFrame(int width, int height, byte *bitmap) = 0;
	virtual void drawSmushFrame(int offsetX, int offsetY) = 0;

	virtual char *getVideoDeviceName() = 0;

	/** @name Events and Time */
	//@{

	typedef unsigned int (*TimerProc)(unsigned int interval, void *param);

	/**
	 * The types of events backends may generate.
	 * @see Event
	 */
	enum EventType {
		/** A key was pressed, details in Event::kbd. */
		EVENT_KEYDOWN = 1,
		/** A key was released, details in Event::kbd. */
		EVENT_KEYUP = 2,

		EVENT_QUIT = 10,
		EVENT_REFRESH = 11
	};

	/**
	 * Keyboard modifier flags, used for Event::kbd::flags.
	 */
	enum {
		KBD_CTRL  = 1 << 0,
		KBD_ALT   = 1 << 1,
		KBD_SHIFT = 1 << 2
	};

	/**
	 * Data structure for an event. A pointer to an instance of Event
	 * can be passed to pollEvent.
	 */
	struct Event {
		/** The type of the event. */
		EventType type;
		/**
		  * Keyboard data; only valid for keyboard events (EVENT_KEYDOWN and
		  * EVENT_KEYUP). For all other event types, content is undefined.
		  */
		struct {
			/**
			 * Abstract control number (will be the same for any given key regardless
			 * of modifiers being held at the same time.
			 * For example, this is the same for both 'A' and Shift-'A'.
			 */
			int num;
			/**
			 * ASCII-value of the pressed key (if any).
			 * This depends on modifiers, i.e. pressing the 'A' key results in
			 * different values here depending on the status of shift, alt and
			 * caps lock.
			 */
			uint16 ascii;
			/**
			 * Status of the modifier keys. Bits are set in this for each
			 * pressed modifier
			 * @see KBD_CTRL, KBD_ALT, KBD_SHIFT
			 */
			byte flags;
		} kbd;
	};

	/**
	 *
	 */
	struct ControlDescriptor {
		char *name;
		int key;
	};

	/**
	 * Get a list of all named controls supported by the driver
	 */
	virtual const ControlDescriptor *listControls() = 0;

	/**
	 * Get the largest control number used by the driver, plus 1
	 */
	virtual int getNumControls() = 0;

	/**
	 * Check whether a control is an axis control
	 */
	virtual bool controlIsAxis(int num) = 0;

	/**
	 * Read the current value of an axis control (-1.0 .. 1.0)
	 */
	virtual float getControlAxis(int num) = 0;

	/**
	 * Read the current state of a non-axis control
	 */
	virtual bool getControlState(int num) = 0;

	/**
	 * Get the next event in the event queue.
	 * @param event	point to an Event struct, which will be filled with the event data.
	 * @return true if an event was retrieved.
	 */
	virtual bool pollEvent(Event &event) = 0;

	/** Get the number of milliseconds since the program was started. */
	virtual uint32 getMillis() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	virtual Common::TimerManager *getTimerManager() = 0;

	//@}

	/**
	 * @name Mutex handling
	 *
	 * Hence backends which do not use threads to implement the timers simply
	 * can use dummy implementations for these methods.
	 */
	//@{

	typedef Common::MutexRef	MutexRef;

	/**
	 * Create a new mutex.
	 * @return the newly created mutex, or 0 if an error occured.
	 */
	virtual MutexRef createMutex() = 0;

	/**
	 * Lock the given mutex.
	 *
	 * @note Code assumes that the mutex implementation supports
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
	virtual void setupMixer() = 0;

	virtual Audio::Mixer *getMixer() = 0;

	//@}

	/** @name Miscellaneous */
	//@{
	/** Quit (exit) the application. */
	virtual void quit() = 0;
	//@}

protected:
	int _screenWidth, _screenHeight, _screenBPP;
	bool _isFullscreen;
	Shadow *_currentShadowArray;
	unsigned char _shadowColorR;
	unsigned char _shadowColorG;
	unsigned char _shadowColorB;
};

extern Driver *g_driver;

#endif
