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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AUDIO_EFFECTS_HMI_HMIFXFP_H
#define AUDIO_EFFECTS_HMI_HMIFXFP_H

#include "audio/effects/hmi/interfaces/envelope.h"
#include "audio/effects/hmi/interfaces/filter1.h"
#include "audio/effects/hmi/interfaces/mono_delay.h"
#include "audio/effects/hmi/interfaces/phasor.h"
#include "audio/effects/hmi/interfaces/resonator.h"
#include "audio/effects/hmi/interfaces/reverb1.h"
#include "audio/effects/hmi/interfaces/reverb2.h"
#include "audio/effects/hmi/interfaces/ring_modulator.h"
#include "audio/effects/hmi/interfaces/stereo_delay.h"

namespace Audio {

class HMIFxFp {
public:
	HMIFxFp();
	int hmiFXGetInterfaceList(HMIInterface ***outList);

private:
	void initializeInterfaces();

	HMIMonoDelay _monoDelay;
	HMIStereoDelay _stereoDelay;
	HMIReverb1 _reverb1;
	HMIFilter1 _filter1;
	HMIRingModulator _ringModulator;
	HMIEnvelope _envelope;
	HMIResonator _resonator;
	HMIPhasor _phasor;
	HMIReverb2 _reverb2;
	HMIInterface *_interfaceList[10] = {nullptr};
};

} // End of namespace Audio

#endif
