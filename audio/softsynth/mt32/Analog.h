/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011-2016 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
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

#ifndef MT32EMU_ANALOG_H
#define MT32EMU_ANALOG_H

#include "globals.h"
#include "internals.h"
#include "Types.h"
#include "Enumerations.h"

namespace MT32Emu {

class AbstractLowPassFilter;

/* Analog class is dedicated to perform fair emulation of analogue circuitry of hardware units that is responsible
 * for processing output signal after the DAC. It appears that the analogue circuit labeled "LPF" on the schematic
 * also applies audible changes to the signal spectra. There is a significant boost of higher frequencies observed
 * aside from quite poor attenuation of the mirror spectra above 16 kHz which is due to a relatively low filter order.
 *
 * As the final mixing of multiplexed output signal is performed after the DAC, this function is migrated here from Synth.
 * Saying precisely, mixing is performed within the LPF as the entrance resistors are actually components of a LPF
 * designed using the multiple feedback topology. Nevertheless, the schematic separates them.
 */
class Analog {
public:
	Analog(const AnalogOutputMode mode, const bool oldMT32AnalogLPF);
	~Analog();
	void process(Sample *outStream, const Sample *nonReverbLeft, const Sample *nonReverbRight, const Sample *reverbDryLeft, const Sample *reverbDryRight, const Sample *reverbWetLeft, const Sample *reverbWetRight, Bit32u outLength);
	unsigned int getOutputSampleRate() const;
	Bit32u getDACStreamsLength(Bit32u outputLength) const;
	void setSynthOutputGain(float synthGain);
	void setReverbOutputGain(float reverbGain, bool mt32ReverbCompatibilityMode);

private:
	AbstractLowPassFilter &leftChannelLPF;
	AbstractLowPassFilter &rightChannelLPF;
	SampleEx synthGain;
	SampleEx reverbGain;

	Analog(Analog &);
};

} // namespace MT32Emu

#endif // #ifndef MT32EMU_ANALOG_H
