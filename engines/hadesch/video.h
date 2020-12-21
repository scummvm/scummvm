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

#ifndef HADESCH_VIDEOROOM_H
#define HADESCH_VIDEOROOM_H

#include "common/scummsys.h"
#include "common/array.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/rect.h"
#include "common/ptr.h"
#include "hadesch/pod_file.h"
#include "hadesch/pod_image.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "hadesch/enums.h"
#include "hadesch/event.h"
#include "hadesch/hotzone.h"
#include "hadesch/table.h"
#include "common/queue.h"

namespace Video {
class SmackerDecoder;
}

namespace Hadesch {

class PodImage;
class HadeschEngine;
class TagFile;

static const int kDefaultSpeed = 100;

class Renderable {
public:
        Renderable(Common::Array<PodImage> images);
	const PodImage &getFrame(int time);
	void startAnimation(int startms, int msperframe,
			    bool loop, int first, int last);
	bool isAnimationFinished(int time);
	void selectFrame(int frame);
	int getAnimationFrameNum(int time);
	int getNumFrames() {
		return _images.size();
	}

private:
	int getLen();
	Common::Array<PodImage> _images;
	int _msperframe;
	int _startms;
	int _first;
	int _last;
	bool _loop;
};

class LayerId {
public:
	LayerId(const Common::String &name) {
		_name = name;
		_idx = -1;
	}

	LayerId() {
		_idx = -1;
	}

	LayerId(const char *name) {
		_name = name;
		_idx = -1;
	}

	LayerId(const Common::String &name, int idx, const Common::String &qualifier) {
		_qualifier = qualifier;
		_name = name;
		_idx = idx;
	}

	Common::String getFilename() const {
		return _name;
	}

	Common::String getDebug() const;

	bool operator== (const LayerId &b) const;

private:
	Common::String _name;
	int _idx;
	Common::String _qualifier;
};

struct Animation {
	Audio::SoundHandle _soundHandle;
	LayerId _animName;
	EventHandlerWrapper _callbackEvent;
	bool _finished;
	bool _keepLastFrame;
	bool _skippable;
	int _subtitleID;
};

class PlayAnimParams {
public:
	static PlayAnimParams loop();
	static PlayAnimParams keepLastFrame();
	static PlayAnimParams disappear();
	bool getKeepLastFrame();
	bool isLoop();
	int getSpeed();
	int getFirstFrame();
	int getLastFrame();
	PlayAnimParams partial(int first, int last) const;
	PlayAnimParams speed(int msperframe) const;
	PlayAnimParams backwards() const;
private:
	PlayAnimParams(bool loop, bool keepLastFrame);
	bool _loop;
	bool _keepLastFrame;
	int _firstFrame;
	int _lastFrame;
	int _msperframe;
};

struct TranscribedSound {
	const char *soundName;
	const char *transcript;

	static TranscribedSound make(const char *s, const char *t) {
		TranscribedSound res;
		res.soundName = s;
		res.transcript = t;
		return res;
	}
};

class VideoRoom {
public:
	VideoRoom(const Common::String &dir, const Common::String &pod,
		  const Common::String &assetMapFile);
	~VideoRoom();

	void nextFrame(Common::SharedPtr<GfxContext> context, int time, bool stopVideo);
	uint getWidth();
	uint getHeight();
	int getCursor();

	// Hotzones and mouse
	void setHotzoneEnabled(const Common::String &name, bool enabled);
	void enableHotzone(const Common::String &name);
	void disableHotzone(const Common::String &name);
	void pushHotZones(const Common::String &hotzoneFile, bool enable = true,
			  Common::Point offset = Common::Point(0, 0));
	void popHotZones();
	void loadHotZones(const Common::String &hotzoneFile, bool enable = true,
			  Common::Point offset = Common::Point(0, 0));
	void computeHotZone(int time, Common::Point mousePos);
	Common::String getHotZone();
	void setHotZoneOffset(const Common::String &name, Common::Point offset);
	Common::String mapClick(Common::Point mousePos);
	void enableMouse() {
		_mouseEnabled = true;
	}
	void disableMouse() {
		_mouseEnabled = false;
	}
	bool isMouseEnabled() {
		return _mouseEnabled;
	}
	int getCursorAnimationFrame(int time);

	// Animations and layers
	void setLayerEnabled(const LayerId &name, bool enabled);
	void setLayerParallax(const LayerId &name, int val);
	void setColorScale(const LayerId &name, int val);
	void setScale(const LayerId &name, int val);
	int getNumFrames(const LayerId &animName);

	// Main animation API
	void playAnimWithSpeech(const LayerId &animName,
				const TranscribedSound &sound,
				int zValue,
				PlayAnimParams params,
				EventHandlerWrapper callbackEvent = EventHandlerWrapper(),
				Common::Point offset = Common::Point(0, 0));
	void playAnimWithSFX(const LayerId &animName,
			     const Common::String &soundName,
			     int zValue,
			     PlayAnimParams params,
			     EventHandlerWrapper callbackEvent = EventHandlerWrapper(),
			     Common::Point offset = Common::Point(0, 0));
    	void playAnimWithMusic(const LayerId &animName,
			       const Common::String &soundName,
			       int zValue,
			       PlayAnimParams params,
			       EventHandlerWrapper callbackEvent = EventHandlerWrapper(),
			       Common::Point offset = Common::Point(0, 0));
	void playAnim(const LayerId &animName, int zValue,
		      PlayAnimParams params,
		      EventHandlerWrapper callbackEvent = EventHandlerWrapper(),
		      Common::Point offset = Common::Point(0, 0));

	void stopAnim(const LayerId &animName);
	// Like stopAnim but also remove layer altogether
	void purgeAnim(const LayerId &animName);
  	bool isAnimationFinished(const LayerId &name, int time);
	void addStaticLayer(const LayerId &name, int zValue, Common::Point offset = Common::Point(0, 0));
	void selectFrame(const LayerId &name, int zValue, int val, Common::Point offset = Common::Point(0, 0));
	bool doesLayerExist(const LayerId &name);
	PodImage getLayerFrame(const LayerId &name);
  	int getAnimFrameNum(const LayerId &name);
	void dumpLayers();

	// Convenience wrappers
	void playAnimLoop(const LayerId &animName, int zValue, Common::Point offset = Common::Point(0, 0));
	void playAnimKeepLastFrame(const LayerId &animName, int zValue, EventHandlerWrapper callbackEvent = EventHandlerWrapper(),
				   Common::Point offset = Common::Point(0, 0));

	// Videos
	void playVideo(const Common::String &name, int zValue,
		       EventHandlerWrapper callbackEvent = EventHandlerWrapper(),
		       Common::Point offset = Common::Point(0, 0));
	void cancelVideo();
	bool isVideoPlaying();

	// Panning
	void panLeftAnim(EventHandlerWrapper callbackEvent = EventHandlerWrapper());
	void panRightAnim(EventHandlerWrapper callbackEvent = EventHandlerWrapper());
	void panRightInstant();
	void setPannable(bool pannable);
	void setUserPanCallback(EventHandlerWrapper leftStart,
				EventHandlerWrapper leftEnd,
				EventHandlerWrapper rightStart,
				EventHandlerWrapper rightEnd);
	bool isPanLeft() {
		return _pan == 0;
	}

	bool isPanRight() {
		return _pan == 640;
	}

	// Hero belt
	void enableHeroBelt() {
		_heroBeltEnabled = true;
	}
	void disableHeroBelt() {
		_heroBeltEnabled = false;
	}
	bool isHeroBeltEnabled() {
		return _heroBeltEnabled;
	}

	// Font
	void renderString(const Common::String &font, const Common::U32String &str,
			  Common::Point startPos, int zVal, int fontDelta = 0, const Common::String &extraId = "letter");
	void renderStringCentered(const Common::String &font, const Common::U32String &str,
				  Common::Point centerPos, int zVal, int fontDelta = 0, const Common::String &extraId = "letter");
	void hideString(const Common::String &font, size_t maxLen, const Common::String &extraId = "letter");
	int computeStringWidth(const Common::String &font, const Common::U32String &str, int fontDelta = 0);
	
	// Misc
	void playSFX(const Common::String &soundName,
		     EventHandlerWrapper callbackEvent = EventHandlerWrapper());
	void playMusic(const Common::String &soundName,
		       EventHandlerWrapper callbackEvent = EventHandlerWrapper());
	void playSFXLoop(const Common::String &soundName);
	void playMusicLoop(const Common::String &soundName);
	void playSpeech(const TranscribedSound &sound,
				 EventHandlerWrapper callbackEvent = EventHandlerWrapper());
	void playStatueSMK(StatueId id, const LayerId &animName, int zValue,
			   const Common::Array<Common::String> &smkNames,
			   int startOfLoop, int startOfEnd,
			   Common::Point offset = Common::Point(0, 0));
	Common::SeekableReadStream *openFile(const Common::String &name);
	void fadeOut(int ms, const EventHandlerWrapper &callback);
	void resetFade();
	void resetLayers();
	void drag(const Common::String &name, int frame, Common::Point hotspot);
	PodImage *getDragged();
	void clearDrag();
	void pause();
	void unpause();
	void finish();
	void cancelAllSubtitles();
	void setViewportOffset(Common::Point vp) {
		_viewportOffset = vp;
	}

private:
	struct Layer {
		Common::SharedPtr<Renderable> renderable;
		LayerId name;
		Common::Point offset;
		bool isEnabled;
		int genCounter;
		int zValue;
		int parallax;
		int colorScale; // From 0 to 0x100
		int scale; // From 0 to 100
	};

	struct SubtitleLine {
		Common::U32String line;
		int32 maxTime;
		int ID;
	};

	void playAnimWithSoundInternal(const LayerId &animName,
				       const Common::String &soundName,
				       Audio::Mixer::SoundType soundType,
				       int zValue,
				       PlayAnimParams params,
				       EventHandlerWrapper callbackEvent,
				       Common::Point offset,
				       int subID = -1);
	void playSubtitles(const char *text, int subID);
	void addLayer(Renderable *renderable, const LayerId &name,
		      int zValue,
		      bool isEnabled = true, Common::Point offset = Common::Point(0, 0));
	void startAnimationInternal(const LayerId &name, int zValue, int msperframe, bool loop,
				    bool fixedFrame, int first, int last, Common::Point offset);
	Audio::RewindableAudioStream *getAudioStream(const Common::String &soundName);
	Common::String mapAsset(const Common::String &name);
	Common::String mapAsset(const LayerId &name);
	void addAnimLayerInternal(const LayerId &name, int zValue, Common::Point offset = Common::Point(0, 0));
	void playSoundInternal(const Common::String &soundName, EventHandlerWrapper callbackEvent, bool loop,
			       bool skippable, Audio::Mixer::SoundType soundType, int subtitleID = -1);
	static int layerComparator (const Layer &a, const Layer &b);
	void loadFontWidth(const Common::String &font);

	uint _videoW, _videoH;
	Common::Point _videoOffset, _videoSurfOffset;
	Common::SharedPtr<byte> _videoPixels;
	byte _videoPalette[256 * 3];

	HotZoneArray _hotZones;
	Common::Array<HotZoneArray> _hotZoneStack;
	Common::SortedArray<Layer, const Layer&> _layers;
	int _layerGenCounter;

	int _startHotTime;
	int _hotZone;
	int _cursor;
	// We need to keep cursor pointer valid for at
	// least one more frame. Hence use circular buffer
	PodImage _draggedImage[5];
	int _draggingPtr;
	bool _isDragging;
	int _pan, _panSpeed;
	EventHandlerWrapper _panCallback;
	EventHandlerWrapper _userPanStartLeftCallback;
	EventHandlerWrapper _userPanStartRightCallback;
	EventHandlerWrapper _userPanEndLeftCallback;
	EventHandlerWrapper _userPanEndRightCallback;

	bool _pannable;
	bool _leftEdge;
	bool _rightEdge;
	bool _heroBeltEnabled;
	int _edgeStartTime;
	Common::String _smkPath;
	Common::String _podPath;

	Common::SharedPtr<Video::SmackerDecoder> _videoDecoder;
	Common::Point _viewportOffset;
	Common::Array<Animation> _anims;
	EventHandlerWrapper _videoDecoderEndEvent;
	int _videoZ;
	Common::SharedPtr<PodFile> _podFile;
	Common::HashMap<Common::String, Common::Array<int> > _fontWidths;
	Common::Queue<SubtitleLine> _subtitles;
	Common::HashMap<int, int> _countQueuedSubtitles;
	TextTable _assetMap;
	bool _mouseEnabled;

	int _finalFade, _finalFadeSpeed;
	EventHandlerWrapper _finalFadeCallback;
};

static const int kVideoWidth = 640;
static const int kVideoHeight = 480;
#define kOffsetRightRoom (Common::Point(kVideoWidth, 0))
#define kZeroPoint (Common::Point(10, 50))

struct PrePoint {
	int x, y;

	Common::Point get() const {
		return Common::Point(x, y);
	}
};

}
#endif
