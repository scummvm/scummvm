// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef DRIVER_RONIN_H
#define DRIVER_RONIN_H

#include "bits.h"
#include "vector3d.h"
#include "color.h"
#include "model.h"
#include "colormap.h"
#include "bitmap.h"
#include "driver.h"
#include "matrix_ops.h"
#include "texture_manager.h"

#include <ronin/soundcommon.h>

#define SOUND_BUFFER_SHIFT 3

class DriverRonin : public Driver, MatrixOps, TextureManager {
public:
	DriverRonin();
	virtual ~DriverRonin();

	void setupCamera(float fov, float nclip, float fclip, float roll);
	void positionCamera(Vector3d pos, Vector3d interest);

	void toggleFullscreenMode();
	void clearScreen(); 
	void flipBuffer();

	bool isHardwareAccelerated();

	void startActorDraw(Vector3d pos, float yaw, float pitch, float roll);
	void finishActorDraw();
	
	void set3DMode();

	void translateViewpoint(Vector3d pos, float pitch, float yaw, float roll);
	void translateViewpoint();

	void drawHierachyNode(const Model::HierNode *node);
	void drawModelFace(const Model::Face *face, float *vertices, float *vertNormals, float *textureVerts);

	void disableLights();
	void setupLight(Scene::Light *light, int lightId);

	void createMaterial(Material *material, const char *data, const CMap *cmap);
	void selectMaterial(const Material *material);
	void destroyMaterial(Material *material);

	void createBitmap(Bitmap *bitmap);
	void drawBitmap(const Bitmap *bitmap);
	void destroyBitmap(Bitmap *bitmap);

	void drawDepthBitmap(int x, int y, int w, int h, char *data);
	void drawBitmap();
	void dimScreen();
	void dimRegion(int x, int y, int w, int h, float level);

	Bitmap *getScreenshot(int w, int h);
	void storeDisplay();
	void copyStoredToDisplay();

	void drawEmergString(int x, int y, const char *text, const Color &fgColor);
	void loadEmergFont();
	TextObjectHandle *createTextBitmap(uint8 *bitmap, int width, int height, const Color &fgColor);
	void drawTextBitmap(int x, int y, TextObjectHandle *handle);
	void destroyTextBitmap(TextObjectHandle *handle);

	void drawRectangle(PrimitiveObject *primitive);
	void drawLine(PrimitiveObject *primitive);

	void prepareSmushFrame(int width, int height, byte *bitmap);
	void drawSmushFrame(int offsetX, int offsetY);

	char *getVideoDeviceName();

	const ControlDescriptor *listControls();
	int getNumControls();
	bool controlIsAxis(int num);
	float getControlAxis(int num);
	bool getControlState(int num);
	bool pollEvent(Event &event);
	uint32 getMillis();
	void delayMillis(uint msecs);
	void setTimerCallback(TimerProc callback, int interval);

	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	bool setSoundCallback(SoundProc proc, void *param);
	void clearSoundCallback();
	int getOutputSampleRate() const;

	void quit();

private:
	// Gfx
	int _polyCount;
	void *_smushTex;
	float _u_scale, _v_scale;
	// Event
	int _devpoll;
	uint32 _msecs;
	unsigned int _t0;
	uint32 _timer_duration, _timer_next_expiry;
	bool _timer_active;
	int (*_timer_callback) (int);
	class JoyState {
	public:
	  bool present;
	  unsigned short buttons;
	  unsigned char rtrigger;
	  unsigned char ltrigger;
	  unsigned char joyx;
	  unsigned char joyy;
	  unsigned char joyx2;
	  unsigned char joyy2;
	} _joy1_state, _joy2_state;
	class MouseState {
	public:
	  bool present;
	  unsigned char buttons;
	  short axis1;
	  short axis2;
	  short axis3;
	  short axis4;
	  short axis5;
	  short axis6;
	  short axis7;
	  short axis8;
	} _mouse_state;
	class KeyboardState {
	public:
	  bool present;
	  byte flags;
	  unsigned char shift;
	  unsigned char key[6];
	} _kbd_state;
	// Sound
	SoundProc _sound_proc;
	void *_sound_proc_param;
	int temp_sound_buffer[RING_BUFFER_SAMPLES>>SOUND_BUFFER_SHIFT];	

	void initEvent();
	void initGfx();
	void initSound();
	bool checkInput(struct mapledev *pad, Event &event);
	bool checkJoystick(struct mapledev *pad, JoyState &state, int base, Event &event);
	bool checkMouse(struct mapledev *pad, Event &event);
	bool checkKeyboard(struct mapledev *pad, Event &event);
	uint16 makeAscii(int keycode, int shift_state);
	void checkSound();
};

#endif
