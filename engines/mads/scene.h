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

#ifndef MADS_SCENE_H
#define MADS_SCENE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "mads/assets.h"

namespace MADS {

enum {
	VERB_LOOK        = 3,
	VERB_TAKE        = 4,
	VERB_PUSH        = 5,
	VERB_OPEN        = 6,
	VERB_PUT         = 7,
	VERB_TALKTO      = 8,
	VERB_GIVE        = 9,
	VERB_PULL        = 10,
	VERB_CLOSE       = 11,
	VERB_THROW       = 12,
	VERB_WALKTO      = 13
};

class VerbInit {
public:
	int _id;
	int _action1;
	int _action2;

	VerbInit() {}
	VerbInit(int id, int action1, int action2): _id(id), _action1(action1), _action2(action2) {}
};

enum SpriteType {
	ST_NONE = 0, ST_FOREGROUND = 1, ST_BACKGROUND = -4, 
	ST_FULL_SCREEN_REFRESH = -2, ST_EXPIRED = -1
};

class SpriteSlot {
public:
	SpriteType _spriteType;
	int _seqIndex;
	int _spriteListIndex;
	int _frameNumber;
	Common::Point _position;
	int _depth;
	int _scale;
public:
	SpriteSlot();
	SpriteSlot(SpriteType type, int seqIndex);
};

class TextDisplay {
public:
	bool _active;
	int _spacing;
	Common::Rect _bounds;
	int _expire;
	int _col1;
	int _col2;
	Common::String _fontName;
	Common::String _msg;

	TextDisplay();
};

class DynamicHotspot {
public:
	int _seqIndex;
	Common::Rect _bounds;
	Common::Point _feetPos;
	int _facing;
	int _descId;
	int _field14;
	int _articleNumber;
	int _cursor;

	DynamicHotspot();
};

class SequenceEntry {
public:
	int _spriteListIndex;
	int _flipped;
	int _frameIndex;
	int _frameStart;
	int _numSprites;
	int _animType;
	int _frameInc;
	int _depth;
	int _scale;
	int _dynamicHotspotIndex;
	
	Common::Point _msgPos;

	int _triggerCountdown;
	bool _doneFlag;
	struct {
		int _count;
		int _mode[5];
		int _frameIndex[5];
		int _abortVal[5];
	} _entries;
	int _abortMode;
	int _actionNouns[3];
	int _numTicks;
	int _extraTicks;
	int _timeout;

	SequenceEntry();
};

class KernelMessage {
public:
	int _flags;
	int _seqInex;
	char _asciiChar;
	char _asciiChar2;
	int _colors;
	Common::Point _posiition;
	int _msgOffset;
	int _numTicks;
	int _frameTimer2;
	int _frameTimer;
	int _timeout;
	int _field1C;
	int _abortMode;
	int _nounList[3];
	Common::String _msg;

	KernelMessage();
};

class Hotspot {
public:
	Common::Rect _bounds;
	Common::Point _feetPos;
	int _facing;
	int _articleNumber;
	int _cursor;
	int _vocabId;
	int _verbId;

	Hotspot();
	Hotspot(Common::SeekableReadStream &f);
};

#define SPRITE_COUNT 50
#define TEXT_DISPLAY_COUNT 40
#define DYNAMIC_HOTSPOT_COUNT 8

class MADSEngine;
class Scene;

class SceneLogic {
protected:
	Scene *_scene;
public:
	/**
	 * Constructor
	 */
	SceneLogic(Scene *scene): _scene(scene) {}

	/**
	 * Called to initially setup a scene
	 */
	virtual void setup() = 0;

	/**
	 * Called as the scene is entered (made active)
	 */
	virtual void enter() = 0;

	/**
	 * Called one per frame
	 */
	virtual void step() = 0;

	/**
	 * Called before an action is started
	 */
	virtual void preActions() = 0;

	/**
	 * Handles scene actions
	 */
	virtual void actions() = 0;

	/**
	 * Post-action handling
	 */
	virtual void postActions() = 0;
};

class Scene {
private:
	/**
	 * Free the voculary list buffer
	 */
	void freeVocab();

	/**
	 * Return the index of a given Vocab in the active vocab list
	 */
	int activeVocabIndexOf(int vocabId);

protected:
	MADSEngine *_vm;
public:
	SceneLogic *_sceneLogic;
	int _priorSectionNum;
	int _sectionNum;
	int _sectionNumPrior;
	int _priorSceneId;
	int _nextSceneId;
	int _currentSceneId;
	Common::Array<VerbInit> _verbList;
	TextDisplay _textDisplay[TEXT_DISPLAY_COUNT];
	Common::Array<SpriteSlot> _spriteSlots;
	Common::Array<SpriteAsset *> _spriteList;
	int _spriteListIndex;
	Common::Array<DynamicHotspot> _dynamicHotspots;
	bool _dynamicHotspotsChanged;
	byte *_vocabBuffer;
	Common::Array<int> _activeVocabs;
	Common::Array<SequenceEntry> _sequenceList;
	Common::Array<KernelMessage> _messageList;
	Common::String _talkFont;
	int _textSpacing;
	Common::Array<Hotspot> _hotspotList;

	/**
	 * Constructor
	 */
	Scene(MADSEngine *vm);

	/**
	 * Destructor
	 */
	~Scene();

	/**
	 * Initialise the sprite data
	 * @param flag		Also reset sprite list
	 */
	void clearSprites(bool flag);

	/**
	 * Delete any sprites used by the player
	 */
	void releasePlayerSprites();

	/**
	 * Delete a sprite entry
	 */
	void deleteSpriteEntry(int listIndex);

	/**
	 * Clear the dynamic hotspot list
	 */
	void clearDynamicHotspots();

	/**
	 * Clear the vocabulary list
	 */
	void clearVocab();

	/**
	 * Add a given vocab entry to the active list
	 */
	void addActiveVocab(int vocabId);

	/**
	 * Clear the sequence list
	 */
	void clearSequenceList();

	/**
	 * Clear the message list
	 */
	void clearMessageList();

	/**
	 * Loads the scene logic for a given scene
	 */
	void loadSceneLogic();

	/**
	 * Loads the hotstpots for the scene
	 */
	void loadHotspots();

	/**
	 * Loads the vocab list
	 */
	void loadVocab();

	/**
	 * Clear the data for the scene
	 */
	void free();
};

} // End of namespace MADS

#endif /* MADS_SCENE_H */
