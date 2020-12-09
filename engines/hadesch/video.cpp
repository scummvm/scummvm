/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 * Copyright 2020 Google
 *
 */

#include "common/file.h"
#include "hadesch/video.h"
#include "hadesch/pod_image.h"
#include "hadesch/tag_file.h"
#include "hadesch/hadesch.h"
#include "common/system.h"
#include "video/smk_decoder.h"
#include "audio/decoders/aiff.h"
#include "hadesch/pod_file.h"
#include "hadesch/baptr.h"
#include "common/translation.h"

static const int kVideoMaxW = 1280;
static const int kVideoMaxH = 480;

namespace Hadesch {

Common::String LayerId::getDebug() const {
	if (_idx == -1 && _qualifier == "")
		return "[" + _name + "]";
	return Common::String::format("[%s/%s/%d]",
				      _name.c_str(), _qualifier.c_str(), _idx);
}

bool LayerId::operator== (const LayerId &b) const {
	return _name == b._name && _idx == b._idx && _qualifier == b._qualifier;
}

PlayAnimParams PlayAnimParams::loop() {
	return PlayAnimParams(true, true);
}

PlayAnimParams PlayAnimParams::keepLastFrame() {
	return PlayAnimParams(false, true);
}
	
PlayAnimParams PlayAnimParams::disappear() {
	return PlayAnimParams(false, false);
}

bool PlayAnimParams::getKeepLastFrame() {
	return _keepLastFrame;
}

bool PlayAnimParams::isLoop() {
	return _loop;
}

int PlayAnimParams::getSpeed() {
	return _msperframe;
}

int PlayAnimParams::getFirstFrame() {
	return _firstFrame;
}
	
int PlayAnimParams::getLastFrame() {
	return _lastFrame;
}
		
PlayAnimParams PlayAnimParams::partial(int first, int last) const {
	PlayAnimParams ret(*this);
	ret._firstFrame = first;
	ret._lastFrame = last;
	return ret;
}

PlayAnimParams PlayAnimParams::speed(int msperframe) const {
	PlayAnimParams ret(*this);
	ret._msperframe = msperframe;
	return ret;
}

PlayAnimParams PlayAnimParams::backwards() const {
	PlayAnimParams ret(*this);
	ret._firstFrame = _lastFrame;
	ret._lastFrame = _firstFrame;
	return ret;
}

PlayAnimParams::PlayAnimParams(bool isLoop, bool isKeepLastFrame) {
	_loop = isLoop;
	_keepLastFrame = isKeepLastFrame;
	_firstFrame = 0;
	_lastFrame = -1;
	_msperframe = kDefaultSpeed;
}

int VideoRoom::layerComparator(const Layer &a, const Layer &b) {
	if (a.zValue == b.zValue)
		return a.genCounter - b.genCounter;
	return b.zValue - a.zValue;
}

VideoRoom::VideoRoom(const Common::String &dir, const Common::String &pod,
		     const Common::String &assetMapFile) : _layers(layerComparator) {
	Common::String podPath = g_vm->getCDScenesPath() + dir + "/" + pod + ".pod";
	_podFile = Common::SharedPtr<PodFile>(new PodFile(podPath));
	_podFile->openStore(podPath);
	_smkPath = g_vm->getCDScenesPath() + dir;
	Common::SharedPtr<Common::SeekableReadStream> assetMapStream(assetMapFile != "" ? openFile(assetMapFile) : nullptr);
	if (assetMapStream) {
		_assetMap = TextTable(assetMapStream, 2);
	}

	_videoPixels = sharedPtrByteAlloc(kVideoMaxW * kVideoMaxH);
	_hotZone = -1;
	_mouseEnabled = true;
	_videoOffset = Common::Point(0, 0);
	_videoSurfOffset = Common::Point(0, 0);
	_pan = 0;
	_panSpeed = 0;
	_panCallback = -1;
	_pannable = false;
	_leftEdge = false;
	_rightEdge = false;
	_heroBeltEnabled = true;
	_userPanStartLeftCallback = -1;
	_userPanStartRightCallback = -1;
	_userPanEndLeftCallback = -1;
	_userPanEndRightCallback = -1;
	_videoZ = 0;
	_layerGenCounter = 0;
	_finalFade = 0x100;
	_finalFadeSpeed = 0;
	_draggingPtr = 0;
	_isDragging = 0;
}

VideoRoom::~VideoRoom() {
	if (_videoDecoder) {
		_videoDecoder->stop();
		_videoDecoder.reset();
	}
	for (unsigned i = 0; i < _anims.size(); i++)
		g_system->getMixer()->stopHandle(_anims[i]._soundHandle);
}

void VideoRoom::setPannable(bool val) {
	_pannable = val;
}

void VideoRoom::setHotzoneEnabled(const Common::String &name, bool enabled) {
       _hotZones.setHotzoneEnabled(name, enabled);
}

void VideoRoom::enableHotzone(const Common::String &name) {
	_hotZones.setHotzoneEnabled(name, true);
}

void VideoRoom::disableHotzone(const Common::String &name) {
	_hotZones.setHotzoneEnabled(name, false);
}

void VideoRoom::setLayerEnabled(const LayerId &name, bool val) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name)
			it->isEnabled = val;
	}
}

void VideoRoom::setLayerParallax(const LayerId &name, int val) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name)
			it->parallax = val;
	}
}

void VideoRoom::setColorScale(const LayerId &name, int val) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name)
			it->colorScale = val;
	}
}

void VideoRoom::setScale(const LayerId &name, int val) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name)
			it->scale = val;
	}
}

int VideoRoom::getNumFrames(const LayerId &name) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name)
			return it->renderable->getNumFrames();
	}

	return 0;
}

void VideoRoom::startAnimationInternal(const LayerId &name, int zValue, int msperframe, bool loop,
				       bool fixedFrame,
				       int first, int last, Common::Point offset) {
	int32 startms = g_vm->getCurrentTime();
	Common::Array<Layer> modifiedZ;
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end();) {
		if (it->name == name) {
			it->isEnabled = true;
			if (fixedFrame)
				it->renderable->selectFrame(first);
			else
				it->renderable->startAnimation(startms, msperframe, loop, first, last);
			it->offset = offset;
			if (it->zValue != zValue) {
				Layer l = *it;
				l.zValue = zValue;
				modifiedZ.push_back(l);
				it = _layers.erase(it);
			} else {
				it++;
			}
		} else {
			it++;
		}
	}
	// This is slow but should rarely happen
	if (!modifiedZ.empty()) {
		for (Common::Array<Layer>::iterator it = modifiedZ.begin(); it != modifiedZ.end(); it++) {
			_layers.insert(*it);
		}		
	}
}

void VideoRoom::selectFrame(const LayerId &name, int zValue, int frame, Common::Point offset) {
  	if (!doesLayerExist(name)) {
		addAnimLayerInternal(name, zValue);
	}

	startAnimationInternal(name, zValue, kDefaultSpeed, true, true, frame, frame, offset);
}

PodImage VideoRoom::getLayerFrame(const Hadesch::LayerId &name) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name) {
			return it->renderable->getFrame(g_vm->getCurrentTime());
		}
	}

	return PodImage();
}

int VideoRoom::getAnimFrameNum(const Hadesch::LayerId &name) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name) {
			return it->renderable->getAnimationFrameNum(g_vm->getCurrentTime());
		}
	}

	return -1;
}

void VideoRoom::stopAnim(const LayerId &name) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name) {
			it->isEnabled = false;
		}
	}
	for (unsigned i = 0; i < _anims.size(); i++) {
		if (_anims[i]._animName == name) {
			g_system->getMixer()->stopHandle(_anims[i]._soundHandle);
			_anims[i]._finished = true;
		}
	}
}

void VideoRoom::purgeAnim(const LayerId &name) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end();) {
		if (it->name == name) {
			it = _layers.erase(it);
		} else
			it++;
	}
}

void VideoRoom::dumpLayers() {
	debug("Current layers:");
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		debug("   %s %s", it->name.getDebug().c_str(), it->isEnabled ? "enabled" : "disabled");
	}
}

void VideoRoom::finish() {
	for (unsigned i = 0; i < _anims.size(); i++) {
		g_system->getMixer()->stopHandle(_anims[i]._soundHandle);
		_anims[i]._finished = true;
	}
}

void VideoRoom::pause() {
	for (unsigned i = 0; i < _anims.size(); i++) {
		g_system->getMixer()->pauseHandle(_anims[i]._soundHandle, true);
	}
}

void VideoRoom::unpause() {
	for (unsigned i = 0; i < _anims.size(); i++) {
		g_system->getMixer()->pauseHandle(_anims[i]._soundHandle, false);
	}
}

bool VideoRoom::doesLayerExist(const LayerId &name) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name) {
			return true;
		}
	}
	return false;
}

bool VideoRoom::isAnimationFinished(const LayerId &name, int time) {
	for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++) {
		if (it->name == name) {
			return it->renderable->isAnimationFinished(time);
		}
	}

	return true;
}

void VideoRoom::addLayer(Renderable *rend, const LayerId &name, int zValue,
			 bool isEnabled, Common::Point offset) {
	Layer l;
	l.renderable = Common::SharedPtr<Renderable>(rend);
	l.name = name;
	l.isEnabled = isEnabled;
	l.offset = offset;
	l.zValue = zValue;
	l.parallax = 0;
	l.colorScale = 0x100;
	l.scale = 100;
	l.genCounter = _layerGenCounter++;
	_layers.insert(l);
}

void VideoRoom::computeHotZone(int time, Common::Point mousePos) {
	bool wasEdge = _leftEdge || _rightEdge;
	_leftEdge = false;
	_rightEdge = false;

	Common::SharedPtr<HeroBelt> belt = g_vm->getHeroBelt();
	
	if (_heroBeltEnabled)
		belt->computeHotZone(time, mousePos, _mouseEnabled);
	if (_heroBeltEnabled && belt->isOverHeroBelt()) {
		_hotZone = -1;
		return;
	}
	if (!_mouseEnabled) {
		_hotZone = -1;
		return;
	}
	Common::Point canvasPos = mousePos + Common::Point(_pan, 0);
	int wasHotZone = _hotZone;
	_hotZone = _hotZones.pointToIndex(canvasPos);
	if (_hotZone >= 0) {
		if (wasHotZone < 0) {
			_startHotTime = time;
		}
		return;
	}
	_leftEdge = (mousePos.x < 20) && _pan > 0;
	_rightEdge = (mousePos.x > 620) && _pan < 640;
	if ((!wasEdge && (_leftEdge || _rightEdge)))
		_edgeStartTime = time;
	_hotZone = -1;
}

void VideoRoom::setHotZoneOffset(const Common::String &name, Common::Point offset) {
	_hotZones.setHotZoneOffset(name, offset);
}

Common::String VideoRoom::mapClick(Common::Point mousePos) {
	if (!_mouseEnabled) {
		return "";
	}
	Common::Point canvasPos = mousePos + Common::Point(_pan, 0) + _viewportOffset;
	return _hotZones.pointToName(canvasPos);
}

Common::String VideoRoom::getHotZone() {
	return _hotZones.indexToName(_hotZone);
}

int VideoRoom::getCursorAnimationFrame(int time) {
	if (_hotZone < 0)
		return -1;
	if (_hotZones.indexToICSH(_hotZone) != 0)
		return -1;
	return (time - _startHotTime) / kDefaultSpeed;
}

bool VideoRoom::isVideoPlaying() {
	return !!_videoDecoder;
}

void VideoRoom::fadeOut(int ms, const EventHandlerWrapper &callback) {
	_finalFadeCallback = callback;
	_finalFadeSpeed = -(kDefaultSpeed * 0x100) / ms;
}

void VideoRoom::nextFrame(Common::SharedPtr<GfxContext> context, int time, bool stopVideo) {
	context->clear();

	Common::SharedPtr<HeroBelt> belt = g_vm->getHeroBelt();

	if (!_mouseEnabled) {
		_cursor = (time / 200) % 10 + 3;
	} else if (_heroBeltEnabled && belt->isOverHeroBelt()) {
		_cursor = belt->getCursor(time);
	} else if (g_vm->getPersistent()->_currentRoomId == kMonsterPuzzle) {
		_cursor = 17;
	} else if (_hotZone >= 0) {
		_cursor = _hotZones.indexToCursor(_hotZone, (time - _startHotTime) / kDefaultSpeed);
	} else if (_leftEdge && _pannable) {
		_cursor = 14;
	} else if (_rightEdge && _pannable) {
		_cursor = 16;
	} else
		_cursor = 0;

	if (_leftEdge && (time > _edgeStartTime + 2000) && _panSpeed == 0 && _pannable && _mouseEnabled) {
		g_vm->handleEvent(_userPanStartLeftCallback);
		panLeftAnim(_userPanEndLeftCallback);
	}
	if (_rightEdge && (time > _edgeStartTime + 2000) && _panSpeed == 0 && _pannable && _mouseEnabled) {
		g_vm->handleEvent(_userPanStartRightCallback);
		panRightAnim(_userPanEndRightCallback);
	}

	if (_panSpeed != 0) {
		_pan += _panSpeed;
		if (_pan <= 0) {
			g_vm->handleEvent(_panCallback);
			_panCallback = -1;
			_pan = 0;
			_panSpeed = 0;
		}
		if (_pan >= 640) {
			g_vm->handleEvent(_panCallback);
			_panCallback = -1;
			_pan = 640;
			_panSpeed = 0;
		}		
	}

	if (_finalFadeSpeed != 0) {
		_finalFade += _finalFadeSpeed;
		if (_finalFade <= 0 && _finalFadeSpeed < 0) {
			_finalFade = 0;
			_finalFadeSpeed = 0;
			_finalFadeCallback();
		}
		if (_finalFade >= 0x100 && _finalFadeSpeed > 0) {
			_finalFade = 0x100;
			_finalFadeSpeed = 0;
			_finalFadeCallback();
		}
	}

	Common::SortedArray<Layer>::iterator layersIterator = _layers.begin();

	for (; layersIterator != _layers.end() && layersIterator->zValue > _videoZ; layersIterator++) {
		if (!layersIterator->isEnabled)
			continue;
		const PodImage &pi = layersIterator->renderable->getFrame(time);

		pi.render (context,
			   layersIterator->offset + Common::Point(_pan * layersIterator->parallax / 640, 0) + kZeroPoint,
			   layersIterator->colorScale, layersIterator->scale);
	}

	if (_videoDecoder && !_videoDecoder->endOfVideo() && _videoDecoder->needsUpdate()) {
		const Graphics::Surface *surf = _videoDecoder->decodeNextFrame();
		_videoW = MIN<int>(surf->w, kVideoMaxW);
		_videoH = MIN<int>(surf->h, kVideoMaxH);
		_videoSurfOffset = _videoOffset;
		memcpy(_videoPixels.get(),
		       surf->getPixels(), _videoW * _videoH);
		const byte *pal = _videoDecoder->getPalette();
		if (pal)
			memcpy(_videoPalette, pal, 256 * 3);
	}

	if (_videoDecoder && _videoW && _videoH) {
		context->blitVideo(_videoPixels.get(), _videoW, _videoW, _videoH, _videoPalette, _videoSurfOffset + kZeroPoint);
	}

	for (; layersIterator != _layers.end(); layersIterator++) {
		if (!layersIterator->isEnabled)
			continue;
		const PodImage &pi = layersIterator->renderable->getFrame(time);

		pi.render (context,
			   layersIterator->offset + Common::Point(_pan * layersIterator->parallax / 640, 0) + kZeroPoint,
			   layersIterator->colorScale, layersIterator->scale);
	}

	if (stopVideo) {
		_subtitles.clear();
		_countQueuedSubtitles.clear();
	}

	while (!_subtitles.empty() && time > _subtitles.front().maxTime) {
		_countQueuedSubtitles[_subtitles.front().ID]--;
		_subtitles.pop();
	}

	if (_subtitles.empty())
		_countQueuedSubtitles.clear();

	if (_videoDecoder && (_videoDecoder->endOfVideo() || (stopVideo && !_mouseEnabled))) {
		debug("videoEnd: %s", _videoDecoderEndEvent.getDebugString().c_str());
		_videoDecoder.reset();
		g_vm->handleEvent(_videoDecoderEndEvent);
	}

	for (unsigned i = 0; i < _anims.size(); i++) {
		if (_anims[i]._finished)
			continue;

		bool soundFinished = !g_system->getMixer()->isSoundHandleActive(_anims[i]._soundHandle);
		const LayerId &animName = _anims[i]._animName;
		bool animFinished = isAnimationFinished(animName, time);
		bool subFinished = (_countQueuedSubtitles.empty() || _countQueuedSubtitles[_anims[i]._subtitleID] == 0);
		bool stopped = stopVideo && _anims[i]._skippable;

		if (stopped) {
			g_system->getMixer()->stopHandle(_anims[i]._soundHandle);
			if (_anims[i]._keepLastFrame)
				for (Common::SortedArray<Layer>::iterator it = _layers.begin(); it != _layers.end(); it++)
					if (it->name == _anims[i]._animName)
						it->renderable->selectFrame(-1);
		}

		if ((soundFinished && animFinished && subFinished) || stopped) {
			_anims[i]._finished = true;
			if (!_anims[i]._keepLastFrame)
				setLayerEnabled(animName, false);
			g_vm->handleEvent(_anims[i]._callbackEvent);
			// Release the reference
			_anims[i]._callbackEvent = EventHandlerWrapper();
		}
	}

	Common::Point viewPoint = Common::Point(_pan, 0) + kZeroPoint + _viewportOffset;

	if (_heroBeltEnabled) {
		belt->render(context, time, viewPoint);
	}

	context->fade(_finalFade);
	if (!_subtitles.empty())
		context->renderSubtitle(_subtitles.front().line, viewPoint);
	
	context->renderToScreen(viewPoint);
}

void VideoRoom::panLeftAnim(EventHandlerWrapper callback) {
	_panSpeed = -6; // TODO: check this speed
	_panCallback = callback;
}

void VideoRoom::panRightAnim(EventHandlerWrapper callback) {
	_panSpeed = +6; // TODO: check this speed
	_panCallback = callback;
}

void VideoRoom::panRightInstant() {
	_pan = 640;
}

void VideoRoom::addStaticLayer(const LayerId &name, int zValue, Common::Point offset) {
	PodFile pf2(name.getDebug());
	if (!pf2.openStore(Common::SharedPtr<Common::SeekableReadStream>(openFile(mapAsset(name) + ".pod")))) {
		debug("Animation %s isn't found", name.getDebug().c_str());
		return;
	}
	for (int idx = 1; ; idx++) {
		PodImage pi;
		if (!pi.loadImage(pf2, idx)) {
			break;
		}
		Common::Array <PodImage> arr;
		arr.push_back(pi);
		addLayer(new Renderable(arr), name, zValue, true, offset);
	}
}

void VideoRoom::addAnimLayerInternal(const LayerId &name, int zValue, Common::Point offset) {
	Common::SharedPtr<Common::SeekableReadStream> rs(openFile(mapAsset(name) + ".pod"));
	if (!rs) {
		debug("Animation %s isn't found", name.getDebug().c_str());
		return;
	}

	PodFile pf2(name.getDebug());
	pf2.openStore(rs);

	addLayer(new Renderable(pf2.loadImageArray()), name, zValue, false,
		 offset);	
}

Renderable::Renderable(Common::Array<PodImage> images) {
	_images = images;
	_msperframe = kDefaultSpeed;
	_loop = false;
	_first = 0;
	_last = 0;
	_startms = 0;
}

int Renderable::getLen() {
	return _first < _last ? _last - _first + 1 : _first - _last + 1;
}

int math_mod(int a, int b) {
	int m = a % b;
	while (m < 0)
		m += ABS(b);
	return m;
}

const PodImage &Renderable::getFrame(int time) {
	return _images[getAnimationFrameNum(time)];
}

int Renderable::getAnimationFrameNum(int time) {
	int f = ((time - _startms) / _msperframe);
	if (f < 0)
		f = 0;
	if (_loop)
		f %= getLen();
	if (f >= getLen())
		f = getLen() - 1;
	if (_first <= _last)
		return math_mod(_first + f, _images.size());
	else
		return math_mod(_first - f, _images.size());
}

void Renderable::startAnimation(int startms, int msperframe, bool loop, int first, int last) {
	_loop = loop;
	_msperframe = msperframe;
	_startms = startms;
	_first = math_mod(first, _images.size());
	_last = math_mod(last, _images.size());
}

void Renderable::selectFrame(int frame) {
	_loop = true;
	_msperframe = kDefaultSpeed;
	_startms = 0;
	frame = math_mod(frame, _images.size());
	_first = frame;
	_last = frame;
}

bool Renderable::isAnimationFinished(int time) {
	return !_loop && time > _startms + getLen() * _msperframe;
}

void VideoRoom::loadHotZones(const Common::String &hotzoneFile, bool enable, Common::Point offset) {
	Common::SharedPtr<Common::SeekableReadStream> hzFile(openFile(hotzoneFile));
	if (!hzFile) {
		debug("Couldn't open %s", hotzoneFile.c_str());
		return;
	}

	_hotZones.readHotzones(hzFile, enable, offset);
}

void VideoRoom::pushHotZones(const Common::String &hotzoneFile, bool enable,
			     Common::Point offset) {
	// TODO: optimize this? maybe. Or maybe not
	_hotZoneStack.push_back(_hotZones);
	_hotZones = HotZoneArray();
	loadHotZones(hotzoneFile, enable, offset);
}

void VideoRoom::popHotZones() {
	// TODO: optimize this? maybe. Or maybe not
	_hotZones = _hotZoneStack.back();
	_hotZoneStack.pop_back();
}

int VideoRoom::getCursor() {
	return _cursor;
}

PodImage *VideoRoom::getDragged() {
	if (_isDragging)
		return &_draggedImage[_draggingPtr];
	else
		return nullptr;
}

void VideoRoom::clearDrag() {
	_isDragging = false;
}

void VideoRoom::drag(const Common::String &name, int frame, Common::Point hotspot) {
	Common::SharedPtr<Common::SeekableReadStream> rs(openFile(mapAsset(name) + ".pod"));
	if (!rs) {
		debug("Animation %s isn't found", name.c_str());
		return;
	}

	PodFile pf2(name);
	pf2.openStore(rs);
	PodImage pi;

	pi.loadImage(pf2, 1 + frame);
	pi.setHotspot(hotspot);

	_draggingPtr = (_draggingPtr + 1) % ARRAYSIZE(_draggedImage);

	_draggedImage[_draggingPtr] = pi;
	_isDragging = true;
}

void VideoRoom::cancelVideo() {
	if (_videoDecoder) {
		_videoDecoder.reset();
		_videoDecoderEndEvent = 0;
		_videoW = 0;
		_videoH = 0;
	}
}

void VideoRoom::playVideo(const Common::String &name, int zValue,
			     EventHandlerWrapper callbackEvent,
			     Common::Point offset) {
	cancelVideo();
	Common::SharedPtr<Video::SmackerDecoder> decoder
	  = Common::SharedPtr<Video::SmackerDecoder>(new Video::SmackerDecoder());

	Common::File *file = new Common::File;
	Common::String mappedName = _assetMap.get(name, 1);
	if (mappedName == "") {
		mappedName = name;
	}
	if (!file->open(_smkPath + "/" + mappedName + ".SMK") || !decoder->loadStream(file)) {
		debug("Video file %s can't be opened", name.c_str());
		g_vm->handleEvent(callbackEvent);
		return;
	}

	decoder->start();
	_videoDecoder = decoder;
	_videoDecoderEndEvent = callbackEvent;
	_videoOffset = offset;
	_videoZ = zValue;
}

Common::SeekableReadStream *VideoRoom::openFile(const Common::String &name) {
	Common::SeekableReadStream *stream = _podFile->getFileStream(name);
	if (stream)
		return stream;
	return g_vm->getWdPodFile()->getFileStream(name);
}

Common::String VideoRoom::mapAsset(const Common::String &name) {
	Common::String mappedName = _assetMap.get(name, 1);
	if (mappedName == "") {
		return name;
	}
	return mappedName;
}

Common::String VideoRoom::mapAsset(const LayerId &name) {
	return mapAsset(name.getFilename());
}

Audio::RewindableAudioStream *VideoRoom::getAudioStream(const Common::String &soundName) {
	Common::SeekableReadStream *stream = openFile(
		mapAsset(soundName) + ".aif");
	if (!stream) {
		debug("Audio %s isn't found", soundName.c_str());
		return nullptr;
	}
	return Audio::makeAIFFStream(stream, DisposeAfterUse::YES);
}

void VideoRoom::playSoundInternal(const Common::String &soundName, EventHandlerWrapper callbackEvent, bool loop,
				  bool skippable, Audio::Mixer::SoundType soundType, int subtitleID) {
	Audio::RewindableAudioStream *rewSoundStream;
	Audio::AudioStream *soundStream;
	Animation anim;
	g_system->getMixer()->stopHandle(anim._soundHandle);
	rewSoundStream = getAudioStream(soundName);
	soundStream = loop ? Audio::makeLoopingAudioStream(rewSoundStream, 0) : rewSoundStream;
	anim._animName = soundName;
	anim._callbackEvent = callbackEvent;
	anim._finished = false;
	anim._keepLastFrame = false;
	anim._skippable = skippable;
	anim._subtitleID = subtitleID;
	g_system->getMixer()->playStream(soundType, &anim._soundHandle, soundStream,
					 -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	_anims.push_back(anim);
}

void VideoRoom::playSFX(const Common::String &soundName, EventHandlerWrapper callbackEvent) {
	playSoundInternal(soundName, callbackEvent, false, false, Audio::Mixer::kSFXSoundType);
}

void VideoRoom::playMusic(const Common::String &soundName, EventHandlerWrapper callbackEvent) {
	playSoundInternal(soundName, callbackEvent, false, false, Audio::Mixer::kMusicSoundType);
}

void VideoRoom::playSpeech(const TranscribedSound &sound,
				    EventHandlerWrapper callbackEvent) {
	int subID = g_vm->genSubtitleID();
	playSoundInternal(sound.soundName, callbackEvent, false, true, Audio::Mixer::kSpeechSoundType, subID);
	playSubtitles(sound.transcript, subID);
}

void VideoRoom::playSFXLoop(const Common::String &soundName) {
	playSoundInternal(soundName, EventHandlerWrapper(), true, false, Audio::Mixer::kSFXSoundType);
}

void VideoRoom::playMusicLoop(const Common::String &soundName) {
	playSoundInternal(soundName, EventHandlerWrapper(), true, false, Audio::Mixer::kMusicSoundType);
}

void VideoRoom::playAnimWithSoundInternal(const LayerId &animName,
					  const Common::String &soundName,
					  Audio::Mixer::SoundType soundType,
					  int zValue,
					  PlayAnimParams params,
					  EventHandlerWrapper callbackEvent,
					  Common::Point offset, int subtitleID) {
	Audio::AudioStream *soundStream;

	if (!doesLayerExist(animName)) {
		addAnimLayerInternal(animName, zValue);
	}

	startAnimationInternal(animName, zValue, params.getSpeed(), params.isLoop(),
			       false, params.getFirstFrame(), params.getLastFrame(), offset);
	Animation anim;
	g_system->getMixer()->stopHandle(anim._soundHandle);
	if (params.isLoop()) {
		soundStream = Audio::makeLoopingAudioStream(getAudioStream(soundName), 0);
	} else {
		soundStream = getAudioStream(soundName);
	}
	anim._animName = animName;
	anim._callbackEvent = callbackEvent;
	anim._finished = false;
	anim._keepLastFrame = params.getKeepLastFrame();
	anim._skippable = false;
	anim._subtitleID = subtitleID;
	g_system->getMixer()->playStream(soundType, &anim._soundHandle, soundStream,
					 -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	_anims.push_back(anim);
}

void VideoRoom::playAnimWithSpeech(const LayerId &animName,
				   const TranscribedSound &sound,
				   int zValue,
				   PlayAnimParams params,
				   EventHandlerWrapper callbackEvent,
				   Common::Point offset) {
	int subID = g_vm->genSubtitleID();
	playAnimWithSoundInternal(animName, sound.soundName, Audio::Mixer::kSpeechSoundType, zValue, params, callbackEvent, offset, subID);
	playSubtitles(sound.transcript, subID);
}

void VideoRoom::playAnimWithSFX(const LayerId &animName,
				const Common::String &soundName,
				int zValue,
				PlayAnimParams params,
				EventHandlerWrapper callbackEvent,
				Common::Point offset) {
	playAnimWithSoundInternal(animName, soundName, Audio::Mixer::kSFXSoundType, zValue, params, callbackEvent, offset);
}

void VideoRoom::playAnimWithMusic(const LayerId &animName,
				  const Common::String &soundName,
				  int zValue,
				  PlayAnimParams params,
				  EventHandlerWrapper callbackEvent,
				  Common::Point offset) {
	playAnimWithSoundInternal(animName, soundName, Audio::Mixer::kMusicSoundType, zValue, params, callbackEvent, offset);
}

void VideoRoom::playAnim(const LayerId &animName, int zValue,
			 PlayAnimParams params,
			 EventHandlerWrapper callbackEvent,
			 Common::Point offset) {
	if (!doesLayerExist(animName)) {
		addAnimLayerInternal(animName, zValue);
	}

	startAnimationInternal(animName, zValue, params.getSpeed(), params.isLoop(), false,
			       params.getFirstFrame(), params.getLastFrame(), offset);
	Animation anim;
	anim._callbackEvent = callbackEvent;
	anim._finished = false;
	anim._keepLastFrame = params.getKeepLastFrame();
	anim._animName = animName;
	anim._skippable = false;
	_anims.push_back(anim);
}

void VideoRoom::playAnimLoop(const LayerId &animName, int zValue, Common::Point offset) {
	playAnim(animName, zValue, PlayAnimParams::loop(), EventHandlerWrapper(), offset);
}

void VideoRoom::playAnimKeepLastFrame(const LayerId &animName, int zValue, EventHandlerWrapper callbackEvent,
				      Common::Point offset) {
	playAnim(animName, zValue, PlayAnimParams::keepLastFrame(), callbackEvent, offset);
}

void VideoRoom::setUserPanCallback(EventHandlerWrapper leftStart,
				   EventHandlerWrapper leftEnd,
				   EventHandlerWrapper rightStart,
				   EventHandlerWrapper rightEnd) {
	_userPanStartLeftCallback = leftStart;
	_userPanEndLeftCallback = leftEnd;
	_userPanStartRightCallback = rightStart;
	_userPanEndRightCallback = rightEnd;
}

class StatuePlayEnd : public EventHandler {
public:
	void operator()() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnim(_animName, _zValue,
			       PlayAnimParams::disappear().partial(_lastLoopFrame, -1),
			       EventHandlerWrapper(), _offset);
		room->enableMouse();
	}

	StatuePlayEnd(const LayerId &animName, int zValue, Common::Point offset, int lastLoopFrame) {
		_animName = animName;
		_zValue = zValue;
		_offset = offset;
		_lastLoopFrame = lastLoopFrame;
	}
private:
	LayerId _animName;
	int _zValue;
	int _lastLoopFrame;
	Common::Point _offset;
};

class StatuePlayTwoThirdsLoop : public EventHandler {
public:
	void operator()() override {
		Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
		room->playAnim(_animName, _zValue,
			       PlayAnimParams::loop().partial(_firstLoopFrame, _lastLoopFrame),
			       EventHandlerWrapper(), _offset);
	}

	StatuePlayTwoThirdsLoop(const LayerId &animName, int zValue, Common::Point offset,
				int firstLoopFrame, int lastLoopFrame) {
		_animName = animName;
		_zValue = zValue;
		_offset = offset;
		_firstLoopFrame = firstLoopFrame;
		_lastLoopFrame = lastLoopFrame;
	}
private:
	LayerId _animName;
	int _zValue;
	int _firstLoopFrame, _lastLoopFrame;
	Common::Point _offset;
};

void VideoRoom::playStatueSMK(StatueId id, const LayerId &animName, int zValue,
			      const Common::Array<Common::String> &smkNames,
			      int startOfLoop, int startOfEnd,
			      Common::Point offset) {
	int phase = g_vm->getPersistent()->_statuePhase[id] % smkNames.size();
	playVideo(smkNames[phase], zValue,
		  Common::SharedPtr<EventHandler>(new StatuePlayEnd(animName, zValue, offset, startOfEnd)));
	if (!doesLayerExist(animName)) {
		addAnimLayerInternal(animName, zValue);
		stopAnim(animName);
	}
	playAnim(animName, zValue, PlayAnimParams::keepLastFrame().partial(0, startOfEnd - 1),
		 Common::SharedPtr<EventHandler>(new StatuePlayTwoThirdsLoop(animName, zValue, offset, startOfLoop, startOfEnd - 1)), offset);

	g_vm->getPersistent()->_statuesTouched[id] = true;
	g_vm->getPersistent()->_statuePhase[id] = (phase + 1) % smkNames.size();
	disableMouse();
}

void VideoRoom::resetFade() {
	_finalFade = 0x100;
	_finalFadeSpeed = 0;
	_finalFadeCallback = EventHandlerWrapper();
}

void VideoRoom::resetLayers() {
	_layers.clear();
}

void VideoRoom::hideString(const Common::String &font, size_t maxLen, const Common::String &extraId) {
	Common::SharedPtr<VideoRoom> room = g_vm->getVideoRoom();
	for (unsigned i = 0; i < maxLen; i++) {
		room->stopAnim(LayerId(font, i, extraId));
	}
}

void VideoRoom::renderString(const Common::String &font, const Common::U32String &str, Common::Point startPos, int zVal, int fontDelta, const Common::String &extraId) {
	Common::Point curPos = startPos;
	bool small = font == "smallascii";
	for (unsigned i = 0; i < str.size(); i++) {
		uint32 c = str[i];
		if (c == ' ') {
			curPos += Common::Point(small ? 6 : 20, 0);
			continue;
		}
		LayerId l(font, i, extraId);
		selectFrame(l, zVal, c + fontDelta, curPos);
		PodImage pi(getLayerFrame(l));
		curPos += Common::Point(pi.getWidth() + pi.getOffset().x + (small ? 1 : 3), 0);
	}
}

void VideoRoom::loadFontWidth(const Common::String &font) {
	if (_fontWidths.contains(font)) {
		return;
	}

	Common::SharedPtr<Common::SeekableReadStream> rs(openFile(mapAsset(font) + ".pod"));
	if (!rs) {
		_fontWidths[font].clear();
		debug("Animation %s isn't found", font.c_str());
		return;
	}

	PodFile pf2(font);
	pf2.openStore(rs);

	Common::Array <PodImage> pi = pf2.loadImageArray();
	bool small = font == "smallascii";

	for (unsigned i = 0; i < pi.size(); i++)
		_fontWidths[font].push_back(pi[i].getWidth()+pi[i].getOffset().x + (small ? 1 : 3));
}

int VideoRoom::computeStringWidth(const Common::String &font, const Common::U32String &str, int fontDelta) {
	int width = 0;
	bool small = font == "smallascii";
	if (!_fontWidths.contains(font)) {
		loadFontWidth(font);
	}
	for (unsigned i = 0; i < str.size(); i++) {
		uint32 c = str[i];
		if (c == ' ') {
			width += small ? 6 : 20;
			continue;
		}
		if (c + fontDelta < _fontWidths[font].size())
			width += _fontWidths[font][c + fontDelta];
	}

	return width;
}

void VideoRoom::renderStringCentered(const Common::String &font, const Common::U32String &str, Common::Point centerPos, int zVal, int fontDelta, const Common::String &extraId) {
	int width = computeStringWidth(font, str, fontDelta);
	renderString(font, str, centerPos - Common::Point(width / 2, 0), zVal, fontDelta, extraId);
}

void VideoRoom::playSubtitles(const char *text, int subID) {
	int delay = g_vm->getSubtitleDelayPerChar();
	if (delay <= 0)
		return;
	Common::U32String s = g_vm->translate(text);
	Common::Array<Common::U32String> lines;
	int32 countTime = g_vm->getCurrentTime();
	g_vm->wrapSubtitles(s, lines);
	for (uint i = 0; i < lines.size(); i++) {
		SubtitleLine l;
		l.line = lines[i];
		l.ID = subID;
		countTime += delay * MAX<uint>(l.line.size(), 20);
		l.maxTime = countTime;
		_subtitles.push(l);
		_countQueuedSubtitles[subID]++;
	} 
}
}
