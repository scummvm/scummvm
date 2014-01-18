/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011, 2012, 2013 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_TABLES_H
#define MT32EMU_TABLES_H

namespace MT32Emu {

// Sample rate to use in mixing. With the progress of development, we've found way too many thing dependent.
// In order to achieve further advance in emulation accuracy, sample rate made fixed throughout the emulator.
// The output from the synth is supposed to be resampled to convert the sample rate.
const unsigned int SAMPLE_RATE = 32000;

// MIDI interface data transfer rate in samples. Used to simulate the transfer delay.
const double MIDI_DATA_TRANSFER_RATE = (double)SAMPLE_RATE / 31250.0 * 8.0;

const float CM32L_REVERB_TO_LA32_ANALOG_OUTPUT_GAIN_FACTOR = 0.68f;

const int MIDDLEC = 60;

class Synth;

class Tables {
private:
	Tables();
	Tables(Tables &);

public:
	static const Tables &getInstance();

	// Constant LUTs

	// CONFIRMED: This is used to convert several parameters to amp-modifying values in the TVA envelope:
	// - PatchTemp.outputLevel
	// - RhythmTemp.outlevel
	// - PartialParam.tva.level
	// - expression
	// It's used to determine how much to subtract from the amp envelope's target value
	Bit8u levelToAmpSubtraction[101];

	// CONFIRMED: ...
	Bit8u envLogarithmicTime[256];

	// CONFIRMED: ...
	Bit8u masterVolToAmpSubtraction[101];

	// CONFIRMED:
	Bit8u pulseWidth100To255[101];

	Bit16u exp9[512];
	Bit16u logsin9[512];

	const Bit8u *resAmpDecayFactor;
};

}

#endif
