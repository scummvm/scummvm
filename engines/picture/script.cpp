/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/animation.h"
#include "picture/input.h"
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/script.h"
#include "picture/screen.h"
#include "picture/segmap.h"

namespace Picture {

ScriptInterpreter::ScriptInterpreter(PictureEngine *vm) : _vm(vm) {

	_stack = new byte[4096 + 4];

	memset(_slots, 0, sizeof(_slots));

	_savedSp = 0;

}

ScriptInterpreter::~ScriptInterpreter() {
	delete[] _stack;
}

void ScriptInterpreter::loadScript(uint resIndex, uint slotIndex) {

	if (_slots[slotIndex].data) {
		delete[] _slots[slotIndex].data;
	}

 	_slots[slotIndex].resIndex = resIndex;
	byte *scriptData = _vm->_res->load(resIndex);
	_slots[slotIndex].size = _vm->_res->getCurItemSize();
 	_slots[slotIndex].data = new byte[_slots[slotIndex].size];
 	memcpy(_slots[slotIndex].data, scriptData, _slots[slotIndex].size);

}

void ScriptInterpreter::runScript(uint slotIndex) {

	_switchLocalDataNear = true;
	_switchLocalDataFar = false;
	_switchLocalDataToStack = false;
	_cmpBitTest = false;

	_regs.reg0 = 0;
	_regs.reg1 = 0;
	_regs.reg2 = 0;
	_regs.reg3 = 0;
	_regs.reg4 = slotIndex;
	_regs.reg5 = 0;
	_regs.reg6 = 0;
	_regs.sp = 4096;
	_regs.reg8 = 0;

	_code = getSlotData(_regs.reg4);

	while (1) {

		if (_vm->_movieSceneFlag)
			_vm->_input->_mouseButton = 0;
			
		if (_switchLocalDataNear) {
			_switchLocalDataNear = false;
			_localData = getSlotData(_regs.reg4);
		}

		if (_switchLocalDataFar) {
			_switchLocalDataFar = false;
			_localData = getSlotData(_regs.reg5);
			_switchLocalDataNear = true;
		}

		if (_switchLocalDataToStack) {
			_switchLocalDataToStack = false;
			_localData = _stack + 2;
			_switchLocalDataNear = true;
		}
		
		byte opcode = readByte();
		execOpcode(opcode);

		// Call updateScreen roughly every 10ms else the mouse cursor will be jerky
		if (_vm->_system->getMillis() % 10 == 0)
			_vm->_system->updateScreen();
		
	}

}

byte ScriptInterpreter::readByte() {
	return *_code++;
}

int16 ScriptInterpreter::readInt16() {
	int16 value = READ_LE_UINT16(_code);
	_code += 2;
	return value;
}

void look(byte *code) {
	char ln[256];
	snprintf(ln, 256, "\t\t\t%02X %02X %02X %02X %02X %02X %02X %02X",
		code[0], code[1], code[2], code[3], code[4], code[5], code[6], code[7]);
	debug(1, "%s", ln);
}

void ScriptInterpreter::execOpcode(byte opcode) {

#if 0
	char ln[256];
	snprintf(ln, 256, "\t\t\t%02X %02X %02X %02X %02X %02X %02X %02X",
		_code[0], _code[1], _code[2], _code[3], _code[4], _code[5], _code[6], _code[7]);
	debug(1, "%s", ln);
#endif

	int16 ofs;

	debug(1, "opcode = %d", opcode);

	switch (opcode) {
	case 0:
	{
		// ok
		_subCode = _code;
		byte length = readByte();
		debug(1, "length = %d", length);
		uint16 kernelOpcode = readInt16();
		debug(1, "callKernel %d", kernelOpcode);
		execKernelOpcode(kernelOpcode);
		_code += length - 2;
		break;
	}
	case 1:
		// ok
		_regs.reg0 = readInt16();
		debug(1, "mov reg0, #%d", _regs.reg0);
		break;
	case 2:
		// ok
		_regs.reg1 = readInt16();
		debug(1, "mov reg1, #%d", _regs.reg1);
		break;
	case 3:
		// ok
		_regs.reg3 = readInt16();
		debug(1, "mov reg3, #%d", _regs.reg3);
		break;
	case 4:
		// ok
		_regs.reg5 = _regs.reg0;
		debug(1, "mov reg5, reg0");
		break;
	case 5:
		// ok
		_regs.reg3 = _regs.reg0;
		debug(1, "mov reg3, reg0");
		break;
	case 6:
		// ok
		_regs.reg1 = _regs.reg0;
		debug(1, "mov reg1, reg0");
		break;
	case 7:
		_regs.reg1 = localRead16(_regs.reg3);
		debug(1, "mov reg1, *%d", _regs.reg3);
		break;
	case 8:
		localWrite16(_regs.reg3, _regs.reg0);
		break;
	case 9:
		localWrite16(readInt16(), _regs.reg0);
		break;
	case 10:
		localWrite8(readInt16(), _regs.reg0);
		break;
	case 11:
		localWrite16(readInt16(), _regs.reg5);
		break;
	case 12:
		localWrite16(readInt16(), _regs.reg4);
		break;
	case 13:
		localWrite16(readInt16(), _regs.reg3);
		break;
	case 14:
		_regs.reg3 = localRead16(readInt16());
		break;
	case 15:
		_regs.reg2 = localRead16(_regs.reg1);
		break;
	case 16:
		_regs.reg2 = localRead16(_regs.reg1 + readInt16());
		break;
	case 17:
		_regs.reg2 = _regs.reg0;
		break;
	case 18:
		_regs.reg0 += readInt16();
		break;
	case 19:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) + _regs.reg0);
		break;
	case 20:
		_regs.reg0 += _regs.reg2;
		break;
	case 21:
		_regs.reg3 += _regs.sp;
		break;
	case 22:
		_regs.reg1 += _regs.sp;
		break;
	case 23:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) - _regs.reg0);
		break;
	case 24:
		_regs.reg0 /= readInt16();
		break;
	case 25:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) / _regs.reg0);
		break;
	case 26:
		// NOP
		break;
	case 27:
		_regs.reg0 *= readInt16();
		break;
	case 28:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) * _regs.reg0);
		break;
	case 29:
		_regs.reg0 *= _regs.reg2;
		break;
	case 30:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) + 1);
		break;
	case 31:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) - 1);
		break;
	case 32:
		_switchLocalDataFar = true;
		break;
	case 33:
		_switchLocalDataToStack = true;
		break;
	case 34:
		pushInt16(_regs.reg0);
		debug(1, "pushw reg0");
		break;
	case 35:
		pushInt16(_regs.reg1);
		debug(1, "pushw reg1");
		break;
	case 36:
		_regs.reg1 = popInt16();
		debug(1, "popw reg1");
		break;
	case 37:
		_regs.reg0 = popInt16();
		debug(1, "popw reg0");
		break;
	case 38:
		_regs.reg2 = -_regs.reg2;
		break;
	case 39:
		_regs.reg8 = readInt16();
		_cmpBitTest = false;
		break;
	case 40:
		_regs.reg8 = _regs.reg0;
		_cmpBitTest = false;
		break;
	case 41:
		_regs.reg8 = readInt16();
		_cmpBitTest = true;
		break;
	case 42:
		_regs.reg8 = _regs.reg0;
		_cmpBitTest = true;
		break;
	case 43:
		debug(1, "retn (slot: %d; ofs: %04X)\n", _regs.reg4, _regs.reg0);
		_code = getSlotData(_regs.reg4) + _regs.reg0;
		break;
	case 44:
		debug(1, "retf (slot: %d; ofs: %04X)\n", _regs.reg5, _regs.reg0);
		_code = getSlotData(_regs.reg5) + _regs.reg0;
		_regs.reg4 = _regs.reg5;
		_switchLocalDataNear = true;
		break;
	case 45:
		debug(1, "callnear %04X (slot: %d; ofs: %04X)\n", _regs.reg0, _regs.reg4, _regs.reg0);
		pushInt16(_code - getSlotData(_regs.reg4));
		pushInt16(_regs.reg4);
		_code = getSlotData(_regs.reg4) + _regs.reg0;
		break;
	case 46:
		debug(1, "callfar %04X (slot: %d; ofs: %04X)\n", _regs.reg0, _regs.reg5, _regs.reg0);
		pushInt16(_code - getSlotData(_regs.reg4));
		pushInt16(_regs.reg4);
		_code = getSlotData(_regs.reg5) + _regs.reg0;
		_regs.reg4 = _regs.reg5;
		_switchLocalDataNear = true;
		break;
	case 47:
		_regs.reg4 = popInt16();
		ofs = popInt16();
		_code = getSlotData(_regs.reg4) + ofs;
		debug(1, "ret (slot: %d; ofs: %04X)\n", _regs.reg4, ofs);
		//_code = getSlotData(_regs.reg4) + popInt16();
		_switchLocalDataNear = true;
		break;
	case 48:
		_regs.reg4 = popInt16();
		ofs = popInt16();
		_code = getSlotData(_regs.reg4) + ofs;
		debug(1, "retsp (slot: %d; ofs: %04X)\n", _regs.reg4, ofs);
		//_code = getSlotData(_regs.reg4) + popInt16();
		_regs.sp += _regs.reg0;
		_switchLocalDataNear = true;
		break;
	case 49:
		ofs = readByte();
		debug(0, "49, len = %d", ofs);
		_code += ofs;
		break;
	case 50:
		if (_cmpBitTest) {
			_regs.reg1 &= _regs.reg8;
			if (_regs.reg1 == 0)
				_code += 4;
		} else {
			if (_regs.reg1 == _regs.reg8)
				_code += 4;
		}
		_code++;
		break;
	case 51:
		if (_cmpBitTest) {
			_regs.reg1 &= _regs.reg8;
			if (_regs.reg1 != 0)
				_code += 4;
		} else {
			if (_regs.reg1 != _regs.reg8)
				_code += 4;
		}
		_code++;
		break;
	case 52:
		if (_regs.reg1 >= _regs.reg8)
			_code += 4;
		_code++;
		break;
	case 53:
		if (_regs.reg1 <= _regs.reg8)
			_code += 4;
		_code++;
		break;
	case 54:
		if (_regs.reg1 < _regs.reg8)
			_code += 4;
		_code++;
		break;
	case 55:
		if (_regs.reg1 > _regs.reg8)
			_code += 4;
		_code++;
		break;
	default:
	{
		/*
		FILE *ex = fopen("error.0", "wb");
		fwrite(_code - 8, 4096, 1, ex);
		fclose(ex);
		*/
		error("Invalid opcode %d", opcode);
	}
	}

}

void ScriptInterpreter::execKernelOpcode(uint16 kernelOpcode) {

	switch (kernelOpcode) {
	
	case 0:
	case 1:
		// ok, NOPs
		break;

	case 2:// ok
	{
		debug(0, "o2_getGameVar(%d, %d)", arg16(3), arg16(5));
		int16 value = getGameVar(arg16(3));
		localWrite16(arg16(5), value);
		break;
	}

	case 3:// ok
	{
		debug(0, "o2_setGameVar(%d, %d)", arg16(3), arg16(5));
		VarType varType = getGameVarType(arg16(3));
		int16 value;
		if (varType == vtByte)
			value = arg8(5);
		else if (varType == vtWord)
			value = arg16(5);
		setGameVar(arg16(3), value);
		break;
	}

	case 4:
	{

		debug(0, "o2_updateScreen()");

		// TODO? updateSamples();

		_vm->_screen->updateShakeScreen();

		if (_vm->_quitGame)
			return;

		if (!_vm->_movieSceneFlag)
			_vm->_input->update();
		else
			_vm->_input->_mouseButton = 0;

		// TODO? Check keyb
		
		_vm->_counter01--;
		if (_vm->_counter01 <= 0) {
			_vm->_counter01 = MIN(_vm->_counter02, 30);
			_vm->_counter02 = 0;
			_vm->updateScreen();
			_vm->_flag01 = 1;
			_vm->_system->delayMillis(5);
			_vm->_counter02 = 1; // ?
		} else {
			_vm->_screen->clearSprites();
			_vm->_flag01 = 0;
			//_vm->_system->updateScreen();
		}

		// TODO
		break;

	}

	case 5:// ok
	{
		debug(0, "o2_mouseDeltaStuff(%d)", arg16(3));
		localWrite16(arg16(5), _vm->_input->getMouseDeltaStuff(arg16(3)));
		break;
	}

	case 6:// ok
	{
		debug(0, "o2_printText()");
		_vm->_screen->printText((byte*)localPtr(arg16(3)));
		break;
	}

	case 7:// ok
	{
		debug(0, "o2_updateVerbLine(slot: %d; offset: %04X)", arg16(5), arg16(3));
		_vm->_screen->updateVerbLine(arg16(5), arg16(3));
		break;
	}

	case 8:// ok
	{
		debug(0, "o2_setFontColor(%d)", arg8(3));
		_vm->_screen->_fontColor1 = 0;
		_vm->_screen->_fontColor2 = arg8(3);
		break;
	}

	case 9:// ok
	{
		debug(0, "o2_getTalkTextDuration()");
		int16 duration = _vm->_screen->getTalkTextDuration();
		localWrite16(arg16(3), duration);
		break;
	}

	case 10:// ok
	{
		debug(0, "o2_talk(slot: %d; offset: %d)", arg16(5), arg16(3));
		_vm->talk(arg16(5), arg16(3));
		break;
	}
	
	case 11:// ok
	{
		debug(0, "o2_findFragment(%d, %d)", arg16(3), arg16(5));
		localWrite16(arg16(5), _vm->_palette->findFragment(arg16(3)));
		break;
	}

	case 12:// ok
	{
		debug(0, "o2_clearPaletteFragments()");
		_vm->_palette->clearFragments();
		break;
	}
	
	case 13:// ok
	{
		debug(0, "o2_addFragment(%d, %d)", arg16(3), arg16(5));
		_vm->_palette->addFragment(arg16(3), arg16(5));
		break;
	}
	
	case 14:// ok
	{
		debug(0, "o2_setDeltaPalette(animPalette, %d, %d, %d, %d)", arg8(6), arg8(5), arg8(4), arg8(3));
		_vm->_palette->setDeltaPalette(_vm->_anim->_palette, arg8(6), (char)arg8(5), arg8(4), arg8(3));
		break;
	}

	case 16:// TODO
	{
		debug(0, "o2_makeTransColorTable");
		break;
	}

	case 17:// ok
	{
		debug(0, "o2_setDeltaPalette(mainPalette, %d, %d, %d, %d)", arg8(6), arg8(5), arg8(4), arg8(3));
		_vm->_palette->setDeltaPalette(_vm->_palette->getMainPalette(), arg8(6), (char)arg8(5), arg8(4), arg8(3));
		break;
	}

	case 18:// ok
	{
		debug(0, "o2_loadScript(resIndex: %d; slotIndex: %d)", arg16(4), arg8(3));
		int16 codeOfs = _code - getSlotData(_regs.reg4);
		loadScript(arg16(4), arg8(3));
		_code = getSlotData(_regs.reg4) + codeOfs;
		_switchLocalDataNear = true;
		break;
	}
	
	case 19:// ok
	{
		debug(0, "o2_registerFont(%d, %d)", arg8(3), arg16(4));
		_vm->_screen->registerFont(arg8(3), arg16(4));
		break;
	}

	case 20:// ok
	{
		debug(0, "o2_loadAddPalette(startIndex: %d; resIndex: %d)", arg8(3), arg16(4));
		_vm->_palette->loadAddPalette(arg16(4), arg8(3));
		break;
	}

	case 21:// TODO
	{
		debug(0, "o2_loadScene(resIndex: %d; flag: %d)", arg16(4), arg8(3));
		if (arg8(3) == 0) {
			_vm->loadScene(arg16(4));
		} else {
			_vm->_screen->loadMouseCursor(arg16(4));
		}
		break;
	}

	case 22:// ok
	{
		debug(0, "o2_setCameraTop(%d)", arg8(3));
		_vm->setCameraTop(arg8(3));
		break;
	}

	case 23:// ok
	{
		debug(0, "o2_findMouseInRectIndex1(offset: %d; slot: %d; elemSize: %d; var: %d; index: %d)", arg16(3), arg16(5), arg16(7), arg16(9), arg16(11));
		int16 index = -1;
		if (_vm->_input->_mouseY < _vm->_cameraHeight) {
			index = _vm->findRectAtPoint(getSlotData(arg16(5)) + arg16(3),
				_vm->_input->_mouseX + _vm->_cameraX,
				_vm->_input->_mouseY + _vm->_cameraY,
				arg16(11) + 1, arg16(7));
		}
		localWrite16(arg16(9), index);
		break;
	}

	case 24:// ok
	{
		debug(0, "o2_findMouseInRectIndex2(offset: %d, slot: %d, elemSize: %d, var: %d)", arg16(3), arg16(5), arg16(7), arg16(9));
		int16 index = -1;

		debug(0, "_vm->_input->_mouseDisabled = %d", _vm->_input->_mouseDisabled);

		/* FIXME: This opcode is called after the Revistronic logo at the beginning,
			but at the slot/offset there's bytecode and not a rect array as expected.
			To avoid crashes we skip searching the rectangle index for now when scene 215 is active.
			I don't know yet whether this is a bug in the original engine as well or just here.
			Needs some more checking.
		*/
		if (_vm->_sceneResIndex != 215) {
			if (_vm->_input->_mouseY < _vm->_cameraHeight) {
				index = _vm->findRectAtPoint(getSlotData(arg16(5)) + arg16(3),
					_vm->_input->_mouseX + _vm->_cameraX,
					_vm->_input->_mouseY + _vm->_cameraY,
					0, arg16(7));
			}
		}
		
		localWrite16(arg16(9), index);
		
		break;
	}

	case 25:// ok
	{
  		debug(0, "o2_drawGuiImage(x: %d; y: %d; resIndex: %d)", arg16(5), arg16(3), arg16(7));
  		_vm->_screen->drawGuiImage(arg16(5), arg16(3), arg16(7));
		break;
	}

	case 26:// ok
	{
		debug(0, "o2_addAnimatedSpriteNoLoop(2; x: %d; y: %d; fragmentId: %d; offset: %d)", arg16(5), arg16(3), arg16(7), arg16(9));
		_vm->_screen->addAnimatedSprite(arg16(5), arg16(3), arg16(7), (byte*)localPtr(0), (int16*)localPtr(arg16(9)), false, 2);
		break;
	}

	case 27:// ok
	{
		debug(0, "o2_addAnimatedSprite(2; x: %d; y: %d; fragmentId: %d; offset: %d)", arg16(5), arg16(3), arg16(7), arg16(9));
		_vm->_screen->addAnimatedSprite(arg16(5), arg16(3), arg16(7), (byte*)localPtr(0), (int16*)localPtr(arg16(9)), true, 2);
		break;
	}

	case 28:// ok
	{
		debug(1, "o2_addStaticSprite()");
		_vm->_screen->addStaticSprite(_subCode + 3);
		break;
	}

	case 29:// ok
	{
		debug(0, "o2_addAnimatedSprite(1; x: %d; y: %d; value: %d; offset: %d)", arg16(5), arg16(3), arg16(7), arg16(9));
		_vm->_screen->addAnimatedSprite(arg16(5), arg16(3), arg16(7), (byte*)localPtr(0), (int16*)localPtr(arg16(9)), true, 1);
		break;
	}

	case 30:// ok
	{
		debug(0, "o2_findPath(sourceX: %d; sourceY: %d; destX: %d; destY: %d; slotIndex: %d; offset: %d)", arg16(5), arg16(3), arg16(9), arg16(7), arg16(13), arg16(11));
		_vm->_segmap->findPath((int16*)(getSlotData(arg16(13)) + arg16(11)), arg16(9), arg16(7), arg16(5), arg16(3));
		break;
	}

	case 31:// ok
	{
		debug(0, "o2_walk()");
		_vm->walk(getSlotData(arg16(5)) + arg16(3));
		break;
	}

	case 32:// ok
	{
		debug(0, "o2_scrollCameraUp()");
		_vm->scrollCameraUp(4);
		break;
	}

	case 33:// ok
	{
		debug(0, "o2_scrollCameraDown()");
		_vm->scrollCameraDown(4);
		break;
	}

	case 34:// ok
	{
		debug(0, "o2_scrollCameraLeft()");
		_vm->scrollCameraLeft(4);
		break;
	}

	case 35:// ok
	{
		debug(0, "o2_scrollCameraRight()");
		_vm->scrollCameraRight(4);
		break;
	}

	case 36:// ok
	{
		debug(0, "o2_scrollCameraUpEx(%d)", arg16(3));
		_vm->scrollCameraUp(arg16(3));
		break;
	}

	case 37:// ok
	{
		debug(0, "o2_scrollCameraDownEx(%d)", arg16(3));
		_vm->scrollCameraDown(arg16(3));
		break;
	}

	case 38:// ok
	{
		debug(0, "o2_scrollCameraLeftEx(%d)", arg16(3));
		_vm->scrollCameraLeft(arg16(3));
		break;
	}

	case 39:// ok
	{
		debug(0, "o2_scrollCameraRightEx(%d)", arg16(3));
		_vm->scrollCameraRight(arg16(3));
		break;
	}

	case 40:// ok
	{
		debug(0, "o2_setCamera(%d, %d)", arg16(5), arg16(3));
		_vm->setCamera(arg16(5), arg16(3));
		break;
	}

	case 42:// ok
	{
		debug(0, "o2_getRgbModifiertAtPoint(x: %d; y: %d; id: %d; varSlot: %d; varOffset: %d)", arg16(5), arg16(3), arg16(7), arg16(11), arg16(9));
		byte *rgb = getSlotData(arg16(11)) + arg16(9);
		_vm->_segmap->getRgbModifiertAtPoint(arg16(5), arg16(3), arg16(7), rgb[0], rgb[1], rgb[2]);
		break;
	}

	case 43:// ok
	{
		debug(0, "o2_startAnim(%d)", arg16(3));
		_vm->_anim->start(arg16(3));
		break;
	}
	
	case 44:// ok
	{
		debug(0, "o2_animNextFrame()");
		_vm->_anim->nextFrame();
		break;
	}
	
	case 45:// ok
	{
		// NOP
		break;
	}

	case 46:// ok
	{
		debug(0, "o2_getAnimFrameNumber(%d)", arg16(3));
		localWrite16(arg16(3), _vm->_anim->getFrameNumber());
		break;
	}

	case 47:
	{
		// almost ok
		debug(0, "o2_getAnimStatus()");
		int16 status = _vm->_anim->getStatus();
		if (status == 0 || status == 1) {
			// TODO mov screenFlag01, 0
		}
		localWrite16(arg16(3), status);
		break;
	}

	case 48:// ok
	{
		_vm->_screen->startShakeScreen(arg16(3));
		break;
	}

	case 49:// ok
	{
		_vm->_screen->stopShakeScreen();
		break;
	}

	case 50:// TODO
	{
		debug(0, "o2_startSequence");
		break;
	}

	case 51:// TODO
	{
		debug(0, "o2_endSequence");
		break;
	}

	case 52:// TODO
	{
		debug(0, "o2_sequenceVolumeStuff");
		break;
	}

	case 53:// TODO
	{
		debug(0, "o2_playSound1(%d, %d, %d, %d)", arg16(9), arg16(7), arg16(5), arg16(3));
		break;
	}

	case 54:// TODO
	{
		debug(0, "o2_playSound2(%d, %d, %d)", arg16(7), arg16(5), arg16(3));
		break;
	}

	case 55:// TODO
		debug(0, "o2_clearScreen()");
		break;

	case 56:// ok
	{
		// NOP
		break;
	}

	case 57:// TODO
	{
		debug(0, "o2_handleInput");
		int16 varOfs = arg16(3);

		localWrite16(varOfs, 0);

		//_vm->_input->update();
		break;
	}

	case 58:// TODO
	{
		debug(0, "o2_runOptionsScreen(%d, %d)", arg16(5), arg16(3));
		break;
	}

	case 59:// TODO
	{
		debug(0, "o2_precacheResources(%04X)", arg16(3));
		break;
	}

	case 60:// TODO
	{
		debug(0, "o2_precacheSounds1(%04X)", arg16(3));
		// CHECKME
		_vm->_screen->clearSprites();
		break;
	}

	case 61:// TODO
	{
		debug(0, "o2_deleteAllPbfFilesByExternalArray()");
		break;
	}

	case 63:// ok
	{
		_regs.sp = _savedSp;
		break;
	}

	case 64:// ok
	{
		_savedSp = _regs.sp;
		break;
	}

	case 65:// TODO
	{
		debug(0, "o2_playMovie(%d, %d)", arg16(3), arg16(5));
		break;
	}
	
	case 66:
		// NOP
		break;

	default:
		error("Invalid kernel opcode %d", kernelOpcode);
	}

}

ScriptInterpreter::VarType ScriptInterpreter::getGameVarType(uint variable) {
	switch (variable) {
		case 0:	 return vtByte;
		case 1:	 return vtWord;
		case 2:	 return vtWord;
		case 3:	 return vtByte;
		case 4:	 return vtWord;
		case 5:	 return vtWord;
		case 6:	 return vtWord;
		case 7:	 return vtWord;
		case 8:	 return vtWord;
		case 9:	 return vtWord;
		case 10:	 return vtWord;
		case 11:	 return vtWord;
		case 12:	 return vtByte;
		case 13:	 return vtWord;
		case 14:	 return vtWord;
		case 15:	 return vtWord;
		case 16:	 return vtWord;
		case 17:	 return vtWord;
		case 18:	 return vtWord;
		case 19:	 return vtWord;
		case 20:	 return vtWord;
		case 21:	 return vtWord;
		default:
			error("Invalid game variable");
	}
}

const char *getVarName(uint variable) {
	switch (variable) {
		case 0: return "mouseDisabled";
		case 1: return "mouseY";
		case 2: return "mouseX";
		case 3: return "mouseButton";
		case 4: return "verbLineY";
		case 5: return "verbLineX";
		case 6: return "verbLineWidth";
		case 7: return "verbLineCount";
		case 8: return "verbLineNum";
		case 9: return "talkTextItemNum";
		case 10: return "talkTextY";
		case 11: return "talkTextX";
		case 12: return "talkTextFontColor";
		case 13: return "cameraY";
		case 14: return "cameraX";
		case 15: return "walkSpeedY";
		case 16: return "walkSpeedX";
		case 17: return "flag01";
		case 18: return "sceneResIndex";
		case 19: return "cameraTop";
		case 20: return "sceneHeight";
		case 21: return "sceneWidth";
	}
	return "(invalid)";
}

int16 ScriptInterpreter::getGameVar(uint variable) {
	debug(0, "ScriptInterpreter::getGameVar(%d{%s})", variable, getVarName(variable));
	
	int16 value = 0;
	
	switch (variable) {
		case 0:
			value = _vm->_input->_mouseDisabled;
			break;
		case 1:
			value = _vm->_input->_mouseY;
			break;
		case 2:
			value = _vm->_input->_mouseX;
			break;
		case 3:
			value = _vm->_input->_mouseButton;
			break;
		case 4:
			value = _vm->_screen->_verbLineY;
			break;
		case 5:
			value = _vm->_screen->_verbLineX;
			break;
		case 6:
			value = _vm->_screen->_verbLineWidth;
			break;
		case 7:
			value = _vm->_screen->_verbLineCount;
			break;
		case 8:
			value = _vm->_screen->_verbLineNum;
			break;
		case 9:
			value = _vm->_screen->_talkTextItemNum;
			break;
		case 10:
			value = _vm->_screen->_talkTextY;
			break;
		case 11:
			value = _vm->_screen->_talkTextX;
			break;
		case 12:
			value = _vm->_screen->_talkTextFontColor;
			break;
		case 13:
			value = _vm->_cameraY;
			break;
		case 14:
			value = _vm->_cameraX;
			break;
		case 15:
			value = _vm->_walkSpeedY;
			break;
		case 16:
			value = _vm->_walkSpeedX;
			break;
		case 17:
			value = _vm->_flag01;
			break;
		case 18:
			value = _vm->_sceneResIndex;
			break;
		case 19:
			value = _vm->_cameraTop;
			break;
		case 20:
			value = _vm->_sceneHeight;
			break;
		case 21:
			value = _vm->_sceneWidth;
			break;
		default:
			warning("Getting unimplemented game variable %s (%d)", getVarName(variable), variable);
			break;
	}

	
	return value;

}

void ScriptInterpreter::setGameVar(uint variable, int16 value) {
	debug(0, "ScriptInterpreter::setGameVar(%d{%s}, %d)", variable, getVarName(variable), value);
	
	switch (variable) {
		case 0:
			_vm->_input->_mouseDisabled = value;
			_vm->_system->showMouse(value == 0);
			break;
		case 3:
			_vm->_input->_mouseButton = value;
			break;
		case 4:
			_vm->_screen->_verbLineY = value;
			break;
		case 5:
			_vm->_screen->_verbLineX = value;
			break;
		case 6:
			_vm->_screen->_verbLineWidth = value;
			break;
		case 7:
			_vm->_screen->_verbLineCount = value;
			break;
		case 8:
			_vm->_screen->_verbLineNum = value;
			break;
		case 9:
			_vm->_screen->_talkTextItemNum = value;
			break;
		case 10:
			_vm->_screen->_talkTextY = value;
			break;
		case 11:
			_vm->_screen->_talkTextX = value;
			break;
		case 12:
			_vm->_screen->_talkTextFontColor = value;
			break;
		case 13:
			_vm->_cameraY = value;
			break;
		case 14:
			_vm->_cameraX = value;
			break;
		case 15:
			_vm->_walkSpeedY = value;
			break;
		case 16:
			_vm->_walkSpeedX = value;
			break;
		case 17:
			_vm->_flag01 = value != 0;
			break;
		case 18:
			_vm->_sceneResIndex = value;
			break;
		case 19:
			_vm->_cameraTop = value;
			break;
		case 20:
			_vm->_sceneHeight = value;
			break;
		case 21:
			_vm->_sceneWidth = value;
			break;
		case 1:
		case 2:
		default:
			warning("Setting unimplemented game variable %s (%d) to %d", getVarName(variable), variable, value);
			break;
	}


}

byte ScriptInterpreter::arg8(int16 offset) {
	return _subCode[offset];
}

int16 ScriptInterpreter::arg16(int16 offset) {
	return READ_LE_UINT16(&_subCode[offset]);
}

int32 ScriptInterpreter::arg32(int16 offset) {
	return READ_LE_UINT32(&_subCode[offset]);
}

void ScriptInterpreter::pushByte(byte value) {
	_stack[_regs.sp] = value;
	_regs.sp--;
}

byte ScriptInterpreter::popByte() {
	_regs.sp++;
	return _stack[_regs.sp];
}

void ScriptInterpreter::pushInt16(int16 value) {
	WRITE_LE_UINT16(_stack + _regs.sp, value);
	_regs.sp -= 2;
}

int16 ScriptInterpreter::popInt16() {
	_regs.sp += 2;
	return READ_LE_UINT16(_stack + _regs.sp);
}

void ScriptInterpreter::pushInt32(int32 value) {
	WRITE_LE_UINT32(_stack + _regs.sp, value);
	_regs.sp -= 4;
}

int32 ScriptInterpreter::popInt32() {
	_regs.sp += 4;
	return READ_LE_UINT32(_stack + _regs.sp);
}

void ScriptInterpreter::localWrite8(int16 offset, byte value) {
	debug(1, "localWrite8(%d, %d)", offset, value);
	_localData[offset] = value;
}

byte ScriptInterpreter::localRead8(int16 offset) {
	debug(1, "localRead8(%d) -> %d", offset, _localData[offset]);
	return _localData[offset];
}

void ScriptInterpreter::localWrite16(int16 offset, int16 value) {
	debug(1, "localWrite16(%d, %d)", offset, value);
	WRITE_LE_UINT16(&_localData[offset], value);
}

int16 ScriptInterpreter::localRead16(int16 offset) {
	debug(1, "localRead16(%d) -> %d", offset, (int16)READ_LE_UINT16(&_localData[offset]));
	return (int16)READ_LE_UINT16(&_localData[offset]);
}

void ScriptInterpreter::localWrite32(int16 offset, int32 value) {
	debug(1, "localWrite32(%d, %d)", offset, value);
	WRITE_LE_UINT32(&_localData[offset], value);
}

int32 ScriptInterpreter::localRead32(int16 offset) {
	debug(1, "localRead32(%d) -> %d", offset, (int32)READ_LE_UINT32(&_localData[offset]));
	return (int32)READ_LE_UINT32(&_localData[offset]);
}

byte *ScriptInterpreter::localPtr(int16 offset) {
	debug(1, "localPtr(%d)", offset);
	return &_localData[offset];
}

} // End of namespace Picture
