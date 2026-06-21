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

#include "audio/effects/hmi/hmifxfp.h"

namespace Audio {

const double HMIInterface::kHmiPi = 3.14159265;
const double HMIInterface::kHmiTwoPi = 6.2831853;
const double HMIInterface::kHmiMillis = 0.001;

static void copyName(char *dst, const char *src) {
	int i = 0;
	while (i != 63 && src[i]) {
		dst[i] = src[i];
		++i;
	}

	dst[i] = 0;
	while (++i != 64)
		dst[i] = 0;
}

HMIInterface::HMIInterface(int size, int channels,
						   const char *dialogName, HMIParamNames params, const float *mins,
						   const float *maxs, const float *defaults, const char *name, int id,
						   int paramCount)
	: _effectStructSize(size), _outChannels(channels),
	  _interfaceName(dialogName), _effectParams(params), _paramMinValues(mins),
	  _paramMaxValues(maxs), _paramDefaultValues(defaults),
	  _presetId(id), _paramCount(paramCount) {
	copyName(_presetName, name);
}

int HMIInterface::getMinDuration(HMIEffectNode *n, uint32 *duration) {
	*duration = 0;
	return 0;
}

HMIFxFp::HMIFxFp() { 
	initializeInterfaces();
}

void HMIFxFp::initializeInterfaces() {
	_interfaceList[0] = &_monoDelay;
	_interfaceList[1] = &_stereoDelay;
	_interfaceList[2] = &_reverb1;
	_interfaceList[3] = &_filter1;
	_interfaceList[4] = &_ringModulator;
	_interfaceList[5] = &_envelope;
	_interfaceList[6] = &_resonator;
	_interfaceList[7] = &_phasor;
	_interfaceList[8] = &_reverb2;
	_interfaceList[9] = nullptr;
}

int HMIFxFp::hmiFXGetInterfaceList(HMIInterface ***outList) {
	*outList = _interfaceList;
	return 0;
}

} // End of namespace Audio
