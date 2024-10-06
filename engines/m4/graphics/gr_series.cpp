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

void Series::play(const char *seriesName, frac16 layer, uint32 flags,
		int16 triggerNum, int32 frameRate, int32 loopCount, int32 s,
		int32 x, int32 y, int32 firstFrame, int32 lastFrame) {
	_series = M4::series_play(seriesName, layer, flags, triggerNum, frameRate,
		loopCount, s, x, y, firstFrame, lastFrame);

	Common::String shadow = Common::String::format("%ss", seriesName);
	_seriesS = M4::series_play(shadow.c_str(), layer + 1, flags, -1, frameRate,
		loopCount, s, x, y, firstFrame, lastFrame);
}

void Series::show(const char *seriesName, frac16 layer, uint32 flags,
		int16 triggerNum, int32 duration, int32 index, int32 s, int32 x, int32 y) {
	_series = M4::series_show(seriesName, layer, flags, triggerNum, duration,
		index, s, x, y);

	Common::String shadow = Common::String::format("%ss", seriesName);
	_seriesS = M4::series_show(shadow.c_str(), layer + 1, flags, -1, duration,
		index, s, x, y);
}

void Series::show(const char *series1, const char *series2, int layer) {
	_series = M4::series_show(series1, layer);
	_seriesS = M4::series_show(series2, layer + 1);
}

void Series::show_index2(const char *series1, const char *series2, int layer, int index1, int index2) {
	_series = M4::series_show(series1, layer, 0, -1, -1, index1);
	_seriesS = M4::series_show(series2, layer + 1, 0, -1, -1, index1 + 1);
}

void Series::series_play(const char *seriesName, frac16 layer, uint32 flags,
		int16 triggerNum, int32 frameRate, int32 loopCount, int32 s,
		int32 x, int32 y, int32 firstFrame, int32 lastFrame) {
	Series tmp;
	tmp.play(seriesName, layer, flags, triggerNum, frameRate,
		loopCount, s, x, y, firstFrame, lastFrame);
}

void Series::series_show(const char *seriesName, frac16 layer, uint32 flags,
		int16 triggerNum, int32 duration, int32 index, int32 s, int32 x, int32 y) {
	Series tmp;
	tmp.show(seriesName, layer, flags, triggerNum, duration,
		index, s, x, y);
}


void Series::terminate() {
	if (_series)
		terminateMachineAndNull(_series);
	if (_seriesS)
		terminateMachineAndNull(_seriesS);
}


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
	M4sprite srcSprite, *srcSpritePtr;
	M4Rect clipRect, updateRect;

	if (!destBuff) {
		error_show(FL, 'BUF!');
		return false;
	}

	srcSpritePtr = &srcSprite;
	if ((srcSpritePtr = GetWSAssetSprite(nullptr, (uint32)spriteHash, (uint32)index, srcSpritePtr, nullptr)) == nullptr)
		error_show(FL, 'SPNF', "hash: %d, index: %d", spriteHash, index);

	HLock(srcSpritePtr->sourceHandle);
	//gr_pal_interface(&master_palette[0]);
	srcSpritePtr->data = (uint8 *)((intptr)*(srcSpritePtr->sourceHandle) + srcSpritePtr->sourceOffset);

	RendGrBuff Destination;
	DrawRequestX dr;
	RendCell Frame;

	Destination.Width = destBuff->stride;
	Destination.Height = destBuff->h;
	Destination.PixMap = (void *)destBuff->data;

	dr.x = x;
	dr.y = y;
	dr.scale_x = 100;
	dr.scale_y = 100;
	dr.depth_map = destBuff->data;
	dr.Pal = nullptr;
	dr.ICT = nullptr;
	dr.depth = 0;

	Frame.hot_x = srcSpritePtr->xOffset;
	Frame.hot_y = srcSpritePtr->yOffset;

	Frame.Width = srcSpritePtr->w;
	Frame.Height = srcSpritePtr->h;
	Frame.Comp = (uint32)srcSpritePtr->encoding;
	Frame.data = srcSpritePtr->data;

	clipRect.x1 = 0;
	clipRect.y1 = 0;
	clipRect.x2 = Destination.Width;
	clipRect.y2 = Destination.Height;

	// and draw the sprite
	render_sprite_to_8BBM(&Destination, &dr, &Frame, &clipRect, &updateRect);

	HUnLock(srcSpritePtr->sourceHandle);
	return true;

}

bool series_show_frame(int32 spriteHash, int32 index, Buffer *destBuff, int32 x, int32 y) {
	return series_draw_sprite(spriteHash, index, destBuff, x, y);
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
	sendWSMessage(0x10000, 0, m, 0, nullptr, 1);

	return true;
}

void series_set_frame_rate(machine *m, int32 newFrameRate) {
	CHECK_SERIES

	if ((!m) || (!m->myAnim8) || !verifyMachineExists(m))
		error_show(FL, 'SSFR');

	m->myAnim8->myRegs[IDX_CELS_FRAME_RATE] = newFrameRate << 16;
}

machine *series_show(const char *seriesName, frac16 layer, uint32 flags, int16 triggerNum,
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

machine *series_place_sprite(const char *seriesName, int32 index, int32 x, int32 y, int32 s, int32 layer) {
	return series_show(seriesName, layer, 0x40, -1, -1, index, s, x, y);
}

machine *series_show_sprite(const char *seriesName, int32 index, int32 layer) {
	return series_show(seriesName, layer, 0x40, -1, -1, index);
}

machine *series_play(const char *seriesName, frac16 layer, uint32 flags, int16 triggerNum,
		int32 frameRate, int32 loopCount, int32 s, int32 x, int32 y,
		int32 firstFrame, int32 lastFrame) {
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

machine *series_ranged_play(const char *seriesName, int32 loopCount, uint32 flags,
		int32 firstFrame, int32 lastFrame, int32 s, uint32 layer,
		int32 frameRate, int32 trigger, bool stickWhenDone) {
	if (loopCount == 1)
		loopCount = 0;
	if (stickWhenDone)
		flags |= 0x10;

	return series_play(seriesName, layer, flags, trigger, frameRate,
		loopCount, s, 0, 0, firstFrame, lastFrame);
}

machine *series_ranged_play_xy(const char *seriesName, int loopCount, int flags,
		int firstFrame, int lastFrame, int x, int y, int s, int layer,
		int frameRate, int trigger, bool stick_when_done) {
	if (loopCount == 1)
		loopCount = 0;
	if (stick_when_done)
		flags |= 0x10;

	return series_play(seriesName, layer, flags, trigger, frameRate,
		loopCount, s, x, y, firstFrame, lastFrame);
}

machine *series_plain_play(const char *seriesName, int32 loopCount, uint32 flags,
		int32 s, int32 layer, int32 frameRate, int32 trigger, bool stickWhenDone) {
	if (stickWhenDone)
		flags |= 0x10;
	if (loopCount == 1)
		loopCount = 0;

	return series_play(seriesName, layer, flags, trigger, frameRate, loopCount, s);
}

machine *series_play_xy(const char *seriesName, int loopCount, int flags,
		int x, int y, int scale, int depth, int layer, int frameRate) {
	// TODO: proper implementation
	warning("TODO: series_play_xy");
	return series_play(seriesName, layer, flags, -1, frameRate, loopCount,
		scale, x, y);
}

machine *series_simple_play(const char *seriesName, frac16 layer, bool stickWhenDone) {
	int flags = 0;
	if (stickWhenDone)
		flags |= 0x10;

	return series_play(seriesName, layer, flags);
}

void series_stream_check_series(machine *m, int val) {
	// TODO: series_stream_check_series
	error("TODO: series_stream_check_series");
}

} // namespace M4
