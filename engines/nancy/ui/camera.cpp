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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/ui/camera.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace UI {

static const byte kNoPictureCountVariable = 0xff;

void Camera::init() {
	_cameraData = GetEngineData(UICM);
	assert(_cameraData);

	setTransparent(true);
	setVisible(false);
}

Common::Rect Camera::viewfinderScreenRect() const {
	const Common::Rect vp = NancySceneState.getViewport().getScreenPosition();
	const int16 w = MIN<int16>(_cameraData->viewRect.width(), vp.width());
	const int16 h = MIN<int16>(_cameraData->viewRect.height(), vp.height());

	Common::Rect box(w, h);
	box.moveTo(vp.left + (vp.width() - w) / 2, vp.top + (vp.height() - h) / 2);
	return box;
}

void Camera::activate() {
	if (_isActive) {
		return;
	}

	_isActive = true;

	// The camera outlives every scene, so its surface is built on first use.
	const Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	if (_drawSurface.w != vpBounds.width() || _drawSurface.h != vpBounds.height()) {
		_drawSurface.create(vpBounds.width(), vpBounds.height(),
			g_nancy->_graphics->getInputPixelFormat());

		if (!_image.w) {
			g_nancy->_resource->loadImage(_cameraData->overlayImageName, _image);
		}

		_image.setTransparentColor(_drawSurface.getTransparentColor());
	}

	moveTo(vpBounds);

	// The viewfinder is fixed, so it only needs drawing once.
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	Common::Rect box = viewfinderScreenRect();
	box.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _cameraData->viewRect, box);

	setVisible(true);
	_needsRedraw = true;
}

void Camera::deactivate() {
	if (!_isActive) {
		return;
	}

	_isActive = false;
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setVisible(false);
	_needsRedraw = true;
}

uint Camera::numPictures() {
	// The roll is shared with the Nancy13 cell phone camera ('CPIC' save chunk).
	const CellPhonePictureData *pd =
		(const CellPhonePictureData *)NancySceneState.getPuzzleData(CellPhonePictureData::getTag());
	return pd ? pd->pictures.size() : 0;
}

void Camera::setPictureCount(uint count) {
	auto *cameraData = GetEngineData(UICM);
	if (!cameraData || cameraData->pictureCount == kNoPictureCountVariable) {
		return;
	}

	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (table) {
		table->setSingleValue(cameraData->pictureCount, (int16)count);
	}
}

void Camera::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

void Camera::takePicture() {
	CellPhonePictureData *pd =
		(CellPhonePictureData *)NancySceneState.getPuzzleData(CellPhonePictureData::getTag());
	if (!pd) {
		return;
	}

	const Common::Rect grab = viewportScreenToBackground(viewfinderScreenRect());

	CapturedPicture picture;
	if (!captureViewportPicture(grab, picture)) {
		return;
	}

	// A subject's ID is the event flag saying it has been photographed; the album
	// clears it again once no picture holds that subject.
	const uint16 sceneID = NancySceneState.getSceneInfo().sceneID;
	for (const UICM::CameraSubject &subject : _cameraData->subjects) {
		if (subject.hotspot.frameID != sceneID || !grab.contains(subject.hotspot.coords)) {
			continue;
		}

		picture.subjects.push_back(subject.subjectID);
		NancySceneState.setEventFlag(subject.subjectID, g_nancy->_true);
		NancySceneState.setEventFlag(subject.flag);
	}

	pd->pictures.push_back(picture);
	setPictureCount(pd->pictures.size());
	playSoundBlock(_cameraData->shutterSound);

	deactivate();
}

void Camera::handleInput(NancyInput &input) {
	// Input outside the viewport is left alone, so the taskbar stays usable.
	if (!_isActive || !NancySceneState.getViewport().getScreenPosition().contains(input.mousePos)) {
		return;
	}

	// The pointer is blanked so only the viewfinder shows.
	g_nancy->_cursor->setCursorType(CursorManager::kNancy13Blank, true, false);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		takePicture();
	}

	input.eatMouseInput();
}

} // End of namespace UI
} // End of namespace Nancy
