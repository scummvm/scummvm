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
// Game initialization
//

#include "ac/common.h"
#include "ac/characterinfo.h"
#include "ac/game.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_game.h"
#include "ac/mouse.h"
#include "ac/room.h"
#include "ac/screen.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"
#include "debug/out.h"
#include "gfx/ali3dexception.h"
#include "main/mainheader.h"
#include "main/game_run.h"
#include "main/game_start.h"
#include "script/script.h"
#include "media/audio/audio_system.h"
#include "ac/timer.h"

using namespace AGS::Common;
using namespace AGS::Engine;

extern int our_eip, displayed_room;
extern volatile char want_exit, abort_engine;
extern GameSetupStruct game;
extern GameState play;
extern const char *loadSaveGameOnStartup;
extern std::vector<ccInstance *> moduleInst;
extern int numScriptModules;
extern CharacterInfo*playerchar;
extern int convert_16bit_bgr;

void start_game_init_editor_debugging()
{
    if (editor_debugging_enabled)
    {
        SetMultitasking(1);
        if (init_editor_debugging())
        {
            auto waitUntil = AGS_Clock::now() + std::chrono::milliseconds(500);
            while (waitUntil > AGS_Clock::now())
            {
                // pick up any breakpoints in game_start
                check_for_messages_from_editor();
            }

            ccSetDebugHook(scriptDebugHook);
        }
    }
}

void start_game_load_savegame_on_startup()
{
    if (loadSaveGameOnStartup != nullptr)
    {
        int saveGameNumber = 1000;
        const char *sgName = strstr(loadSaveGameOnStartup, "agssave.");
        if (sgName != nullptr)
        {
            sscanf(sgName, "agssave.%03d", &saveGameNumber);
        }
        current_fade_out_effect();
        try_restore_save(loadSaveGameOnStartup, saveGameNumber);
    }
}

void start_game() {
    set_cursor_mode(MODE_WALK);
    Mouse::SetPosition(Point(160, 100));
    newmusic(0);

    our_eip = -42;

    // skip ticks to account for initialisation or a restored game.
    skipMissedTicks();

    for (int kk = 0; kk < numScriptModules; kk++)
        RunTextScript(moduleInst[kk], "game_start");

    RunTextScript(gameinst, "game_start");

    our_eip = -43;

    SetRestartPoint();

    our_eip=-3;

    if (displayed_room < 0) {
        current_fade_out_effect();
        load_new_room(playerchar->room,playerchar);
        // load_new_room updates it, but it should be -1 in the first room
        playerchar->prevroom = -1;
    }

    first_room_initialization();
}

void do_start_game()
{
    // only start if replay playback hasn't loaded a game
    if (displayed_room < 0)
        start_game();
}

void initialize_start_and_play_game(int override_start_room, const char *loadSaveGameOnStartup)
{
    try { // BEGIN try for ALI3DEXception

        set_cursor_mode (MODE_WALK);

        if (convert_16bit_bgr) {
            // Disable text as speech while displaying the warning message
            // This happens if the user's graphics card does BGR order 16-bit colour
            int oldalways = game.options[OPT_ALWAYSSPCH];
            game.options[OPT_ALWAYSSPCH] = 0;
            // PSP: This is normal. Don't show a warning.
            //Display ("WARNING: AGS has detected that you have an incompatible graphics card for this game. You may experience colour problems during the game. Try running the game with \"--15bit\" command line parameter and see if that helps.[[Click the mouse to continue.");
            game.options[OPT_ALWAYSSPCH] = oldalways;
        }

        srand (play.randseed);
        if (override_start_room)
            playerchar->room = override_start_room;

        Debug::Printf(kDbgMsg_Info, "Engine initialization complete");
        Debug::Printf(kDbgMsg_Info, "Starting game");

        start_game_init_editor_debugging();

        start_game_load_savegame_on_startup();

        do_start_game();

        RunGameUntilAborted();

    } catch (Ali3DException gfxException)
    {
        quit((char*)gfxException._message);
    }
}
