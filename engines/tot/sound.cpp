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

#include "common/file.h"
#include "common/textconsole.h"

#include "tot/sound.h"
#include "tot/tot.h"

namespace Tot {

byte *currentMidi;

void stopVoc() {
	g_engine->_sound->stopVoc();
}

void setSfxVolume(byte voll, byte volr) {

	if (voll == volr) {
		int volume = (voll) / (float)7 * 255;
		g_engine->_sound->setSfxVolume(volume);
		g_engine->_sound->setSfxBalance(true, true);
	} else {
		if (voll == 0) {
			g_engine->_sound->setSfxBalance(false, true);
		} else {
			g_engine->_sound->setSfxBalance(true, false);
		}
	}
}

void setMidiVolume(byte voll, byte volr) {
	int volume = (voll) / (float)7 * 255;
	g_engine->_sound->setMusicVolume(volume);
}

void fadeOutMusic(byte voll, byte volr) {
	byte volpaso = (voll + volr) / 2;
	for (int ivol = volpaso; ivol >= 0; ivol--) {
		setMidiVolume(ivol, ivol);
		delay(10);
	}
}

void fadeInMusic(byte voll, byte volr) {
	byte ivol, volpaso;

	volpaso = (voll + volr) / 2;
	for (ivol = 0; ivol <= volpaso; ivol++) {
		setMidiVolume(ivol, ivol);
		delay(10);
	}
}

void setMasterVolume(byte voll, byte volr) {
}

void loadVoc(Common::String vocFile, int32 startPos, uint vocSize) {
	g_engine->_sound->loadVoc(vocFile, startPos, vocSize);
}

void autoPlayVoc(Common::String vocFile, int32 startPos, uint vocSize) {
	g_engine->_sound->loadVoc(vocFile, startPos, vocSize);
	g_engine->_sound->autoPlayVoc();
}

void pitavocmem() {
	g_engine->_sound->playVoc();
}

void playVoc(Common::String vocFile, int32 startPos, uint vocSize) {
	loadVoc(vocFile, startPos, vocSize);
	pitavocmem();
}

void playMidiFile(Common::String nomfich, bool loop) {

	Common::File musicFile;
	if (!musicFile.open(Common::Path(nomfich + ".MUS"))) {
		showError(267);
	}
	currentMidi = (byte *)malloc(musicFile.size());
	musicFile.read(currentMidi, musicFile.size());

	g_engine->_sound->playMidi(currentMidi, musicFile.size(), true);
	musicFile.close();
}

void initSound() {
	setMidiVolume(3, 3);
	playMidiFile("Silent", false);
	setSfxVolume(6, 6);
}
} // End of namespace Tot
