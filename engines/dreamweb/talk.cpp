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
 */

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamGenContext::talk() {
	data.byte(kTalkpos) = 0;
	data.byte(kInmaparea) = 0;
	data.byte(kCharacter) = data.byte(kCommand);
	createPanel();
	showPanel();
	showMan();
	showExit();
	underTextLine();
	convIcons();
	startTalk();
	data.byte(kCommandtype) = 255;
	readMouse();
	showPointer();
	workToScreenCPP();

	RectWithCallback<DreamGenContext> talkList[] = {
		{ 273,320,157,198,&DreamBase::getBack1 },
		{ 240,290,2,44,&DreamGenContext::moreTalk },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		data.byte(kGetback) = 0;
		checkCoords(talkList);
		if (data.byte(kQuitrequested))
			break;
	} while (!data.byte(kGetback));

	if (data.byte(kTalkpos) >= 4)
		_personData->b7 |= 128;

	redrawMainScrn();
	workToScreenM();
	if (data.byte(kSpeechloaded) == 1) {
		cancelCh1();
		data.byte(kVolumedirection) = (byte)-1;
		data.byte(kVolumeto) = 0;
	}
}

void DreamGenContext::convIcons() {
	uint8 index = data.byte(kCharacter) & 127;
	uint16 frame = getPersFrame(index);
	const Frame *base = findSource(frame);
	showFrame(base, 234, 2, frame, 0);
}

uint16 DreamGenContext::getPersFrame(uint8 index) {
	return getSegment(data.word(kPeople)).word(kPersonframes + index * 2);
}

// TODO: put Starttalk here

// TODO: put Getpersontext here

void DreamGenContext::moreTalk() {
	if (data.byte(kTalkmode) != 0) {
		redes();
		return;
	}

	if (data.byte(kCommandtype) != 215) {
		data.byte(kCommandtype) = 215;
		commandOnly(49);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;	// nomore

	if (!(data.word(kMousebutton) & 1))
		return;

	data.byte(kTalkmode) = 2;
	data.byte(kTalkpos) = 4;

	if (data.byte(kCharacter) >= 100)
		data.byte(kTalkpos) = 48; // second part
	doSomeTalk();
}

// TODO: put Dosometalk here

void DreamGenContext::hangOnPQ() {
	data.byte(kGetback) = 0;

	RectWithCallback<DreamBase> quitList[] = {
		{ 273,320,157,198,&DreamBase::getBack1 },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	uint16 speechFlag = 0;

	do {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		checkCoords(quitList);

		if (data.byte(kGetback) == 1 || data.byte(kQuitrequested)) {
			// Quit conversation
			delPointer();
			data.byte(kPointermode) = 0;
			cancelCh1();
			flags._c = true;
			return;
		}

		if (data.byte(kSpeechloaded) == 1 && data.byte(kCh1playing) == 255) {
			speechFlag++;
			if (speechFlag == 40)
				break;
		}
	} while (!data.word(kMousebutton) || data.word(kOldbutton));

	delPointer();
	data.byte(kPointermode) = 0;
	flags._c = false;
}

void DreamGenContext::redes() {
	if (data.byte(kCh1playing) != 255 || data.byte(kTalkmode) != 2) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 217) {
		data.byte(kCommandtype) = 217;
		commandOnly(50);
	}

	if (!(data.word(kMousebutton) & 1))
		return;

	delPointer();
	createPanel();
	showPanel();
	showMan();
	showExit();
	convIcons();
	startTalk();
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
}

} // End of namespace DreamGen
