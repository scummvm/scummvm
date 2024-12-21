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

class Scripts {
private:
	long  num_var[26] = {};        // numeric variables
	char  str_var[26][81] = {};    // string vars
	char  line_label[32][9] = {};  // line label look up table
	char  *line_ptr[32] = {};      // line label pointers
	char  *new_ptr = nullptr;
	int   num_labels = 0;          // number of labels
	char  *gosub_stack[32] = {};   // stack for GOSUB return addresses
	char  gosub_ptr = 0;           // GOSUB stack pointer
	char  *for_stack[10] = {};     // FOR stack
	long  for_val[10] = {};        // current FOR value
	char  for_var[10] = {};        // ending FOR value (target var)
	char  for_ptr = 0;	           // FOR stack pointer
	char  *buff_ptr = nullptr;     // pointer to current command
	char  *buff_end = nullptr;	   // pointer to end of buffer
	char  *buffer = nullptr;       // buffer space (alloc'ed)
	long  scr_index = 0;
	Gfx::Pics *scr_pic = nullptr;
	long  lvalue = 0;
	long  ltemp = 0;
	char  temps[255] = {};

private:
	int  read_script_file();
	void script_error(int err_num);
	int  get_command();
	int  skip_colon();
	int  calc_value();
	int  get_next_val();
	int  calc_string(int mode);
	void get_str();
	int  get_internal_variable();
	int  exec_command(int num);
	int get_line(char *src, char *dst);
	void script_entry() {}
	void script_exit();

	int cmd_goto();
	int cmd_if();
	int cmd_run();
	int cmd_addjewels();
	int cmd_addhealth();
	int cmd_addmagic();
	int cmd_addkeys();
	int cmd_addscore();
	int cmd_say(int mode, int type);
	int cmd_ask();
	int cmd_sound();
	int cmd_settile();
	int cmd_itemgive();
	int cmd_itemtake();
	int cmd_setflag();
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

public:
	Scripts();
	~Scripts();

	void execute_script(long index, Gfx::Pics *pic);
};

extern void execute_script(long index, Gfx::Pics *pic);

} // namespace Got

#endif
