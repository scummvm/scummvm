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

void DreamBase::talk() {
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
	workToScreen();

	RectWithCallback<DreamGenContext> talkList[] = {
		{ 273,320,157,198,&DreamBase::getBack1 },
		{ 240,290,2,44,&DreamBase::moreTalk },
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

void DreamBase::convIcons() {
	uint8 index = data.byte(kCharacter) & 127;
	uint16 frame = getPersFrame(index);
	const Frame *base = findSource(frame);
	showFrame(base, 234, 2, frame, 0);
}

uint16 DreamBase::getPersFrame(uint8 index) {
	return getSegment(data.word(kPeople)).word(kPersonframes + index * 2);
}

void DreamBase::startTalk() {
	data.byte(kTalkmode) = 0;

	const uint8 *str = getPersonText(data.byte(kCharacter) & 0x7F, 0);
	uint16 y;

	data.word(kCharshift) = 91+91;
	y = 64;
	printDirect(&str, 66, &y, 241, true);

	data.word(kCharshift) = 0;
	y = 80;
	printDirect(&str, 66, &y, 241, true);

	data.byte(kSpeechloaded) = 0;
	loadSpeech('R', data.byte(kReallocation), 'C', 64*(data.byte(kCharacter) & 0x7F));
	if (data.byte(kSpeechloaded) == 1) {
		data.byte(kVolumedirection) = 1;
		data.byte(kVolumeto) = 6;
		playChannel1(50 + 12);
	}
}

const uint8 *DreamBase::getPersonText(uint8 index, uint8 talkPos) {
	uint16 offset = kPersontext + getSegment(data.word(kPeople)).word(((index * 64 + talkPos) * 2) + kPersontxtdat);
	return getSegment(data.word(kPeople)).ptr(offset, 0);
}

void DreamBase::moreTalk() {
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

void DreamBase::doSomeTalk() {
	while (true) {
		const uint8 *str = getPersonText(data.byte(kCharacter) & 0x7F, data.byte(kTalkpos));

		if (*str == 0) {
			// endheartalk
			data.byte(kPointermode) = 0;
			return;
		}

		createPanel();
		showPanel();
		showMan();
		showExit();
		convIcons();

		printDirect(str, 164, 64, 144, false);

		loadSpeech('R', data.byte(kReallocation), 'C', (64 * (data.byte(kCharacter) & 0x7F)) + data.byte(kTalkpos));
		if (data.byte(kSpeechloaded) != 0)
			playChannel1(62);

		data.byte(kPointermode) = 3;
		workToScreenM();
		if (hangOnPQ())
			return;

		data.byte(kTalkpos)++;

		str = getPersonText(data.byte(kCharacter) & 0x7F, data.byte(kTalkpos));
		if (*str == 0) {
			// endheartalk
			data.byte(kPointermode) = 0;
			return;
		}

		if (*str != ':' && *str != 32) {
			createPanel();
			showPanel();
			showMan();
			showExit();
			convIcons();
			printDirect(str, 48, 128, 144, false);

			loadSpeech('R', data.byte(kReallocation), 'C', (64 * (data.byte(kCharacter) & 0x7F)) + data.byte(kTalkpos));
			if (data.byte(kSpeechloaded) != 0)
				playChannel1(62);

			data.byte(kPointermode) = 3;
			workToScreenM();
			if (hangOnPQ())
				return;
		}

		data.byte(kTalkpos)++;
	}
}

bool DreamBase::hangOnPQ() {
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
			return true;
		}

		if (data.byte(kSpeechloaded) == 1 && data.byte(kCh1playing) == 255) {
			speechFlag++;
			if (speechFlag == 40)
				break;
		}
	} while (!data.word(kMousebutton) || data.word(kOldbutton));

	delPointer();
	data.byte(kPointermode) = 0;
	return false;
}

void DreamBase::redes() {
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
	workToScreen();
	delPointer();
}

} // End of namespace DreamGen
