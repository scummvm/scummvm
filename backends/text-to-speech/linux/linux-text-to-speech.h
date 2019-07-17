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

#ifndef BACKENDS_TEXT_TO_SPEECH_LINUX_H
#define BACKENDS_TEXT_TO_SPEECH_LINUX_H

#include "common/scummsys.h"

#if defined(USE_LINUX_TTS)

#include "common/text-to-speech.h"
#include "common/str.h"

class LinuxTextToSpeechManager : public Common::TextToSpeechManager {
public:
	enum SpeechState {
		READY,
		PAUSED,
		SPEAKING,
		BROKEN
	};

	LinuxTextToSpeechManager();
	virtual ~LinuxTextToSpeechManager();

	virtual bool say(Common::String str);

	virtual bool stop();
	virtual bool pause();
	virtual bool resume();

	virtual bool isSpeaking();
	virtual bool isPaused();
	virtual bool isReady();
	
	virtual void setVoice(unsigned index);

	virtual void setRate(int rate);

	virtual void setPitch(int pitch);

	virtual void setVolume(unsigned volume);
	virtual int getVolume();

	virtual void setLanguage(Common::String language);

	virtual bool popState();

	void updateState(SpeechState state);

private:
	void init();
	virtual void updateVoices();
	void createVoice(int typeNumber, Common::TTSVoice::Gender, char *description);
	SpeechState _speechState;
};

#endif

#endif // BACKENDS_UPDATES_LINUX_H
