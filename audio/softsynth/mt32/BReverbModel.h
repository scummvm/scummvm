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

#ifndef MT32EMU_B_REVERB_MODEL_H
#define MT32EMU_B_REVERB_MODEL_H

#include "globals.h"
#include "internals.h"
#include "Types.h"

namespace MT32Emu {

struct BReverbSettings {
	const Bit32u numberOfAllpasses;
	const Bit32u * const allpassSizes;
	const Bit32u numberOfCombs;
	const Bit32u * const combSizes;
	const Bit32u * const outLPositions;
	const Bit32u * const outRPositions;
	const Bit8u * const filterFactors;
	const Bit8u * const feedbackFactors;
	const Bit8u * const dryAmps;
	const Bit8u * const wetLevels;
	const Bit8u lpfAmp;
};

class RingBuffer {
protected:
	Sample *buffer;
	const Bit32u size;
	Bit32u index;

public:
	RingBuffer(const Bit32u size);
	virtual ~RingBuffer();
	Sample next();
	bool isEmpty() const;
	void mute();
};

class AllpassFilter : public RingBuffer {
public:
	AllpassFilter(const Bit32u size);
	Sample process(const Sample in);
};

class CombFilter : public RingBuffer {
protected:
	const Bit8u filterFactor;
	Bit8u feedbackFactor;

public:
	CombFilter(const Bit32u size, const Bit8u useFilterFactor);
	virtual void process(const Sample in);
	Sample getOutputAt(const Bit32u outIndex) const;
	void setFeedbackFactor(const Bit8u useFeedbackFactor);
};

class DelayWithLowPassFilter : public CombFilter {
	Bit8u amp;

public:
	DelayWithLowPassFilter(const Bit32u useSize, const Bit8u useFilterFactor, const Bit8u useAmp);
	void process(const Sample in);
	void setFeedbackFactor(const Bit8u) {}
};

class TapDelayCombFilter : public CombFilter {
	Bit32u outL;
	Bit32u outR;

public:
	TapDelayCombFilter(const Bit32u useSize, const Bit8u useFilterFactor);
	void process(const Sample in);
	Sample getLeftOutput() const;
	Sample getRightOutput() const;
	void setOutputPositions(const Bit32u useOutL, const Bit32u useOutR);
};

class BReverbModel {
	AllpassFilter **allpasses;
	CombFilter **combs;

	const BReverbSettings &currentSettings;
	const bool tapDelayMode;
	Bit8u dryAmp;
	Bit8u wetLevel;

	static const BReverbSettings &getCM32L_LAPCSettings(const ReverbMode mode);
	static const BReverbSettings &getMT32Settings(const ReverbMode mode);

public:
	BReverbModel(const ReverbMode mode, const bool mt32CompatibleModel = false);
	~BReverbModel();
	// After construction or a close(), open() must be called at least once before any other call (with the exception of close()).
	void open();
	// May be called multiple times without an open() in between.
	void close();
	void mute();
	void setParameters(Bit8u time, Bit8u level);
	void process(const Sample *inLeft, const Sample *inRight, Sample *outLeft, Sample *outRight, Bit32u numSamples);
	bool isActive() const;
	bool isMT32Compatible(const ReverbMode mode) const;
};

} // namespace MT32Emu

#endif // #ifndef MT32EMU_B_REVERB_MODEL_H
