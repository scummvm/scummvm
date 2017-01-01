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
 */

#ifndef DIRECTOR_SCORE_H
#define DIRECTOR_SCORE_H

#include "common/substream.h"
#include "common/rect.h"

namespace Graphics {
	class ManagedSurface;
	class Font;
}

namespace Director {

class Archive;
struct CastInfo;
class DirectorEngine;
class DirectorSound;
class Frame;
struct Label;
class Lingo;
class Sprite;

enum ScriptType {
	kMovieScript = 0,
	kSpriteScript = 1,
	kFrameScript = 2,
	kNoneScript = -1,
	kMaxScriptType = 2
};

class Score {
public:
	Score(DirectorEngine *vm, Archive *);
	~Score();

	static Common::Rect readRect(Common::ReadStreamEndian &stream);
	static int compareLabels(const void *a, const void *b);
	void loadArchive();
	void setStartToLabel(Common::String label);
	void gotoloop();
	void gotonext();
	void gotoprevious();
	void startLoop();
	void processEvents();
	Archive *getArchive() const { return _movieArchive; };
	void loadConfig(Common::SeekableSubReadStreamEndian &stream);
	void loadCastDataVWCR(Common::SeekableSubReadStreamEndian &stream);
	void loadCastData(Common::SeekableSubReadStreamEndian &stream, uint16 id);
	void setCurrentFrame(uint16 frameId) { _currentFrame = frameId; }
	int getCurrentFrame() { return _currentFrame; }
	Common::String getMacName() const { return _macName; }
	Sprite *getSpriteById(uint16 id);
	void setSpriteCasts();

private:
	void update();
	void readVersion(uint32 rid);
	void loadPalette(Common::SeekableSubReadStreamEndian &stream);
	void loadFrames(Common::SeekableSubReadStreamEndian &stream);
	void loadLabels(Common::SeekableSubReadStreamEndian &stream);
	void loadActions(Common::SeekableSubReadStreamEndian &stream);
	void loadCastInfo(Common::SeekableSubReadStreamEndian &stream, uint16 id);
	void loadScriptText(Common::SeekableSubReadStreamEndian &stream);
	void loadFileInfo(Common::SeekableSubReadStreamEndian &stream);
	void loadFontMap(Common::SeekableSubReadStreamEndian &stream);
	void dumpScript(const char *script, ScriptType type, uint16 id);
	Common::String getString(Common::String str);
	Common::Array<Common::String> loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader = true);

public:
	Common::Array<Frame *> _frames;
	Common::HashMap<int, Cast *> _casts;
	Common::HashMap<uint16, CastInfo *> _castsInfo;
	Common::SortedArray<Label *> *_labels;
	Common::HashMap<uint16, Common::String> _actions;
	Common::HashMap<uint16, Common::String> _fontMap;
	Graphics::ManagedSurface *_surface;
	Graphics::ManagedSurface *_trailSurface;
	Graphics::Font *_font;
	Archive *_movieArchive;
	Common::Rect _movieRect;

private:
	uint16 _versionMinor;
	uint16 _versionMajor;
	Common::String _macName;
	Common::String _createdBy;
	Common::String _changedBy;
	Common::String _script;
	Common::String _directory;
	byte _currentFrameRate;
	uint16 _castArrayStart;
	uint16 _currentFrame;
	Common::String _currentLabel;
	uint32 _nextFrameTime;
	uint32 _flags;
	bool _stopPlay;
	uint16 _castArrayEnd;
	uint16 _movieScriptCount;
	uint16 _stageColor;
	Lingo *_lingo;
	DirectorSound *_soundManager;
	DirectorEngine *_vm;
};

} // End of namespace Director

#endif
