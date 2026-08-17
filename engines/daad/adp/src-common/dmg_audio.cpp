#include <dmg.h>
#include <ddb.h>
#include <os_mem.h>
#include <os_lib.h>

// ----------------------------------------------------------------------------
//  Shared audio sample conversion for the memory-starved native players.
//
//  DAT5 audio entries may be stored at rates and depths a given machine's
//  mixer cannot feed directly (16-bit samples, or rates above what the sound
//  hardware can sustain). Rather than allocate a scratch buffer per playback,
//  we convert in place inside the same reusable cache slot the image decoder
//  uses: the raw stored bytes are read and the playback-ready bytes are written
//  over them in a single forward pass. Because every supported conversion only
//  ever shrinks the data (16->8 halves it, decimation drops samples), the write
//  cursor always trails the read cursor and never clobbers unread input. An
//  upward stretch would instead require an end-to-start pass; none is defined
//  today, so only the forward (shrink/equal) direction is implemented.
//
//  Reusing the cache slot also inherits its eviction protection: when the slot
//  is recycled the cache calls VID_StopSampleIfOverlaps, which halts any async
//  playback still reading from it.
// ----------------------------------------------------------------------------

uint32_t DMG_AudioRateHz(int16_t rateCode)
{
	switch (rateCode)
	{
		case DMG_5KHZ:     return  5000;
		case DMG_7KHZ:     return  7000;
		case DMG_9_5KHZ:   return  9500;
		case DMG_15KHZ:    return 15000;
		case DMG_20KHZ:    return 20000;
		case DMG_30KHZ:    return 30000;
		case DMG_44_1KHZ:  return 44100;
		case DMG_48KHZ:    return 48000;
		default:           return 11025;
	}
}

static uint8_t DMG_SourceSampleToU8(const uint8_t* src, uint32_t sample, uint8_t srcDepth)
{
	if (srcDepth >= 16)
	{
		int32_t v = (int16_t)(src[sample * 2] | (src[sample * 2 + 1] << 8));
		return (uint8_t)((v + 32768) >> 8);
	}
	return src[sample];
}

// Converts src (8- or 16-bit, unsigned/native) into 8-bit mono at dst, capping
// the rate to dstRate by nearest-sample decimation and optionally flipping the
// sign. Returns the number of bytes written. Forward pass: safe in place, and
// safe when src and dst overlap provided dst <= src (output never exceeds input).
static uint32_t DMG_ConvertAudioForward(const uint8_t* src, uint32_t srcBytes,
                                        uint8_t srcDepth, uint32_t srcRate,
                                        uint8_t* dst, uint32_t dstRate, bool toSigned)
{
	uint32_t srcSamples = (srcDepth >= 16) ? (srcBytes >> 1) : srcBytes;
	uint8_t  signMask = toSigned ? 0x80 : 0x00;
	uint32_t out = 0;

	if (srcRate == 0 || dstRate >= srcRate)
	{
		for (uint32_t i = 0; i < srcSamples; i++)
			dst[out++] = DMG_SourceSampleToU8(src, i, srcDepth) ^ signMask;
	}
	else
	{
		uint32_t acc = 0;
		for (uint32_t i = 0; i < srcSamples; i++)
		{
			acc += dstRate;
			if (acc >= srcRate)
			{
				acc -= srcRate;
				dst[out++] = DMG_SourceSampleToU8(src, i, srcDepth) ^ signMask;
			}
		}
	}
	return out;
}

// Deterministic output length for the already-converted (cached) fast path,
// matching the accumulator loop above exactly.
static uint32_t DMG_ConvertedByteCount(uint32_t srcSamples, uint32_t srcRate, uint32_t dstRate)
{
	if (srcRate == 0 || dstRate >= srcRate)
		return srcSamples;

	uint32_t acc = 0;
	uint32_t out = 0;
	for (uint32_t i = 0; i < srcSamples; i++)
	{
		acc += dstRate;
		if (acc >= srcRate)
		{
			acc -= srcRate;
			out++;
		}
	}
	return out;
}

uint8_t* DMG_GetEntryAudioConverted(DMG* dmg, uint8_t index,
                                    const DMG_AudioTarget* sink,
                                    uint32_t* outLength, uint32_t* outRate)
{
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0 || entry->type != DMGEntry_Audio || entry->length == 0)
		return 0;

	uint32_t srcRate  = DMG_AudioRateHz(entry->x);
	uint8_t  srcDepth = (entry->bitDepth == 16) ? 16 : 8;
	uint32_t dstRate  = (sink->maxRate != 0 && srcRate > sink->maxRate) ? sink->maxRate : srcRate;
	uint8_t  dstDepth = (sink->bitDepth == 16) ? 16 : 8;

	bool depthChange = (srcDepth == 16 && dstDepth == 8);
	bool rateChange  = (dstRate != srcRate);
	bool signChange  = sink->signedOutput;

	// Fast path: the stored bytes already match the sink; hand them back
	// with no copy or conversion (the common 8-bit, in-range case).
	if (!depthChange && !rateChange && !signChange)
	{
		uint8_t* raw = DMG_GetEntryData(dmg, index, ImageMode_Audio);
		if (raw == 0)
			return 0;
		if (outLength) *outLength = entry->length;
		if (outRate)   *outRate = srcRate;
		return raw;
	}

	uint32_t srcSamples = (srcDepth == 16) ? (entry->length >> 1) : entry->length;

	// Reuse the entry's cache slot as the conversion workspace. If a prior
	// play already converted it, return that result without reworking it.
	DMG_Cache* cache = DMG_GetImageCache(dmg, index, entry, entry->length);
	if (cache != 0 && cache->populated && cache->imageMode == ImageMode_AudioConverted)
	{
		if (outLength) *outLength = DMG_ConvertedByteCount(srcSamples, srcRate, dstRate);
		if (outRate)   *outRate = dstRate;
		return (uint8_t*)(cache + 1);
	}

	// Load the raw stored bytes (into the cache slot for V5, or the file
	// cache / temporary buffer otherwise).
	uint8_t* raw = DMG_GetEntryData(dmg, index, ImageMode_Audio);
	if (raw == 0)
		return 0;

	uint8_t* dst;
	if (cache != 0)
	{
		dst = (uint8_t*)(cache + 1);
	}
	else
	{
		// No cache slot: convert inside the temporary buffer instead of the
		// (shared, read-only) file cache raw may point into.
		dst = DMG_GetTemporaryBuffer(ImageMode_Audio);
		if (dst == 0 || DMG_GetTemporaryBufferSize() < entry->length)
		{
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}
	}

	uint32_t outBytes = DMG_ConvertAudioForward(raw, entry->length, srcDepth, srcRate,
	                                             dst, dstRate, signChange);

	if (cache != 0)
	{
		// Tag the slot as holding converted (not raw) audio so a later
		// DMG_GetEntryData(Audio) reloads the original bytes, while our own
		// fast path above can still reuse this result.
		cache->imageMode = ImageMode_AudioConverted;
		cache->populated = true;
	}

	if (outLength) *outLength = outBytes;
	if (outRate)   *outRate = dstRate;
	return dst;
}
