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
#include "sci/vocabulary.h"
#include "sci/resource.h"
#include "sci/engine/kernel.h"	// for kfunct_sig_pair_t
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/gfx/gfx_system.h"
#include "sci/sfx/core.h"

namespace Sci {

class Menubar;
class SciGui;
class SciGuiCursor;
class MessageState;

struct GfxState;
struct GfxPort;
struct GfxVisual;
struct GfxContainer;
struct GfxList;


class DirSeeker {
protected:
	EngineState *_state;
	reg_t _outbuffer;
	Common::StringList _savefiles;
	Common::StringList::const_iterator _iter;

public:
	DirSeeker(EngineState *s) : _state(s) {
		_outbuffer = NULL_REG;
		_iter = _savefiles.begin();
	}

	void firstFile(const char *mask, reg_t buffer);
	void nextFile();
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
	EngineState(ResourceManager *res, Kernel *kernel, Vocabulary *voc, SciGui *gui, SciGuiCursor *cursor);
	virtual ~EngineState();

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

	kLanguage getLanguage();
public:
	ResourceManager *resMan; /**< The resource manager */
	Kernel *_kernel;
	Vocabulary *_voc;

	Common::String _gameName; /**< Designation of the primary object (which inherits from Game) */

	/* Non-VM information */

	SciGui *_gui; /* Currently active Gui */
	SciGuiCursor *_cursor;	/* Cursor functions */

	GfxState *gfx_state; /**< Graphics state and driver */

	SfxState _sound; /**< sound subsystem */
	int sfx_init_flags; /**< flags the sfx subsystem was initialised with */
	unsigned int sound_volume; /**< 0x0 -> 0xf Current volume of sound system */
	unsigned int sound_mute; /**< 0 = not, else == saved value */

	byte restarting_flags; /**< Flags used for restarting */

	/** Text on the status bar, or NULL if the title bar is blank */
	Common::String _statusBarText;

#ifdef INCLUDE_OLDGFX
	int *pic_priority_table; /**< 16 entries with priorities or NULL if not present */
	byte pic_not_valid; /**< Is 0 if the background picture is "valid" */
	byte pic_is_new; /**< New pic was loaded or port was opened */
	gfx_pixmap_t *old_screen; /**< Old screen content: Stored during kDrawPic() for kAnimate() */

	GfxPort *port; /**< The currently active port */

	gfx_color_t ega_colors[16]; /**< The 16 EGA colors- for SCI0(1) */

	GfxVisual *visual; /**< A visual widget, containing all ports */

	GfxPort *titlebar_port; /**< Title bar viewport (0,0,9,319) */
	GfxPort *wm_port; /**< window manager viewport and designated &heap[0] view (10,0,199,319) */
	GfxPort *picture_port; /**< The background picture viewport (10,0,199,319) */
	GfxPort *iconbar_port; /**< Full-screen port used for non-clipped icon bar draw in SCI1 */

	gfx_map_mask_t pic_visible_map; /**< The number of the map to display in update commands */
	int pic_animate; /**< The animation used by Animate() to display the picture */

	GfxList *dyn_views; /**< Pointers to pic and dynamic view lists */
	GfxList *drop_views; /**< A list Animate() can dump dropped dynviews into */
#endif

	Menubar *_menubar; /**< The menu bar */

	int priority_first; /**< The line where priority zone 0 ends */
	int priority_last; /**< The line where the highest priority zone starts */

	uint32 game_start_time; /**< The time at which the interpreter was started */
	uint32 last_wait_time; /**< The last time the game invoked Wait() */

	uint32 _lastAnimateCounter; /**< total times kAnimate was invoked */
	uint32 _lastAnimateTime; /**< last time kAnimate was invoked */

	/* Kernel File IO stuff */

	Common::Array<FileHandle> _fileHandles; /**< Array of file handles. Dynamically increased if required. */

	DirSeeker _dirseeker;

	/* Parser data: */
	reg_t parser_base; /**< Base address for the parser error reporting mechanism */
	reg_t parser_event; /**< The event passed to Parse() and later used by Said() */
	bool parserIsValid; /**< If something has been correctly parsed */

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
	 * @return DoSound type, SCI_VERSION_0_EARLY / SCI_VERSION_1_EARLY / SCI_VERSION_1_LATE
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
	
	uint32 _audioCdStart;
private:
	SciVersion _doSoundType, _setCursorType, _lofsType, _gfxFunctionsType;
	MoveCountType _moveCountType;
	kLanguage charToLanguage(const char c) const;
	int methodChecksum(reg_t objAddress, Selector sel, int offset, uint size) const;
	uint16 firstRetOffset(reg_t objectAddress) const;
	bool _usesCdTrack;
};

#ifdef INCLUDE_OLDGFX

/**
 * Retrieves the gfx_pixmap_color_t associated with a game color index.
 * @param s			game state
 * @param color		color to look up
 * @return the requested color
 */
PaletteEntry get_pic_color(EngineState *s, int color);

/* Functions used in gui32\gui32.cpp */
reg_t graph_save_box(EngineState *s, rect_t area);
void graph_restore_box(EngineState *s, reg_t handle);
void assert_primary_widget_lists(EngineState *s);
void reparentize_primary_widget_lists(EngineState *s, GfxPort *newport);

#endif

} // End of namespace Sci

#endif // SCI_INCLUDE_ENGINE_H
