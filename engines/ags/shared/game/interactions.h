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

//=============================================================================
//
// Interaction structs.
//
//-----------------------------------------------------------------------------
//
// Most of the interaction types here were used before the script and have
// very limited capabilities. They were removed from AGS completely in
// generation 3.0. The code is left for backwards compatibility.
//
//-----------------------------------------------------------------------------
//
/* THE WAY THIS WORKS:
*
* Interaction (Hotspot 1)
*  |
*  +-- eventTypes [NUM_EVENTS]
*  +-- InteractionCommandList [NUM_EVENTS]   (Look at hotspot)
*        |
*        +-- InteractionCommand [NUM_COMMANDS]   (Display message)
*             |
*             +-- InteractionValue [NUM_ARGUMENTS]   (5)
*/
//
//=============================================================================

#ifndef AGS_SHARED_GAME_INTEREACTIONS_H
#define AGS_SHARED_GAME_INTEREACTIONS_H

#include "common/std/memory.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {

#define LOCAL_VARIABLE_OFFSET       10000
#define MAX_GLOBAL_VARIABLES        100
#define MAX_ACTION_ARGS             5
#define MAX_NEWINTERACTION_EVENTS   30
#define MAX_COMMANDS_PER_LIST       40

namespace AGS {
namespace Shared {

enum InterValType : int8_t {
	kInterValInvalid = 0,
	kInterValLiteralInt = 1,
	kInterValVariable = 2,
	kInterValBoolean = 3,
	kInterValCharnum = 4
};

enum InteractionVersion {
	kInteractionVersion_Initial = 1
};

// InteractionValue represents an argument of interaction command
struct InteractionValue {
	InterValType Type;  // value type
	int          Value; // value definition
	int          Extra;

	InteractionValue();

	void clear();

	void Read(Stream *in);
	void Write(Stream *out) const;
};


struct InteractionCommandList;
typedef std::unique_ptr<InteractionCommandList> UInterCmdList;

// InteractionCommand represents a single command (action), an item of Command List
struct InteractionCommand {
	int                     Type;       // type of action
	InteractionValue        Data[MAX_ACTION_ARGS]; // action arguments
	UInterCmdList           Children;   // list of sub-actions
	InteractionCommandList *Parent;     // action parent (optional)

	InteractionCommand();
	InteractionCommand(const InteractionCommand &ic);

	void Assign(const InteractionCommand &ic, InteractionCommandList *parent);
	void Reset();

	void Read(Stream *in, bool &has_children);
	void Write(Stream *out) const;

	InteractionCommand &operator = (const InteractionCommand &ic);

	private:
	void ReadValues(Stream *in);
	void WriteValues(Stream *out) const;
};


typedef std::vector<InteractionCommand> InterCmdVector;
// InteractionCommandList represents a list of commands (actions) that need to be
// performed on particular game event
struct InteractionCommandList {
	InterCmdVector  Cmds;     // actions to run
	int             TimesRun; // used by engine to track score changes

	InteractionCommandList();
	InteractionCommandList(const InteractionCommandList &icmd_list);

	void Reset();

	void Read(Stream *in);
	void Write(Stream *out) const;

protected:
	void ReadCommands(Shared::Stream *in, std::vector<bool> &cmd_children);
	void WriteCommands(Shared::Stream *out) const;
};


// InteractionEvent is a single event with a list of commands to performed
struct InteractionEvent {
	int           Type;     // type of event
	int           TimesRun; // used by engine to track score changes
	UInterCmdList Response; // list of commands to run

	InteractionEvent();
	InteractionEvent(const InteractionEvent &ie);

	InteractionEvent &operator = (const InteractionEvent &ic);
};

typedef std::vector<InteractionEvent> InterEvtVector;
// Interaction is the list of events and responses for a game or game object
struct Interaction {
	// The first few event types depend on the item - ID's of 100+ are
	// custom events (used for subroutines)
	InterEvtVector Events;

	Interaction();
	Interaction(const Interaction &inter);

	// Copy information on number of times events of this interaction were fired
	void CopyTimesRun(const Interaction &inter);
	void Reset();

	// Game static data (de)serialization
	static Interaction *CreateFromStream(Stream *in);
	void                Write(Stream *out) const;

	// Reading and writing runtime data from/to savedgame;
	// NOTE: these are backwards-compatible methods, that do not always
	// have practical sense
	void ReadFromSavedgame_v321(Stream *in);
	void WriteToSavedgame_v321(Stream *out) const;
	void ReadTimesRunFromSave_v321(Stream *in);
	void WriteTimesRunToSave_v321(Stream *out) const;

	Interaction &operator =(const Interaction &inter);
};

typedef std::shared_ptr<Interaction> PInteraction;


// Legacy pre-3.0 kind of global and local room variables
struct InteractionVariable {
	String Name{};
	char   Type{ '\0' };
	int    Value{ 0 };

	InteractionVariable();
	InteractionVariable(const String &name, char type, int val);

	void Read(Stream *in);
	void Write(Stream *out) const;
};

typedef std::vector<InteractionVariable> InterVarVector;


// A list of script function names for all supported events
struct InteractionScripts {
	StringV ScriptFuncNames;

	static InteractionScripts *CreateFromStream(Stream *in);
};

typedef std::shared_ptr<InteractionScripts> PInteractionScripts;

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
