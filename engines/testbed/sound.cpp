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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "sound/mixer.h"
#include "testbed/sound.h"
#include "sound/softsynth/pcspk.h"

namespace Testbed {

enum {
	kPlayChannel1 = 'pch1',
	kPlayChannel2 = 'pch2',
	kPlayChannel3 = 'pch3',
	kPauseChannel1 = 'pac1',
	kPauseChannel2 = 'pac2',
	kPauseChannel3 = 'pac3'
};

SoundSubsystemDialog::SoundSubsystemDialog() : TestbedInteractionDialog(80, 60, 400, 170) {
	_xOffset = 25;
	_yOffset = 0;
	Common::String text = "Sound Subsystem Tests: Test Mixing of Audio Streams.";
	addText(350, 20, text, Graphics::kTextAlignCenter, _xOffset, 15);
	addButton(200, 20, "Play Channel #1", kPlayChannel1);
	addButton(200, 20, "Play Channel #2", kPlayChannel2);
	addButton(200, 20, "Play Channel #3", kPlayChannel3);
	addButton(50, 20, "Close", GUI::kCloseCmd, 160, 15);
}


void SoundSubsystemDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
		case kPlayChannel1:
			_buttonArray[0]->setLabel("Pause Channel #1");
			_buttonArray[0]->setCmd(kPauseChannel1);
			// TODO: Play music #1 here
			break;
		case kPlayChannel2:
			_buttonArray[1]->setLabel("Pause Channel #2");
			_buttonArray[1]->setCmd(kPauseChannel2);
			break;
		case kPlayChannel3:
			_buttonArray[2]->setLabel("Pause Channel #3");
			_buttonArray[2]->setCmd(kPauseChannel3);
			break;
		case kPauseChannel1:
			_buttonArray[0]->setLabel("Play Channel #1");
			_buttonArray[0]->setCmd(kPlayChannel1);
			break;
		case kPauseChannel2:
			_buttonArray[1]->setLabel("Play Channel #2");
			_buttonArray[1]->setCmd(kPlayChannel2);
			break;
		case kPauseChannel3:
			_buttonArray[2]->setLabel("Play Channel #3");
			_buttonArray[2]->setCmd(kPlayChannel3);
			break;
		default:
			GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

bool SoundSubsystem::playPCSpkSound() {
	Audio::PCSpeaker *speaker = new Audio::PCSpeaker();
	Audio::Mixer *mixer = g_system->getMixer();
	Audio::SoundHandle handle;
	mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, speaker);
	speaker->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);
	g_system->delayMillis(1000);
	mixer->setChannelBalance(handle, -127);
	g_system->delayMillis(1000);
	mixer->setChannelBalance(handle, 127);
	g_system->delayMillis(1000);
	mixer->stopAll();
	return true;
}

bool SoundSubsystem::mixSounds() {
	SoundSubsystemDialog sDialog;
	sDialog.runModal();
	return true;
}

SoundSubsystemTestSuite::SoundSubsystemTestSuite() {
	addTest("PCSpkrSound", &SoundSubsystem::playPCSpkSound, true);
	addTest("MixSounds", &SoundSubsystem::mixSounds, true);
}

}	// End of namespace Testbed
