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


#ifndef QDENGINE_SYSTEM_SOUND_SND_SOUND_H
#define QDENGINE_SYSTEM_SOUND_SND_SOUND_H


namespace QDEngine {

class wavSound;
class qdNamedObject;

//! Класс для управления звуками.
class sndHandle {
public:
	sndHandle() { };
	virtual ~sndHandle() { };
};

//! Базовый класс для звуков.
class sndSound {
public:
	explicit sndSound(const wavSound *snd, const sndHandle *h = NULL) : _sound(snd), _handle(h) { }
	sndSound(const sndSound &snd) : _sound(snd._sound), _handle(snd._handle) { }
	virtual ~sndSound() { };

	sndSound &operator = (const sndSound &s) {
		if (this == &s) return *this;

		_sound = s._sound;
		_handle = s._handle;

		return *this;
	}

	//! Состояние звука.
	enum status_t {
		//! звук не проигрывается
		SOUND_STOPPED,
		//! звук приостановлен
		SOUND_PAUSED,
		//! звук пригрывается
		SOUND_PLAYING
	};

	//! Возвращает состояние звука.
	virtual status_t status() const {
		return SOUND_STOPPED;
	}

	//! Возвращает указатель на данные звука.
	const wavSound *sound() const {
		return _sound;
	}
	//! Возвращает указатель на хэндл звука.
	const sndHandle *handle() const {
		return _handle;
	}

private:
	//! Указатель на данные.
	const wavSound *_sound;
	//! Указатель на хэндл звука.
	const sndHandle *_handle;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_SOUND_SND_SOUND_H
