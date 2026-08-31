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

#ifndef NANCY_UI_CAMERA_H
#define NANCY_UI_CAMERA_H

#include "engines/nancy/enginedata.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;

namespace UI {

// The standalone camera introduced in Nancy14 (UICM), switched on by its photo
// album (CameraAction). While it is on, a viewfinder box sits at the centre of
// the viewport and the scene's hotspots and panning are suppressed; a click
// photographs every subject the box frames.
class Camera : public RenderObject {
public:
	Camera() : RenderObject(9) {}
	virtual ~Camera() {}

	void init() override;

	void activate();
	void deactivate();
	bool isActive() const { return _isActive; }

	void handleInput(NancyInput &input);

	// -- The camera roll, shared with the photo album --

	static uint numPictures();
	// Publishes the roll's size to the game variable UICM names, if it names one.
	static void setPictureCount(uint count);
	static void playSoundBlock(const RandomSoundBlock &block);

protected:
	// UICM's picture size, centred in the viewport.
	Common::Rect viewfinderScreenRect() const;
	void takePicture();

	const UICM *_cameraData = nullptr;
	Graphics::ManagedSurface _image;
	bool _isActive = false;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CAMERA_H
