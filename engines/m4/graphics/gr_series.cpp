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

#include "m4/graphics/gr_series.h"
#include "m4/core/errors.h"
#include "m4/wscript/ws_load.h"
#include "m4/wscript/ws_machine.h"
#include "m4/wscript/wst_regs.h"
#include "m4/vars.h"

namespace M4 {

#define CHECK_SERIES if (!_G(globals)) error_show(FL, 'SERI');

static void series_trigger_dispatch_callback(frac16 myMessage, machine * /*sender*/) {
	kernel_trigger_dispatchx(myMessage);
}

int32 series_load(const char *seriesName, int32 assetIndex, RGB8 *myPal) {
	int32 myAssetIndex = AddWSAssetCELS(seriesName, assetIndex, myPal);

	if ((myAssetIndex < 0) || (myAssetIndex >= 256))
		error_show(FL, 'SPNF', seriesName);

	return myAssetIndex;
}

void series_unload(int32 assetIndex) {
	ClearWSAssets(_WS_ASSET_CELS, assetIndex, assetIndex);
}

bool series_draw_sprite(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y) {
	error("TODO: series_draw_sprite");
	return false;
}

bool series_show_frame(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y) {
	return series_draw_sprite(spriteHash, index, destBuff, x, y);
}

machine *series_play_xy(char *seriesName, int32 loopCount, uint32 flags,
		int32 x, int32 y, int32 s, int32 layer, int32 frameRate, int16 triggerNum) {
	error("TODO: series_play_xy");
	return nullptr;
}

machine *series_stream(const char *seriesName, int32 frameRate, int32 layer, int32 trigger) {
	machine *m;
	SysFile *sysFile = new SysFile(seriesName);

	// Store the frameRate in g_temp1
	// If it is < 0, the default frame rate for the ss will be used
	_G(globals)[GLB_TEMP_1] = frameRate << 16;

	// Store the SysFile pointer
	_G(globals)[GLB_TEMP_4] = (intptr)sysFile;

	// Set the callback trigger
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);

	// Set the layer
	_G(globals)[GLB_TEMP_6] = layer << 16;

	m = kernel_spawn_machine(seriesName, HASH_STREAM_MACHINE, series_trigger_dispatch_callback);
	return m;
}

bool series_stream_break_on_frame(machine *m, int32 frameNum, int32 trigger) {
	CHECK_SERIES

	// Parameter verification
	if (!m)
		return false;

	_G(globals)[GLB_TEMP_2] = frameNum << 16;
	_G(globals)[GLB_TEMP_3] = kernel_trigger_create(trigger);

	// Send the message to the machine to accept the new callback frame num and trigger
	SendWSMessage(0x10000, 0, m, 0, nullptr, 1);

	return true;
}

void series_set_frame_rate(machine *m, int32 newFrameRate) {
	CHECK_SERIES

	if ((!m) || (!m->myAnim8) || !VerifyMachineExists(m))
		error_show(FL, 'SSFR');

	m->myAnim8->myRegs[IDX_CELS_FRAME_RATE] = newFrameRate << 16;
}

machine *series_show_(const char *seriesName, frac16 layer, uint32 flags, int16 triggerNum,
	int32 duration, int32 index, int32 s, int32 x, int32 y) {
	CHECK_SERIES

	int32 myAssetIndex;
	RGB8 *tempPalettePtr = nullptr;

	term_message(seriesName);

	if (flags & SERIES_LOAD_PALETTE)
		tempPalettePtr = &_G(master_palette)[0];

	if ((myAssetIndex = AddWSAssetCELS(seriesName, -1, tempPalettePtr)) < 0)
		error_show(FL, 'SPNF', seriesName);

	_G(globals)[GLB_TEMP_1] = (frac16)myAssetIndex << 24;			// cels hash
	_G(globals)[GLB_TEMP_2] = layer << 16;							// layer

	_G(globals)[GLB_TEMP_3] = kernel_trigger_create(triggerNum);	// trigger

	_G(globals)[GLB_TEMP_4] = duration << 16;						// frame duration (-1=forever, 0=default)
	_G(globals)[GLB_TEMP_5] = index << 16;							// index of series to show

	_G(globals)[GLB_TEMP_6] = (s << 16) / 100;						// scale
	_G(globals)[GLB_TEMP_7] = x << 16;														// x
	_G(globals)[GLB_TEMP_8] = y << 16;														// y

	_G(globals)[GLB_TEMP_14] = (flags & SERIES_STICK) ? 0x10000 : 0;	// stick to screen after trigger?
	_G(globals)[GLB_TEMP_16] = (flags & SERIES_HORZ_FLIP) ? 0x10000 : 0;// horizontal flip

	machine *m = kernel_spawn_machine(seriesName, HASH_SERIES_SHOW_MACHINE, series_trigger_dispatch_callback);

	if (!m)
		error_show(FL, 'WSMF', seriesName);

	return m;
}

machine *series_play_(const char *seriesName, frac16 layer, uint32 flags, int16 triggerNum,
		int32 frameRate, int32 loopCount, int32 s, int32 x, int32 y,
		int32 firstFrame, int32 lastFrame) {
	CHECK_SERIES

	int32 myAssetIndex;
	RGB8 *tempPalettePtr = NULL;

	term_message(seriesName);

	if (flags & SERIES_LOAD_PALETTE)
		tempPalettePtr = &_G(master_palette)[0];

	if ((myAssetIndex = AddWSAssetCELS(seriesName, -1, tempPalettePtr)) < 0)
		error_show(FL, 'SPNF', seriesName);

	_G(globals)[GLB_TEMP_1] = (frac16)myAssetIndex << 24;			// cels hash
	_G(globals)[GLB_TEMP_2] = layer << 16;							// layer

	_G(globals)[GLB_TEMP_3] = kernel_trigger_create(triggerNum);	// trigger

	_G(globals)[GLB_TEMP_4] = frameRate << 16;						// framerate
	_G(globals)[GLB_TEMP_5] = loopCount << 16;						// loop count

	_G(globals)[GLB_TEMP_6] = (s << 16) / 100;						// scale
	_G(globals)[GLB_TEMP_7] = x << 16;								// x
	_G(globals)[GLB_TEMP_8] = y << 16;								// y

	_G(globals)[GLB_TEMP_9] = firstFrame << 16;						// first frame
	_G(globals)[GLB_TEMP_10] = lastFrame << 16;						// last frame

	_G(globals)[GLB_TEMP_11] = (flags & SERIES_PINGPONG) ? 0x10000 : 0;	// ping pong
	_G(globals)[GLB_TEMP_12] = (flags & SERIES_BACKWARD) ? 0x10000 : 0;	// backwards
	_G(globals)[GLB_TEMP_13] = (flags & SERIES_RANDOM) ? 0x10000 : 0;	// random
	_G(globals)[GLB_TEMP_14] = (flags & SERIES_STICK) ? 0x10000 : 0;	// stick to screen
	_G(globals)[GLB_TEMP_15] = (flags & SERIES_LOOP_TRIGGER) ? 0x10000 : 0;	// trigger back every loop?
	_G(globals)[GLB_TEMP_16] = (flags & SERIES_HORZ_FLIP) ? 0x10000 : 0;	// horizontal flip

	machine *m = kernel_spawn_machine(seriesName, HASH_SERIES_PLAY_MACHINE, series_trigger_dispatch_callback);

	if (!m)
		error_show(FL, 'WSMF', seriesName);

	return m;
}

} // namespace M4
