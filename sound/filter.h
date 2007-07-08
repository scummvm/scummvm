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
 * $URL$
 * $Id$
 *
 */

#ifndef SOUND_FILTER_H
#define SOUND_FILTER_H

#include "common/scummsys.h"

namespace Audio {

class FIRFilter {

private:
	double _passbandEdge;
	double _stopbandEdge;
	double _dBPassbandRipple;
	double _dBStopbandAtten;
	uint16 _samplingFreq;
	
	double _ripple;
	uint16 _length;
	double *_coeffs;

public:
/* Generates lowpass filter coefficients using the parameters provided */
	FIRFilter(
			double passbandEdge,
			double stopbandEdge,
			double dBPassbandRipple,
			double dBStopbandAtten,
			uint16 samplingFreq);
	
	uint16 getLength() { return _length; }
	
	double *getCoeffs() { return _coeffs; }

private:
	double equiripple(
			double dBPassbandRipple,
			double dBStopbandAtten);
	
	uint16 windowLength(
			double ripple,
			double transitionBW,
			uint16 samplingFreq);
	
	void windowDesign(double *coeffs, uint16 length, double ripple);
	
	double sinc(double arg);
	
	void LPDesign(
			double *coeffs,
			uint16 length,
			double passbandEdge,
			double stopbandEdge,
			uint16 samplingFreq);

};

} // End of namespace Audio

#endif
