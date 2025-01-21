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

#ifndef GOT_GAME_SCRIPT_H
#define GOT_GAME_SCRIPT_H

#include "got/data/defines.h"
#include "got/gfx/gfx_pics.h"

namespace Got {

typedef void (*ScriptEndFn)();

enum ScriptPause {
    SCRIPT_READY, SCRIPT_PAUSED, SCRIPT_RESUMING
};

class Scripts {
public:
	Scripts();
	~Scripts();

	void executeScript(long index, const Gfx::Pics &speakerIcon, ScriptEndFn endFn = nullptr);
	void pause();
	void resume();
	void setAskResponse(int option);
	void runIfResuming();

private:
    ScriptEndFn _endFn = nullptr;
    long  _numVar[26] = {};        // numeric variables
    char  _strVar[26][81] = {};    // string vars
    char  _lineLabel[32][9] = {};  // line label look up table
    char  *_linePtr[32] = {};      // line label pointers
    char  *_newPtr = nullptr;
    int   _numLabels = 0;          // number of labels
    char  *_gosubStack[32] = {};   // stack for GOSUB return addresses
    int   _gosubPtr = 0;           // GOSUB stack pointer
    char  *_forStack[11] = {};     // FOR stack
    long  _forVal[11] = {};        // current FOR value
    int8  _forVar[11] = {};        // ending FOR value (target var)
    int8  _forPtr = 0;	           // FOR stack pointer
    char  *_buffPtr = nullptr;     // pointer to current command
    char  *_buffEnd = nullptr;	   // pointer to end of buffer
    char  *_buffer = nullptr;      // buffer space (alloc'ed)
    long  _scrIndex = 0;
    Gfx::Pics _scrPic;
    long  _lValue = 0;
    long  _lTemp = 0;
    char  _tempS[255] = {};
    ScriptPause _paused = SCRIPT_READY;
    int _askVar = -1;

private:
    int  readScriptFile();
    void scriptError(int err_num);
    int  getCommand();
    int  skipColon();
    int  calcValue();
    int  getNextValue();
    int  calcString(int mode);
    void getStr();
    int  getInternalVariable();
    int  execCommand(int num);
    int getLine(char *src, char *dst);
    void scriptEntry() {}
    void scriptExit();

    int cmd_goto();
    int cmd_if();
    int cmd_run();
    int cmd_addJewels();
    int cmd_addHealth();
    int cmd_addMagic();
    int cmd_addKeys();
    int cmd_addScore();
    int cmd_say(int mode, int type);
    int cmd_ask();
    int cmd_sound();
    int cmd_setTile();
    int cmd_itemGive();
    int cmd_itemTake();
    int cmd_setFlag();
    int cmd_ltoa();
    int cmd_pause();
    int cmd_visible();
    int cmd_random();
    int cmd_exec();

    void scr_func1();
    void scr_func2();
    void scr_func3();
    void scr_func4();
    void scr_func5();

    typedef void (Scripts:: *ScrFunction)();
    static ScrFunction scr_func[5];

    void runScript(bool firstTime = true);
    void scriptLoop();

};

extern void executeScript(long index, const Gfx::Pics &speakerIcon,
                           ScriptEndFn endFn = nullptr);

} // namespace Got

#endif
