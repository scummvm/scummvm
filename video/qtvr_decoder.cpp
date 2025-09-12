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

//
// Partially based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "video/qt_decoder.h"
#include "video/qt_data.h"

#include "audio/audiostream.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/util.h"

#include "common/compression/unzip.h"

#include "graphics/cursorman.h"
#include "image/icocur.h"
#include "image/png.h"

// Video codecs
#include "image/codecs/codec.h"

namespace Video {

static const char * const MACGUI_DATA_BUNDLE = "macgui.dat";

////////////////////////////////////////////
// QuickTimeDecoder methods related to QTVR
////////////////////////////////////////////

static float readAppleFloatField(Common::SeekableReadStream *stream) {
	int16 a = stream->readSint16BE();
	uint16 b = stream->readUint16BE();

	float value = (float)a + (float)b / 65536.0f;

	return value;
}

QuickTimeDecoder::PanoSampleDesc::PanoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) : Common::QuickTimeParser::SampleDesc(parentTrack, codecTag) {
}

QuickTimeDecoder::PanoSampleDesc::~PanoSampleDesc() {
}

//
// Panorama Track Sample Description
//
// Source: https://developer.apple.com/library/archive/technotes/tn/tn1035.html
Common::QuickTimeParser::SampleDesc *QuickTimeDecoder::readPanoSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize) {

	PanoSampleDesc *entry = new PanoSampleDesc(track, format);

	entry->_majorVersion      = _fd->readSint16BE(); // must be zero, also observed to be 1
	entry->_minorVersion      = _fd->readSint16BE();
	entry->_sceneTrackID      = _fd->readSint32BE();
	entry->_loResSceneTrackID = _fd->readSint32BE();
	_fd->read(entry->_reserved3, 4 * 6);
	entry->_hotSpotTrackID    = _fd->readSint32BE();
	_fd->read(entry->_reserved4, 4 * 9);
	entry->_hPanStart         = readAppleFloatField(_fd);
	entry->_hPanEnd           = readAppleFloatField(_fd);
	entry->_vPanTop           = readAppleFloatField(_fd);
	entry->_vPanBottom        = readAppleFloatField(_fd);
	entry->_minimumZoom       = readAppleFloatField(_fd);
	entry->_maximumZoom       = readAppleFloatField(_fd);

	// info for the highest res version of scene track
	entry->_sceneSizeX        = _fd->readUint32BE();
	entry->_sceneSizeY        = _fd->readUint32BE();
	entry->_numFrames         = _fd->readUint32BE();
	entry->_reserved5         = _fd->readSint16BE();
	entry->_sceneNumFramesX   = _fd->readSint16BE();
	entry->_sceneNumFramesY   = _fd->readSint16BE();
	entry->_sceneColorDepth   = _fd->readSint16BE();

	// info for the highest rest version of hotSpot track
	entry->_hotSpotSizeX      = _fd->readSint32BE(); // pixel width of the hot spot panorama
	entry->_hotSpotSizeY      = _fd->readSint32BE(); // pixel height of the hot spot panorama
	entry->_reserved6         = _fd->readSint16BE();
	entry->_hotSpotNumFramesX = _fd->readSint16BE(); // diced frames wide
	entry->_hotSpotNumFramesY = _fd->readSint16BE(); // dices frame high
	entry->_hotSpotColorDepth = _fd->readSint16BE(); // must be 8

	if (entry->_minimumZoom == 0.0)
		entry->_minimumZoom = 2.0;

	if (entry->_maximumZoom == 0.0)
		entry->_maximumZoom = abs(entry->_vPanTop - entry->_vPanBottom);

	debugC(2, kDebugLevelGVideo, "    version: %d.%d sceneTrackID: %d loResSceneTrackID: %d hotSpotTrackID: %d",
		entry->_majorVersion, entry->_minorVersion, entry->_sceneTrackID, entry->_loResSceneTrackID, entry->_hotSpotTrackID);
	debugC(2, kDebugLevelGVideo, "    hpan: [%f - %f] vpan: [%f - %f] zoom: [%f - %f]",
		entry->_hPanStart, entry->_hPanEnd, entry->_vPanTop, entry->_vPanBottom, entry->_minimumZoom, entry->_maximumZoom);
	debugC(2, kDebugLevelGVideo, "    sceneDims: [%d x %d] frames: %d sceneFrm: [%d x %d] bpp: %d",
		entry->_sceneSizeX, entry->_sceneSizeY, entry->_numFrames, entry->_sceneNumFramesX,
		entry->_sceneNumFramesY, entry->_sceneColorDepth);
	debugC(2, kDebugLevelGVideo, "    hotspotDims: [%d x %d] hotspotFrm: [%d x %d] bpp: %d",
		entry->_hotSpotSizeX, entry->_hotSpotSizeY, entry->_hotSpotNumFramesX, entry->_hotSpotNumFramesY,
		entry->_hotSpotColorDepth);

		return entry;
}

void QuickTimeDecoder::closeQTVR() {
	delete _dataBundle;
	_dataBundle = nullptr;
	cleanupCursors();
}

void QuickTimeDecoder::renderHotspots(bool mode) {
	_renderHotspots = mode;

	((PanoTrackHandler *)getTrack(_panoTrack->targetTrack))->setDirty();
}

void QuickTimeDecoder::setQuality(float quality) {
	_quality = CLIP<float>(quality, 0.0f, 4.0f);

	// 4.0 Highest quality rendering. The rendered image is fully anti-aliased.
	//
	// 2.0 The rendered image is partially anti-aliased.
	//
	// 1.0 The rendered image is not anti-aliased.
	//
	// 0.0 Images are rendered at quality 1.0 when the user is interactively
	//     panning or zooming, and are automatically updated at quality
	//     4.0 during idle time when the user stops panning or zooming.
	//     All other updates are rendered at quality 4.0.

	PanoTrackHandler *track = ((PanoTrackHandler *)getTrack(_panoTrack->targetTrack));
	track->setDirty();
}

void QuickTimeDecoder::setWarpMode(int warpMode) {
	// Curretnly the warp mode 1 is not implemented correctly
	// So, forcing the warp mode 1 to warp mode 2
	if (warpMode == 1) warpMode = 2;

	_warpMode = CLIP(warpMode, 0, 2);

	// 2 Two-dimensional warping. This produces perspectively correct
	//   images from a panoramic source picture.
	//
	// 1 One-dimensional warping.
	//
	// 0 No warping. This reproduces the source panorama directly,
	//   without warping it at all.

	PanoTrackHandler *track = ((PanoTrackHandler *)getTrack(_panoTrack->targetTrack));
	track->setDirty();
}

void QuickTimeDecoder::setTransitionMode(Common::String mode) {
	if (mode.equalsIgnoreCase("swing")) {
		_transitionMode = kTransitionModeSwing;
	} else {
		_transitionMode = kTransitionModeNormal;
	}

	// normal The new view is imaged and displayed. No transition effect is
	//        used. The user sees a "cut" from the current view to the new
	//        view.
	//
	// swing  If the new view is in the current node, the view point moves
	//        smoothly from the current view to the new view, taking the
	//        shortest path if the panorama wraps around. The speed of the
	//        swing is controlled by the transitionSpeed property. If the new
	//        view is in a different node, the new view is imaged and
	//        displayed with a "normal" transition.
	((PanoTrackHandler *)getTrack(_panoTrack->targetTrack))->setDirty();
}

void QuickTimeDecoder::setTransitionSpeed(float speed) {
	_transitionSpeed = speed;

	// The TransitionSpeed is a floating point quantity that provides the slowest swing transition
	// at 1.0 and faster transitions at higher values. On mid-range computers, a rate of 4.0
	// performs well off CD-ROM.
	//
	// If the TransitionSpeed property value is set to a negative number, swing updates will act
	// the same as normal updates

	((PanoTrackHandler *)getTrack(_panoTrack->targetTrack))->setDirty();
}

Common::String QuickTimeDecoder::getUpdateMode() const {
	switch (_updateMode) {
	case kUpdateModeUpdateBoth:
		return "updateBoth";
	case kUpdateModeOffscreenOnly:
		return "offscreenOnly";
	case kUpdateModeFromOffscreen:
		return "fromOffscreen";
	case kUpdateModeDirectToScreen:
		return "directToScreen";
	case kUpdateModeNormal:
	default:
		return "normal";
	}
}

void QuickTimeDecoder::setUpdateMode(Common::String mode) {
	if (mode.equalsIgnoreCase("updateBoth"))
		_updateMode = kUpdateModeUpdateBoth;
	else if (mode.equalsIgnoreCase("offscreenOnly"))
		_updateMode = kUpdateModeOffscreenOnly;
	else if (mode.equalsIgnoreCase("fromOffscreen"))
		_updateMode = kUpdateModeFromOffscreen;
	else if (mode.equalsIgnoreCase("directToScreen"))
		_updateMode = kUpdateModeDirectToScreen;
	else
		_updateMode = kUpdateModeNormal;

	warning("STUB: Update mode set to '%s'", getUpdateMode().c_str());

	// normal         Images are computed and displayed directly onto the screen
	//                while interactively panning and zooming. Provides the fastest
	//                overall frame rate, but individual frame draw times may be
	//                relatively slow for high-quality images on lower performance
	//                systems. Programmatic updates are displayed into the offscreen
	//                buffer, and then onto the screen.
	//
	// update         Both Images are computed and displayed into the offscreen buffer,and
	//                then onto the screen. The use of the back buffer reduces the
	//                overall frame rate but provides the fastest imaging time for each
	//                frame.
	//
	// offscreenOnly  Images are computed and displayed into the offscreen buffer
	//                only, and are not copied to the screen. Useful for preparing for a
	//                screen refresh that will have to happen quickly.
	//
	// fromOffscreen  The offscreen buffer is copied directly to the screen. The offscreen
	//                buffer is refreshed only if it is out of date. The offscreen buffer is
	//                automatically updated if necessary to keep it in sync with the
	//                screen image.
	//
	// directToScreen Images are computed and displayed directly to the screen,
	//                without changing the offscreen buffer. Provides the fastest
	//                overall frame rate, but individual frame draw times may be
	//                relatively slow for high-quality images on lower performance
	//                systems.
	((PanoTrackHandler *)getTrack(_panoTrack->targetTrack))->setDirty();
}

void QuickTimeDecoder::setTargetSize(uint16 w, uint16 h) {
	if (!isVR())
		warning("QuickTimeDecoder::setTargetSize() called on non-VR movie");

	if (_qtvrType == QTVRType::PANORAMA) {
		_width = w;
		_height = h;

		setFOV(_fov);
	} else if (_qtvrType == QTVRType::OBJECT) {
		if (_width != w)
			_scaleFactorX *= Common::Rational(_width, w);

		if (_height != h)
			_scaleFactorY *= Common::Rational(_height, h);

		_width = w;
		_height = h;
	}
	// Set up the _hfov properly for the very first frame of the pano
	// After our setFOV will handle the _hfov
	_hfov = _fov * (float)_width / (float)_height;
}

void QuickTimeDecoder::setPanAngle(float angle) {
	PanoSampleDesc *desc = (PanoSampleDesc *)_panoTrack->sampleDescs[0];

	float panRange = abs(desc->_hPanEnd - desc->_hPanStart);
	angle = fmod(angle, panRange);

	if (desc->_hPanStart != desc->_hPanEnd && (desc->_hPanStart != 0.0 || desc->_hPanEnd != 360.0)) {
		if (angle < desc->_hPanStart + _hfov) {
			angle = desc->_hPanStart + _hfov;
		} else if (angle > desc->_hPanEnd - _hfov) {
			angle = desc->_hPanEnd - _hfov;
		}
	}

	if (_panAngle != angle) {
		PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);

		track->_currentPanAngle = _panAngle;
		_panAngle = angle;
		track->setDirty();
	}
}

void QuickTimeDecoder::setTiltAngle(float angle) {
	PanoSampleDesc *desc = (PanoSampleDesc *)_panoTrack->sampleDescs[0];

	if (angle < desc->_vPanBottom + _fov / 2) {
		angle = desc->_vPanBottom + _fov / 2;
	} else if (angle > desc->_vPanTop - _fov / 2) {
		angle = desc->_vPanTop - _fov / 2;
	}

	if (_tiltAngle != angle) {
		PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);

		track->_currentTiltAngle = _tiltAngle;
		_tiltAngle = angle;
		track->setDirty();
	}
}

bool QuickTimeDecoder::setFOV(float fov) {
	PanoSampleDesc *desc = (PanoSampleDesc *)_panoTrack->sampleDescs[0];
	bool success = true;

	if (fov == 0.0f && _zoomState == kZoomNone)	// This is reference to default FOV
		fov = _panoTrack->panoInfo.defZoom;

	if (fov <= desc->_minimumZoom) {
		fov = desc->_minimumZoom;
		success = false;
	} else if (fov >= desc->_maximumZoom) {
		fov = desc->_maximumZoom;
		success = false;
	}

	if (_fov != fov) {
		PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);

		debugC(3, kDebugLevelGVideo, "QuickTimeDecoder::setFOV: fov: %f (was %f)", fov, _fov);

		track->_currentFOV = _fov;
		_fov = fov;

		track->_currentHFOV = _hfov;
		_hfov = _fov * (float)_width / (float)_height;

		// We need to recalculate the pan angle and tilt angle to see if it has went
		// out of bound for the current value of FOV
		// This solves the distortion that we got sometimes when we zoom out at Tilt Angle != 0
		setPanAngle(_panAngle);
		setTiltAngle(_tiltAngle);
		track->setDirty();
	}

	return success;
}

Common::String QuickTimeDecoder::getCurrentNodeName() {
	if (_currentSample == -1)
		return Common::String();

	PanoTrackSample *sample = &_panoTrack->panoSamples[_currentSample];

	return sample->strTable.getString(sample->hdr.nameStrOffset);
}

void QuickTimeDecoder::updateAngles() {
	if (_qtvrType == QTVRType::OBJECT) {
		_panAngle = (float)getCurrentColumn() / (float)_nav.columns * 360.0;
		_tiltAngle = ((_nav.rows - 1) / 2.0 - (float)getCurrentRow()) / (float)(_nav.rows - 1) * 180.0;

		debugC(1, kDebugLevelGVideo, "QTVR: row: %d col: %d  (%d x %d) pan: %f tilt: %f", getCurrentRow(), getCurrentColumn(), _nav.rows, _nav.columns, getPanAngle(), getTiltAngle());
	}
}

void QuickTimeDecoder::setCurrentRow(int row) {
	VideoTrackHandler *track = (VideoTrackHandler *)_nextVideoTrack;

	int currentColumn = track->getCurFrame() % _nav.columns;
	int newFrame = row * _nav.columns + currentColumn;

	if (newFrame >= 0 && newFrame < track->getFrameCount()) {
		track->setCurFrame(newFrame);
	}
}

void QuickTimeDecoder::setCurrentColumn(int column) {
	VideoTrackHandler *track = (VideoTrackHandler *)_nextVideoTrack;

	int currentRow = track->getCurFrame() / _nav.columns;
	int newFrame = currentRow * _nav.columns + column;

	if (newFrame >= 0 && newFrame < track->getFrameCount()) {
		track->setCurFrame(newFrame);
	}
}

void QuickTimeDecoder::nudge(const Common::String &direction) {
	VideoTrackHandler *track = (VideoTrackHandler *)_nextVideoTrack;

	int curFrame = track->getCurFrame();
	int currentRow = curFrame / _nav.columns;
	int currentRowStart = currentRow * _nav.columns;
	int newFrame = curFrame;

	if (direction.equalsIgnoreCase("left")) {
		newFrame = (curFrame - 1 - currentRowStart) % _nav.columns + currentRowStart;
	} else if (direction.equalsIgnoreCase("right")) {
		newFrame = (curFrame + 1 - currentRowStart) % _nav.columns + currentRowStart;
	} else if (direction.equalsIgnoreCase("up")) {
		newFrame = curFrame - _nav.columns;
		if (newFrame < 0)
			return;
	} else if (direction.equalsIgnoreCase("down")) {
		newFrame = curFrame + _nav.columns;
		if (newFrame >= track->getFrameCount())
			return;
	} else {
		error("QuickTimeDecoder::nudge(): Invald direction: ('%s')!", direction.c_str());
	}

	track->setCurFrame(newFrame);
}

QuickTimeDecoder::NodeData QuickTimeDecoder::getNodeData(uint32 nodeID) {
	for (const auto &sample : _panoTrack->panoSamples) {
		if (sample.hdr.nodeID == nodeID) {
			return {
				nodeID,
				sample.hdr.defHPan,
				sample.hdr.defVPan,
				sample.hdr.defZoom,
				sample.hdr.minHPan,
				sample.hdr.minVPan,
				sample.hdr.maxHPan,
				sample.hdr.maxVPan,
				sample.hdr.minZoom,
				sample.strTable.getString(sample.hdr.nameStrOffset)};
		}
	}

	error("QuickTimeDecoder::getNodeData(): Node with nodeID %d not found!", nodeID);

	return {};
}

void QuickTimeDecoder::goToNode(uint32 nodeID) {
	int idx = -1;
	for (uint i = 0; i < _panoTrack->panoSamples.size(); i++) {
		if (_panoTrack->panoSamples[i].hdr.nodeID == nodeID) {
			idx = i;
			break;
		}
	}

	if (idx == -1) {
		warning("QuickTimeDecoder::goToNode(): Incorrect nodeID: %d (numNodes: %d)", nodeID, _panoTrack->panoSamples.size());
		idx = 0;
	}

	_currentSample = idx;

	debugC(3, kDebugLevelGVideo, "QuickTimeDecoder::goToNode(): Moving to nodeID: %d (index: %d)", nodeID, idx);

	setPanAngle(_panoTrack->panoSamples[_currentSample].hdr.defHPan);
	setTiltAngle(_panoTrack->panoSamples[_currentSample].hdr.defVPan);
	setFOV(_panoTrack->panoSamples[_currentSample].hdr.defZoom);

	((PanoTrackHandler *)getTrack(_panoTrack->targetTrack))->constructPanorama();
}

/////////////////////////
// PANO Track
////////////////////////

QuickTimeDecoder::PanoTrackHandler::PanoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent) : _decoder(decoder), _parent(parent) {
	if (decoder->_qtvrType != QTVRType::PANORAMA)
		error("QuickTimeDecoder::PanoTrackHandler: Incorrect track passed");

	_isPanoConstructed = false;

	_constructedPano = nullptr;
	_constructedHotspots = nullptr;
	_upscaledConstructedPano = nullptr;
	_projectedPano = nullptr;
	_planarProjection = nullptr;

	_dirty = true;

	_decoder->updateQTVRCursor(0, 0); // Initialize all things for cursor
}

QuickTimeDecoder::PanoTrackHandler::~PanoTrackHandler() {
	if (_isPanoConstructed) {
		_constructedPano->free();
		delete _constructedPano;

		_upscaledConstructedPano->free();
		delete _upscaledConstructedPano;

		if (_constructedHotspots) {
			_constructedHotspots->free();
			delete _constructedHotspots;
		}
	}

	if (_projectedPano) {
		_projectedPano->free();
		delete _projectedPano;

		_planarProjection->free();
		delete _planarProjection;
	}
}

uint16 QuickTimeDecoder::PanoTrackHandler::getWidth() const {
	return getScaledWidth().toInt();
}

uint16 QuickTimeDecoder::PanoTrackHandler::getHeight() const {
	return getScaledHeight().toInt();
}

Graphics::PixelFormat QuickTimeDecoder::PanoTrackHandler::getPixelFormat() const {
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];
	VideoTrackHandler *track = (VideoTrackHandler *)(_decoder->getTrack(_decoder->Common::QuickTimeParser::_tracks[desc->_sceneTrackID - 1]->targetTrack));

	return track->getPixelFormat();
}

Common::Rational QuickTimeDecoder::PanoTrackHandler::getScaledWidth() const {
	return Common::Rational(_parent->width) / _parent->scaleFactorX;
}

Common::Rational QuickTimeDecoder::PanoTrackHandler::getScaledHeight() const {
	return Common::Rational(_parent->height) / _parent->scaleFactorY;
}

void QuickTimeDecoder::PanoTrackHandler::swingTransitionHandler() {
	// The number of steps (the speed) of the transition is decided by the _transitionSpeed
	const int NUM_STEPS = 300 / _decoder->_transitionSpeed;

	// Calculate the step
	float stepFOV = (_decoder->_fov - _currentFOV) / NUM_STEPS;
	float stepHFOV = (_decoder->_hfov - _currentHFOV) / NUM_STEPS;

	float stepTiltAngle = (_decoder->_tiltAngle - _currentTiltAngle) / NUM_STEPS;

	float targetPanAngle = _decoder->_panAngle;
	float difference = ABS(_currentPanAngle - targetPanAngle);
	float stepPanAngle;

	// All of this is just because the panorama is supposed to take the smallest path
	if (targetPanAngle > _currentPanAngle) {
		if (difference <= 180) {
			stepPanAngle = difference / NUM_STEPS;
		} else {
			stepPanAngle = (- (360 - difference)) / NUM_STEPS;
		}
	} else {
		if (difference <= 180) {
			stepPanAngle = (- difference) / NUM_STEPS;
		} else {
			stepPanAngle = (360 - difference) / NUM_STEPS;
		}
	}

	int16 rectLeft = _decoder->_origin.x;
	int16 rectRight = _decoder->_origin.y;
	Common::Point mouse;
	int mw = _decoder->_width, mh = _decoder->_height;

	for (int i = 0; i < NUM_STEPS; i++) {
		projectPanorama(1,
						_currentFOV + i * stepFOV,
						_currentHFOV + i * stepHFOV,
						_currentTiltAngle + i * stepTiltAngle,
						_currentPanAngle + i * stepPanAngle);

		g_system->copyRectToScreen(_projectedPano->getPixels(),
									_projectedPano->pitch,
									rectLeft,
									rectRight,
									_projectedPano->w,
									_projectedPano->h);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT) {
				debugC(1, kDebugLevelGVideo, "EVENT_QUIT passed during the Swing Transition.");
				g_system->quit();
				return;
			}

			if (Common::isMouseEvent(event)) {
				mouse = event.mouse;
			}

			// Referenced from video.cpp in testbed engine
			if (mouse.x >= _decoder->_prevMouse.x &&
				mouse.x < _decoder->_prevMouse.x + mw &&
				mouse.y >= _decoder->_prevMouse.y &&
				mouse.y < _decoder->_prevMouse.y + mh) {

				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					_decoder->handleMouseMove(event.mouse.x - _decoder->_prevMouse.x, event.mouse.y - _decoder->_prevMouse.y);
					break;

				default:
					break;
				}
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	// This is so if we call swingTransitionHandler() twice
	// The second time there will be no transition
	_currentFOV = _decoder->_fov;
	_currentPanAngle = _decoder->_panAngle;
	_currentHFOV = _decoder->_hfov;
	_currentTiltAngle = _decoder->_tiltAngle;

	// Due to floating point errors, we may end a few degrees here and there
	// Make sure we reach the destination at the end of this loop
	// Also we have to go back to our original quality
	projectPanorama(3, _decoder->_fov, _decoder->_hfov, _decoder->_tiltAngle, _decoder->_panAngle);
}

const Graphics::Surface *QuickTimeDecoder::PanoTrackHandler::decodeNextFrame() {
	if (!_isPanoConstructed)
		return nullptr;

	// inject fake key/mouse events if button is held down
	if (_decoder->_isKeyDown)
		_decoder->handleKey(_decoder->_lastKey, true, true);

	if (_decoder->_isMouseButtonDown)
		_decoder->handleMouseButton(true, _decoder->_prevMouse.x, _decoder->_prevMouse.y, true);

	if (_dirty && _decoder->_transitionMode == kTransitionModeNormal) {
		float quality = _decoder->getQuality();
		float fov = _decoder->_fov;
		float hfov = _decoder->_hfov;
		float tiltAngle = _decoder->_tiltAngle;
		float panAngle = _decoder->_panAngle;

		switch ((int)quality) {
		case 1:
			projectPanorama(1, fov, hfov, tiltAngle, panAngle);
			break;

		case 2:
			projectPanorama(2, fov, hfov, tiltAngle, panAngle);
			break;

		case 4:
			projectPanorama(3, fov, hfov, tiltAngle, panAngle);
			break;

		case 0:
			if (_decoder->_isMouseButtonDown || _decoder->_isKeyDown) {
				projectPanorama(1, fov, hfov, tiltAngle, panAngle);
			} else {
				projectPanorama(3, fov, hfov, tiltAngle, panAngle);
			}
			break;

		default:
			projectPanorama(3, fov, hfov, tiltAngle, panAngle);
			break;
		}
	} else if (_decoder->_transitionMode == kTransitionModeSwing) {
		swingTransitionHandler();
	}

	if (_decoder->_cursorDirty) {
		_decoder->_cursorDirty = false;
		_decoder->updateQTVRCursor(_decoder->_cursorPos.x, _decoder->_cursorPos.y);
	}

	return _projectedPano;
}

const Graphics::Surface *QuickTimeDecoder::PanoTrackHandler::bufferNextFrame() {
	return nullptr;
}

Graphics::Surface *QuickTimeDecoder::PanoTrackHandler::constructMosaic(VideoTrackHandler *track, uint w, uint h, Common::String fname) {
	int16 framew = track->getWidth();
	int16 frameh = track->getHeight();

	Graphics::Surface *target = new Graphics::Surface();
	target->create(w * framew, h * frameh, track->getPixelFormat());

	debugC(1, kDebugLevelGVideo, "Pixel format: %s", track->getPixelFormat().toString().c_str());

	Common::Rect srcRect(0, 0, framew, frameh);

	for (uint y = 0; y < h; y++) {
		for (uint x = 0; x < w; x++) {
			const Graphics::Surface *frame = track->bufferNextFrame();

			if (!frame) {
				warning("QuickTimeDecoder::PanoTrackHandler::constructMosaic(): Out of frames at: %d, %d", x, y);
				break;
			}

			target->copyRectToSurface(*frame, x * framew, y * frameh, srcRect);
		}
	}

	if (ConfMan.getBool("dump_scripts")) {
		Common::Path path = Common::Path(fname);

		Common::DumpFile bitmapFile;
		if (!bitmapFile.open(path, true)) {
			warning("Cannot dump panorama into file '%s'", path.toString().c_str());
			target->free();
			delete target;
			return nullptr;
		}

		Image::writePNG(bitmapFile, *target, track->getPalette());
		bitmapFile.close();

		debug(0, "Dumped panorama %s of %d x %d", path.toString().c_str(), target->w, target->h);
	}

	return target;
}

void QuickTimeDecoder::PanoTrackHandler::initPanorama() {
	if (_decoder->_panoTrack->panoInfo.nodes.size() == 0) {
		// This is a single node panorama
		// We add one node to the list, so the rest of our code
		// is happy
		PanoTrackSample *sample = &_parent->panoSamples[0];

		_decoder->_panoTrack->panoInfo.nodes.resize(1);
		_decoder->_panoTrack->panoInfo.nodes[0].nodeID = sample->hdr.nodeID;
		_decoder->_panoTrack->panoInfo.nodes[0].timestamp = 0;

		_decoder->_panoTrack->panoInfo.defNodeID = sample->hdr.nodeID;
	}

	_decoder->goToNode(_decoder->_panoTrack->panoInfo.defNodeID);
}

Graphics::Surface *QuickTimeDecoder::PanoTrackHandler::upscalePanorama(Graphics::Surface *sourceSurface, int8 level) {
	// This algorithm (bilinear upscaling) takes quite some time to complete
	//
	// upscaling method
	//
	// for level 1 upscaling
	// a | b			a | avg(a, b) | b
	// -----	=>		-----------------
	// c | d			c | avg(c, d) | d
	//
	// for level 2 upscaling
	//                a              | avg(a, b)     | b
	// a | b          -----------------------------------------
	// -----    =>    avg(a, c) [e]	 | avg(e, f)     | avg(b, d) [f]
	// c | d		  -----------------------------------------
	//                c              | avg(c, d)     | d

	uint w = sourceSurface->w;
	uint h = sourceSurface->h;

	Graphics::Surface *target = new Graphics::Surface();

	if (level == 2) {
		target->create(w * 2, h * 2, sourceSurface->format);

		for (uint y = 0; y < h; ++y) {
			for (uint x = 0; x < w; ++x) {
				// For the row x==w and column y==h, wrap around the panorama
				// and average these pixels with row x==0 and column y==0 respectively
				uint x1 = (x + 1) % w;
				uint y1 = (y + 1) % h;

				// Couldn't find a better way to do this
				// Should I write a function in the Surface or PixelFormat class?
				int32 p00 = sourceSurface->getPixel(x, y);
				int32 p01 = sourceSurface->getPixel(x, y1);
				int32 p10 = sourceSurface->getPixel(x1, y);
				int32 p11 = sourceSurface->getPixel(x1, y1);

				uint8 a00, r00, g00, b00;
				uint8 a01, r01, g01, b01;
				uint8 a10, r10, g10, b10;
				uint8 a11, r11, g11, b11;

				sourceSurface->format.colorToARGB(p00, a00, r00, g00, b00);
				sourceSurface->format.colorToARGB(p01, a01, r01, g01, b01);
				sourceSurface->format.colorToARGB(p10, a10, r10, g10, b10);
				sourceSurface->format.colorToARGB(p11, a11, r11, g11, b11);

				int32 avgPixel00 = p00;
				int32 avgPixel01 = sourceSurface->format.ARGBToColor((a00 + a01) >> 1, (r00 + r01) >> 1, (g00 + g01) >> 1, (b00 + b01) >> 1);
				int32 avgPixel10 = sourceSurface->format.ARGBToColor((a00 + a10) >> 1, (r00 + r10) >> 1, (g00 + g10) >> 1, (b00 + b10) >> 1);
				int32 avgPixel11 =  sourceSurface->format.ARGBToColor(
					(a00 + a01 + a10 + a11) >> 2,
					(r00 + r01 + r10 + r11) >> 2,
					(g00 + g01 + g10 + g11) >> 2,
					(b00 + b01 + b10 + b11) >> 2);

				target->setPixel(x * 2, y * 2, avgPixel00);
				target->setPixel(x * 2, y * 2 + 1, avgPixel01);
				target->setPixel(x * 2 + 1, y * 2, avgPixel10);
				target->setPixel(x * 2 + 1, y * 2 + 1,avgPixel11);
			}
		}
	} else {
		target->create(w * 2, h, sourceSurface->format);

		for (uint y = 0; y < h; ++y) {
			for (uint x = 0; x < w; ++x) {
				uint x1 = (x + 1) % w;

				int32 p00 = sourceSurface->getPixel(x, y);
				int32 p01 = sourceSurface->getPixel(x1, y);

				uint8 a00, r00, g00, b00;
				uint8 a01, r01, g01, b01;
				sourceSurface->format.colorToARGB(p00, a00, r00, g00, b00);
				sourceSurface->format.colorToARGB(p01, a01, r01, g01, b01);

				int32 avgPixel00 = p00;
				int32 avgPixel01 = sourceSurface->format.ARGBToColor((a00 + a01) >> 1, (r00 + r01) >> 1, (g00 + g01) >> 1, (b00 + b01) >> 1);

				target->setPixel(x * 2, y, avgPixel00);
				target->setPixel(x * 2 + 1, y, avgPixel01);
			}
		}
	}

	return target;
}

void QuickTimeDecoder::PanoTrackHandler::boxAverage(Graphics::Surface *sourceSurface, uint8 scaleFactor) {
	uint16 h = sourceSurface->h;
	uint16 w = sourceSurface->w;

	// Apply box average if quality is higher than 1
	// Otherwise it'll be quicker to just copy the _planarProjection onto _projectedPano
	if (scaleFactor == 1) {
		_projectedPano->copyFrom(*sourceSurface);
		return;
	}

	uint8 scaleSquare = scaleFactor * scaleFactor;

	// Average out the pixels from the larger image
	for (uint16 y = 0; y < h / scaleFactor; y++) {
		for (uint16 x = 0; x < w / scaleFactor; x++) {
			uint16 avgA = 0, avgR = 0, avgG = 0, avgB = 0;

			for (uint8 row = 0; row < scaleFactor; row++) {
				for (uint8 column = 0; column < scaleFactor; column++) {
					uint8 a00, r00, g00, b00;
					uint32 pixel00 = sourceSurface->getPixel(MIN((x * scaleFactor + row), w - 1), MIN((y * scaleFactor + column), h - 1));
					_projectedPano->format.colorToARGB(pixel00, a00, r00, g00, b00);

					avgA += a00;
					avgR += r00;
					avgG += g00;
					avgB += b00;
				}
			}

			avgA /= scaleSquare;
			avgR /= scaleSquare;
			avgG /= scaleSquare;
			avgB /= scaleSquare;

			uint32 avgPixel = _projectedPano->format.ARGBToColor(avgA, avgR, avgG, avgB);
			_projectedPano->setPixel(x, y, avgPixel);
		}
	}
}

void QuickTimeDecoder::PanoTrackHandler::constructPanorama() {
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];
	PanoTrackSample *sample = &_parent->panoSamples[_decoder->_currentSample];

	if (_constructedPano) {
		_constructedPano->free();
		delete _constructedPano;

		if (_constructedHotspots) {
			_constructedHotspots->free();
			delete _constructedHotspots;
		}
	}

	debugC(1, kDebugLevelGVideo, "scene: %d (%d x %d) hotspots: %d (%d x %d)", desc->_sceneTrackID, desc->_sceneSizeX, desc->_sceneSizeY,
			desc->_hotSpotTrackID, desc->_hotSpotSizeX, desc->_hotSpotSizeY);

	debugC(1, kDebugLevelGVideo, "sceneNumFrames: %d x %d sceneColorDepth: %d", desc->_sceneNumFramesX, desc->_sceneNumFramesY, desc->_sceneColorDepth);

	debugC(1, kDebugLevelGVideo, "Node idx: %d", sample->hdr.nodeID);

	int nodeidx = -1;
	for (int i = 0; i < (int)_parent->panoInfo.nodes.size(); i++)
		if (_parent->panoInfo.nodes[i].nodeID == sample->hdr.nodeID) {
			nodeidx = i;
			break;
		}

	if (nodeidx == -1) {
		warning("constructPanorama(): Missing node %d in panoInfo", sample->hdr.nodeID);
		nodeidx = 0;
	}

	uint32 timestamp = _parent->panoInfo.nodes[nodeidx].timestamp;

	debugC(1, kDebugLevelGVideo, "Timestamp: %d", timestamp);

	VideoTrackHandler *track = (VideoTrackHandler *)(_decoder->getTrack(_decoder->Common::QuickTimeParser::_tracks[desc->_sceneTrackID - 1]->targetTrack));

	track->seek(Audio::Timestamp(0, timestamp, _decoder->_timeScale));

	_constructedPano = constructMosaic(track, desc->_sceneNumFramesX, desc->_sceneNumFramesY, "dumps/pano-full.png");

	// _upscaleLevel = 0 means _contructedPano has just been constructed, hasn't been upscaled yet
	// or that the upscaledConstructedPanorama has upscaled a different panorama, not the current constructedPano
	_upscaleLevel = 0;

	if (desc->_hotSpotTrackID) {
		track = (VideoTrackHandler *)(_decoder->getTrack(_decoder->Common::QuickTimeParser::_tracks[desc->_hotSpotTrackID - 1]->targetTrack));

		track->seek(Audio::Timestamp(0, timestamp, _decoder->_timeScale));

		_constructedHotspots = constructMosaic(track, desc->_hotSpotNumFramesX, desc->_hotSpotNumFramesY, "dumps/pano-hotspot.png");
	}

	_isPanoConstructed = true;
}

Common::Point QuickTimeDecoder::PanoTrackHandler::projectPoint(int16 mx, int16 my) {
	if (!_isPanoConstructed || !_constructedHotspots)
		return Common::Point(-1, -1);

	uint16 w = _decoder->getWidth(), h = _decoder->getHeight();
	uint16 hotWidth = _constructedHotspots->w, hotHeight = _constructedHotspots->h;

	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];

	float cornerVectors[3][3];

	float *topRightVector = cornerVectors[0];
	float *bottomRightVector = cornerVectors[1];
	float *mousePixelVector = cornerVectors[2];

	bottomRightVector[1] = tan(_decoder->_fov * M_PI / 360.0);
	bottomRightVector[0] = bottomRightVector[1] * (float)w / (float)h;
	bottomRightVector[2] = 1.0f;

	topRightVector[0] = bottomRightVector[0];
	topRightVector[1] = -bottomRightVector[1];
	topRightVector[2] = bottomRightVector[2];

	// Apply pitch (tilt) rotation
	float cosTilt = cos(-_decoder->_tiltAngle * M_PI / 180.0);
	float sinTilt = sin(-_decoder->_tiltAngle * M_PI / 180.0);

	for (int v = 0; v < 2; v++) {
		float y = cornerVectors[v][1];
		float z = cornerVectors[v][2];

		float newZ = z * cosTilt - y * sinTilt;
		float newY = y * cosTilt + z * sinTilt;

		cornerVectors[v][1] = newY;
		cornerVectors[v][2] = newZ;
	}

	float minTiltY = tan(desc->_vPanBottom * M_PI / 180.0f);
	float maxTiltY = tan(desc->_vPanTop * M_PI / 180.0f);

	int warpMode = _decoder->_warpMode;
	int hotX, hotY;

	// Compute the largest projected X value, which determines the horizontal angle range
	float maxProjectedX = 0.0f;

	// X coords are the same here so whichever has the lower Z coord will have the maximum projected X
	if (topRightVector[2] < bottomRightVector[2])
		maxProjectedX = topRightVector[0] / topRightVector[2];
	else
		maxProjectedX = bottomRightVector[0] / bottomRightVector[2];

	// Compute the side edge vector by interpolating between topRightVector and
	// bottomRightVector based on the mouse Y position
	float yRatio = (float)my / (float)h;
	mousePixelVector[0] = topRightVector[0] + yRatio * (bottomRightVector[0] - topRightVector[0]);
	mousePixelVector[1] = topRightVector[1] + yRatio * (bottomRightVector[1] - topRightVector[1]);
	mousePixelVector[2] = topRightVector[2] + yRatio * (bottomRightVector[2] - topRightVector[2]);

	float t, xCoord = 0, yawRatio = 0;
	float horizontalFovRadians = _decoder->_hfov * M_PI / 180.0f;
	float verticalFovRadians = _decoder->_fov * M_PI / 180.0f;
	float tiltAngleRadians = -_decoder->_tiltAngle * M_PI / 180.0f;

	if (warpMode == 0) {
		t = (float)mx / (float)(w) - 0.5f;
		xCoord = t * horizontalFovRadians / M_PI / 2.0f;
		yawRatio = xCoord;
	} else {
		t = ((float)(mx - w / 2) / (float)w * 2.0);
		if (warpMode == 1) {
			xCoord = t * maxProjectedX;
		} else if (warpMode == 2) {
			xCoord = t * mousePixelVector[0] / mousePixelVector[2];
		}
		yawRatio = atan(xCoord) / (2.0 * M_PI);
	}

	float angleT = (360.0f - _decoder->_panAngle) / 360.0f;

	// panorama is turned 90 degrees, width is height
	hotX = (1.0f - (angleT + yawRatio)) * (float)hotHeight;

	if (warpMode == 0) {
		float tiltFactor = tan(verticalFovRadians / 2.0f);
		float normalizedY = ((float)my / (float)(h - 1)) * 2.0f - 1.0f;  // Range [-1, 1]
		float projectedY = (normalizedY * tiltFactor) + tan(tiltAngleRadians);
		hotY = static_cast<int32>((projectedY + 1.0f) / 2.0f * hotWidth);
	} else {
		// To get the vertical coordinate, need to project the vector on to a unit cylinder.
		// To do that, compute the length of the XZ vector,
		float xzVectorLen = sqrt(xCoord * xCoord + 1.0f);

		float projectedY = xzVectorLen * mousePixelVector[1] / mousePixelVector[2];
		float normalizedYCoordinate = (projectedY - minTiltY) / (maxTiltY - minTiltY);
		hotY = (int)(normalizedYCoordinate * (float)hotWidth);
	}

	hotX = hotX % hotHeight;
	if (hotX < 0)
		hotX += hotHeight;

	if (hotY < 0)
		hotY = 0;
	else if (hotY > hotWidth)
		hotY = hotWidth;

	return Common::Point(hotX, hotY);
}

// It may get confusing what the three terms _quality, scaleVector and _upscaleLevel mean
// They are all related to the quality of the _projectedPanorama
// These are relevant for the functions upscalePanorama(), boxAverage() and projectPanorama()
//
// _quality is the attribute of the panorama that dictates how good the panorama looks
// This is taken directly from the original QTVR Xtra documentation by Apple
// (float) _quality can take three values:
//		1.0f; (higher
//		2.0f; means
//	 	4.0f; better)
// Excluding _quality = 0.0f (which is a combination of _quality = 1.0f and _quality = 4.0f)
// See setQuality() function to get a better idea
//
// This _quality determines what the scaleFactor value will be passed to projectPanorama()
// scaleFactor is the parameter to projectPanorama used for anti-aliasing the panorama
// We project a Surface object that has [scaleFactor] times the target height and width from _constructedPano
// Then we take a boxAverage() in a [scaleFactor * scaleFactor] box to make the panorama target sized
// This effectively removes aliasing from the image
// (uint8) scaleFactor can take three values:
//		1; (no-anti-aliasing)
//  	2; (some anti-aliasing)
// 		3; (better anti-aliasing) (For better Performance, we may remove this option)
//
// _quality also controls another factor that is the scale of original panorama (_constructedPano)
// Basically we upscalePanorama() at higher _quality values, exactly by how much is given by _upscaleLevel
// (uint8) _upscaleLevel can take two values:
//		1 (upscale to twice the width);
//		2 (upscale to twice the width and height);
//
// _quality = 1.0f => scaleFactor = 1; No upscaling;
// _quality = 2.0f => scaleFactor = 2; _upscaleLevel = 1;
// _quality = 4.0f => scaleFactor = 3; _upscaleLevel = 2;

void QuickTimeDecoder::PanoTrackHandler::projectPanorama(uint8 scaleFactor,
                                                         float fov,
                                                         float hfov,
                                                         float tiltAngle,
                                                         float panAngle) {
	if (!_isPanoConstructed)
		return;

	uint16 w = _decoder->getWidth() * scaleFactor, h = _decoder->getHeight() * scaleFactor;

	if (!_projectedPano) {
		if (w == 0 || h == 0)
			error("QuickTimeDecoder::PanoTrackHandler::projectPanorama(): setTargetSize() was not called");

		_projectedPano = new Graphics::Surface();
		_projectedPano->create(w / scaleFactor, h / scaleFactor, _constructedPano->format);
	}

	// The size of _planarProjection will keep changing in quality mode 0
	// It might (will definitely) cause some out of bound access if left as it is
	if (!_planarProjection || _planarProjection->w != w || _planarProjection->h != h) {
		// If _planarProjection holds some pixels, first need to free all the pixel data
		if (_planarProjection) {
			_planarProjection->free();
		}
		delete _planarProjection;

		_planarProjection = new Graphics::Surface();
		_planarProjection->create(w, h, _constructedPano->format);
	}

	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];

	float cornerVectors[2][3];

	float *topRightVector = cornerVectors[0];
	float *bottomRightVector = cornerVectors[1];

	// tangent of half of fov angle, is the bottom edge point
	// the negative of that is the top edge point
	bottomRightVector[1] = tan(fov * M_PI / 360.0);
	bottomRightVector[0] = bottomRightVector[1] * (float)w / (float)h;
	bottomRightVector[2] = 1.0f;

	topRightVector[0] = bottomRightVector[0];
	topRightVector[1] = -bottomRightVector[1];
	topRightVector[2] = bottomRightVector[2];

	// Apply pitch (tilt) rotation
	float cosTilt = cos(-tiltAngle * M_PI / 180.0);
	float sinTilt = sin(-tiltAngle * M_PI / 180.0);

	// Using trigonometry to account for the tilt
	for (int v = 0; v < 2; v++) {
		float y = cornerVectors[v][1];
		float z = cornerVectors[v][2];

		float newZ = z * cosTilt - y * sinTilt;
		float newY = y * cosTilt + z * sinTilt;

		cornerVectors[v][1] = newY;
		cornerVectors[v][2] = newZ;
	}

	float minTiltY = tan(desc->_vPanBottom * M_PI / 180.0f);
	float maxTiltY = tan(desc->_vPanTop * M_PI / 180.0f);

	// Compute the largest projected X value, which determines the horizontal angle range
	// Same with max projected y as well
	float maxProjectedX = 0.0f;

	// X coords are the same here so whichever has the lower Z coord will have the maximum projected X
	if (topRightVector[2] < bottomRightVector[2])
		maxProjectedX = topRightVector[0] / topRightVector[2];
	else
		maxProjectedX = bottomRightVector[0] / bottomRightVector[2];

	float minProjectedY = topRightVector[1] / topRightVector[2];
	float maxProjectedY = bottomRightVector[1] / bottomRightVector[2];

	float panRange = abs(desc->_hPanEnd - desc->_hPanStart);
	float angleT = fmod((panRange - panAngle) / panRange, 1.0f);
	if (angleT < 0.0f) {
		angleT += 1.0f;
	}

	Graphics::Surface *sourceSurface;

	switch (scaleFactor) {
	case 1:
		sourceSurface = _decoder->_renderHotspots ? _constructedHotspots : _constructedPano;
		break;

	case 2:
		if (!_upscaledConstructedPano || _upscaleLevel != 1) {
			// Avoid memory leakage if _upscaledConstructedPano already points to a Panorama
			if (_upscaledConstructedPano) {
				_upscaledConstructedPano->free();
				delete _upscaledConstructedPano;
			}
			_upscaleLevel = 1;
			_upscaledConstructedPano = upscalePanorama(_constructedPano, _upscaleLevel);
		}
		sourceSurface = _decoder->_renderHotspots ? _constructedHotspots : _upscaledConstructedPano;
		break;

	case 3:
		if (!_upscaledConstructedPano || _upscaleLevel != 2) {
			// Avoid memory leakage if _upscaledConstructedPano already points to a Panorama
			if (_upscaledConstructedPano) {
				_upscaledConstructedPano->free();
				delete _upscaledConstructedPano;
			}
			_upscaleLevel = 2;
			_upscaledConstructedPano = upscalePanorama(_constructedPano, _upscaleLevel);
		}
		sourceSurface = _decoder->_renderHotspots ? _constructedHotspots : _upscaledConstructedPano;
		break;

	default:
		sourceSurface = _decoder->_renderHotspots ? _constructedHotspots : _constructedPano;
		break;
	}

	int32 panoWidth = sourceSurface->h;
	int32 panoHeight = sourceSurface->w;

	// This angle offset will tell you exactly which portion of Cylindrical panorama
	// (when you construct a rectangular mosaic out of it) you're projecting on the plane surface
	uint16 angleOffset = static_cast<uint32>(angleT * panoWidth);

	const bool isWidthOdd = ((w % 2) == 1);
	uint16 halfWidthRoundedUp = (w + 1) / 2;
	float halfWidthFloat = (float)w * 0.5f;

	float verticalFovRadians = fov * M_PI / 180.0f;
	float horizontalFovRadians = hfov * M_PI / 180.0f;
	float tiltAngleRadians = tiltAngle * M_PI / 180.0f;

	float warpMode = _decoder->_warpMode;

	Common::Array<float> cylinderProjectionRanges;
	Common::Array<float> cylinderAngleOffsets;

	cylinderProjectionRanges.resize(halfWidthRoundedUp * 2);
	cylinderAngleOffsets.resize(halfWidthRoundedUp);

	if (warpMode == 0) {
		for (uint16 x = 0; x < halfWidthRoundedUp; x++) {
			float xFloat = (float) x;

			if (!isWidthOdd) {
				xFloat += 0.5f;
			}

			float normalizedX = (float)xFloat / (float)(w - 1);
			cylinderAngleOffsets[x] = normalizedX * horizontalFovRadians / M_PI / 2.0f;  // Scale to FOV
		}
	} else {
		for (uint16 x = 0; x < halfWidthRoundedUp; x++) {
			float xFloat = (float)x;

			// If width is odd, then the first column is on the pixel center
			// If width is even, then the first column is on the pixel boundary
			if (!isWidthOdd) {
				xFloat += 0.5f;
			}

			float t = xFloat / halfWidthFloat;
			float xCoord = t * maxProjectedX;

			float yCoords[2] = {minProjectedY, maxProjectedY};
			float length = sqrt(xCoord * xCoord + 1.0f);

			// Compute projection ranges
			for (int v = 0; v < 2; v++) {
				// Intersect (xCoord, yCoord[v], 1) with a 1-radius cylinder
				float newY = yCoords[v] / length;
				cylinderProjectionRanges[x * 2 + v] = (newY - minTiltY) / (maxTiltY - minTiltY);
			}

			cylinderAngleOffsets[x] = atan(xCoord) * 0.5f / M_PI;
		}
	}

	for (uint16 x = 0; x < halfWidthRoundedUp; x++) {
		int32 centerXImageCoord = static_cast<int32>(angleOffset);
		int32 edgeCoordOffset = static_cast<int32>(cylinderAngleOffsets[x] * panoWidth);

		int32 leftSourceXCoord = centerXImageCoord - edgeCoordOffset;
		int32 rightSourceXCoord = centerXImageCoord + edgeCoordOffset;

		int32 topSrcCoord = 0, bottomSrcCoord = 0;
		if (warpMode != 0) {
			topSrcCoord = static_cast<int32>(cylinderProjectionRanges[x * 2 + 0] * panoHeight);
			bottomSrcCoord = static_cast<int32>(cylinderProjectionRanges[x * 2 + 1] * panoHeight);

			if (topSrcCoord < 0) {
				topSrcCoord = 0;
			} else if (topSrcCoord >= panoHeight) {
				topSrcCoord = panoHeight - 1;
			}

			if (bottomSrcCoord >= panoHeight) {
				bottomSrcCoord = panoHeight - 1;
			}
		}

		// The descriptor has the original sceneSizeX and sceneSizeY of the movie
		// But in quality mode 3 we're increasing them to twice the original
		// This factor just make sure we're wrapping leftSourceXCoord and rightSourceXCoord
		// According to the sourceSurface's dimensions rather than the original's
		int factor = scaleFactor == 3 && !_decoder->_renderHotspots ? 2 : 1;

		// Wrap around if out of range
		leftSourceXCoord = leftSourceXCoord % static_cast<int32>(panoWidth);
		if (leftSourceXCoord < 0) {
			leftSourceXCoord += panoWidth;
		}

		leftSourceXCoord = desc->_sceneSizeY * factor - 1 - leftSourceXCoord;

		rightSourceXCoord = rightSourceXCoord % static_cast<int32>(panoWidth);
		if (rightSourceXCoord < 0) {
			rightSourceXCoord += panoWidth;
		}

		rightSourceXCoord = desc->_sceneSizeY * factor - 1 - rightSourceXCoord;

		uint16 x1 = halfWidthRoundedUp - 1 - x;
		uint16 x2 = w - halfWidthRoundedUp + x;

		for (uint16 y = 0; y < h; y++) {
			int32 sourceYCoord;

			if (warpMode == 0) {
				float tiltFactor = tan(verticalFovRadians / 2.0f);
				float normalizedY = ((float)y / (float)(h - 1)) * 2.0f - 1.0f;
				float projectedY = (normalizedY * tiltFactor) - tan(tiltAngleRadians);
				sourceYCoord = static_cast<int32>((projectedY + 1.0f) / 2.0f * panoHeight);
			} else {
				sourceYCoord = (2 * y + 1) * (bottomSrcCoord - topSrcCoord) / (2 * h) + topSrcCoord;
			}

			if (sourceYCoord < 0)
				sourceYCoord = 0;
			if (sourceYCoord >= panoHeight)
				sourceYCoord = panoHeight - 1;

			// Our panorma is apparently vertical, that's why we're passing Y co-ord before X co-ord
			uint32 pixel1 = sourceSurface->getPixel(sourceYCoord, leftSourceXCoord);
			uint32 pixel2 = sourceSurface->getPixel(sourceYCoord, rightSourceXCoord);

			if (_decoder->_renderHotspots) {
				const byte *col1 = &quickTimeDefaultPalette256[pixel1 * 3];
				pixel1 = _planarProjection->format.RGBToColor(col1[0], col1[1], col1[2]);
				const byte *col2 = &quickTimeDefaultPalette256[pixel2 * 3];
				pixel2 = _planarProjection->format.RGBToColor(col2[0], col2[1], col2[2]);
			}

			_planarProjection->setPixel(x1, y, pixel1);
			_planarProjection->setPixel(x2, y, pixel2);
		}
	}

	if (warpMode != 2) {
		boxAverage(_planarProjection, scaleFactor);
		_dirty = false;
		return;
	}

	// If warp mode is set to 2, also apply the perspective projection
	Common::Array<float> sideEdgeXYInterpolators;

	// The X interpolators are from 0 to maxProjectedX
	// The Y interpolators are the interpolator from minProjectedY to maxProjectedY
	sideEdgeXYInterpolators.resize(h * 2);

	for (uint16 y = 0; y < h; y++) {
		float t = ((float)y + 0.5f) / (float)h;

		float vector[3];
		for (int v = 0; v < 3; v++) {
			vector[v] = cornerVectors[0][v] * (1.0f - t) + cornerVectors[1][v] * t;
		}

		float projectedX = vector[0] / vector[2];
		float projectedY = vector[1] / vector[2];

		sideEdgeXYInterpolators[y * 2 + 0] = projectedX / maxProjectedX;
		sideEdgeXYInterpolators[y * 2 + 1] = (projectedY - minProjectedY) / (maxProjectedY - minProjectedY);
	}

	Graphics::Surface *aliasedProjectedPano = new Graphics::Surface();
	aliasedProjectedPano->create(w, h, _planarProjection->format);

	// Convert planar projection into perspective projection
	for (uint16 y = 0; y < h; y++) {
		float xInterpolator = sideEdgeXYInterpolators[y * 2 + 0];
		float yInterpolator = sideEdgeXYInterpolators[y * 2 + 1];

		int32 srcY = static_cast<int32>(yInterpolator * (float)h);
		int32 scanlineWidth = static_cast<int32>(xInterpolator * w);
		int32 startX = (w - scanlineWidth) / 2;
		//int32 endX = startX + scanlineWidth;

		// It would be better to compute a reciprocal and do this as a multiply instead,
		// doing divides per pixel is SLOW!
		for (uint16 x = 0; x < w; x++) {
			int32 srcX = (x + 0.5f) * xInterpolator + startX;
			uint32 pixel = _planarProjection->getPixel(srcX, srcY);

			// I increased the size of the surface (made it twice as wide and twice as tall)
			// Now I'm just getting the perspective projected pixel and creating a 2x2 box
			aliasedProjectedPano->setPixel(x, y, pixel);
		}
	}

	boxAverage(aliasedProjectedPano, scaleFactor);
	// Memory leakage avoided; Should I use a stack allocated object?
	aliasedProjectedPano->free();
	delete aliasedProjectedPano;

	_dirty = false;
}

Common::Point QuickTimeDecoder::getPanLoc(int16 x, int16 y) {
	PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);

	return track->projectPoint(x, y);
}

Graphics::FloatPoint QuickTimeDecoder::getPanAngles(int16 x, int16 y) {
	PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);
	PanoSampleDesc *desc = (PanoSampleDesc *)_panoTrack->sampleDescs[0];
	Common::Point pos = track->projectPoint(x, y);

	Graphics::FloatPoint res;

	res.x = desc->_hPanStart + (desc->_hPanStart - desc->_hPanEnd) * (float)pos.x / (float)desc->_sceneSizeY;
	res.y = desc->_vPanTop + (desc->_vPanBottom - desc->_vPanTop) * (float)pos.y / (float)desc->_sceneSizeX;

	return res;
}

void QuickTimeDecoder::lookupHotspot(int16 x, int16 y) {
	PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);

	if (!track->_constructedHotspots)
		return;

	Common::Point hotspotPoint = track->projectPoint(x, y);

	if (hotspotPoint.x < 0) {
		_rolloverHotspot = nullptr;
		_rolloverHotspotID = 0;
	} else {
		int hotspotId = (int)track->_constructedHotspots->getPixel(hotspotPoint.y, hotspotPoint.x);

		_rolloverHotspotID = hotspotId;

		if (hotspotId && _currentSample != -1) {
			if (!_rolloverHotspot || _rolloverHotspot->id != hotspotId)
				_rolloverHotspot = _panoTrack->panoSamples[_currentSample].hotSpotTable.get(hotspotId);

			if (!_rolloverHotspot)
				debugC(1, kDebugLevelGVideo, "Hotspot id: %d has no data", hotspotId);
			else
				debugC(1, kDebugLevelGVideo, "Hotspot id: %d is of type '%s'", hotspotId, tag2str(_rolloverHotspot->type));
		} else {
			_rolloverHotspot = nullptr;
		}
	}
}

Common::String QuickTimeDecoder::getHotSpotName(int id) {
	if (id <= 0)
		return "";

	PanoHotSpot *hotspot = _panoTrack->panoSamples[_currentSample].hotSpotTable.get(id);

	return _panoTrack->panoSamples[_currentSample].strTable.getString(hotspot->nameStrOffset);
}

const QuickTimeDecoder::PanoHotSpot *QuickTimeDecoder::getHotSpotByID(int id) {
	return _panoTrack->panoSamples[_currentSample].hotSpotTable.get(id);
}

const QuickTimeDecoder::PanoNavigation *QuickTimeDecoder::getHotSpotNavByID(int id) {
	const QuickTimeDecoder::PanoHotSpot *hotspot = getHotSpotByID(id);

	if (hotspot->type != MKTAG('n','a','v','g'))
		return nullptr;

	return _panoTrack->panoSamples[_currentSample].navTable.get(hotspot->typeData);
}

void QuickTimeDecoder::setClickedHotSpot(int id) {
	_clickedHotspot = getHotSpotByID(id);
	_clickedHotspotID = id;
}


///////////////////////////////
// INTERACTIVITY
//////////////////////////////

void QuickTimeDecoder::handleQuit() {
}

void QuickTimeDecoder::handleMouseMove(int16 x, int16 y) {
	if (_qtvrType == QTVRType::OBJECT)
		handleObjectMouseMove(x, y);
	else if (_qtvrType == QTVRType::PANORAMA)
		handlePanoMouseMove(x, y);

	_cursorDirty = true;
	_cursorPos = Common::Point(x, y);
}

void QuickTimeDecoder::handleObjectMouseMove(int16 x, int16 y) {
	if (!_isMouseButtonDown)
		return;

	VideoTrackHandler *track = (VideoTrackHandler *)_nextVideoTrack;

	// HACK: FIXME: Hard coded for now
	const int sensitivity = 10;
	const float speedFactor = 0.1f;

	int16 mouseDeltaX = x - _prevMouse.x;
	int16 mouseDeltaY = y - _prevMouse.y;

	float speedX = (float)mouseDeltaX * speedFactor;
	float speedY = (float)mouseDeltaY * speedFactor;

	bool changed = false;

	if (ABS(mouseDeltaY) >= sensitivity) {
		int newFrame = track->getCurFrame() - round(speedY) * _nav.columns;

		if (newFrame >= 0 && newFrame < track->getFrameCount()) {
			track->setCurFrame(newFrame);
			changed = true;
		}
	}

	if (ABS(mouseDeltaX) >= sensitivity) {
		int currentRow = track->getCurFrame() / _nav.columns;
		int currentRowStart = currentRow * _nav.columns;

		int newFrame = (track->getCurFrame() - (int)roundf(speedX) - currentRowStart) % _nav.columns + currentRowStart;

		if (newFrame >= 0 && newFrame < track->getFrameCount()) {
			track->setCurFrame(newFrame);
			changed = true;
		}
	}

	if (changed) {
		_prevMouse.x = x;
		_prevMouse.y = y;
	}
}

void QuickTimeDecoder::handlePanoMouseMove(int16 x, int16 y) {
	_prevMouse.x = x;
	_prevMouse.y = y;

	lookupHotspot(x, y);
}

void QuickTimeDecoder::handleMouseButton(bool isDown, int16 x, int16 y, bool repeat) {
	if (_qtvrType == QTVRType::OBJECT)
		handleObjectMouseButton(isDown, x, y, repeat);
	else if (_qtvrType == QTVRType::PANORAMA)
		handlePanoMouseButton(isDown, x, y, repeat);

	_cursorDirty = true;
	_cursorPos = Common::Point(x, y);
}

void QuickTimeDecoder::handleObjectMouseButton(bool isDown, int16 x, int16 y, bool repeat) {
	if (isDown) {
		if (y < _curBbox.top) {
			setCurrentRow(getCurrentRow() + 1);
		} else if (y > _curBbox.bottom) {
			setCurrentRow(getCurrentRow() - 1);
		} else if (x < _curBbox.left) {
			setCurrentColumn((getCurrentColumn() + 1) % _nav.columns);
		} else if (x > _curBbox.right) {
			setCurrentColumn((getCurrentColumn() - 1 + _nav.columns) % _nav.columns);
		} else {
			_prevMouse.x = x;
			_prevMouse.y = y;
		}
	}

	_isMouseButtonDown = isDown;
}

void QuickTimeDecoder::handlePanoMouseButton(bool isDown, int16 x, int16 y, bool repeat) {
	_isMouseButtonDown = isDown;

	lookupHotspot(x, y);

	if (isDown && !repeat) {
		_prevMouse.x = x;
		_prevMouse.y = y;

		_mouseDrag.x = x;
		_mouseDrag.y = y;

		_clickedHotspot = _rolloverHotspot;
		_clickedHotspotID = _rolloverHotspotID;
	}

	if (!repeat && !isDown && _rolloverHotspot && _prevMouse == _mouseDrag) {
		if (_rolloverHotspot->type == MKTAG('l','i','n','k')) {
			PanoLink *link = _panoTrack->panoSamples[_currentSample].linkTable.get(_rolloverHotspot->typeData);

			if (link) {
				goToNode(link->toNodeID);

				setPanAngle(link->toHPan);
				setTiltAngle(link->toVPan);
				setFOV(link->toZoom);
			}
		} else if (_rolloverHotspot->type == MKTAG('n','a','v','g')) {
			warning("navg hotpot id #%d not processed", _rolloverHotspot->id);
		}
	}

	// This line is necessary to let the decodeNextFrame() function know that the _isMouseButtonDown
	// variable has been updated, otherwise, it won't call projectPanorama() function
	if (!isDown) {
		PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);
		track->setDirty();
	}

	// Further we have simulated mouse button which are generated by timer, e.g. those
	// are used for dragging
	if (!repeat)
		return;

	// HACK: FIXME: Hard coded for now
	const int sensitivity = 5;
	const float speedFactor = 0.1f;

	int16 mouseDeltaX = x - _mouseDrag.x;
	int16 mouseDeltaY = y - _mouseDrag.y;

	float speedX = (float)mouseDeltaX * speedFactor;
	float speedY = (float)mouseDeltaY * speedFactor;

	if (ABS(mouseDeltaX) >= sensitivity)
		setPanAngle(getPanAngle() - speedX);

	if (ABS(mouseDeltaY) >= sensitivity)
		setTiltAngle(getTiltAngle() - speedY);
}

void QuickTimeDecoder::handleKey(Common::KeyState &state, bool down, bool repeat) {
	if (_qtvrType == QTVRType::OBJECT) {
		handleObjectKey(state, down, repeat);
	} else if (_qtvrType == QTVRType::PANORAMA) {
		handlePanoKey(state, down, repeat);
	}

	if (down) {
		_lastKey = state;
		_isKeyDown = true;
	} else {
		_isKeyDown = false;
	}

	_cursorDirty = true;
	_cursorPos = Common::Point(_prevMouse.x, _prevMouse.y);
}

void QuickTimeDecoder::handleObjectKey(Common::KeyState &state, bool down, bool repeat) {
	if (!down)
		return;

	switch (state.keycode) {
	case Common::KEYCODE_LEFT:
		nudge("left");
		break;
	case Common::KEYCODE_RIGHT:
		nudge("right");
		break;
	case Common::KEYCODE_UP:
		nudge("up");
		break;
	case Common::KEYCODE_DOWN:
		nudge("bottom");
		break;
	default:
		break;
	}
}

void QuickTimeDecoder::handlePanoKey(Common::KeyState &state, bool down, bool repeat) {
	PanoTrackHandler *track = (PanoTrackHandler *)getTrack(_panoTrack->targetTrack);
	track->setDirty();

	if ((state.flags & Common::KBD_SHIFT) && (state.flags & Common::KBD_CTRL)) {
		_zoomState = kZoomQuestion;
	} else if (state.flags & Common::KBD_SHIFT) {
		_zoomState = kZoomIn;

		if (!setFOV(getFOV() - 2.0))
			_zoomState = kZoomLimit;
	} else if (state.flags & Common::KBD_CTRL) {
		_zoomState = kZoomOut;

		if (!setFOV(getFOV() + 2.0))
			_zoomState = kZoomLimit;
	} else {
		_zoomState = kZoomNone;
	}

	if (state.keycode == Common::KEYCODE_h && down && !repeat) {
		if (track->_constructedHotspots)
			renderHotspots(!_renderHotspots);
	}
}

enum {
	kCurHand = 129,
	kCurGrab = 130,
	kCurObjUp = 131,
	kCurObjDown = 132,
	kCurObjLeft90 = 133,
	kCurObjRight90 = 134,
	kCurObjLeftM90 = 149,
	kCurObjRightM90 = 150,
	kCurObjUpLimit = 151,
	kCurObjDownLimit = 152,

	kCurDirAll = 211,
	kCurDirL = 212,
	kCurDirR = 213,
	kCurDirD = 214,
	kCurDirDL = 215,
	kCurDirDR = 216,
	kCurDirU = 217,
	kCurDirUL = 218,
	kCurDirUR = 219,

	kCurDirAllDown = 220,
	kCurDirURDown = 228,

	kCursorPano = 480,
	kCursorPanoLinkOver = 481,
	kCursorPanoLinkDown = 482,
	kCursorPanoLinkUp = 482,
	kCursorPanoObjOver = 484,
	kCursorPanoObjDown = 485,
	kCursorPanoObjUp = 486,

	kCursorZoomIn = 500,
	kCursorZoomOut = 501,
	kCursorZoomQuestion = 502,
	kCursorZoomLimit = 503,

	kCursorPanoNav = 510,
	kCursorPanoL = 511,
	kCursorPanoLS = 512,
	kCursorPanoR = 513,
	kCursorPanoRS = 514,
	kCursorPanoD = 515,
	kCursorPanoDS = 516,
	kCursorPanoU = 525,
	kCursorPanoUS = 526,

	kCursorPano2 = 540,

	kCurLastCursor
};

static const char *keyMatrix[] = {
  // 76543210
	"l.......", // 511
	"lL......", // 512
	"..r.....", // 513
	"..rR....", // 514
	"....d...", // 515
	"....dD..", // 516
	"l...d...", // 517
	"l...dD..", // 518
	"lL..d...", // 519
	"lL..dD..", // 520
	"..r.d...", // 521
	"..r.dD..", // 522
	"..rRd...", // 523
	"..rRdD..", // 524
	"......u.", // 525
	"......uU", // 526
	"l.....u.", // 527
	"l.....uU", // 528
	"lL....u.", // 529
	"lL....uU", // 530
	"..r...u.", // 531
	"..r...uU", // 532
	"..rR..u.", // 533
	"..rR..uU", // 534
	0
};

void QuickTimeDecoder::updateQTVRCursor(int16 x, int16 y) {
	if (_qtvrType == QTVRType::OBJECT) {
		int tiltIdx = int((-_tiltAngle + 90.0) / 21) * 2;

		if (y < _curBbox.top)
			setCursor(tiltIdx == 16 ? kCurObjUpLimit : kCurObjUp);
		else if (y > _curBbox.bottom)
			setCursor(tiltIdx == 0 ? kCurObjDownLimit : kCurObjDown);
		else if (x < _curBbox.left)
			setCursor(kCurObjLeft90 + tiltIdx);
		else if (x > _curBbox.right)
			setCursor(kCurObjRight90 + tiltIdx);
		else
			setCursor(_isMouseButtonDown ? kCurGrab : kCurHand);
	} else if (_qtvrType == QTVRType::PANORAMA) {
		if (_zoomState != kZoomNone) {
			switch (_zoomState) {
			case kZoomIn:
				setCursor(kCursorZoomIn);
				break;
			case kZoomOut:
				setCursor(kCursorZoomOut);
				break;
			case kZoomQuestion:
				setCursor(kCursorZoomQuestion);
				break;
			case kZoomLimit:
				setCursor(kCursorZoomLimit);
				break;
			}

			return;
		}

		// Get hotspot cursors
		uint32 hsType = MKTAG('u','n','d','f');

		if (_rolloverHotspot)
			hsType = _rolloverHotspot->type;

		int hsOver, hsDown, hsUp;

		switch (hsType) {
		case MKTAG('l','i','n','k'):
			hsOver = kCursorPanoLinkOver;
			hsDown = kCursorPanoLinkDown;
			hsUp = kCursorPanoLinkUp;
			break;

		case MKTAG('n','a','v','g'):
			debug(3, "Hotspot type: %s", tag2str((uint32)hsType));
			// TODO FIXME: Implement
			// fallthrough

		default:
			hsOver = kCursorPanoObjOver;
			hsDown = kCursorPanoObjDown;
			hsUp = kCursorPanoObjUp;
			break;
		}

		if (_rolloverHotspot) {
			if (_rolloverHotspot->mouseOverCursorID)
				hsOver = _rolloverHotspot->mouseOverCursorID;

			if (_rolloverHotspot->mouseDownCursorID)
				hsDown = _rolloverHotspot->mouseDownCursorID;

			if (_rolloverHotspot->mouseUpCursorID)
				hsUp = _rolloverHotspot->mouseUpCursorID;
		}

		int sensitivity = 5;

		if (!_isMouseButtonDown) {
			setCursor(_rolloverHotspot ? hsOver : kCursorPano);
		} else {
			int res = 0;
			PanoSampleDesc *desc = (PanoSampleDesc *)_panoTrack->sampleDescs[0];
			bool pano360 = !(desc->_hPanStart != desc->_hPanEnd && (desc->_hPanStart != 0.0 || desc->_hPanEnd != 360.0));
			debugC(4, kDebugLevelGVideo, "pano360: %d _panAngle: %f [%f - %f] +%f  -%f fov: %f hfov: %f", pano360, _panAngle, desc->_hPanStart, desc->_hPanEnd, desc->_hPanStart + _fov, desc->_hPanEnd - _fov, _fov, _hfov);

			// left
			if (x < _mouseDrag.x - sensitivity) {
				res |= 1;
				res <<= 1;

				// left stop
				if (!pano360 && _panAngle >= desc->_hPanEnd - _hfov)
					res |= 1;
				res <<= 1;
			} else {
				res <<= 2;
			}

			// right
			if (x > _mouseDrag.x + sensitivity) {
				res |= 1;
				res <<= 1;

				// right stop
				if (!pano360 && _panAngle <= desc->_hPanStart + _hfov)
					res |= 1;
				res <<= 1;
			} else {
				res <<= 2;
			}

			// down
			if (y > _mouseDrag.y + sensitivity) {
				res |= 1;
				res <<= 1;

				// down stop
				if (_tiltAngle <= desc->_vPanBottom + _fov / 2)
					res |= 1;
				res <<= 1;
			} else {
				res <<= 2;
			}

			// up
			if (y < _mouseDrag.y - sensitivity) {
				res |= 1;
				res <<= 1;

				// up stop
				if (_tiltAngle >= desc->_vPanTop - _fov / 2)
					res |= 1;
			} else {
				res <<= 1;
			}

			(void)hsUp;
			setCursor(_cursorDirMap[res] ? _cursorDirMap[res] : _rolloverHotspot ? hsDown : kCursorPanoNav);
		}
	}
}

void QuickTimeDecoder::cleanupCursors() {
	if (!_cursorCache)
		return;

	for (int i = 0; i < kCurLastCursor; i++)
		delete _cursorCache[i];

	free(_cursorCache);
	_cursorCache = nullptr;
}

void QuickTimeDecoder::setCursor(int curId) {
	_currentQTVRCursor = curId;

	if (!_dataBundle) {
		_dataBundle = Common::makeZipArchive(MACGUI_DATA_BUNDLE);

		if (!_dataBundle) {
			warning("QTVR: Couldn't load data bundle '%s'.", MACGUI_DATA_BUNDLE);
		}
	}

	if (!_cursorCache) {
		_cursorCache = (Graphics::Cursor **)calloc(kCurLastCursor, sizeof(Graphics::Cursor *));

		computeInteractivityZones();
		memset(_cursorDirMap, 0, 256 * sizeof(int));

		int n = 511;
		for (const char **p = keyMatrix; *p; p++, n++) {
			int res = 0;

			for (int i = 0; i < 8; i++) {
				res <<= 1;

				if ((*p)[i] != '.')
					res |= 1;
			}

			_cursorDirMap[res] = n;
		}
	}

	if (curId >= kCurLastCursor)
		error("QTVR: Incorrect cursor ID: %d > %d", curId, kCurLastCursor);

	if (!_cursorCache[curId]) {
		Common::Path path(Common::String::format("qtvr/CURSOR%d_1.cur", curId));

		Common::SeekableReadStream *stream = _dataBundle->createReadStreamForMember(path);

		if (!stream) {
			warning("QTVR: Cannot load cursor ID %d, file '%s' does not exist", curId, path.toString().c_str());
			return;
		}

		Image::IcoCurDecoder decoder;
		if (!decoder.open(*stream, DisposeAfterUse::YES)) {
			warning("QTVR: Cannot load cursor ID %d, file '%s' bad format", curId, path.toString().c_str());
			return;
		}

		_cursorCache[curId] = decoder.loadItemAsCursor(0);
	}

	CursorMan.replaceCursor(_cursorCache[curId]);
	CursorMan.showMouse(true);
}

void QuickTimeDecoder::computeInteractivityZones() {
	_curBbox.left = MIN(20, getWidth() / 10);
	_curBbox.right = getWidth() - _curBbox.left;

	_curBbox.top = MIN(20, getHeight() / 10);
	_curBbox.bottom = getHeight() - _curBbox.top;
}

} // End of namespace Video
