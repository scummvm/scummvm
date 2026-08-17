#include <ddb.h>
#include <ddb_vid.h>
#include <os_bito.h>
#include <os_mem.h>
#include <os_lib.h>

#if HAS_PSG

#ifdef _ATARIST
extern "C" void PlayDosound(const uint8_t* data);
#endif

static const uint32_t PSG_MAX_TICKS = DDB_PSG_TICK_HZ * 10;
static const uint32_t PSG_VOLUME_RAMP_SAMPLES = 32;
static const uint32_t PSG_MAX_SAMPLES = PSG_MAX_TICKS * DDB_PSG_SAMPLES_PER_TICK;
static uint8_t* psgPlaybackBuffer = 0;
static uint32_t psgPlaybackBufferSize = 0;

bool DDB_InitializePSGPlayback()
{
	#ifdef _ATARIST
	return true;
	#else
	if (psgPlaybackBuffer != 0)
		return true;

	psgPlaybackBuffer = Allocate<uint8_t>("PSG playback buffer", PSG_MAX_SAMPLES, false);
	if (psgPlaybackBuffer == 0)
		return false;

	psgPlaybackBufferSize = PSG_MAX_SAMPLES;
	return true;
	#endif
}

static uint32_t GetMinimumPsgHeaderSize(DDB_Version version)
{
	return version == DDB_VERSION_1 ? 34 : 36;
}

static uint32_t GetVolumeLevel(uint8_t level)
{
	uint32_t value = level;
	return (value * value * 255 + 480) / 961;
}

static uint32_t GetStreamOffset(const DDB* ddb, uint8_t soundIndex)
{
	uint32_t offset = 0;
	DDB_DecodeStoredOffset(ddb, read16((const uint8_t*)ddb->externPsgTable + soundIndex * 2, ddb->littleEndian), ddb->dataSize, false, &offset);
	return offset;
}

bool DDB_GetExternalPSGStreamRange(const DDB* ddb, uint8_t soundIndex, uint32_t* start, uint32_t* end)
{
	if (ddb->externPsgTable == 0 || ddb->externPsgCount == 0)
		return false;
	if (soundIndex >= ddb->externPsgCount)
		return false;

	*start = GetStreamOffset(ddb, soundIndex);
	*end = soundIndex + 1 < ddb->externPsgCount ?
		GetStreamOffset(ddb, soundIndex + 1) :
		(uint32_t)((const uint8_t*)ddb->externPsgTable - ddb->data);
	return *start < *end && *end <= ddb->dataSize;
}

static void UpdateToneStep(DDB_PSGState* state, int channel)
{
	uint16_t period = (state->regs[channel * 2] | ((state->regs[channel * 2 + 1] & 0x0F) << 8));
	if (period == 0)
		period = 1;
	state->toneStep[channel] = 2000000.0 / (16.0 * (double)period * (double)DDB_PSG_SAMPLE_RATE);
}

static void UpdateNoiseStep(DDB_PSGState* state)
{
	uint8_t period = state->regs[6] & 0x1F;
	if (period == 0)
		period = 1;
	state->noiseStep = 2000000.0 / (16.0 * (double)period * (double)DDB_PSG_SAMPLE_RATE);
}

static void ResetEnvelope(DDB_PSGState* state)
{
	uint8_t shape = state->regs[13] & 0x0F;
	state->envelopeContinue = (shape & 0x08) != 0;
	state->envelopeAttack = (shape & 0x04) != 0;
	state->envelopeAlternate = (shape & 0x02) != 0;
	state->envelopeHold = (shape & 0x01) != 0;
	state->envelopeHolding = false;
	state->envelopeAscending = state->envelopeAttack;
	state->envelopeLevel = state->envelopeAscending ? 0 : 31;
	state->envelopePhase = 0.0;
}

static void UpdateEnvelopeStep(DDB_PSGState* state)
{
	uint16_t period = (state->regs[11] | (state->regs[12] << 8));
	if (period == 0)
		period = 1;
	state->envelopeStep = 2000000.0 / (256.0 * (double)period * (double)DDB_PSG_SAMPLE_RATE);
}

static void WriteRegister(DDB_PSGState* state, uint8_t reg, uint8_t value)
{
	if (reg > 15)
		return;

	uint8_t previous = state->regs[reg];
	state->regs[reg] = value;
	switch (reg)
	{
		case 0:
		case 1:
			UpdateToneStep(state, 0);
			break;
		case 2:
		case 3:
			UpdateToneStep(state, 1);
			break;
		case 4:
		case 5:
			UpdateToneStep(state, 2);
			break;
		case 6:
			UpdateNoiseStep(state);
			break;
		case 11:
		case 12:
			UpdateEnvelopeStep(state);
			break;
		case 13:
			ResetEnvelope(state);
			break;
	}

	if ((reg >= 7 && reg <= 10) || reg == 13)
	{
		if (previous != value || reg == 13)
			state->rampRemaining = PSG_VOLUME_RAMP_SAMPLES;
	}
}

static void InitializeState(DDB_PSGState* state)
{
	MemClear(state, sizeof(*state));
	state->lfsr = 0x1FFFF;
	for (int channel = 0; channel < 3; channel++)
		UpdateToneStep(state, channel);
	UpdateNoiseStep(state);
	UpdateEnvelopeStep(state);
	ResetEnvelope(state);
}

static void AdvanceEnvelope(DDB_PSGState* state)
{
	if (state->envelopeHolding)
		return;

	if (state->envelopeAscending)
	{
		if (state->envelopeLevel < 31)
		{
			state->envelopeLevel++;
			return;
		}
	}
	else
	{
		if (state->envelopeLevel > 0)
		{
			state->envelopeLevel--;
			return;
		}
	}

	if (!state->envelopeContinue)
	{
		state->envelopeHolding = true;
		state->envelopeLevel = 0;
		return;
	}

	if (state->envelopeHold)
	{
		if (state->envelopeAlternate)
			state->envelopeAscending = !state->envelopeAscending;
		state->envelopeHolding = true;
		state->envelopeLevel = state->envelopeAscending ? 31 : 0;
		return;
	}

	if (state->envelopeAlternate)
		state->envelopeAscending = !state->envelopeAscending;
	state->envelopeLevel = state->envelopeAscending ? 0 : 31;
}

static uint8_t RenderSample(DDB_PSGState* state)
{
	state->envelopePhase += state->envelopeStep;
	while (state->envelopePhase >= 1.0)
	{
		state->envelopePhase -= 1.0;
		AdvanceEnvelope(state);
	}

	state->noisePhase += state->noiseStep;
	while (state->noisePhase >= 1.0)
	{
		uint32_t feedback = ((state->lfsr ^ (state->lfsr >> 3)) & 1) << 16;
		state->lfsr = (state->lfsr >> 1) | feedback;
		state->noisePhase -= 1.0;
	}

	int mix = 0;
	for (int channel = 0; channel < 3; channel++)
	{
		state->tonePhase[channel] += state->toneStep[channel];
		while (state->tonePhase[channel] >= 1.0)
			state->tonePhase[channel] -= 1.0;

		bool toneDisabled = (state->regs[7] & (1 << channel)) != 0;
		bool noiseDisabled = (state->regs[7] & (1 << (channel + 3))) != 0;
		int toneSign = state->tonePhase[channel] < 0.5 ? 1 : -1;
		int noiseSign = (state->lfsr & 1) != 0 ? 1 : -1;
		int sign = 0;
		if (!toneDisabled && !noiseDisabled)
			sign = toneSign * noiseSign;
		else if (!toneDisabled)
			sign = toneSign;
		else if (!noiseDisabled)
			sign = noiseSign;

		if (sign == 0)
			continue;

		uint8_t volumeReg = state->regs[8 + channel];
		uint32_t amplitude = (volumeReg & 0x10) != 0 ?
			GetVolumeLevel(state->envelopeLevel) :
			GetVolumeLevel((volumeReg & 0x0F) << 1);
		mix += sign * (int)amplitude;
	}

	mix /= 4;
	if (!state->outputInitialized)
	{
		state->smoothedMix = 0;
		state->outputInitialized = true;
	}
	if (state->rampRemaining != 0)
	{
		state->smoothedMix += (int16_t)((mix - state->smoothedMix) / (int)state->rampRemaining);
		mix = state->smoothedMix;
		state->rampRemaining--;
	}
	else
	{
		state->smoothedMix = (int16_t)mix;
	}
	if (mix < -127)
		mix = -127;
	else if (mix > 127)
		mix = 127;
	return (uint8_t)(128 + mix);
}

void DDB_RenderPSGTicks(DDB_PSGState* state, uint8_t* output, uint32_t ticks)
{
	for (uint32_t n = 0; n < ticks * DDB_PSG_SAMPLES_PER_TICK; n++)
		output[n] = RenderSample(state);
}

static int CountSweepTicks(uint8_t start, int8_t delta, uint8_t target)
{
	int value = start;
	if (value == target)
		return 1;
	if (delta == 0)
		return -1;

	int ticks = 0;
	while (ticks <= 256)
	{
		ticks++;
		if ((delta > 0 && value >= target) || (delta < 0 && value <= target))
			return ticks;
		value += delta;
	}
	return -1;
}

bool DDB_EstimatePSGStreamTicks(const uint8_t* start, const uint8_t* end, uint32_t maxTicks, uint32_t* totalTicks)
{
	const uint8_t* ptr = start;
	uint8_t tempRegister = 0;
	*totalTicks = 0;

	while (ptr < end)
	{
		uint8_t command = *ptr++;
		if (command <= 0x0F)
		{
			if (ptr >= end)
				return false;
			ptr++;
		}
		else if (command == 0x80)
		{
			if (ptr >= end)
				return false;
			tempRegister = *ptr++;
		}
		else if (command == 0x81)
		{
			if (end - ptr < 3)
				return false;
			ptr++;
			int8_t delta = (int8_t)*ptr++;
			uint8_t target = *ptr++;
			int ticks = CountSweepTicks(tempRegister, delta, target);
			if (ticks < 0)
				return false;
			*totalTicks += (uint32_t)ticks;
			tempRegister = target;
		}
		else
		{
			if (ptr >= end)
				return false;
			uint8_t argument = *ptr++;
			if (argument == 0)
				return ptr == end;
			*totalTicks += argument;
		}

		if (*totalTicks > maxTicks)
			return false;
	}

	return false;
}

bool DDB_RenderPSGStream(const uint8_t* start, const uint8_t* end, uint8_t* output, uint32_t totalTicks, DDB_PSGState* finalState)
{
	DDB_PSGState state;
	InitializeState(&state);

	const uint8_t* ptr = start;
	uint8_t* dst = output;
	uint32_t remainingTicks = totalTicks;

	while (ptr < end)
	{
		uint8_t command = *ptr++;
		if (command <= 0x0F)
		{
			WriteRegister(&state, command, *ptr++);
		}
		else if (command == 0x80)
		{
			state.tempRegister = *ptr++;
		}
		else if (command == 0x81)
		{
			uint8_t reg = *ptr++;
			int8_t delta = (int8_t)*ptr++;
			uint8_t target = *ptr++;
			int value = state.tempRegister;
			for (;;)
			{
				state.tempRegister = (uint8_t)value;
				WriteRegister(&state, reg, state.tempRegister);
				DDB_RenderPSGTicks(&state, dst, 1);
				dst += DDB_PSG_SAMPLES_PER_TICK;
				remainingTicks--;
				if ((delta > 0 && value >= target) || (delta < 0 && value <= target))
					break;
				value += delta;
			}
			state.tempRegister = (uint8_t)value;
		}
		else
		{
			uint8_t argument = *ptr++;
			if (argument == 0)
			{
				if (finalState != 0)
					*finalState = state;
				return remainingTicks == 0;
			}
			DDB_RenderPSGTicks(&state, dst, argument);
			dst += argument * DDB_PSG_SAMPLES_PER_TICK;
			remainingTicks -= argument;
		}
	}

	if (finalState != 0)
		*finalState = state;
	return false;
}

static bool IsValidPsgStream(const uint8_t* start, const uint8_t* end)
{
	const uint8_t* ptr = start;
	while (ptr < end)
	{
		uint8_t command = *ptr++;
		if (command <= 0x0F || command == 0x80)
		{
			if (ptr >= end)
				return false;
			ptr++;
		}
		else if (command == 0x81)
		{
			if (end - ptr < 3)
				return false;
			ptr += 3;
		}
		else
		{
			if (ptr >= end)
				return false;
			uint8_t argument = *ptr++;
			if (argument == 0)
				return ptr == end;
		}
	}

	return false;
}

void DDB_DetectPSGExternalTable(DDB* ddb, uint32_t firstSectionOffset)
{
	ddb->externPsgTable = 0;
	ddb->externPsgCount = 0;

	if (ddb->version < DDB_VERSION_2)
		return;

	if (ddb->target != DDB_MACHINE_ATARIST && ddb->target != DDB_MACHINE_AMIGA)
		return;

	if (ddb->dataSize < 0x2A || firstSectionOffset <= GetMinimumPsgHeaderSize(ddb->version))
		return;

	uint16_t rawTableOffset = read16(ddb->data + 0x28, ddb->littleEndian);
	uint32_t tableOffset = 0;
	if (!DDB_DecodeStoredOffset(ddb, rawTableOffset, ddb->dataSize, false, &tableOffset))
		return;

	uint32_t minimumHeaderSize = GetMinimumPsgHeaderSize(ddb->version);
	if (tableOffset < minimumHeaderSize || tableOffset >= firstSectionOffset)
		return;

	uint32_t tableSize = firstSectionOffset - tableOffset;
	if (tableSize == 0 || (tableSize & 1) != 0)
		return;

	uint32_t count = tableSize / 2;
	if (count == 0 || count > 255)
		return;

	uint32_t previousStart = 0;
	for (uint32_t n = 0; n < count; n++)
	{
		uint16_t rawStreamOffset = read16(ddb->data + tableOffset + n * 2, ddb->littleEndian);
		uint32_t streamOffset = 0;
		if (!DDB_DecodeStoredOffset(ddb, rawStreamOffset, ddb->dataSize, false, &streamOffset))
			return;
		if (streamOffset < minimumHeaderSize || streamOffset >= tableOffset)
			return;
		if (n != 0 && streamOffset <= previousStart)
			return;

		previousStart = streamOffset;
	}

	for (uint32_t n = 0; n < count; n++)
	{
		uint32_t streamStart = 0;
		uint32_t streamEnd = tableOffset;
		if (!DDB_DecodeStoredOffset(ddb, read16(ddb->data + tableOffset + n * 2, ddb->littleEndian), ddb->dataSize, false, &streamStart))
			return;
		if (n + 1 < count && !DDB_DecodeStoredOffset(ddb, read16(ddb->data + tableOffset + (n + 1) * 2, ddb->littleEndian), ddb->dataSize, false, &streamEnd))
			return;

		if (streamStart >= streamEnd)
			return;
		if (!IsValidPsgStream(ddb->data + streamStart, ddb->data + streamEnd))
			return;
	}

	ddb->externPsgTable = (uint16_t*)(ddb->data + tableOffset);
	ddb->externPsgCount = (uint8_t)count;
}

bool DDB_PlayExternalPSG(DDB* ddb, uint8_t soundIndex)
{
	if (ddb == 0)
		return false;

	uint32_t streamStart = 0;
	uint32_t streamEnd = 0;
	if (!DDB_GetExternalPSGStreamRange(ddb, soundIndex, &streamStart, &streamEnd))
		return false;

	#ifdef _ATARIST
	if (ddb->target == DDB_MACHINE_ATARIST)
	{
		PlayDosound(ddb->data + streamStart);
		return true;
	}
	#endif

	uint32_t totalTicks = 0;
	if (!DDB_EstimatePSGStreamTicks(ddb->data + streamStart, ddb->data + streamEnd, PSG_MAX_TICKS, &totalTicks) || totalTicks == 0)
		return false;

	uint32_t sampleCount = totalTicks * DDB_PSG_SAMPLES_PER_TICK;
	if (sampleCount > psgPlaybackBufferSize)
		return false;
	if (psgPlaybackBuffer == 0 && !DDB_InitializePSGPlayback())
		return false;

	if (!DDB_RenderPSGStream(ddb->data + streamStart, ddb->data + streamEnd, psgPlaybackBuffer, totalTicks, 0))
		return false;

	VID_PlaySampleBuffer(psgPlaybackBuffer, (int)sampleCount, DDB_PSG_SAMPLE_RATE, 64);
	return true;
}

#endif