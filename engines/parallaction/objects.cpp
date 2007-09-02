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

#include "common/stdafx.h"
#include "parallaction/objects.h"

namespace Parallaction {



Command::Command() {
	_id = 0;
	_flagsOn = 0;
	_flagsOff = 0;
}

Command::~Command() {

}


Animation::Animation() {
	_cnv = NULL;
	_program = NULL;
	_scriptName = 0;
	_frame = 0;
	_z = 0;
}

Animation::~Animation() {
	if (_program)
		delete _program;

	if (_scriptName)
		free(_scriptName);

	if (_cnv)
		delete _cnv;
}

uint16 Animation::width() const {
	if (!_cnv) return 0;
	Common::Rect r;
	_cnv->getRect(0, r);
	return r.width();
}

uint16 Animation::height() const {
	if (!_cnv) return 0;
	Common::Rect r;
	_cnv->getRect(0, r);
	return r.height();
}

uint16 Animation::getFrameNum() const {
	if (!_cnv) return 0;
	return _cnv->getNum();
}

byte* Animation::getFrameData(uint32 index) const {
	if (!_cnv) return NULL;
	return _cnv->getData(index);
}


#define NUM_LOCALS	10
char	_localNames[NUM_LOCALS][10];

Program::Program() {
	_loopCounter = 0;
	_locals = new LocalVariable[NUM_LOCALS];
	_numLocals = 0;
}

Program::~Program() {
	delete[] _locals;
}

int16 Program::findLocal(const char* name) {
	for (uint16 _si = 0; _si < NUM_LOCALS; _si++) {
		if (!scumm_stricmp(name, _localNames[_si]))
			return _si;
	}

	return -1;
}

int16 Program::addLocal(const char *name, int16 value, int16 min, int16 max) {
	assert(_numLocals < NUM_LOCALS);

	strcpy(_localNames[_numLocals], name);
	_locals[_numLocals]._value = value;

	_locals[_numLocals]._min = min;
	_locals[_numLocals]._max = max;

	return _numLocals++;
}



Zone::Zone() {
	_left = _top = _right = _bottom = 0;

	_type = 0;
	_flags = 0;
}

Zone::~Zone() {
//	printf("~Zone(%s)\n", _label._text);

	switch (_type & 0xFFFF) {
	case kZoneExamine:
		free(u.examine->_filename);
		free(u.examine->_description);
		delete u.examine;
		break;

	case kZoneDoor:
		free(u.door->_location);
		free(u.door->_background);
		if (u.door->_cnv)
			delete u.door->_cnv;
		delete u.door;
		break;

	case kZoneSpeak:
		delete u.speak->_dialogue;
		delete u.speak;
		break;

	case kZoneGet:
		free(u.get->_backup);
		if (u.get->_cnv) {
			u.get->_cnv->free();
			delete u.get->_cnv;
		}
		delete u.get;
		break;

	case kZoneHear:
		delete u.hear;
		break;

	case kZoneMerge:
		delete u.merge;
		break;

	default:
		break;
	}
}

void Zone::getRect(Common::Rect& r) const {
	r.left = _left;
	r.right = _right;
	r.top = _top;
	r.bottom = _bottom;
}

void Zone::translate(int16 x, int16 y) {
	_left += x;
	_right += x;
	_top += y;
	_bottom += y;
}

uint16 Zone::width() const {
	return _right - _left;
}

uint16 Zone::height() const {
	return _bottom - _top;
}

Label::Label() {
	resetPosition();
	_text = 0;
}

Label::~Label() {
	free();
}

void Label::free() {
	_cnv.free();
	if (_text)
		::free(_text);
	_text = 0;

	resetPosition();
}

void Label::resetPosition() {
	_pos.x = -1000;
	_pos.y = -1000;
	_old.x = -1000;
	_old.y = -1000;
}

void Label::getRect(Common::Rect &r, bool old) {
	r.setWidth(_cnv.w);
	r.setHeight(_cnv.h);

	if (old) {
		r.moveTo(_old);
	} else {
		r.moveTo(_pos);
	}
}

Answer::Answer() {
	_text = NULL;
	_mood = 0;
	_following._question =  NULL;
	_noFlags = 0;
	_yesFlags = 0;
}

Answer::~Answer() {
	if (_text)
		free(_text);
}

Question::Question() {
	_text = NULL;
	_mood = 0;

	for (uint32 i = 0; i < NUM_ANSWERS; i++)
		_answers[i] = NULL;

}

Question::~Question() {

	for (uint32 i = 0; i < NUM_ANSWERS; i++)
		if (_answers[i]) delete _answers[i];

	free(_text);
}

Instruction::Instruction() {
	memset(this, 0, sizeof(Instruction));
}

Instruction::~Instruction() {
	if (_text)
		free(_text);
	if (_text2)
		free(_text2);
}

int16 ScriptVar::getRValue() {

	if (_flags & kParaImmediate) {
		return _value;
	}

	if (_flags & kParaLocal) {
		return _local->_value;
	}

	if (_flags & kParaField) {
		return *_pvalue;
	}

	if (_flags & kParaRandom) {
		return (rand() * _value) / 32767;
	}

	error("Parameter is not an r-value");

	return 0;
}

int16* ScriptVar::getLValue() {

	if (_flags & kParaLocal) {
		return &_local->_value;
	}

	if (_flags & kParaField) {
		return _pvalue;
	}

	error("Parameter is not an l-value");

}

void ScriptVar::setLocal(LocalVariable *local) {
	_local = local;
	_flags |= kParaLocal;
}

void ScriptVar::setField(int16 *field) {
	_pvalue = field;
	_flags |= kParaField;
}

void ScriptVar::setImmediate(int16 value) {
	_value = value;
	_flags |= kParaImmediate;
}

void ScriptVar::setRandom(int16 seed) {
	_value = seed;
	_flags |= kParaRandom;
}


ScriptVar::ScriptVar() {
	_flags = 0;
	_local = 0;
	_value = 0;
	_pvalue = 0;
}


} // namespace Parallaction
