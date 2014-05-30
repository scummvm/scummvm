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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_H
#define PRINCE_H

#include "common/random.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/events.h"

#include "image/bmp.h"

#include "gui/debugger.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "audio/mixer.h"

#include "video/flic_decoder.h"

#include "prince/mob.h"
#include "prince/object.h"


namespace Prince {

struct PrinceGameDescription;

class PrinceEngine;
class GraphicsMan;
class Script;
class Interpreter;
class InterpreterFlags;
class Debugger;
class MusicPlayer;
class VariaTxt;
class Cursor;
class MhwanhDecoder;
class Font;
class Hero;
class Animation;
class Room;

struct Text {
	const char *_str;
	uint16 _x, _y;
	uint16 _time;
	uint32 _color;

	Text() : _str(NULL), _x(0), _y(0), _time(0), _color(255){
	}
};

struct AnimListItem {
	uint16 _type; // type of animation - for background anims RND of frame
	uint16 _fileNumber;
	uint16 _startPhase; // first phase number
	uint16 _endPhase;
	uint16 _loopPhase;
	int16 _x;
	int16 _y;
	uint16 _loopType;
	uint16 _nextAnim; // number of animation to do for loop = 3
	uint16 _flags; // byte 0 - draw overlays, byte 1 - draw in front of overlay, byte 2 - load but turn off drawing
	bool loadFromStream(Common::SeekableReadStream &stream);
};

struct BAS {
	int32 _type; // type of sequence
	int32 _data; // additional data
	int32 _anims; // number of animations
	int32 _current; // actual number of animation
	int32 _counter; // time counter for animation
	int32 _currRelative; //actual relative number for animation
	int32 _data2; // additional data for measurements
};

struct BASA {
	int16 _num;	// animation number
	int16 _start;	// initial frame
	int16 _end;	// final frame
	int16 _pad;	// fulfilment to 8 bytes
};

// background and normal animation
struct Anim {
	BASA _basaData;
	int32 _addr; //animation adress
	int16 _usage;
	int16 _state; // state of animation: 0 - turning on, 1 - turning off
	int16 _flags;
	int16 _frame; // number of phase to show
	int16 _lastFrame; // last phase
	int16 _loopFrame; // first frame of loop
	int16 _showFrame; // actual visible frame of animation
	int16 _loopType;	 // type of loop (0 - last frame; 1 - normal loop (begin from _loopFrame); 2 - no loop; 3 - load new animation)
	int16 _nextAnim; // number of next animation to load after actual
	int16 _x;
	int16 _y;
	int32 _currFrame;
	int16 _currX;
	int16 _currY;
	int16 _currW;
	int16 _currH;
	int16 _packFlag;
	int32 _currShadowFrame;
	int16 _packShadowFlag;
	int32 _shadowBack;
	int16 _relX;
	int16 _relY;
	Animation *_animData;
	Animation *_shadowData;
};

struct BackgroundAnim {
	BAS _seq;
	Common::Array<Anim> backAnims;
};

struct DebugChannel {

enum Type {
	kScript,
	kEngine 
};

};

class PrinceEngine : public Engine {
protected:
	Common::Error run();

public:
	PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc);
	virtual ~PrinceEngine();

	virtual bool hasFeature(EngineFeature f) const;

	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	const PrinceGameDescription *_gameDescription;
	Video::FlicDecoder _flicPlayer;
	VariaTxt *_variaTxt;

	uint32 _talkTxtSize;
	byte *_talkTxt;

	bool loadLocation(uint16 locationNr);
	bool loadAnim(uint16 animNr, bool loop);
	bool loadVoice(uint32 textSlot, uint32 sampleSlot, const Common::String &name);
	bool loadSample(uint32 sampleSlot, const Common::String &name);
	bool loadShadow(byte *shadowBitmap, uint32 dataSize, const char *resourceName1, const char *resourceName2);

	void playSample(uint16 sampleId, uint16 loopType);
	void stopSample(uint16 sampleId);

	virtual GUI::Debugger *getDebugger();

	void changeCursor(uint16 curId);
	void printAt(uint32 slot, uint8 color, const char *s, uint16 x, uint16 y);

	static const uint8 MAXTEXTS = 32;
	Text _textSlots[MAXTEXTS];

	uint64 _frameNr;
	Hero *_mainHero;
	Hero *_secondHero;

	uint16 _cameraX;
	uint16 _newCameraX;
	uint16 _sceneWidth;
	int32 _picWindowX;
	int32 _picWindowY;
	Image::BitmapDecoder *_roomBmp;

	Common::Array<AnimListItem> _animList;
	//Common::Array<Anim> _backAnimList;
	Common::Array<BackgroundAnim> _backAnimList;

	Common::RandomSource _randomSource;

	static const int16 kNormalWidth = 640;
	static const int16 kNormalHeight = 480;

	int testAnimNr;
	int testAnimFrame;

private:
	bool playNextFrame();
	void keyHandler(Common::Event event);
	void hotspot();
	void scrollCameraRight(int16 delta);
	void scrollCameraLeft(int16 delta);
	void drawScreen();
	void showTexts();
	void init();
	void showLogo();
	void showBackAnims();
	bool spriteCheck(Graphics::Surface *backAnimSurface, int destX, int destY);
	void showSprite(Graphics::Surface *backAnimSurface, int destX, int destY);
	void showSpriteShadow(Graphics::Surface *shadowSurface, int destX, int destY);
	void makeShadowTable(int brightness);

	uint32 getTextWidth(const char *s);
	void debugEngine(const char *s, ...);

	uint16 _locationNr;
	uint8 _cursorNr;

	Common::RandomSource *_rnd;
	Cursor *_cursor1;
	Cursor *_cursor2;
	MhwanhDecoder *_suitcaseBmp;
	Debugger *_debugger;
	GraphicsMan *_graph;
	Script *_script;
	InterpreterFlags *_flags;
	Interpreter *_interpreter;
	Font *_font;
	MusicPlayer *_midiPlayer;
	Room *_room;

	static const uint32 MAX_SAMPLES = 60;	
	Common::SeekableReadStream *_voiceStream[MAX_SAMPLES];
	Audio::SoundHandle _soundHandle[MAX_SAMPLES];

	Animation *_zoom;
	Common::Array<Mob> _mobList;
	Common::Array<Object *> _objList;

	bool _flicLooped;
	
	void mainLoop();

};

} // End of namespace Prince

#endif

/* vim: set tabstop=4 noexpandtab: */
