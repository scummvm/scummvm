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

#include <math.h>

#include "core/platform.h"
#include "ac/audiocliptype.h"
#include "ac/global_game.h"
#include "ac/common.h"
#include "ac/view.h"
#include "ac/character.h"
#include "ac/draw.h"
#include "ac/dynamicsprite.h"
#include "ac/event.h"
#include "ac/game.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_character.h"
#include "ac/global_gui.h"
#include "ac/global_hotspot.h"
#include "ac/global_inventoryitem.h"
#include "ac/global_translation.h"
#include "ac/gui.h"
#include "ac/hotspot.h"
#include "ac/keycode.h"
#include "ac/mouse.h"
#include "ac/object.h"
#include "ac/path_helper.h"
#include "ac/sys_events.h"
#include "ac/room.h"
#include "ac/roomstatus.h"
#include "ac/string.h"
#include "ac/system.h"
#include "debug/debugger.h"
#include "debug/debug_log.h"
#include "gui/guidialog.h"
#include "main/engine.h"
#include "main/game_start.h"
#include "main/game_run.h"
#include "main/graphics_mode.h"
#include "script/script.h"
#include "script/script_runtime.h"
#include "ac/spritecache.h"
#include "gfx/bitmap.h"
#include "gfx/graphicsdriver.h"
#include "core/assetmanager.h"
#include "main/config.h"
#include "main/game_file.h"
#include "util/string_utils.h"
#include "media/audio/audio_system.h"

using namespace AGS::Common;

#define ALLEGRO_KEYBOARD_HANDLER

extern GameState play;
extern ExecutingScript*curscript;
extern int displayed_room;
extern int game_paused;
extern SpriteCache spriteset;
extern char gamefilenamebuf[200];
extern GameSetup usetup;
extern unsigned int load_new_game;
extern int load_new_game_restore;
extern GameSetupStruct game;
extern ViewStruct*views;
extern RoomStatus*croom;
extern int gui_disabled_style;
extern RoomStruct thisroom;
extern int getloctype_index;
extern IGraphicsDriver *gfxDriver;
extern color palette[256];

#if AGS_PLATFORM_OS_IOS || AGS_PLATFORM_OS_ANDROID
extern int psp_gfx_renderer;
#endif

void GiveScore(int amnt) 
{
    guis_need_update = 1;
    play.score += amnt;

    if ((amnt > 0) && (play.score_sound >= 0))
        play_audio_clip_by_index(play.score_sound);

    run_on_event (GE_GOT_SCORE, RuntimeScriptValue().SetInt32(amnt));
}

void restart_game() {
    can_run_delayed_command();
    if (inside_script) {
        curscript->queue_action(ePSARestartGame, 0, "RestartGame");
        return;
    }
    try_restore_save(RESTART_POINT_SAVE_GAME_NUMBER);
}

void RestoreGameSlot(int slnum) {
    if (displayed_room < 0)
        quit("!RestoreGameSlot: a game cannot be restored from within game_start");

    can_run_delayed_command();
    if (inside_script) {
        curscript->queue_action(ePSARestoreGame, slnum, "RestoreGameSlot");
        return;
    }
    try_restore_save(slnum);
}

void DeleteSaveSlot (int slnum) {
    String nametouse;
    nametouse = get_save_game_path(slnum);
    ::remove (nametouse);
    if ((slnum >= 1) && (slnum <= MAXSAVEGAMES)) {
        String thisname;
        for (int i = MAXSAVEGAMES; i > slnum; i--) {
            thisname = get_save_game_path(i);
            if (Common::File::TestReadFile(thisname)) {
                // Rename the highest save game to fill in the gap
                rename (thisname, nametouse);
                break;
            }
        }

    }
}

void PauseGame() {
    game_paused++;
    debug_script_log("Game paused");
}
void UnPauseGame() {
    if (game_paused > 0)
        game_paused--;
    debug_script_log("Game UnPaused, pause level now %d", game_paused);
}


int IsGamePaused() {
    if (game_paused>0) return 1;
    return 0;
}

int GetSaveSlotDescription(int slnum,char*desbuf) {
    VALIDATE_STRING(desbuf);
    String description;
    if (read_savedgame_description(get_save_game_path(slnum), description))
    {
        strcpy(desbuf, description);
        return 1;
    }
    sprintf(desbuf,"INVALID SLOT %d", slnum);
    return 0;
}

int LoadSaveSlotScreenshot(int slnum, int width, int height) {
    int gotSlot;
    data_to_game_coords(&width, &height);

    if (!read_savedgame_screenshot(get_save_game_path(slnum), gotSlot))
        return 0;

    if (gotSlot == 0)
        return 0;

    if ((game.SpriteInfos[gotSlot].Width == width) && (game.SpriteInfos[gotSlot].Height == height))
        return gotSlot;

    // resize the sprite to the requested size
    Bitmap *newPic = BitmapHelper::CreateBitmap(width, height, spriteset[gotSlot]->GetColorDepth());
    newPic->StretchBlt(spriteset[gotSlot],
        RectWH(0, 0, game.SpriteInfos[gotSlot].Width, game.SpriteInfos[gotSlot].Height),
        RectWH(0, 0, width, height));

    update_polled_stuff_if_runtime();

    // replace the bitmap in the sprite set
    free_dynamic_sprite(gotSlot);
    add_dynamic_sprite(gotSlot, newPic);

    return gotSlot;
}

void SetGlobalInt(int index,int valu) {
    if ((index<0) | (index>=MAXGSVALUES))
        quit("!SetGlobalInt: invalid index");

    if (play.globalscriptvars[index] != valu) {
        debug_script_log("GlobalInt %d set to %d", index, valu);
    }

    play.globalscriptvars[index]=valu;
}


int GetGlobalInt(int index) {
    if ((index<0) | (index>=MAXGSVALUES))
        quit("!GetGlobalInt: invalid index");
    return play.globalscriptvars[index];
}

void SetGlobalString (int index, const char *newval) {
    if ((index<0) | (index >= MAXGLOBALSTRINGS))
        quit("!SetGlobalString: invalid index");
    debug_script_log("GlobalString %d set to '%s'", index, newval);
    strncpy(play.globalstrings[index], newval, MAX_MAXSTRLEN);
    // truncate it to 200 chars, to be sure
    play.globalstrings[index][MAX_MAXSTRLEN - 1] = 0;
}

void GetGlobalString (int index, char *strval) {
    if ((index<0) | (index >= MAXGLOBALSTRINGS))
        quit("!GetGlobalString: invalid index");
    strcpy (strval, play.globalstrings[index]);
}

// TODO: refactor this method, and use same shared procedure at both normal stop/startup and in RunAGSGame
int RunAGSGame (const char *newgame, unsigned int mode, int data) {

    can_run_delayed_command();

    int AllowedModes = RAGMODE_PRESERVEGLOBALINT | RAGMODE_LOADNOW;

    if ((mode & (~AllowedModes)) != 0)
        quit("!RunAGSGame: mode value unknown");

    if (editor_debugging_enabled)
    {
        quit("!RunAGSGame cannot be used while running the game from within the AGS Editor. You must build the game EXE and run it from there to use this function.");
    }

    if ((mode & RAGMODE_LOADNOW) == 0) {
        // need to copy, since the script gets destroyed
        get_install_dir_path(gamefilenamebuf, newgame);
        ResPaths.GamePak.Path = gamefilenamebuf;
        ResPaths.GamePak.Name = get_filename(gamefilenamebuf);
        play.takeover_data = data;
        load_new_game_restore = -1;

        if (inside_script) {
            curscript->queue_action(ePSARunAGSGame, mode | RAGMODE_LOADNOW, "RunAGSGame");
            ccInstance::GetCurrentInstance()->Abort();
        }
        else
            load_new_game = mode | RAGMODE_LOADNOW;

        return 0;
    }

    int ee;

    unload_old_room();
    displayed_room = -10;

    save_config_file(); // save current user config in case engine fails to run new game
    unload_game_file();

    // Adjust config (NOTE: normally, RunAGSGame would need a redesign to allow separate config etc per each game)
    usetup.translation = ""; // reset to default, prevent from trying translation file of game A in game B

    if (Common::AssetManager::SetDataFile(ResPaths.GamePak.Path) != Common::kAssetNoError)
        quitprintf("!RunAGSGame: unable to load new game file '%s'", ResPaths.GamePak.Path.GetCStr());

    show_preload();

    HError err = load_game_file();
    if (!err)
        quitprintf("!RunAGSGame: error loading new game file:\n%s", err->FullMessage().GetCStr());

    spriteset.Reset();
    err = spriteset.InitFile(SpriteCache::DefaultSpriteFileName, SpriteCache::DefaultSpriteIndexName);
    if (!err)
        quitprintf("!RunAGSGame: error loading new sprites:\n%s", err->FullMessage().GetCStr());

    if ((mode & RAGMODE_PRESERVEGLOBALINT) == 0) {
        // reset GlobalInts
        for (ee = 0; ee < MAXGSVALUES; ee++)
            play.globalscriptvars[ee] = 0;  
    }

    engine_init_game_settings();
    play.screen_is_faded_out = 1;

    if (load_new_game_restore >= 0) {
        try_restore_save(load_new_game_restore);
        load_new_game_restore = -1;
    }
    else
        start_game();

    return 0;
}

int GetGameParameter (int parm, int data1, int data2, int data3) {
    switch (parm) {
   case GP_SPRITEWIDTH:
       return Game_GetSpriteWidth(data1);
   case GP_SPRITEHEIGHT:
       return Game_GetSpriteHeight(data1);
   case GP_NUMLOOPS:
       return Game_GetLoopCountForView(data1);
   case GP_NUMFRAMES:
       return Game_GetFrameCountForLoop(data1, data2);
   case GP_FRAMESPEED:
   case GP_FRAMEIMAGE:
   case GP_FRAMESOUND:
   case GP_ISFRAMEFLIPPED:
       {
           if ((data1 < 1) || (data1 > game.numviews)) {
               quitprintf("!GetGameParameter: invalid view specified (v: %d, l: %d, f: %d)", data1, data2, data3);
           }
           if ((data2 < 0) || (data2 >= views[data1 - 1].numLoops)) {
               quitprintf("!GetGameParameter: invalid loop specified (v: %d, l: %d, f: %d)", data1, data2, data3);
           }
           if ((data3 < 0) || (data3 >= views[data1 - 1].loops[data2].numFrames)) {
               quitprintf("!GetGameParameter: invalid frame specified (v: %d, l: %d, f: %d)", data1, data2, data3);
           }

           ViewFrame *pvf = &views[data1 - 1].loops[data2].frames[data3];

           if (parm == GP_FRAMESPEED)
               return pvf->speed;
           else if (parm == GP_FRAMEIMAGE)
               return pvf->pic;
           else if (parm == GP_FRAMESOUND)
               return get_old_style_number_for_sound(pvf->sound);
           else if (parm == GP_ISFRAMEFLIPPED)
               return (pvf->flags & VFLG_FLIPSPRITE) ? 1 : 0;
           else
               quit("GetGameParameter internal error");
       }
   case GP_ISRUNNEXTLOOP:
       return Game_GetRunNextSettingForLoop(data1, data2);
   case GP_NUMGUIS:
       return game.numgui;
   case GP_NUMOBJECTS:
       return croom->numobj;
   case GP_NUMCHARACTERS:
       return game.numcharacters;
   case GP_NUMINVITEMS:
       return game.numinvitems;
   default:
       quit("!GetGameParameter: unknown parameter specified");
    }
    return 0;
}

void QuitGame(int dialog) {
    if (dialog) {
        int rcode;
        setup_for_dialog();
        rcode=quitdialog();
        restore_after_dialog();
        if (rcode==0) return;
    }
    quit("|You have exited.");
}




void SetRestartPoint() {
    save_game(RESTART_POINT_SAVE_GAME_NUMBER, "Restart Game Auto-Save");
}



void SetGameSpeed(int newspd) {
    newspd += play.game_speed_modifier;
    if (newspd>1000) newspd=1000;
    if (newspd<10) newspd=10;
    set_game_speed(newspd);
    debug_script_log("Game speed set to %d", newspd);
}

int GetGameSpeed() {
    return ::lround(get_current_fps()) - play.game_speed_modifier;
}

int SetGameOption (int opt, int setting) {
    if (((opt < 1) || (opt > OPT_HIGHESTOPTION)) && (opt != OPT_LIPSYNCTEXT))
        quit("!SetGameOption: invalid option specified");

    if (opt == OPT_ANTIGLIDE)
    {
        for (int i = 0; i < game.numcharacters; i++)
        {
            if (setting)
                game.chars[i].flags |= CHF_ANTIGLIDE;
            else
                game.chars[i].flags &= ~CHF_ANTIGLIDE;
        }
    }

    if ((opt == OPT_CROSSFADEMUSIC) && (game.audioClipTypes.size() > AUDIOTYPE_LEGACY_MUSIC))
    {
        // legacy compatibility -- changing crossfade speed here also
        // updates the new audio clip type style
        game.audioClipTypes[AUDIOTYPE_LEGACY_MUSIC].crossfadeSpeed = setting;
    }

    int oldval = game.options[opt];
    game.options[opt] = setting;

    if (opt == OPT_DUPLICATEINV)
        update_invorder();
    else if (opt == OPT_DISABLEOFF)
        gui_disabled_style = convert_gui_disabled_style(game.options[OPT_DISABLEOFF]);
    else if (opt == OPT_PORTRAITSIDE) {
        if (setting == 0)  // set back to Left
            play.swap_portrait_side = 0;
    }

    return oldval;
}

int GetGameOption (int opt) {
    if (((opt < 1) || (opt > OPT_HIGHESTOPTION)) && (opt != OPT_LIPSYNCTEXT))
        quit("!GetGameOption: invalid option specified");

    return game.options[opt];
}

void SkipUntilCharacterStops(int cc) {
    if (!is_valid_character(cc))
        quit("!SkipUntilCharacterStops: invalid character specified");
    if (game.chars[cc].room!=displayed_room)
        quit("!SkipUntilCharacterStops: specified character not in current room");

    // if they are not currently moving, do nothing
    if (!game.chars[cc].walking)
        return;

    if (is_in_cutscene())
        quit("!SkipUntilCharacterStops: cannot be used within a cutscene");

    initialize_skippable_cutscene();
    play.fast_forward = 2;
    play.skip_until_char_stops = cc;
}

void EndSkippingUntilCharStops() {
    // not currently skipping, so ignore
    if (play.skip_until_char_stops < 0)
        return;

    stop_fast_forwarding();
    play.skip_until_char_stops = -1;
}

void StartCutscene (int skipwith) {
    static ScriptPosition last_cutscene_script_pos;

    if (is_in_cutscene()) {
        quitprintf("!StartCutscene: already in a cutscene; previous started in \"%s\", line %d",
            last_cutscene_script_pos.Section.GetCStr(), last_cutscene_script_pos.Line);
    }

    if ((skipwith < 1) || (skipwith > 6))
        quit("!StartCutscene: invalid argument, must be 1 to 5.");

    get_script_position(last_cutscene_script_pos);

    // make sure they can't be skipping and cutsceneing at the same time
    EndSkippingUntilCharStops();

    play.in_cutscene = skipwith;
    initialize_skippable_cutscene();
}

void SkipCutscene()
{
    if (is_in_cutscene())
        start_skipping_cutscene();
}

int EndCutscene () {
    if (!is_in_cutscene())
        quit("!EndCutscene: not in a cutscene");

    int retval = play.fast_forward;
    play.in_cutscene = 0;
    // Stop it fast-forwarding
    stop_fast_forwarding();

    // make sure that the screen redraws
    invalidate_screen();

    // Return whether the player skipped it
    return retval;
}

void sc_inputbox(const char*msg,char*bufr) {
    VALIDATE_STRING(bufr);
    setup_for_dialog();
    enterstringwindow(get_translation(msg),bufr);
    restore_after_dialog();
}

// GetLocationType exported function - just call through
// to the main function with default 0
int GetLocationType(int xxx,int yyy) {
    return __GetLocationType(xxx, yyy, 0);
}

void SaveCursorForLocationChange() {
    // update the current location name
    char tempo[100];
    GetLocationName(game_to_data_coord(mousex), game_to_data_coord(mousey), tempo);

    if (play.get_loc_name_save_cursor != play.get_loc_name_last_time) {
        play.get_loc_name_save_cursor = play.get_loc_name_last_time;
        play.restore_cursor_mode_to = GetCursorMode();
        play.restore_cursor_image_to = GetMouseCursor();
        debug_script_log("Saving mouse: mode %d cursor %d", play.restore_cursor_mode_to, play.restore_cursor_image_to);
    }
}

void GetLocationName(int xxx,int yyy,char*tempo) {
    if (displayed_room < 0)
        quit("!GetLocationName: no room has been loaded");

    VALIDATE_STRING(tempo);

    tempo[0] = 0;

    if (GetGUIAt(xxx, yyy) >= 0) {
        int mover = GetInvAt (xxx, yyy);
        if (mover > 0) {
            if (play.get_loc_name_last_time != 1000 + mover)
                guis_need_update = 1;
            play.get_loc_name_last_time = 1000 + mover;
            strcpy(tempo,get_translation(game.invinfo[mover].name));
        }
        else if ((play.get_loc_name_last_time > 1000) && (play.get_loc_name_last_time < 1000 + MAX_INV)) {
            // no longer selecting an item
            guis_need_update = 1;
            play.get_loc_name_last_time = -1;
        }
        return;
    }

    int loctype = GetLocationType(xxx, yyy); // GetLocationType takes screen coords
    VpPoint vpt = play.ScreenToRoomDivDown(xxx, yyy);
    if (vpt.second < 0)
        return;
    xxx = vpt.first.X;
    yyy = vpt.first.Y;
    if ((xxx>=thisroom.Width) | (xxx<0) | (yyy<0) | (yyy>=thisroom.Height))
        return;

    int onhs,aa;
    if (loctype == 0) {
        if (play.get_loc_name_last_time != 0) {
            play.get_loc_name_last_time = 0;
            guis_need_update = 1;
        }
        return;
    }

    // on character
    if (loctype == LOCTYPE_CHAR) {
        onhs = getloctype_index;
        strcpy(tempo,get_translation(game.chars[onhs].name));
        if (play.get_loc_name_last_time != 2000+onhs)
            guis_need_update = 1;
        play.get_loc_name_last_time = 2000+onhs;
        return;
    }
    // on object
    if (loctype == LOCTYPE_OBJ) {
        aa = getloctype_index;
        strcpy(tempo,get_translation(thisroom.Objects[aa].Name));
        // Compatibility: < 3.1.1 games returned space for nameless object
        // (presumably was a bug, but fixing it affected certain games behavior)
        if (loaded_game_file_version < kGameVersion_311 && tempo[0] == 0) {
            tempo[0] = ' ';
            tempo[1] = 0;
        }
        if (play.get_loc_name_last_time != 3000+aa)
            guis_need_update = 1;
        play.get_loc_name_last_time = 3000+aa;
        return;
    }
    onhs = getloctype_index;
    if (onhs>0) strcpy(tempo,get_translation(thisroom.Hotspots[onhs].Name));
    if (play.get_loc_name_last_time != onhs)
        guis_need_update = 1;
    play.get_loc_name_last_time = onhs;
}

int IsKeyPressed (int keycode) {
#ifdef ALLEGRO_KEYBOARD_HANDLER
    if (keyboard_needs_poll())
        poll_keyboard();

    switch(keycode) {
        case eAGSKeyCodeBackspace: return ags_iskeypressed(__allegro_KEY_BACKSPACE); break;
        case eAGSKeyCodeTab: return ags_iskeypressed(__allegro_KEY_TAB); break;
        case eAGSKeyCodeReturn: return ags_iskeypressed(__allegro_KEY_ENTER) || ags_iskeypressed(__allegro_KEY_ENTER_PAD); break;
        case eAGSKeyCodeEscape: return ags_iskeypressed(__allegro_KEY_ESC); break;
        case eAGSKeyCodeSpace: return ags_iskeypressed(__allegro_KEY_SPACE); break;
        case eAGSKeyCodeSingleQuote: return ags_iskeypressed(__allegro_KEY_QUOTE); break;
        case eAGSKeyCodeComma: return ags_iskeypressed(__allegro_KEY_COMMA); break;
        case eAGSKeyCodePeriod: return ags_iskeypressed(__allegro_KEY_STOP); break;
        case eAGSKeyCodeForwardSlash: return ags_iskeypressed(__allegro_KEY_SLASH) || ags_iskeypressed(__allegro_KEY_SLASH_PAD); break;
        case eAGSKeyCodeBackSlash: return ags_iskeypressed(__allegro_KEY_BACKSLASH) || ags_iskeypressed(__allegro_KEY_BACKSLASH2); break;
        case eAGSKeyCodeSemiColon: return ags_iskeypressed(__allegro_KEY_SEMICOLON); break;
        case eAGSKeyCodeEquals: return ags_iskeypressed(__allegro_KEY_EQUALS) || ags_iskeypressed(__allegro_KEY_EQUALS_PAD); break;
        case eAGSKeyCodeOpenBracket: return ags_iskeypressed(__allegro_KEY_OPENBRACE); break;
        case eAGSKeyCodeCloseBracket: return ags_iskeypressed(__allegro_KEY_CLOSEBRACE); break;
        // NOTE: we're treating EQUALS like PLUS, even though it is only available shifted.
        case eAGSKeyCodePlus: return ags_iskeypressed(__allegro_KEY_EQUALS) || ags_iskeypressed(__allegro_KEY_PLUS_PAD); break;
        case eAGSKeyCodeHyphen: return ags_iskeypressed(__allegro_KEY_MINUS) || ags_iskeypressed(__allegro_KEY_MINUS_PAD); break;

        // non-shifted versions of keys
        case eAGSKeyCodeColon: return ags_iskeypressed(__allegro_KEY_COLON) || ags_iskeypressed(__allegro_KEY_COLON2); break;
        case eAGSKeyCodeAsterisk: return ags_iskeypressed(__allegro_KEY_ASTERISK); break;
        case eAGSKeyCodeAt: return ags_iskeypressed(__allegro_KEY_AT); break;

        case eAGSKeyCode0: return ags_iskeypressed(__allegro_KEY_0); break;
        case eAGSKeyCode1: return ags_iskeypressed(__allegro_KEY_1); break;
        case eAGSKeyCode2: return ags_iskeypressed(__allegro_KEY_2); break;
        case eAGSKeyCode3: return ags_iskeypressed(__allegro_KEY_3); break;
        case eAGSKeyCode4: return ags_iskeypressed(__allegro_KEY_4); break;
        case eAGSKeyCode5: return ags_iskeypressed(__allegro_KEY_5); break;
        case eAGSKeyCode6: return ags_iskeypressed(__allegro_KEY_6); break;
        case eAGSKeyCode7: return ags_iskeypressed(__allegro_KEY_7); break;
        case eAGSKeyCode8: return ags_iskeypressed(__allegro_KEY_8); break;
        case eAGSKeyCode9: return ags_iskeypressed(__allegro_KEY_9); break;

        case eAGSKeyCodeA: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('A')); break;
        case eAGSKeyCodeB: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('B')); break;
        case eAGSKeyCodeC: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('C')); break;
        case eAGSKeyCodeD: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('D')); break;
        case eAGSKeyCodeE: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('E')); break;
        case eAGSKeyCodeF: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('F')); break;
        case eAGSKeyCodeG: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('G')); break;
        case eAGSKeyCodeH: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('H')); break;
        case eAGSKeyCodeI: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('I')); break;
        case eAGSKeyCodeJ: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('J')); break;
        case eAGSKeyCodeK: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('K')); break;
        case eAGSKeyCodeL: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('L')); break;
        case eAGSKeyCodeM: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('M')); break;
        case eAGSKeyCodeN: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('N')); break;
        case eAGSKeyCodeO: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('O')); break;
        case eAGSKeyCodeP: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('P')); break;
        case eAGSKeyCodeQ: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('Q')); break;
        case eAGSKeyCodeR: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('R')); break;
        case eAGSKeyCodeS: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('S')); break;
        case eAGSKeyCodeT: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('T')); break;
        case eAGSKeyCodeU: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('U')); break;
        case eAGSKeyCodeV: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('V')); break;
        case eAGSKeyCodeW: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('W')); break;
        case eAGSKeyCodeX: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('X')); break;
        case eAGSKeyCodeY: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('Y')); break;
        case eAGSKeyCodeZ: return ags_iskeypressed(platform->ConvertKeycodeToScanCode('Z')); break;

        case eAGSKeyCodeF1: return ags_iskeypressed(__allegro_KEY_F1); break;
        case eAGSKeyCodeF2: return ags_iskeypressed(__allegro_KEY_F2); break;
        case eAGSKeyCodeF3: return ags_iskeypressed(__allegro_KEY_F3); break;
        case eAGSKeyCodeF4: return ags_iskeypressed(__allegro_KEY_F4); break;
        case eAGSKeyCodeF5: return ags_iskeypressed(__allegro_KEY_F5); break;
        case eAGSKeyCodeF6: return ags_iskeypressed(__allegro_KEY_F6); break;
        case eAGSKeyCodeF7: return ags_iskeypressed(__allegro_KEY_F7); break;
        case eAGSKeyCodeF8: return ags_iskeypressed(__allegro_KEY_F8); break;
        case eAGSKeyCodeF9: return ags_iskeypressed(__allegro_KEY_F9); break;
        case eAGSKeyCodeF10: return ags_iskeypressed(__allegro_KEY_F10); break;
        case eAGSKeyCodeF11: return ags_iskeypressed(__allegro_KEY_F11); break;
        case eAGSKeyCodeF12: return ags_iskeypressed(__allegro_KEY_F12); break;

        case eAGSKeyCodeHome: return ags_iskeypressed(__allegro_KEY_HOME) || ags_iskeypressed(__allegro_KEY_7_PAD); break;
        case eAGSKeyCodeUpArrow: return ags_iskeypressed(__allegro_KEY_UP) || ags_iskeypressed(__allegro_KEY_8_PAD); break;
        case eAGSKeyCodePageUp: return ags_iskeypressed(__allegro_KEY_PGUP) || ags_iskeypressed(__allegro_KEY_9_PAD); break;
        case eAGSKeyCodeLeftArrow: return ags_iskeypressed(__allegro_KEY_LEFT) || ags_iskeypressed(__allegro_KEY_4_PAD); break;
        case eAGSKeyCodeNumPad5: return ags_iskeypressed(__allegro_KEY_5_PAD); break;
        case eAGSKeyCodeRightArrow: return ags_iskeypressed(__allegro_KEY_RIGHT) || ags_iskeypressed(__allegro_KEY_6_PAD); break;
        case eAGSKeyCodeEnd: return ags_iskeypressed(__allegro_KEY_END) || ags_iskeypressed(__allegro_KEY_1_PAD); break;
        case eAGSKeyCodeDownArrow: return ags_iskeypressed(__allegro_KEY_DOWN) || ags_iskeypressed(__allegro_KEY_2_PAD); break;
        case eAGSKeyCodePageDown: return ags_iskeypressed(__allegro_KEY_PGDN) || ags_iskeypressed(__allegro_KEY_3_PAD); break;
        case eAGSKeyCodeInsert: return ags_iskeypressed(__allegro_KEY_INSERT) || ags_iskeypressed(__allegro_KEY_0_PAD); break;
        case eAGSKeyCodeDelete: return ags_iskeypressed(__allegro_KEY_DEL) || ags_iskeypressed(__allegro_KEY_DEL_PAD); break;

        // These keys are not defined in the eAGSKey enum but are in the manual
        // https://adventuregamestudio.github.io/ags-manual/ASCIIcodes.html

        case 403: return ags_iskeypressed(__allegro_KEY_LSHIFT); break;
        case 404: return ags_iskeypressed(__allegro_KEY_RSHIFT); break;
        case 405: return ags_iskeypressed(__allegro_KEY_LCONTROL); break;
        case 406: return ags_iskeypressed(__allegro_KEY_RCONTROL); break;
        case 407: return ags_iskeypressed(__allegro_KEY_ALT); break;

        // (noted here for interest)
        // The following are the AGS_EXT_KEY_SHIFT, derived from applying arithmetic to the original keycodes.
        // These do not have a corresponding ags key enum, do not appear in the manual and may not be accessible because of OS contraints.

        case 392: return ags_iskeypressed(__allegro_KEY_PRTSCR); break;
        case 393: return ags_iskeypressed(__allegro_KEY_PAUSE); break;
        case 394: return ags_iskeypressed(__allegro_KEY_ABNT_C1); break;  // The ABNT_C1 (Brazilian) key
        case 395: return ags_iskeypressed(__allegro_KEY_YEN); break;
        case 396: return ags_iskeypressed(__allegro_KEY_KANA); break;
        case 397: return ags_iskeypressed(__allegro_KEY_CONVERT); break;
        case 398: return ags_iskeypressed(__allegro_KEY_NOCONVERT); break;
        case 400: return ags_iskeypressed(__allegro_KEY_CIRCUMFLEX); break;
        case 402: return ags_iskeypressed(__allegro_KEY_KANJI); break;
        case 420: return ags_iskeypressed(__allegro_KEY_ALTGR); break;
        case 421: return ags_iskeypressed(__allegro_KEY_LWIN); break;
        case 422: return ags_iskeypressed(__allegro_KEY_RWIN); break;
        case 423: return ags_iskeypressed(__allegro_KEY_MENU); break;
        case 424: return ags_iskeypressed(__allegro_KEY_SCRLOCK); break;
        case 425: return ags_iskeypressed(__allegro_KEY_NUMLOCK); break;
        case 426: return ags_iskeypressed(__allegro_KEY_CAPSLOCK); break;

        // Allegro4 keys that were never supported:
        // __allegro_KEY_COMMAND
        // __allegro_KEY_TILDE
        // __allegro_KEY_BACKQUOTE

        default:
            // Remaining Allegro4 keycodes are offset by AGS_EXT_KEY_SHIFT
            if (keycode >= AGS_EXT_KEY_SHIFT) {
                if (ags_iskeypressed(keycode - AGS_EXT_KEY_SHIFT)) { return 1; }
            }
            debug_script_log("IsKeyPressed: unsupported keycode %d", keycode);
            return 0;
    }
#else
    // old allegro version
    quit("allegro keyboard handler not in use??");
#endif
}

int SaveScreenShot(const char*namm) {
    String fileName;
    String svg_dir = get_save_game_directory();

    if (strchr(namm,'.') == nullptr)
        fileName.Format("%s%s.bmp", svg_dir.GetCStr(), namm);
    else
        fileName.Format("%s%s", svg_dir.GetCStr(), namm);

    Bitmap *buffer = CopyScreenIntoBitmap(play.GetMainViewport().GetWidth(), play.GetMainViewport().GetHeight());
    if (!buffer->SaveToFile(fileName, palette) != 0)
    {
        delete buffer;
        return 0;
    }
    delete buffer;
    return 1;  // successful
}

void SetMultitasking (int mode) {
    if ((mode < 0) | (mode > 1))
        quit("!SetMultitasking: invalid mode parameter");

    if (usetup.override_multitasking >= 0)
    {
        mode = usetup.override_multitasking;
    }

    // Don't allow background running if full screen
    if ((mode == 1) && (!scsystem.windowed))
        mode = 0;

    if (mode == 0) {
        if (set_display_switch_mode(SWITCH_PAUSE) == -1)
            set_display_switch_mode(SWITCH_AMNESIA);
        // install callbacks to stop the sound when switching away
        set_display_switch_callback(SWITCH_IN, display_switch_in_resume);
        set_display_switch_callback(SWITCH_OUT, display_switch_out_suspend);
    }
    else {
        if (set_display_switch_mode (SWITCH_BACKGROUND) == -1)
            set_display_switch_mode(SWITCH_BACKAMNESIA);
        set_display_switch_callback(SWITCH_IN, display_switch_in);
        set_display_switch_callback(SWITCH_OUT, display_switch_out);
    }
}

extern int getloctype_throughgui, getloctype_index;

void RoomProcessClick(int xx,int yy,int mood) {
    getloctype_throughgui = 1;
    int loctype = GetLocationType (xx, yy);
    VpPoint vpt = play.ScreenToRoomDivDown(xx, yy);
    if (vpt.second < 0)
        return;
    xx = vpt.first.X;
    yy = vpt.first.Y;

    if ((mood==MODE_WALK) && (game.options[OPT_NOWALKMODE]==0)) {
        int hsnum=get_hotspot_at(xx,yy);
        if (hsnum<1) ;
        else if (thisroom.Hotspots[hsnum].WalkTo.X<1) ;
        else if (play.auto_use_walkto_points == 0) ;
        else {
            xx=thisroom.Hotspots[hsnum].WalkTo.X;
            yy=thisroom.Hotspots[hsnum].WalkTo.Y;
            debug_script_log("Move to walk-to point hotspot %d", hsnum);
        }
        walk_character(game.playercharacter,xx,yy,0, true);
        return;
    }
    play.usedmode=mood;

    if (loctype == 0) {
        // click on nothing -> hotspot 0
        getloctype_index = 0;
        loctype = LOCTYPE_HOTSPOT;
    }

    if (loctype == LOCTYPE_CHAR) {
        if (check_click_on_character(xx,yy,mood)) return;
    }
    else if (loctype == LOCTYPE_OBJ) {
        if (check_click_on_object(xx,yy,mood)) return;
    }
    else if (loctype == LOCTYPE_HOTSPOT) 
        RunHotspotInteraction (getloctype_index, mood);
}

int IsInteractionAvailable (int xx,int yy,int mood) {
    getloctype_throughgui = 1;
    int loctype = GetLocationType (xx, yy);
    VpPoint vpt = play.ScreenToRoomDivDown(xx, yy);
    if (vpt.second < 0)
        return 0;
    xx = vpt.first.X;
    yy = vpt.first.Y;

    // You can always walk places
    if ((mood==MODE_WALK) && (game.options[OPT_NOWALKMODE]==0))
        return 1;

    play.check_interaction_only = 1;

    if (loctype == 0) {
        // click on nothing -> hotspot 0
        getloctype_index = 0;
        loctype = LOCTYPE_HOTSPOT;
    }

    if (loctype == LOCTYPE_CHAR) {
        check_click_on_character(xx,yy,mood);
    }
    else if (loctype == LOCTYPE_OBJ) {
        check_click_on_object(xx,yy,mood);
    }
    else if (loctype == LOCTYPE_HOTSPOT)
        RunHotspotInteraction (getloctype_index, mood);

    int ciwas = play.check_interaction_only;
    play.check_interaction_only = 0;

    if (ciwas == 2)
        return 1;

    return 0;
}

void GetMessageText (int msg, char *buffer) {
    VALIDATE_STRING(buffer);
    get_message_text (msg, buffer, 0);
}

void SetSpeechFont (int fontnum) {
    if ((fontnum < 0) || (fontnum >= game.numfonts))
        quit("!SetSpeechFont: invalid font number.");
    play.speech_font = fontnum;
}

void SetNormalFont (int fontnum) {
    if ((fontnum < 0) || (fontnum >= game.numfonts))
        quit("!SetNormalFont: invalid font number.");
    play.normal_font = fontnum;
}

void _sc_AbortGame(const char* text) {
    char displbuf[STD_BUFFER_SIZE] = "!?";
    snprintf(&displbuf[2], STD_BUFFER_SIZE - 3, "%s", text);
    quit(displbuf);
}

int GetGraphicalVariable (const char *varName) {
    InteractionVariable *theVar = FindGraphicalVariable(varName);
    if (theVar == nullptr) {
        quitprintf("!GetGraphicalVariable: interaction variable '%s' not found", varName);
        return 0;
    }
    return theVar->Value;
}

void SetGraphicalVariable (const char *varName, int p_value) {
    InteractionVariable *theVar = FindGraphicalVariable(varName);
    if (theVar == nullptr) {
        quitprintf("!SetGraphicalVariable: interaction variable '%s' not found", varName);
    }
    else
        theVar->Value = p_value;
}

int WaitImpl(int skip_type, int nloops)
{
    play.wait_counter = nloops;
    play.wait_skipped_by = SKIP_AUTOTIMER; // we set timer flag by default to simplify that case
    play.wait_skipped_by_data = 0;
    play.key_skip_wait = skip_type;

    GameLoopUntilValueIsZero(&play.wait_counter);

    if (game.options[OPT_BASESCRIPTAPI] < kScriptAPI_v351)
    {
        // < 3.5.1 return 1 is skipped by user input, otherwise 0
        return (play.wait_skipped_by & (SKIP_KEYPRESS | SKIP_MOUSECLICK) != 0) ? 1 : 0;
    }
    // >= 3.5.1 return positive keycode, negative mouse button code, or 0 as time-out
    switch (play.wait_skipped_by)
    {
    case SKIP_KEYPRESS: return play.wait_skipped_by_data;
    case SKIP_MOUSECLICK: return -(play.wait_skipped_by_data + 1); // convert to 1-based code and negate
    default: return 0;
    }
}

void scrWait(int nloops) {
    WaitImpl(SKIP_AUTOTIMER, nloops);
}

int WaitKey(int nloops) {
    return WaitImpl(SKIP_KEYPRESS | SKIP_AUTOTIMER, nloops);
}

int WaitMouse(int nloops) {
    return WaitImpl(SKIP_MOUSECLICK | SKIP_AUTOTIMER, nloops);
}

int WaitMouseKey(int nloops) {
    return WaitImpl(SKIP_KEYPRESS | SKIP_MOUSECLICK | SKIP_AUTOTIMER, nloops);
}

void SkipWait() {
    play.wait_counter = 0;
}
