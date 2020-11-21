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

#include <string.h>
#include "executingscript.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"

QueuedScript::QueuedScript()
    : Instance(kScInstGame)
    , ParamCount(0)
{
}

int ExecutingScript::queue_action(PostScriptAction act, int data, const char *aname) {
    if (numPostScriptActions >= MAX_QUEUED_ACTIONS)
        quitprintf("!%s: Cannot queue action, post-script queue full", aname);

    if (numPostScriptActions > 0) {
        // if something that will terminate the room has already
        // been queued, don't allow a second thing to be queued
        switch (postScriptActions[numPostScriptActions - 1]) {
    case ePSANewRoom:
    case ePSARestoreGame:
    case ePSARestoreGameDialog:
    case ePSARunAGSGame:
    case ePSARestartGame:
        quitprintf("!%s: Cannot run this command, since there was a %s command already queued to run in \"%s\", line %d",
            aname, postScriptActionNames[numPostScriptActions - 1],
            postScriptActionPositions[numPostScriptActions - 1].Section.GetCStr(), postScriptActionPositions[numPostScriptActions - 1].Line);
        break;
        // MACPORT FIX 9/6/5: added default clause to remove warning
    default:
        break;
        }
    }

    postScriptActions[numPostScriptActions] = act;
    postScriptActionData[numPostScriptActions] = data;
    postScriptActionNames[numPostScriptActions] = aname;
    get_script_position(postScriptActionPositions[numPostScriptActions]);
    numPostScriptActions++;
    return numPostScriptActions - 1;
}

void ExecutingScript::run_another(const char *namm, ScriptInstType scinst, size_t param_count, const RuntimeScriptValue &p1, const RuntimeScriptValue &p2) {
    if (numanother < MAX_QUEUED_SCRIPTS)
        numanother++;
    else {
        /*debug_script_warn("Warning: too many scripts to run, ignored %s(%d,%d)",
        script_run_another[numanother - 1], run_another_p1[numanother - 1],
        run_another_p2[numanother - 1]);*/
    }
    int thisslot = numanother - 1;
    QueuedScript &script = ScFnQueue[thisslot];
    script.FnName.SetString(namm, MAX_FUNCTION_NAME_LEN);
    script.Instance = scinst;
    script.ParamCount = param_count;
    script.Param1 = p1;
    script.Param2 = p2;
}

void ExecutingScript::init() {
    inst = nullptr;
    forked = 0;
    numanother = 0;
    numPostScriptActions = 0;

    memset(postScriptActions, 0, sizeof(postScriptActions));
    memset(postScriptActionNames, 0, sizeof(postScriptActionNames));
    memset(postScriptSaveSlotDescription, 0, sizeof(postScriptSaveSlotDescription));
    memset(postScriptActionData, 0, sizeof(postScriptActionData));
}

ExecutingScript::ExecutingScript() {
    init();
}
