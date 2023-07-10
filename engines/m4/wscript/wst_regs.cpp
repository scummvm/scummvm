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

#include "m4/wscript/wst_regs.h"

namespace M4 {

const char *myRegLabels[IDX_COUNT] = {
	"timer",							//0
	"tag",								//1
	"layer",							//2
	
	"w",								//3
	"h",								//4
	"x",								//5
	"y",								//6
	"s",								//7
	"r",								//8

	"cels_hash",						//9
	"cels_index",						//10
	"cels_count",						//11
	"cels_frame_rate",					//12
	"cels_pix_speed",					//13

	"targ_s",							//14
	"targ_r",							//15
	"targ_x",							//16
	"targ_y",							//17

	"delta_s",							//18
	"delta_r",							//19
	"delta_x",							//20
	"delta_y",							//21
	
	"velocity",							//22
	"theta",							//23
	
	"ztemp1",							//24
	"ztemp2",							//25
	"ztemp3",							//26
	"ztemp4",							//27
	"ztemp5",							//28
	"ztemp6",							//29
	"ztemp7",							//30
	"ztemp8",							//31
	
	"mach_id"							//32
};

const char *myGlobLabels[GLOB_COUNT] = {
	"***SYSTEM - TIME",
	"***SYSTEM - WATCHDOG",
	"min_y",
	"max_y",
	"min_scale",
	"max_scale",
	"scaler",
	"g_temp1",
	"g_temp2",
	"g_temp3",
	"g_temp4",
	"g_temp5",
	"g_temp6",
	"g_temp7",
	"g_temp8",
	"g_temp9",
	"g_temp10",
	"g_temp11",
	"g_temp12",
	"g_temp13",
	"g_temp14",
	"g_temp15",
	"g_temp16",
	"g_temp17",
	"g_temp18",
	"g_temp19",
	"g_temp20",
	"g_temp21",
	"g_temp22",
	"g_temp23",
	"g_temp24",
	"g_temp25",
	"g_temp26",
	"g_temp27",
	"g_temp28",
	"g_temp29",
	"g_temp30",
	"g_temp31",
	"g_temp32",
};

const char *tagLabels[TAG_COUNT] = {
	"tag_none",
	"tag_targs",
	"tag_bez",
	"tag_deltas",
	"tag_vectors",
	"tag_text",
	"tag_move_cel",
	"tag_map_cel"
};

} // End of namespace M4
