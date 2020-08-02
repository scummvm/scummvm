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

#include "backends/platform/3ds/osystem.h"
#include "audio/mixer.h"

namespace _3DS {

static bool hasAudio = false;

static void audioThreadFunc(void *arg) {
	Audio::MixerImpl *mixer = (Audio::MixerImpl *)arg;
	OSystem_3DS *osys = dynamic_cast<OSystem_3DS *>(g_system);

	const int channel = 0;
	int bufferIndex = 0;
	const int bufferCount = 2;
	const int sampleRate = mixer->getOutputRate();
	const int bufferSamples = 1024;
	const int bufferSize = bufferSamples * 4;

	ndspWaveBuf buffers[bufferCount];

	for (int i = 0; i < bufferCount; ++i) {
		memset(&buffers[i], 0, sizeof(ndspWaveBuf));
		buffers[i].data_vaddr = linearAlloc(bufferSize);
		buffers[i].looping = false;
		buffers[i].status = NDSP_WBUF_FREE;
	}

	ndspChnReset(channel);
	ndspChnSetInterp(channel, NDSP_INTERP_LINEAR);
	ndspChnSetRate(channel, sampleRate);
	ndspChnSetFormat(channel, NDSP_FORMAT_STEREO_PCM16);

	while (!osys->exiting) {
		svcSleepThread(5000 * 1000); // Wake up the thread every 5 ms

		if (osys->sleeping) {
			continue;
		}

		ndspWaveBuf *buf = &buffers[bufferIndex];
		if (buf->status == NDSP_WBUF_FREE || buf->status == NDSP_WBUF_DONE) {
			buf->nsamples = mixer->mixCallback(buf->data_adpcm, bufferSize);
			if (buf->nsamples > 0) {
				DSP_FlushDataCache(buf->data_vaddr, bufferSize);
				ndspChnWaveBufAdd(channel, buf);
			}

			bufferIndex++;
			bufferIndex %= bufferCount;
		}
	}

	for (int i = 0; i < bufferCount; ++i)
		linearFree(buffers[i].data_pcm16);
}

void OSystem_3DS::initAudio() {
	_mixer = new Audio::MixerImpl(22050);

	hasAudio = R_SUCCEEDED(ndspInit());
	_mixer->setReady(false);

	if (hasAudio) {
		s32 prio = 0;
		svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
		audioThread = threadCreate(&audioThreadFunc, _mixer, 32 * 1048, prio - 1, -2, false);
	}
}

void OSystem_3DS::destroyAudio() {
	if (hasAudio) {
		threadJoin(audioThread, U64_MAX);
		threadFree(audioThread);
		ndspExit();
	}

	delete _mixer;
	_mixer = 0;
}

Audio::Mixer *OSystem_3DS::getMixer() {
	assert(_mixer);
	return _mixer;
}

} // namespace _3DS
