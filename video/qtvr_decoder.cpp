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
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
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

	return entry;
}

void QuickTimeDecoder::closeQTVR() {
	delete _dataBundle;
	_dataBundle = nullptr;
	cleanupCursors();
}

void QuickTimeDecoder::setTargetSize(uint16 w, uint16 h) {
	if (!isVR())
		error("QuickTimeDecoder::setTargetSize() called on non-VR movie");

	_width = w;
	_height = h;

	setFOV(_fov);
}

void QuickTimeDecoder::setFOV(float fov) {
	_fov = fov;

	_hfov = _fov * (float)_width / (float)_height;
}

void QuickTimeDecoder::updateAngles() {
	_panAngle = (float)getCurrentColumn() / (float)_nav.columns * 360.0;
	_tiltAngle = ((_nav.rows - 1) / 2.0 - (float)getCurrentRow()) / (float)(_nav.rows - 1) * 180.0;

	debugC(1, kDebugLevelMacGUI, "QTVR: row: %d col: %d  (%d x %d) pan: %f tilt: %f", getCurrentRow(), getCurrentColumn(), _nav.rows, _nav.columns, getPanAngle(), getTiltAngle());
}

void QuickTimeDecoder::handleMouseMove(int16 x, int16 y) {
	if (_qtvrType == QTVRType::OBJECT)
		handleObjectMouseMove(x, y);
	else if (_qtvrType == QTVRType::PANORAMA)
		handlePanoMouseMove(x, y);
}

void QuickTimeDecoder::handleObjectMouseMove(int16 x, int16 y) {
	updateQTVRCursor(x, y);

	if (!_isMouseButtonDown)
		return;

	VideoTrackHandler *track = (VideoTrackHandler *)_nextVideoTrack;

	// HACK: FIXME: Hard coded for now
	const int sensitivity = 10;
	const float speedFactor = 0.1f;

	int16 mouseDeltaX = x - _prevMouseX;
	int16 mouseDeltaY = y - _prevMouseY;

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
		_prevMouseX = x;
		_prevMouseY = y;
	}
}

void QuickTimeDecoder::handlePanoMouseMove(int16 x, int16 y) {
	if (!_isMouseButtonDown)
		return;

	PanoTrackHandler *track = (PanoTrackHandler *)_nextVideoTrack;

	// HACK: FIXME: Hard coded for now
	const int sensitivity = 10;
	const float speedFactor = 0.5f;

	int16 mouseDeltaX = x - _prevMouseX;
	int16 mouseDeltaY = y - _prevMouseY;

	float speedX = (float)mouseDeltaX * speedFactor;
	float speedY = (float)mouseDeltaY * speedFactor;

	bool changed = false;

	if (ABS(mouseDeltaX) >= sensitivity) {
		track->setPanAngle(track->getPanAngle() + speedX);

		changed = true;
	}

	if (ABS(mouseDeltaY) >= sensitivity) {
		track->setTiltAngle(track->getTiltAngle() + speedY);

		changed = true;
	}

	if (changed) {
		_prevMouseX = x;
		_prevMouseY = y;
	}
}


void QuickTimeDecoder::handleMouseButton(bool isDown, int16 x, int16 y) {
	if (_qtvrType == QTVRType::OBJECT)
		handleObjectMouseButton(isDown, x, y);
	else if (_qtvrType == QTVRType::PANORAMA)
		handlePanoMouseButton(isDown, x, y);
}

void QuickTimeDecoder::handleObjectMouseButton(bool isDown, int16 x, int16 y) {
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
			_prevMouseX = x;
			_prevMouseY = y;
			_isMouseButtonDown = isDown;
		}
	} else {
		_isMouseButtonDown = isDown;
	}

	updateQTVRCursor(x, y);
}

void QuickTimeDecoder::handlePanoMouseButton(bool isDown, int16 x, int16 y) {
	_isMouseButtonDown = isDown;

	if (isDown) {
		_prevMouseX = x;
		_prevMouseY = y;
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
	} else if (direction.equalsIgnoreCase("top")) {
		newFrame = curFrame - _nav.columns;
		if (newFrame < 0)
			return;
	} else if (direction.equalsIgnoreCase("bottom")) {
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

QuickTimeDecoder::PanoTrackHandler::PanoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent) : _decoder(decoder), _parent(parent) {
	if (decoder->_qtvrType != QTVRType::PANORAMA)
		error("QuickTimeDecoder::PanoTrackHandler: Incorrect track passed");

	_isPanoConstructed = false;

	_constructedPano = nullptr;
	_constructedHotspots = nullptr;
	_projectedPano = nullptr;

	_curPanAngle = 0.0f;
	_curTiltAngle = 0.0f;

	_dirty = true;
}

QuickTimeDecoder::PanoTrackHandler::~PanoTrackHandler() {
	if (_isPanoConstructed) {
		_constructedPano->free();
		delete _constructedPano;

		_constructedHotspots->free();
		delete _constructedHotspots;
	}

	if (_projectedPano) {
		_projectedPano->free();
		delete _projectedPano;
	}
}

uint16 QuickTimeDecoder::PanoTrackHandler::getWidth() const {
	return getScaledWidth().toInt();
}

uint16 QuickTimeDecoder::PanoTrackHandler::getHeight() const {
	return getScaledHeight().toInt();
}

Graphics::PixelFormat QuickTimeDecoder::PanoTrackHandler::getPixelFormat() const {
	return Graphics::PixelFormat::createFormatCLUT8();
}

Common::Rational QuickTimeDecoder::PanoTrackHandler::getScaledWidth() const {
	return Common::Rational(_parent->width) / _parent->scaleFactorX;
}

Common::Rational QuickTimeDecoder::PanoTrackHandler::getScaledHeight() const {
	return Common::Rational(_parent->height) / _parent->scaleFactorY;
}

bool QuickTimeDecoder::PanoTrackHandler::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	return true;
}

bool QuickTimeDecoder::PanoTrackHandler::canDither() const {
	return false;
}

void QuickTimeDecoder::PanoTrackHandler::setDither(const byte *palette) {
	assert(canDither());
}

const byte *QuickTimeDecoder::PanoTrackHandler::getPalette() const {
	return _curPalette;
}

const Graphics::Surface *QuickTimeDecoder::PanoTrackHandler::decodeNextFrame() {
	if (!_isPanoConstructed)
		return nullptr;

	if (_dirty)
		projectPanorama();

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

	Common::Rect srcRect(0, 0, framew, frameh);

	for (uint y = 0; y < h; y++) {
		for (uint x = 0; x < w; x++) {
			const Graphics::Surface *frame = track->bufferNextFrame();

			if (!frame) {
				warning("QuickTimeDecoder::PanoTrackHandler::constructPanorama(): Out of frames at: %d, %d", x, y);
				break;
			}

			target->copyRectToSurface(*frame, x * framew, y * frameh, srcRect);
		}
	}

	Common::Path path = Common::Path(fname);

	Common::DumpFile bitmapFile;
	if (!bitmapFile.open(path, true)) {
		warning("Cannot dump panorama into file '%s'", path.toString().c_str());
		return nullptr;
	}

	Image::writePNG(bitmapFile, *target, track->getPalette());
	bitmapFile.close();

	debug(0, "Dumped panorama %s of %d x %d", path.toString().c_str(), target->w, target->h);

	return target;
}


void QuickTimeDecoder::PanoTrackHandler::constructPanorama() {
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];

	warning("scene: %d (%d x %d) hotspots: %d (%d x %d)", desc->_sceneTrackID, desc->_sceneSizeX, desc->_sceneSizeY,
			desc->_hotSpotTrackID, desc->_hotSpotSizeX, desc->_hotSpotSizeY);

	warning("sceneNumFrames: %d x %d sceneColorDepth: %d", desc->_sceneNumFramesX, desc->_sceneNumFramesY, desc->_sceneColorDepth);
	warning("targetTrackID: %d", _parent->targetTrack);

	VideoTrackHandler *track = (VideoTrackHandler *)(_decoder->getTrack(_decoder->Common::QuickTimeParser::_tracks[desc->_sceneTrackID - 1]->targetTrack));

	_constructedPano = constructMosaic(track, desc->_sceneNumFramesX, desc->_sceneNumFramesY, "dumps/pano-full.png");

	track = (VideoTrackHandler *)(_decoder->getTrack(_decoder->Common::QuickTimeParser::_tracks[desc->_hotSpotTrackID - 1]->targetTrack));

	_constructedHotspots = constructMosaic(track, desc->_hotSpotNumFramesX, desc->_hotSpotNumFramesY, "dumps/pano-hotspot.png");

	_isPanoConstructed = true;

	_curPanAngle = desc->_hPanStart;
	_curTiltAngle = 0.0f;
}

void QuickTimeDecoder::PanoTrackHandler::projectPanorama() {
	if (!_isPanoConstructed)
		return;

	uint16 w = _decoder->getWidth(), h = _decoder->getHeight();

	if (!_projectedPano) {
		_projectedPano = new Graphics::Surface();
		_projectedPano->create(w, h, _constructedPano->format);
	}

#if 1
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];
	int startY = (((float)desc->_sceneSizeY - w) / (desc->_hPanEnd - desc->_hPanStart)) * (_curPanAngle - desc->_hPanStart);
	int startX = ((float)(desc->_sceneSizeX - h) / (desc->_vPanTop - desc->_vPanBottom)) * (_curTiltAngle - desc->_vPanBottom);

	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			int setX = y + startX;
			int setY = x + startY;

			uint32 pixel = _constructedPano->getPixel(setX, setY);
			_projectedPano->setPixel(x, y, pixel);
		}
	}

	_dirty = false;
#else
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];

	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			float panAngle = _curPanAngle + (x - w / 2) * _decoder->_hfov / (float)w;
			float tiltAngle = _curTiltAngle + (y - h / 2) * _decoder->_fov / (float)h;
			float vprecalc = (sin(tiltAngle) + 1.0) / 2.0;

			// It is flipped 90 degrees
			int u = (((float)desc->_sceneSizeY - w) / (desc->_hPanEnd - desc->_hPanStart)) * (panAngle - desc->_hPanStart);
			int v = desc->_sceneSizeX * vprecalc;

			warning("u: %d of %d, v: %d of %d", u, _constructedPano->w, v, _constructedPano->h);

			uint32 pixel = _constructedPano->getPixel(v, u);
			_projectedPano->setPixel(x, y, pixel);
		}
	}

#endif

#if 0
	const float c = _projectedPano->w;
	const float r = c / (2 * M_PI);

	// HACK: FIXME: Hard coded for now
	const float d = 500.0f;

	for (int16 y = 0; y < _projectedPano->h; y++) {
		for (int16 x = 0; x < _projectedPano->w; x++) {
			double u = atan(x / d) / (2.0 * M_PI);
			double v = y * r * cos(u) / d;

			int setX = round(u * _constructedPano->w);
			int setY = round(v);

			if (setX >= 0 && setX < _constructedPano->w && setY >= 0 && setY < _constructedPano->h) {
				uint32 pixel = _constructedPano->getPixel(setX, setY);
				_projectedPano->setPixel(x, y, pixel);
			}
		}
	}

#endif

	_dirty = false;
}

void QuickTimeDecoder::PanoTrackHandler::setPanAngle(float angle) {
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];

	if (angle < desc->_hPanStart + _decoder->_hfov / 2)
		angle = desc->_hPanStart + _decoder->_hfov / 2;

	if (angle > desc->_hPanEnd - _decoder->_hfov / 2)
		angle = desc->_hPanEnd - _decoder->_hfov / 2;

	if (_curPanAngle != angle) {
		_curPanAngle = angle;

		_dirty = true;
	}
}

void QuickTimeDecoder::PanoTrackHandler::setTiltAngle(float angle) {
	PanoSampleDesc *desc = (PanoSampleDesc *)_parent->sampleDescs[0];

	if (angle < desc->_vPanBottom + _decoder->_fov / 2)
		angle = desc->_vPanBottom + _decoder->_fov / 2;

	if (angle > desc->_vPanTop - _decoder->_fov / 2)
		angle = desc->_vPanTop - _decoder->_fov / 2;

	if (_curTiltAngle != angle) {
		_curTiltAngle = angle;

		_dirty = true;
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
	kCurLastCursor
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
	if (_currentQTVRCursor == curId)
		return;

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
