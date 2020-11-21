//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
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
#ifndef __AGS_CN_GAME__INTEREACTIONS_H
#define __AGS_CN_GAME__INTEREACTIONS_H

#include <memory>
#include "util/string_types.h"

#define LOCAL_VARIABLE_OFFSET       10000
#define MAX_GLOBAL_VARIABLES        100
#define MAX_ACTION_ARGS             5
#define MAX_NEWINTERACTION_EVENTS   30
#define MAX_COMMANDS_PER_LIST       40

namespace AGS
{
namespace Common
{

enum InterValType
{
    kInterValLiteralInt = 1,
    kInterValVariable   = 2,
    kInterValBoolean    = 3,
    kInterValCharnum    = 4
};

enum InteractionVersion
{
    kInteractionVersion_Initial = 1
};

// InteractionValue represents an argument of interaction command
struct InteractionValue
{
    InterValType Type;  // value type
    int          Value; // value definition
    int          Extra;

    InteractionValue();

    void Read(Stream *in);
    void Write(Stream *out) const;
};


struct InteractionCommandList;
typedef std::unique_ptr<InteractionCommandList> UInterCmdList;

// InteractionCommand represents a single command (action), an item of Command List
struct InteractionCommand
{
    int                     Type;       // type of action
    InteractionValue        Data[MAX_ACTION_ARGS]; // action arguments
    UInterCmdList           Children;   // list of sub-actions
    InteractionCommandList *Parent;     // action parent (optional)

    InteractionCommand();
    InteractionCommand(const InteractionCommand &ic);

    void Assign(const InteractionCommand &ic, InteractionCommandList *parent);
    void Reset();

    void Read_v321(Stream *in, bool &has_children);
    void Write_v321(Stream *out) const;

    InteractionCommand &operator = (const InteractionCommand &ic);

private:
    void ReadValues_Aligned(Stream *in);
    void WriteValues_Aligned(Stream *out) const;
};


typedef std::vector<InteractionCommand> InterCmdVector;
// InteractionCommandList represents a list of commands (actions) that need to be
// performed on particular game event
struct InteractionCommandList
{
    InterCmdVector  Cmds;     // actions to run
    int             TimesRun; // used by engine to track score changes

    InteractionCommandList();
    InteractionCommandList(const InteractionCommandList &icmd_list);

    void Reset();

    void Read_v321(Stream *in);
    void Write_v321(Stream *out) const;

protected:
    void Read_Aligned(Common::Stream *in, std::vector<bool> &cmd_children);
    void Write_Aligned(Common::Stream *out) const;
};


// InteractionEvent is a single event with a list of commands to performed
struct InteractionEvent
{
    int           Type;     // type of event
    int           TimesRun; // used by engine to track score changes
    UInterCmdList Response; // list of commands to run

    InteractionEvent();
    InteractionEvent(const InteractionEvent &ie);

    InteractionEvent &operator = (const InteractionEvent &ic);
};

typedef std::vector<InteractionEvent> InterEvtVector;
// Interaction is the list of events and responses for a game or game object
struct Interaction
{
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
struct InteractionVariable
{
    String Name {};
    char   Type {'\0'};
    int    Value {0};

    InteractionVariable();
    InteractionVariable(const String &name, char type, int val);

    void Read(Stream *in);
    void Write(Stream *out) const;
};

typedef std::vector<InteractionVariable> InterVarVector;


// A list of script function names for all supported events
struct InteractionScripts
{
    StringV ScriptFuncNames;

    static InteractionScripts *CreateFromStream(Stream *in);
};

typedef std::shared_ptr<InteractionScripts> PInteractionScripts;

} // namespace Common
} // namespace AGS

// Legacy global variables
extern AGS::Common::InteractionVariable globalvars[MAX_GLOBAL_VARIABLES];
extern int numGlobalVars;

#endif // __AGS_CN_GAME__INTEREACTIONS_H
