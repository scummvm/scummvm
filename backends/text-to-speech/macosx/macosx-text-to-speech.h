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

#ifndef BACKENDS_TEXT_TO_SPEECH_MACOSX_H
#define BACKENDS_TEXT_TO_SPEECH_MACOSX_H

#include "common/scummsys.h"

#if defined(USE_TTS) && defined(MACOSX)

#include "common/text-to-speech.h"
#include "common/queue.h"
#include "common/ustr.h"

class MacOSXTextToSpeechManager : public Common::TextToSpeechManager {
public:
	MacOSXTextToSpeechManager();
	virtual ~MacOSXTextToSpeechManager() override;

	virtual bool say(const Common::U32String &str, Action action) override;

	virtual bool stop() override;
	virtual bool pause() override;
	virtual bool resume() override;

	virtual bool isSpeaking() override;
	virtual bool isPaused() override;
	virtual bool isReady() override;

	virtual void setVoice(unsigned index) override;

	virtual void setRate(int rate) override;

	virtual void setPitch(int pitch) override;

	virtual void setVolume(unsigned volume) override;

	virtual void setLanguage(Common::String language) override;

	virtual int getDefaultVoice() override;

	virtual void freeVoiceData(void *data) override;

	bool startNextSpeech();

private:
	virtual void updateVoices() override;

	Common::Queue<Common::String> _messageQueue;
	Common::String _currentSpeech;
	bool _paused;
};

#endif

#endif // BACKENDS_TEXT_TO_SPEECH_MACOSX_H

