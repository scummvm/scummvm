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
#include "sci/engine/seg_manager.h"

#include "sci/parser/vocabulary.h"

#include "sci/sound/audio.h"
#ifdef USE_OLD_MUSIC_FUNCTIONS
#include "sci/sound/iterator/core.h"
#endif
#include "sci/sound/soundcmd.h"

namespace Sci {

class SciEvent;
class MessageState;
class SoundCommandParser;

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
	EngineState(Vocabulary *voc, SegManager *segMan, AudioPlayer *audio);
	virtual ~EngineState();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

public:
	SegManager *_segMan; /**< The segment manager */
	Vocabulary *_voc;

	Common::String _gameId; /**< Designation of the primary object (which inherits from Game) */

	/* Non-VM information */

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

public:
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

	StackPtr stack_base; /**< Pointer to the least stack element */
	StackPtr stack_top; /**< First invalid stack element */

	Script *script_000;  /**< script 000, e.g. for globals */

	uint16 currentRoomNumber() const;
	void setRoomNumber(uint16 roomNumber);

	/* Debugger data: */
	Common::List<Breakpoint> _breakpoints;   /**< List of breakpoints */
	int _activeBreakpointTypes;  /**< Bit mask specifying which types of breakpoints are active */

	/* System strings */
	SegmentId sys_strings_segment;
	SystemStrings *sys_strings;

	reg_t _gameObj; /**< Pointer to the game object */

	int gc_countdown; /**< Number of kernel calls until next gc */

public:
	MessageState *_msgState;

	// MemorySegment provides access to a 256-byte block of memory that remains
	// intact across restarts and restores
	enum {
		kMemorySegmentMax = 256
	};
	uint _memorySegmentSize;
	byte _memorySegment[kMemorySegmentMax];

	EngineState *successor; /**< Successor of this state: Used for restoring */
};

} // End of namespace Sci

#endif // SCI_INCLUDE_ENGINE_H
