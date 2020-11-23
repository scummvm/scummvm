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

#ifndef _ANDROID_H_
#define _ANDROID_H_

#if defined(__ANDROID__)

#include "backends/platform/android/portdefs.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/mutex.h"
#include "common/ustr.h"
#include "audio/mixer_intern.h"
#include "backends/modular-backend.h"
#include "backends/plugins/posix/posix-provider.h"
#include "backends/fs/posix/posix-fs-factory.h"

#include <pthread.h>

#include <android/log.h>

// toggles start
//#define ANDROID_DEBUG_ENTER
// toggles end

extern const char *android_log_tag;

#define _ANDROID_LOG(prio, fmt, args...) __android_log_print(prio, android_log_tag, fmt, ## args)
#define LOGD(fmt, args...) _ANDROID_LOG(ANDROID_LOG_DEBUG, fmt, ##args)
#define LOGI(fmt, args...) _ANDROID_LOG(ANDROID_LOG_INFO, fmt, ##args)
#define LOGW(fmt, args...) _ANDROID_LOG(ANDROID_LOG_WARN, fmt, ##args)
#define LOGE(fmt, args...) _ANDROID_LOG(ANDROID_LOG_ERROR, fmt, ##args)

#ifdef ANDROID_DEBUG_ENTER
#define ENTER(fmt, args...) LOGD("%s(" fmt ")", __FUNCTION__, ##args)
#else
#define ENTER(fmt, args...) do {  } while (false)
#endif

class OSystem_Android : public ModularMutexBackend, public ModularGraphicsBackend, Common::EventSource {
private:
	// passed from the dark side
	int _audio_sample_rate;
	int _audio_buffer_size;

	int _screen_changeid;

	pthread_t _main_thread;

	bool _timer_thread_exit;
	pthread_t _timer_thread;
	static void *timerThreadFunc(void *arg);

	bool _audio_thread_exit;
	pthread_t _audio_thread;
	static void *audioThreadFunc(void *arg);

	bool _virtkeybd_on;

	Audio::MixerImpl *_mixer;
	timeval _startTime;

	Common::String getSystemProperty(const char *name) const;

public:
	OSystem_Android(int audio_sample_rate, int audio_buffer_size);
	virtual ~OSystem_Android();

	virtual void initBackend() override;

	virtual bool hasFeature(OSystem::Feature f) override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) override;

public:
	void pushEvent(int type, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);

private:
	Common::Queue<Common::Event> _event_queue;
	Common::Event _queuedEvent;
	uint32 _queuedEventTime;
	Common::Mutex *_event_queue_lock;

	Common::Point _touch_pt_down, _touch_pt_scroll, _touch_pt_dt;
	int _eventScaleX;
	int _eventScaleY;
	bool _touchpad_mode;
	int _touchpad_scale;
	int _trackball_scale;
	int _dpad_scale;
	int _joystick_scale;
	int _fingersDown;

	void pushEvent(const Common::Event &event);

public:
	virtual bool pollEvent(Common::Event &event) override;
	virtual Common::HardwareInputSet *getHardwareInputSet() override;
	virtual Common::KeymapArray getGlobalKeymaps() override;
	virtual Common::KeymapperDefaultBindings *getKeymapperDefaultBindings() override;

	virtual void registerDefaultSettings(const Common::String &target) const override;
	virtual GUI::OptionsContainerWidget *buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
	virtual void applyBackendSettings() override;

	virtual uint32 getMillis(bool skipRecord = false) override;
	virtual void delayMillis(uint msecs) override;

	virtual void quit() override;

	virtual void setWindowCaption(const Common::U32String &caption) override;

	virtual Audio::Mixer *getMixer() override;
	virtual void getTimeAndDate(TimeDate &t) const override;
	virtual void logMessage(LogMessageType::Type type, const char *message) override;
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0) override;
	virtual bool openUrl(const Common::String &url) override;
	virtual bool hasTextInClipboard() override;
	virtual Common::U32String getTextFromClipboard() override;
	virtual bool setTextInClipboard(const Common::U32String &text) override;
	virtual bool isConnectionLimited() override;
	virtual Common::String getSystemLanguage() const override;
};

#endif
#endif
