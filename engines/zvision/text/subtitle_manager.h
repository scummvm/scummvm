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

#ifndef ZVISION_SUBTITLES_H
#define ZVISION_SUBTITLES_H

#include "audio/mixer.h"
#include "zvision/zvision.h"
#include "zvision/common/focus_list.h"

namespace ZVision {

class ZVision;

class Subtitle {
	friend class SubtitleManager;
public:
	Subtitle(ZVision *engine, const Common::Path &subname, bool vob = false); // For scripted subtitles
	Subtitle(ZVision *engine, const Common::String &str, const Common::Rect &textArea);  // For other text messages
	virtual ~Subtitle();
	bool update(int32 count); // Return true if necessary to redraw
	virtual bool selfUpdate() {
		return false;
	}

protected:
	virtual bool process(int32 deltatime);  // Return true if to be deleted
	ZVision *_engine;
	Common::Rect _textArea;
	int16 _timer; // Always in milliseconds; countdown to deletion
	bool _toDelete;
	bool _redraw;

	int16 _lineId;
	struct Line {
		int start;
		int stop;
		Common::String subStr;
	};
	// NB: start & stop do not always use the same units between different instances of this struct!
	// Sound effect & music subtitles use milliseconds
	// Video subtitle timings are specified in video frames at 15fps, i.e. in multiples of 66.6' milliseconds!
	// AVI videos run at 15fps and can have frames counted directly
	// DVD videos in VOB format run at 29.97 fps and must be converted to work with the subtitle files, which were made for AVI.

	Common::Array<Line> _lines;
};

class AutomaticSubtitle : public Subtitle {
public:
	AutomaticSubtitle(ZVision *engine, const Common::Path &subname, Audio::SoundHandle handle);  // For scripted audio subtitles
	~AutomaticSubtitle() {}

private:
	bool process(int32 deltatime);  // Return true if to be deleted
	bool selfUpdate(); // Return true if necessary to redraw
	Audio::SoundHandle _handle;
};

class SubtitleManager {
public:
	SubtitleManager(ZVision *engine, const ScreenLayout layout, const Graphics::PixelFormat pixelFormat, bool doubleFPS);
	~SubtitleManager();
private:
	ZVision *_engine;
	OSystem *_system;
	RenderManager *_renderManager;
	const Graphics::PixelFormat _pixelFormat;
	const Common::Point _textOffset;  // Position vector of text area origin relative to working window origin
	const Common::Rect _textArea;
	bool _redraw;
	bool _doubleFPS;
	// Internal subtitle ID counter
	uint16 _subId;

	typedef Common::HashMap<uint16, Subtitle *> SubtitleMap;

	// Subtitle list
	SubtitleMap _subsList;
	// Subtitle focus history
	FocusList<uint16> _subsFocus;

public:
	// Update all subtitle objects' deletion timers, delete expired subtitles, & redraw most recent.  Does NOT update any subtitle's count value or displayed string!
	void process(int32 deltatime);  // deltatime is always milliseconds
	// Update counter value of referenced subtitle id & set current line to display, if any.
	void update(int32 count, uint16 subid);  // Count is milliseconds for sound & music; frames for video playback.

	const Common::Point &getTextOffset() const {
		return _textOffset;
	}

	// Create subtitle object and return ID
	uint16 create(const Common::Path &subname, bool vob = false);
	uint16 create(const Common::Path &subname, Audio::SoundHandle handle);  // NB this creates an automatic subtitle
	uint16 create(const Common::String &str);

	// Delete subtitle object by ID
	void destroy(uint16 id);
	void destroy(uint16 id, int16 delay);

	bool askQuestion(const Common::String &str, bool streaming = false, bool safeDefault = false);
	void delayedMessage(const Common::String &str, uint16 milsecs);
	void timedMessage(const Common::String &str, uint16 milsecs);
	void showDebugMsg(const Common::String &msg, int16 delay = 3000);
};

}

#endif
