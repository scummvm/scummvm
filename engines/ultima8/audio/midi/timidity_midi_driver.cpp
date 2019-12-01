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
 */

#include "ultima8/misc/pent_include.h"
#include "TimidityMidiDriver.h"

#ifdef USE_TIMIDITY_MIDI

#include "timidity/timidity.h"

#ifndef PENTAGRAM_IN_EXULT
#include "ultima8/games/game_data.h"
#include "MusicFlex.h"
#include "XMidiFile.h"
#include "XMidiEvent.h"
#include "XMidiEventList.h"
#endif

namespace Ultima8 {

const MidiDriver::MidiDriverDesc TimidityMidiDriver::desc =
    MidiDriver::MidiDriverDesc("Timidity", createInstance);

TimidityMidiDriver::TimidityMidiDriver() :
	LowLevelMidiDriver() {
}

int TimidityMidiDriver::open() {
	int32 encoding = PE_16BIT | PE_SIGNED;
	if (!stereo) encoding |= PE_MONO;
	if (NS_TIMIDITY::Timidity_Init_Simple(sample_rate, 65536, encoding)) {
		perr << NS_TIMIDITY::Timidity_Error() << std::endl;
		return 1;
	}


#if defined(DEBUG) || defined(PENTAGRAM_IN_EXULT)
	// Going through all the XMidi files just takes far too long
	// in DEBUG Builds
	memset(used_inst, true, sizeof(bool) * 128);
	memset(used_drums, true, sizeof(bool) * 128);
#else
	MusicFlex *music = GameData::get_instance()->getMusic();

	memset(used_inst, false, sizeof(bool) * 128);
	memset(used_drums, false, sizeof(bool) * 128);

	// For all the XMIDIs
	for (int i = 0; i < 128; i++) {
		XMidiFile *x = music->getXMidi(i);
		if (!x) continue;

		// For every track in the XMIDI
		for (int t = 0; t < x->number_of_tracks(); t++) {
			// Get the event list
			XMidiEventList *list = x->GetEventList(t);

			// No list
			if (!list) continue;

			// For each event in the track
			for (XMidiEvent *event = list->events; event; event = event->next) {
				// Normal channel patch change
				if ((event->status & 0xF0) == 0xC0 && (event->status & 0x0F) != 9) {
					used_inst[event->data[0]] = true;
				}
				// Drum channel note on
				else if (event->status == 0x99) {
					if (event->data[1])
						used_drums[event->data[0]] = true;
				}
			}
		}
	}
#endif

	NS_TIMIDITY::Timidity_FinalInit(used_inst, used_drums);
	return 0;
}

void TimidityMidiDriver::close() {
	NS_TIMIDITY::Timidity_DeInit();
}

void TimidityMidiDriver::send(uint32 b) {
	NS_TIMIDITY::Timidity_PlayEvent(b & 0xFF, (b >> 8) & 0x7F, (b >> 16) & 0x7F);
}

void TimidityMidiDriver::lowLevelProduceSamples(int16 *samples, uint32 num_samples) {
	NS_TIMIDITY::Timidity_GenerateSamples(samples, num_samples);
}

} // End of namespace Ultima8

#endif
