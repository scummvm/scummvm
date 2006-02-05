// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef DRIVER_H
#define DRIVER_H

#include "bits.h"
#include "vector3d.h"
#include "color.h"
#include "model.h"
#include "scene.h"
#include "colormap.h"
#include "font.h"
#include "primitives.h"

class Material;
class Bitmap;

class Driver {
public:
	Driver() { ; }
	virtual ~Driver() { ; }
	Driver(int screenW, int screenH, int screenBPP, bool fullscreen = false) {
		_screenWidth = screenW;
		_screenHeight = screenH;
		_screenBPP = screenBPP;
		_isFullscreen = fullscreen;
	}

	struct TextObjectHandle {
		uint16 *bitmapData;
		void *surface;
		int numTex;
		void *texIds;
		int width;
		int height;
	};

	virtual void toggleFullscreenMode() = 0;

	virtual bool isHardwareAccelerated() = 0;

	virtual void setupCamera(float fov, float nclip, float fclip, float roll) = 0;
	virtual void positionCamera(Vector3d pos, Vector3d interest) = 0;

	virtual void clearScreen() = 0;
	virtual void flipBuffer() = 0;

	virtual void startActorDraw(Vector3d pos, float yaw, float pitch, float roll) = 0;
	virtual void finishActorDraw() = 0;
	
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

	virtual void prepareSmushFrame(int width, int height, byte *bitmap) = 0;
	virtual void drawSmushFrame(int offsetX, int offsetY) = 0;


	/** @name Events and Time */
	//@{

	typedef int (*TimerProc)(int interval);

	/** Get the number of milliseconds since the program was started. */
	virtual uint32 getMillis() = 0;

	/** Delay/sleep for the specified amount of milliseconds. */
	virtual void delayMillis(uint msecs) = 0;

	/**
	 * Set the timer callback, a function which is periodically invoked by the
	 * driver. This can for example be done via a background thread.
	 * There is at most one active timer; if this method is called while there
	 * is already an active timer, then the new timer callback should replace
	 * the previous one. In particular, passing a callback pointer value of 0
	 * is legal and can be used to clear the current timer callback.
	 * @see Timer
	 * @note The implementation of this method must be 'atomic' in the sense
	 *       that when the method returns, the previously set callback must
	 *       not be in use anymore (in particular, if timers are implemented
	 *       via threads, then it must be ensured that the timer thread is
	 *       not using the old callback function anymore).
	 *
	 * @param callback	pointer to the callback. May be 0 to reset the timer
	 * @param interval	the interval (in milliseconds) between invocations
	 *                  of the callback
	 */
	virtual void setTimerCallback(TimerProc callback, int interval) = 0;

	//@}

	/** @name Sound */
	//@{
	typedef void (*SoundProc)(void *param, byte *buf, int len);

	/**
	 * Set the audio callback which is invoked whenever samples need to be generated.
	 * Currently, only the 16-bit signed mode is ever used for GF
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
	 * @note Client code other than the sound mixer should _not_ use this
	 *       method. Instead, call Mixer::getOutputRate()!
	 * @return the output sample rate
	 */
	virtual int getOutputSampleRate() const = 0;

	//@}

protected:
	int _screenWidth, _screenHeight, _screenBPP;
	bool _isFullscreen;
};

extern Driver *g_driver;

#endif
