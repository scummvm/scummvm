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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include <sys/time.h>
#include <time.h>

#include <android/log.h>
#include <oboe/Oboe.h>

#include "backends/mixer/android/ringbuffer.h"

#include "backends/mixer/android/android-mixer.h"
#include "backends/platform/android/android.h"
#include "backends/platform/android/jni-android.h"

//#define OBOE_DEBUG

extern const char *android_log_tag;

class AndroidMixerManagerImpl : public AndroidMixerManager, public oboe::AudioStreamDataCallback {
public:
	AndroidMixerManagerImpl() : _chunkSize(0), _buffer(nullptr), _stream(nullptr), _latency(nullptr), _audio_thread_exit(false), _underflows(0) {}
	~AndroidMixerManagerImpl() override;

	void init() override;
	void signalQuit() override { _audio_thread_exit = true; }
	void quit() override;

	void notifyAudioDisconnect() { _plugEvent.store(true); }
private:
	oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) override;

	void initStream();
	static void *audioThreadFunc(void *arg);

	// We only support 16-bits interleaved stereo streams
	static constexpr size_t kChannelCount = 2;
	typedef int16 frame_t[kChannelCount];

	size_t _chunkSize;

	RingBuffer<frame_t> *_buffer;
	std::shared_ptr<oboe::AudioStream> _stream_ptr;
	oboe::LatencyTuner *_latency;

	bool _audio_thread_exit;
	pthread_t _audio_thread;

	// LLVM between 8 and 10 wrongfully said they supported this
#if 0 && __cpp_lib_hardware_interference_size
	static constexpr size_t hardware_destructive_interference_size = std::hardware_destructive_interference_size;
#else
	// 64 is a good fit for most platforms
	static constexpr size_t hardware_destructive_interference_size = 64;
#endif

	// Written in mixing thread, read in callback
	// As there is no atomic support for shared_ptr before C++20, copy the pointer here to allow for checking in callback
	alignas(hardware_destructive_interference_size) std::atomic<oboe::AudioStream *> _stream;
	// Written in data callback, read in mixing thread and data callback
	alignas(hardware_destructive_interference_size) std::atomic<size_t> _underflows;
	// Written in Android threads, read/written in mixing thread
	alignas(hardware_destructive_interference_size) std::atomic<bool> _plugEvent;
};

AndroidMixerManager *AndroidMixerManager::make() {
	return new AndroidMixerManagerImpl();
}

AndroidMixerManagerImpl::~AndroidMixerManagerImpl() {
	delete _latency;
	_stream_ptr.reset();
	delete _buffer;
}

void AndroidMixerManagerImpl::init() {
	initStream();

	// Our mixer code does IO while rendering.
	// This is forbidden for Oboe callbacks: use another thread for it.
	_audio_thread_exit = false;
	pthread_create(&_audio_thread, 0, audioThreadFunc, this);
}

void AndroidMixerManagerImpl::quit() {
	pthread_join(_audio_thread, 0);

	LOGD("Stopping audio stream");
	_stream_ptr->stop();

	LOGD("Closing audio stream");
	_stream_ptr->close();

	// After this point, the stream is expected to be unused

	LOGD("Cleanup audio stream");

	delete _latency;
	_latency = nullptr;

	_stream_ptr.reset();
}

void AndroidMixerManagerImpl::initStream() {
	oboe::AudioStreamBuilder builder;
	std::shared_ptr<oboe::AudioStream> stream;

	stream.swap(_stream_ptr);

	if (stream) {
		LOGD("Delete old audio stream");
		stream->close();

		delete _latency;
		_latency = nullptr;
		// After this point, the stream is expected to be unused

		stream.reset();
	}

	builder.setDirection(oboe::Direction::Output)
		//->setAudioApi(oboe::AudioApi::OpenSLES)
		->setSharingMode(oboe::SharingMode::Exclusive)
		->setPerformanceMode(oboe::PerformanceMode::LowLatency)
		->setChannelConversionAllowed(true)
		->setChannelCount(oboe::ChannelCount::Stereo)
		->setFormatConversionAllowed(true)
		->setFormat(oboe::AudioFormat::I16)
		->setDataCallback(this);

	if (_mixer) {
		// We can't tear down our mixer when started.
		// So, reuse its sample rate
		LOGD("Reuse old mixer rate: %d", _mixer->getOutputRate());
		builder.setSampleRate(_mixer->getOutputRate());
		// Let's stick to Oboe's default of Medium quality resampling
	}

	oboe::Result result = builder.openStream(stream);
	if (result != oboe::Result::OK) {
		// Calling log_assert allows us to get the error message reported through Google
		// This aborts.
		__android_log_assert(nullptr, android_log_tag,
		                     "Failed to create audio stream. Error: %s",
		                     oboe::convertToText(result));
	}

	oboe::AudioApi streamApi = stream->getAudioApi();
	LOGD("Got new stream %s -> %p", oboe::convertToText(streamApi), stream.get());

	int32_t sampleRate = stream->getSampleRate();
	size_t chunkSize = stream->getFramesPerBurst();

	// Configure the RingBuffer using full buffer capacity to make sure we can provide the requested samples
	size_t bufferCapacity = stream->getBufferCapacityInFrames();

	if (streamApi == oboe::AudioApi::OpenSLES) {
		// With OpenSL ES, Oboe has 2 buffers of size defined by Java (AudioTrack or AudioManager)
		// This is not really enough so we can get several consecutive callbacks which drain the ring buffer
		// Instead of using bigger bursts, increase here the ring buffer size up to ~150ms
		size_t minCapacity = sampleRate / 6;
		if (bufferCapacity < minCapacity) {
			bufferCapacity = minCapacity;
		}
	}

	if (!_buffer) {
		LOGD("Setting up ring buffer with capacity: %zu", bufferCapacity);
		_buffer = new RingBuffer<frame_t>(bufferCapacity);
	} else if (_chunkSize != chunkSize) {
		LOGD("Reconfiguring ring buffer with capacity: %zu", bufferCapacity);
		RingBuffer<frame_t> *old_buffer = _buffer;
		_buffer = new RingBuffer<frame_t>(bufferCapacity, std::move(*old_buffer));
		delete old_buffer;
	}

	if (!_mixer) {
		LOGD("Setting up mixer with settings sample rate: %d and buffer: %zu", sampleRate, chunkSize);
		_mixer = new Audio::MixerImpl(sampleRate, kChannelCount, chunkSize);
		_mixer->setReady(true);
	} else if (_chunkSize != chunkSize) {
		LOGD("Reconfiguring mixer with buffer %zu", chunkSize);
		_mixer->setOutputBufSize(chunkSize);
	}

	stream->setBufferSizeInFrames(chunkSize);
	_chunkSize = chunkSize;

	_latency = new oboe::LatencyTuner(*stream);

	_stream_ptr.swap(stream);
	LOGD("Finished initStream");
}


oboe::DataCallbackResult AndroidMixerManagerImpl::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
	if (audioStream != _stream.load()) {
		// That's not our stream: stop here
		return oboe::DataCallbackResult::Stop;
	}

#ifdef OBOE_DEBUG
	const int32_t numFrames_ = numFrames;
#endif

	frame_t *outputData = static_cast<frame_t *>(audioData);

	while (numFrames > 0) {
		size_t n = numFrames;
		frame_t *inputData = _buffer->try_consume(&n);
		if (!inputData) {
			break;
		}
		memcpy(outputData, inputData, n * sizeof(frame_t));
		_buffer->consumed();
		numFrames -= n;
		outputData += n;
		_underflows.store(0, std::memory_order_release);
	}
#ifdef OBOE_DEBUG
	LOGD("onAudioReady: numFrames=%d/%d => underflows=%zu", numFrames, numFrames_, _underflows.load(std::memory_order_relaxed));
#endif
	if (numFrames > 0) {
		memset(outputData, 0, numFrames * sizeof(frame_t));

		size_t underflows = _underflows.load(std::memory_order_relaxed);
		underflows += numFrames;
		if (underflows < 0x80000000) {
			_underflows.store(underflows, std::memory_order_release);
		}
	}

	_latency->tune();
	return oboe::DataCallbackResult::Continue;
}

static void samples_to_ts(struct timespec *ts, uint64 samples, uint64 sampleRate) {
	static constexpr uint64 kNS = 1000000000; // 1s == 1e9ns

	const uint64 ns = samples * kNS / sampleRate;
	ts->tv_sec  = ns / kNS;
	ts->tv_nsec = ns % kNS;
}

static int64_t diff_ts_ns(const struct timespec *tv_a, const struct timespec *tv_b) {
	static constexpr uint64 kNS = 1000000000; // 1s == 1e9ns

	return (int64_t)(tv_a->tv_sec - tv_b->tv_sec) * kNS +
		        (tv_a->tv_nsec - tv_b->tv_nsec);
}

void *AndroidMixerManagerImpl::audioThreadFunc(void *arg) {
	AndroidMixerManagerImpl *this_ = (AndroidMixerManagerImpl *)arg;
	Audio::MixerImpl *mixer = this_->_mixer;

	std::shared_ptr<oboe::AudioStream> stream = this_->_stream_ptr;
	this_->_stream.store(stream.get());

	int32_t sampleRate = stream->getSampleRate();

	// Wait for 2 secs of silence before pausing the stream
	const size_t silencePause = sampleRate * 2;
	size_t silence_count = 0;

	struct timespec tv_chunk, tv_silence;
	// We try to fill half by half
	size_t chunkSize = this_->_chunkSize / 2;
	samples_to_ts(&tv_chunk, chunkSize, sampleRate);

	// Variables of Android P/Q worakaround for AAudio
	bool plugWaiting = false;
	struct timespec tv_plugEvent, tv_recreated;

#define STREAM_RECREATE()                                                      \
	do {                                                                   \
		stream.reset();                                                \
		this_->_stream.store(nullptr);                                 \
		this_->initStream();                                           \
		/* Refresh our local copy and synchronize for callback */      \
		stream = this_->_stream_ptr;                                   \
		this_->_stream.store(stream.get());                            \
		chunkSize = this_->_chunkSize / 2;                             \
		samples_to_ts(&tv_chunk, chunkSize, stream->getSampleRate());  \
		clock_gettime(CLOCK_MONOTONIC, &tv_recreated);                 \
	} while (false)


	while (!this_->_audio_thread_exit) {
		const oboe::StreamState state = stream->getState();
		const bool started = (state == oboe::StreamState::Starting ||
				state == oboe::StreamState::Started);
		if (JNI::pause) {
			if (started) {
				LOGD("Pausing audio stream");
				stream->requestPause();
			}

			LOGD("audio thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("audio thread woke up");

			if (started) {
				LOGD("Restarting audio stream");
				oboe::Result result = stream->start();
				if (result != oboe::Result::OK) {
					// Oops something went wrong: recreate the stream
					STREAM_RECREATE();
				}
			}

			// Redo to refresh the state
			continue;
		}

		// begin Android P/Q workaround for AAudio not detection disconnections
		if (plugWaiting) {
			timespec tv_now;
			clock_gettime(CLOCK_MONOTONIC, &tv_now);

			if (diff_ts_ns(&tv_now, &tv_plugEvent) > 3000000000) {
				// timeout is now elapsed, check if a recreate happened since the first plug event
				plugWaiting = false;

				bool recreate = diff_ts_ns(&tv_recreated, &tv_plugEvent) < 0;
				LOGI("Plug event timeout elpased: will recreate=%d", recreate);
				if (recreate) {
					stream->stop();
					STREAM_RECREATE();
				}
				continue;
			}
		}
		if (this_->_plugEvent.exchange(false, std::memory_order_acquire)) {
			LOGI("Got a plug/unplug event from Java");
			if (!plugWaiting) {
				if (stream->usesAAudio() && oboe::OboeExtensions::isMMapUsed(stream.get())) {
					// Register the event time
					clock_gettime(CLOCK_MONOTONIC, &tv_plugEvent);

					if (diff_ts_ns(&tv_plugEvent, &tv_recreated) > 1000000000) {
						// The last recreate happened more than 1s before: that's not tied
						plugWaiting = true;
						// leave some iterations to get a disconnect from Oboe
						if (!started) {
							// Force a start to detect the disconnection
							// We will pause at the next loop
							oboe::Result result = stream->start();
							if (result != oboe::Result::OK) {
								// Oops something went wrong: recreate the stream
								STREAM_RECREATE();
							}
							continue;
						}
					} else {
						LOGI("Plug event already catched by Oboe");
					}
				} else {
					LOGI("Not a MMap stream: no workaround needed => ignoring");
				}
			}
		}
		// end Android P/Q workaround for AAudio not detection disconnections

		size_t req = chunkSize;
		frame_t *outputData = this_->_buffer->try_produce(&req);
		if (!outputData) {
			// buffer is full: either we have to wait for a new slot or to restart the stream
			if (started) {
#ifdef OBOE_DEBUG
				LOGD("Waiting for buffer space for %ld.%09ld", tv_chunk.tv_sec, tv_chunk.tv_nsec);
#endif
				nanosleep(&tv_chunk, nullptr);
				continue;
			}

			oboe::Result result = stream->start();
			if (result != oboe::Result::OK) {
				// Oops something went wrong: recreate the stream
				STREAM_RECREATE();
				// Don't restart in this case to let the time for the buffer to fill
			}

			// Stream is restarted: let's try again
			continue;
		}

		// mixCallback returns a number of frames although we pass to it a buffer size in bytes
		size_t n = mixer->mixCallback((byte *)outputData, req * sizeof(frame_t));

		// check if the burst is full of silence so we can shut the stream
		bool silence = true;
		for (int i = 0; i < n; i++) {
			// SID streams constant crap
			if (outputData[i][0] > 32 || outputData[i][1] > 32) {
				silence = false;
				break;
			}
		}
		if (silence) {
			// The whole chunk was silence: add it to our count
			silence_count += n;
			if (silence_count > 0x80000000) {
				silence_count = 0x80000000;
			}
			if (!started) {
				// if the stream is not playing, don't feed it with silence: that would make it start
				n = 0;
			}
		} else {
			silence_count = 0;
		}

		// If the mixer didn't produce any data (because there is no stream), the buffer will starve.
		// The callback will then fill with placeholder silence and increase the underflow count.
		// We will then pause the stream below.
		this_->_buffer->produced(n);

#ifdef OBOE_DEBUG
		if (n > 0) {
			LOGD("Requested mix of %zu frames and got %zu%s - Buffer size: %d / %d", req, n, silence ? " S" : "", stream->getBufferSizeInFrames(), stream->getBufferCapacityInFrames());
		}
#endif

		if (silence && started) {
			// nothing was produced
			size_t underflows = this_->_underflows.load(std::memory_order_acquire);
			if (underflows + silence_count >= silencePause) {
				LOGD("Silence %zu+%zu: pausing audio stream", underflows, silence_count);
				stream->pause();
				stream->requestFlush();
			}
		}

		if (n == 0) {
			// we either didn't produce anything or produced silence while stopped
			// wait as if we pushed it
			samples_to_ts(&tv_silence, req, sampleRate);
			nanosleep(&tv_silence, nullptr);
		}
	}

#undef STREAM_RECREATE

	this_->_stream.store(nullptr);
	return 0;
}

void JNI::setDefaultAudioValues(JNIEnv *env, jclass clazz, jint sampleRate, jint framesPerBurst) {
	LOGD("Default audio values are sr=%d fpb=%d", sampleRate, framesPerBurst);
	oboe::DefaultStreamValues::SampleRate = sampleRate;
	oboe::DefaultStreamValues::FramesPerBurst = framesPerBurst;
}

void JNI::notifyAudioDisconnect(JNIEnv *env, jclass clazz) {
	if (!g_system) {
		return;
	}
	MixerManager *mm = dynamic_cast<OSystem_Android *>(g_system)->getMixerManager();
	if (!mm) {
		return;
	}
	dynamic_cast<AndroidMixerManagerImpl *>(mm)->notifyAudioDisconnect();
}
