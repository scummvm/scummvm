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

#ifndef TOWNS_EUP_H
#define TOWNS_EUP_H

#define EUP_USE_MEMPOOL

#include "audio/softsynth/fmtowns_pc98/towns_audio.h"
#include "common/array.h"
#include "common/func.h"

#ifdef EUP_USE_MEMPOOL
#include "common/memorypool.h"
#endif

class EuphonyBaseDriver {
public:
	EuphonyBaseDriver() {}
	virtual ~EuphonyBaseDriver() {}

	virtual bool init() { return true; }

	virtual void send(uint8 command) = 0;
};

class EuphonyPlayer;

class EuphonyDriver : public EuphonyBaseDriver {
public:
	EuphonyDriver(Audio::Mixer *mixer, EuphonyPlayer *pl);
	~EuphonyDriver();

	bool init();
	void reset();

	int assignPartToChannel(int chan, int part);

	void send(uint8 command);

	void setTimerA(bool enable, int tempo);
	void setTimerB(bool enable, int tempo);

	void loadInstrument(int chanType, int id, const uint8 *data);
	void setInstrument(int chan, int instrID);
	void loadWaveTable(const uint8 *data);
	void unloadWaveTable(int id);

	void reserveSoundEffectChannels(int num);
	void playSoundEffect(int chan, int note, int velo, const uint8 *data);
	void stopSoundEffect(int chan);
	bool soundEffectIsPlaying(int chan);

	void channelPan(int chan, int mode);
	void channelPitch(int chan, int pitch);
	void channelVolume(int chan, int vol);

	void setOutputVolume(int chanType, int volLeft, int volRight);
	void cdaToggle(int a);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

private:
	void noteOff();
	void noteOn();
	void controlChange_volume();
	void controlChange_panPos();
	void controlChange_allNotesOff();
	void programChange();
	void pitchWheel();

	Common::Array<uint8> _currentEvent;
	int8 *_partToChanMapping;
	int8 *_sustainChannels;

	struct Channel {
		int8 part;
		int8 next;
		uint8 note;
		uint8 pri;
	} *_channels;

	TownsAudioInterface *_intf;
};

class Type0Driver : public EuphonyBaseDriver {
public:
	Type0Driver(EuphonyPlayer *pl);
	~Type0Driver();

	bool init();

	void send(uint8 command);
};

class EuphonyPlayer : public TownsAudioInterfacePluginDriver {
public:
	EuphonyPlayer(Audio::Mixer *mixer);
	virtual ~EuphonyPlayer();

	bool init();

	int startTrack(const uint8 *data, int trackSize, int barLen);
	void stop();
	void pause();
	void resume();

	int setTempo(int tempo);
	void setLoopStatus(bool loop);

	bool isPlaying() {return _playing; }

	int configPart_enable(int part, int val);
	int configPart_setType(int part, int val);
	int configPart_remap(int part, int val);
	int configPart_adjustVolume(int part, int val);
	int configPart_setTranspose(int part, int val);

	void timerCallback(int timerId);

	EuphonyDriver *driver() { return _eupDriver; }

private:
	void reset();
	void resetPartConfig();
	void resetTempo();

	void updatePulseCounters();
	void updateBeat();
	void updateParser();
	void updateCheckEot();

	bool parseEvent();
	void proceedToNextEvent();

	void updateHangingNotes();
	void clearHangingNotes();

	void resetAllControls();
	void allPartsOff();

	uint8 appendEvent(uint8 evt, uint8 chan);

	bool event_notImpl();
	bool event_noteOn();
	bool event_polyphonicAftertouch();
	bool event_controlChange_pitchWheel();
	bool event_programChange_channelAftertouch();

	bool event_sysex();
	bool event_advanceBar();
	bool event_setTempo();
	bool event_typeOrdrChange();

	uint8 applyTranspose(uint8 in);
	uint8 applyVolumeAdjust(uint8 in);

	void sendByte(uint8 type, uint8 command);
	void sendPendingEvent(int type, int evt, int note, int velo);
	void sendControllerReset(int type, int part);
	void sendAllNotesOff(int type, int part);
	void sendTempo(int tempo);

	uint8 *_partConfig_enable;
	uint8 *_partConfig_type;
	uint8 *_partConfig_ordr;
	int8 *_partConfig_volume;
	int8 *_partConfig_transpose;

	struct PendingEvent {
		PendingEvent(int ev, int tp, int nt, int vl, int ln, PendingEvent *chain) : evt(ev), type(tp), note(nt), velo(vl), len(ln), next(chain) {}
		uint8 evt;
		uint8 type;
		uint8 note;
		uint8 velo;
		uint16 len;
		PendingEvent *next;
	};

#ifdef EUP_USE_MEMPOOL
	Common::ObjectPool<PendingEvent> _pendingEventsPool;
#endif
	PendingEvent *_pendingEventsChain;

	typedef Common::Functor0Mem<bool, EuphonyPlayer> EuphonyEvent;
	typedef Common::Array<const EuphonyEvent*> EuphonyEventsArray;
	EuphonyEventsArray _euphonyEvents;

	uint8 _defaultBarLength;
	uint8 _barLength;
	int _playerUpdatesLeft;
	int _tempoControlMode;
	int _updatesPerPulseRemainder;
	int _updatesPerPulse;
	int _timerSetting;
	int8 _tempoMode1PulseCounter;
	int _tempoModifier;
	uint32 _bar;
	uint32 _parseToBar;
	int8 _tempoMode1UpdateF8;
	uint8 _deltaTicks;
	uint32 _beat;
	uint8 _defaultTempo;
	int _trackTempo;

	bool _loop;
	bool _playing;
	bool _endOfTrack;
	bool _paused;

	const uint8 *_musicStart;
	const uint8 *_musicPos;
	uint32 _musicTrackSize;

	EuphonyDriver *_eupDriver;
	EuphonyBaseDriver *_drivers[3];
};

#endif
