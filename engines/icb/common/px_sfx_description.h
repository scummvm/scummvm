/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX_SFX_DESCRIPTION_H
#define ICB_PX_SFX_DESCRIPTION_H

namespace ICB {

// versions...
// 100 initial
// 101 rearanged
// 105 added also to sfxdesc files to be checked by converter
//
#define SFX_VERSION 105

#define NO_LOOPING 0x00
#define WAV_LOOPING_FLAG 0x01
#define SFX_LOOPING_FLAG 0x02

// this class contains an envelope of the form y=ax^3+bx^2+cx+d
class CEnvelope {
public:
	int32 a;
	int32 b;
	int32 c;
	int32 d;
	int8 div; // dividing value on the time scale
	CEnvelope() { Reset(); }
	void Reset() {
		a = b = c = d = 0;
		div = 1;
	}
};

// this class contains a single sfx ready to be saved out. This will go in it's own file eventually...
class CSfx {
public:
	CSfx() { Reset(); }

	CEnvelope m_volume; // volume where v<=0 => none >=128 => max
	CEnvelope m_pitch;  // pitch addition to base pitch (in PSX pitch units where 0x1000=44100hz, etc)

	int32 m_duration;   // duration in 128th of second
	int32 m_rand_pitch; // random value to add to pitch at start (in PSX pitch units where 0x1000=44100hz, etc)

	int32 m_min_distance; // in cm
	int32 m_max_distance; // in cm

	int8 m_sampleNameOffset; // offset into structure of sampleName...
	int8 m_descOffset;       // generally this will be 0 when outputing for the engine since the engine will not need descriptions

	int8 m_looping; // BIT 0 is hardware flag, bit 1 is software flag.

	int8 m_rand_mode; // mode=0 is normal, choose a random value at startup, mode>0 is some divider of the envelope

	void Reset() {
		m_volume.Reset();
		m_pitch.Reset();
		m_duration = 0;
		m_looping = 0;
		m_rand_pitch = 0;
		m_rand_mode = 0;
		m_sampleNameOffset = 0;
		m_descOffset = 0;
		m_min_distance = 0;
		m_max_distance = 0;
	}

	const char *GetSampleName() { return (cstr) this + m_sampleNameOffset; }
};

} // End of namespace ICB

#endif
