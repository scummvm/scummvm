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

#include "common/system.h"

#include "sci/tools.h"
#include "sci/sfx/mixer.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Sci {

class PCMFeedAudioStream : public Audio::AudioStream {
protected:
	enum FeedMode {
		FEED_MODE_ALIVE,
		FEED_MODE_IDLE,
		FEED_MODE_DEAD,
		FEED_MODE_RESTART
	};

	/* Whether feed is alive or dead. */
	FeedMode _mode;

	/* Blank gap in frames. */
	int _gap;

	/* Feed. */
	sfx_pcm_feed_t *_feed;

	/* Timestamp of next frame requested by stream driver. */
	sfx_timestamp_t _time;

public:
	PCMFeedAudioStream(sfx_pcm_feed_t *feed) : _feed(feed) {
		_feed->frame_size = (_feed->conf.stereo ? 2 : 1) * ((_feed->conf.format & SFX_PCM_FORMAT_16) ? 2 : 1);
		_mode = FEED_MODE_ALIVE;
		_gap = 0;
		_time = sfx_new_timestamp(g_system->getMillis(), _feed->conf.rate);
	}

	~PCMFeedAudioStream() {
		_feed->destroy(_feed);
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

	virtual bool isStereo() const { return _feed->conf.stereo; }
	virtual int getRate() const { return _feed->conf.rate; }

	virtual bool endOfData() const { return _mode == FEED_MODE_DEAD; }

protected:
	void queryTimestamp();
};

void PCMFeedAudioStream::queryTimestamp() {
	if (_feed->get_timestamp) {
		sfx_timestamp_t stamp;
		int val = _feed->get_timestamp(_feed, &stamp);

		switch (val) {
		case PCM_FEED_TIMESTAMP:
			_gap = sfx_timestamp_frame_diff(stamp, _time);

			if (_gap >= 0)
				_mode = FEED_MODE_ALIVE;
			else {
				// FIXME: I don't quite understand what FEED_MODE_RESTART is for.
				// The original DC mixer seemed to just stop and restart the stream.
				// But why? To catch up with lagging sound?
				//
				// Walter says the following:
				// "The FEED_MODE_RESTART might be there to re-sync after a debugger
				//  session where time passes for the mixer but not for the engine.
				//  I may have added this as a workaround for not being able to come
				//  up with a convenient way to implement mixer->pause() and mixer->resume()
				//  on DC."
				// That makes some sense.
				_mode = FEED_MODE_RESTART;
				_time = sfx_new_timestamp(g_system->getMillis(), _feed->conf.rate);
				_gap = sfx_timestamp_frame_diff(stamp, _time);

				if (_gap < 0)
					_gap = 0;
			}
			break;
		case PCM_FEED_IDLE:
			_mode = FEED_MODE_IDLE;
			break;
		case PCM_FEED_EMPTY:
			_mode = FEED_MODE_DEAD;
			_gap = 0;
		}
	} else {
		_mode = FEED_MODE_DEAD;
		_gap = 0;
	}
}

static void U8_to_S16(byte *buf, int samples) {
	for (int i = samples - 1; i >= 0; i--) {
		buf[i * 2 + 1] = buf[i] - 0x80;
		buf[i * 2] = 0;
	}
}

int PCMFeedAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	// FIXME: If ScummVM's mixer supported timestamps, then it would pass them
	// as a parameter to this function. But currently, it doesn't. Therefore, we
	// create a fake timestamp based on the current time. For comparison, a real
	// timestamp could be adjusted for pauses in sound processing. And it would
	// be synced for all audio streams.
	sfx_timestamp_t timestamp = sfx_new_timestamp(g_system->getMillis(), _feed->conf.rate);

	int channels, frames_req;
	int frames_recv = 0;

	_time = timestamp;
	channels = _feed->conf.stereo == SFX_PCM_MONO ? 1 : 2;
	frames_req = numSamples / channels;

	while (frames_req != frames_recv) {
		int frames_left = frames_req - frames_recv;
		byte *buf_pos = ((byte *)buffer) + frames_recv * channels * 2;

		if (_mode == FEED_MODE_IDLE)
			queryTimestamp();

		if (_mode == FEED_MODE_IDLE || _mode == FEED_MODE_DEAD) {
			memset(buf_pos, 0, frames_left * channels * 2);

			_time = sfx_timestamp_add(_time, frames_left);
			break;
		}

		if (_gap) {
			int frames = _gap;

			if (frames > frames_left)
				frames = frames_left;

			memset(buf_pos, 0, frames * channels * 2);

			_gap -= frames;
			frames_recv += frames;
			_time = sfx_timestamp_add(_time, frames);
		} else {
			int frames = _feed->poll(_feed, buf_pos, frames_left);

			if (_feed->conf.format == SFX_PCM_FORMAT_U8)
				U8_to_S16(buf_pos, frames * channels);

			frames_recv += frames;
			_time = sfx_timestamp_add(_time, frames);

			if (frames < frames_left)
				queryTimestamp();
		}
	}

	return numSamples;
}

void mixer_subscribe(sfx_pcm_feed_t *feed) {
	if ((feed->conf.format != SFX_PCM_FORMAT_S16_NATIVE) && (feed->conf.format != SFX_PCM_FORMAT_U8)) {
		error("[soft-mixer] Unsupported feed format %d", feed->conf.format);
	}
	
	PCMFeedAudioStream *newStream = new PCMFeedAudioStream(feed);

	// FIXME: Is this sound type appropriate? The mixer seems to handle music, too.
	g_system->getMixer()->playInputStream(Audio::Mixer::kSFXSoundType, 0, newStream);

	debug(2, "[soft-mixer] Subscribed %s-%x (%d Hz, %d/%x)",
	          feed->debug_name, feed->debug_nr, feed->conf.rate, feed->conf.stereo, feed->conf.format);
}

} // End of namespace Sci
