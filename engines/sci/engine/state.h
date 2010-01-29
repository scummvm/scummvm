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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_INCLUDE_ENGINE_H
#define SCI_INCLUDE_ENGINE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/serializer.h"

namespace Common {
	class SeekableReadStream;
	class WriteStream;
}

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/seg_manager.h"

#include "sci/parser/vocabulary.h"

#include "sci/sound/audio.h"
#ifdef USE_OLD_MUSIC_FUNCTIONS
#include "sci/sound/iterator/core.h"
#endif
#include "sci/sound/soundcmd.h"

namespace Sci {

class SciEvent;
class Menubar;
class SciGui;
class Cursor;
class MessageState;
class SoundCommandParser;

struct GfxState;
struct GfxPort;
struct GfxVisual;
struct GfxContainer;
struct GfxList;


class DirSeeker {
protected:
	reg_t _outbuffer;
	Common::StringList _savefiles;
	Common::StringList::const_iterator _iter;

public:
	DirSeeker() {
		_outbuffer = NULL_REG;
		_iter = _savefiles.begin();
	}

	reg_t firstFile(const Common::String &mask, reg_t buffer, SegManager *segMan);
	reg_t nextFile(SegManager *segMan);
};

enum {
	MAX_SAVE_DIR_SIZE = MAXPATHLEN
};

/** values for EngineState.restarting_flag */
enum {
	SCI_GAME_IS_NOT_RESTARTING = 0,
	SCI_GAME_WAS_RESTARTED = 1,
	SCI_GAME_IS_RESTARTING_NOW = 2,
	SCI_GAME_WAS_RESTARTED_AT_LEAST_ONCE = 4
};

/** Supported languages */
enum kLanguage {
	K_LANG_NONE = 0,
	K_LANG_ENGLISH = 1,
	K_LANG_FRENCH = 33,
	K_LANG_SPANISH = 34,
	K_LANG_ITALIAN = 39,
	K_LANG_GERMAN = 49,
	K_LANG_JAPANESE = 81,
	K_LANG_PORTUGUESE = 351
};

enum FeatureDetection {
	kDetectGfxFunctions = 0,
	kDetectMoveCountType = 1,
	kDetectSoundType = 2,
	kDetectSetCursorType = 3,
	kDetectLofsType = 4,
	kDetectSci21KernelTable = 5
};

class FileHandle {
public:
	Common::String _name;
	Common::SeekableReadStream *_in;
	Common::WriteStream *_out;

public:
	FileHandle();
	~FileHandle();

	void close();
	bool isOpen() const;
};

struct EngineState : public Common::Serializable {
public:
	EngineState(ResourceManager *res, Kernel *kernel, Vocabulary *voc, SegManager *segMan, SciGui *gui, AudioPlayer *audio);
	virtual ~EngineState();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	kLanguage getLanguage();
public:
	ResourceManager *resMan; /**< The resource manager */
	Kernel *_kernel;
	Vocabulary *_voc;

	Common::String _gameId; /**< Designation of the primary object (which inherits from Game) */

	/* Non-VM information */

	SciGui *_gui; /* Currently active Gui */

	SciEvent *_event; // Event handling

	AudioPlayer *_audio;
#ifdef USE_OLD_MUSIC_FUNCTIONS
	SfxState _sound; /**< sound subsystem */
	int sfx_init_flags; /**< flags the sfx subsystem was initialised with */
#endif
	SoundCommandParser *_soundCmd;

	byte restarting_flags; /**< Flags used for restarting */

	uint32 game_start_time; /**< The time at which the interpreter was started */
	uint32 last_wait_time; /**< The last time the game invoked Wait() */

	uint32 _throttleCounter; /**< total times kAnimate was invoked */
	uint32 _throttleLastTime; /**< last time kAnimate was invoked */
	bool _throttleTrigger;

	/* Kernel File IO stuff */

	Common::Array<FileHandle> _fileHandles; /**< Array of file handles. Dynamically increased if required. */

	DirSeeker _dirseeker;

	/* VM Information */

	Common::List<ExecStack> _executionStack; /**< The execution stack */
	/**
	 * When called from kernel functions, the vm is re-started recursively on
	 * the same stack. This variable contains the stack base for the current vm.
	 */
	int execution_stack_base;
	bool _executionStackPosChanged;   /**< Set to true if the execution stack position should be re-evaluated by the vm */

	reg_t r_acc; /**< Accumulator */
	int16 restAdjust; /**< &rest register (only used for save games) */
	reg_t r_prev; /**< previous comparison result */

	SegmentId stack_segment; /**< Heap area for the stack to use */
	StackPtr stack_base; /**< Pointer to the least stack element */
	StackPtr stack_top; /**< First invalid stack element */

	Script *script_000;  /**< script 000, e.g. for globals */

	uint16 currentRoomNumber() const;
	void setRoomNumber(uint16 roomNumber);

	/**
	 * Processes a multilanguage string based on the current language settings and
	 * returns a string that is ready to be displayed.
	 * @param str		the multilanguage string
	 * @param sep		optional seperator between main language and subtitle language,
	 *					if NULL is passed no subtitle will be added to the returned string
	 * @return processed string
	 */
	Common::String strSplit(const char *str, const char *sep = "\r----------\r");

	/**
	 * Autodetects the DoSound type
	 * @return DoSound type, SCI_VERSION_0_EARLY / SCI_VERSION_0_LATE /
	 *                       SCI_VERSION_1_EARLY / SCI_VERSION_1_LATE
	 */
	SciVersion detectDoSoundType();

	/**
	 * Autodetects the SetCursor type
	 * @return SetCursor type, SCI_VERSION_0_EARLY / SCI_VERSION_1_1
	 */
	SciVersion detectSetCursorType();

	/**
	 * Autodetects the Lofs type
	 * @return Lofs type, SCI_VERSION_0_EARLY / SCI_VERSION_1_MIDDLE / SCI_VERSION_1_1
	 */
	SciVersion detectLofsType();

	/**
	 * Autodetects the graphics functions used
	 * @return Graphics functions type, SCI_VERSION_0_EARLY / SCI_VERSION_0_LATE
	 */
	SciVersion detectGfxFunctionsType();
	
#ifdef ENABLE_SCI32
	/**
	 * Autodetects the kernel functions used in SCI2.1
	 * @return Graphics functions type, SCI_VERSION_2 / SCI_VERSION_2_1
	 */
	SciVersion detectSci21KernelType();
#endif

	/**
	 * Applies to all versions before 0.000.502
	 * Old SCI versions used to interpret the third DrawPic() parameter inversely,
	 * with the opposite default value (obviously).
	 * Also, they used 15 priority zones from 42 to 200 instead of 14 priority
	 * zones from 42 to 190.
	 */
	bool usesOldGfxFunctions() { return detectGfxFunctionsType() == SCI_VERSION_0_EARLY; }

	/**
	 * Autodetects the Bresenham routine used in the actor movement functions
	 * @return Move count type, kIncrementMoveCnt / kIgnoreMoveCnt
	 */
	MoveCountType detectMoveCountType();

	bool handleMoveCount() { return detectMoveCountType() == kIncrementMoveCount; }

	bool usesCdTrack() { return _usesCdTrack; }

	/* Debugger data: */
	Breakpoint *bp_list;   /**< List of breakpoints */
	int have_bp;  /**< Bit mask specifying which types of breakpoints are used in bp_list */

	/* System strings */
	SegmentId sys_strings_segment;
	SystemStrings *sys_strings;

	reg_t _gameObj; /**< Pointer to the game object */

	SegManager *_segMan;
	int gc_countdown; /**< Number of kernel calls until next gc */

	MessageState *_msgState;

	EngineState *successor; /**< Successor of this state: Used for restoring */

	Common::String getLanguageString(const char *str, kLanguage lang) const;

private:
	bool autoDetectFeature(FeatureDetection featureDetection, int methodNum = -1);

	SciVersion _doSoundType, _setCursorType, _lofsType, _gfxFunctionsType;
#ifdef ENABLE_SCI32
	SciVersion _sci21KernelType;
#endif

	MoveCountType _moveCountType;
	kLanguage charToLanguage(const char c) const;
	bool _usesCdTrack;
};

} // End of namespace Sci

#endif // SCI_INCLUDE_ENGINE_H
