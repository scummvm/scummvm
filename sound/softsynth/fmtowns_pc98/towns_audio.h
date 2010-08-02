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

#ifndef TOWNS_AUDIO_H
#define TOWNS_AUDIO_H

#include "sound/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"

class TownsAudioInterfacePluginDriver {
public:
	virtual ~TownsAudioInterfacePluginDriver() {}
	virtual void timerCallback(int timerId) = 0;
};

class TownsAudio_PcmChannel;
class TownsAudio_WaveTable;

class TownsAudioInterface : public TownsPC98_FmSynth {
public:
	TownsAudioInterface(Audio::Mixer *mixer, TownsAudioInterfacePluginDriver *driver);
	~TownsAudioInterface();

	bool init();

	int callback(int command, ...);	

private:
	void nextTickEx(int32 *buffer, uint32 bufferSize);

	void timerCallbackA();
	void timerCallbackB();

	typedef int (TownsAudioInterface::*TownsAudioIntfCallback)(va_list&);
	const TownsAudioIntfCallback *_intfOpcodes;

	int intf_reset(va_list &args);
	int intf_keyOn(va_list &args);
	int intf_keyOff(va_list &args);
	int intf_setPanPos(va_list &args);
	int intf_setInstrument(va_list &args);
	int intf_loadInstrument(va_list &args);
	int intf_setPitch(va_list &args);
	int intf_setLevel(va_list &args);
	int intf_chanOff(va_list &args);
	int intf_writeReg(va_list &args);
	int intf_bufferedWriteReg(va_list &args);
	int intf_readRegBuffer(va_list &args);
	int intf_setTimerA(va_list &args);
	int intf_setTimerB(va_list &args);
	int intf_enableTimerA(va_list &args);
	int intf_enableTimerB(va_list &args);	
	int intf_reserveEffectChannels(va_list &args);
	int intf_loadWaveTable(va_list &args);
	int intf_unloadWaveTable(va_list &args);
	int intf_pcmPlayEffect(va_list &args);
	int intf_pcmChanOff(va_list &args);
	int intf_pcmEffectPlaying(va_list &args);
	int intf_fmKeyOn(va_list &args);
	int intf_fmKeyOff(va_list &args);
	int intf_fmSetPanPos(va_list &args);
	int intf_fmSetInstrument(va_list &args);
	int intf_fmLoadInstrument(va_list &args);
	int intf_fmSetPitch(va_list &args);
	int intf_fmSetLevel(va_list &args);
	int intf_fmReset(va_list &args);
	int intf_cdaSetVolume(va_list &args);
	int intf_cdaReset(va_list &args);
	int intf_pcmUpdateEnvelopeGenerator(va_list &args);

	int intf_notImpl(va_list &args);
	
	void fmReset();
	int fmKeyOn(int chan, int note, int velo);
	int fmKeyOff(int chan);
	int fmChanOff(int chan);
	int fmSetPanPos(int chan, int mode);	
	int fmSetInstrument(int chan, int instrId);
	int fmLoadInstrument(int instrId, const uint8 *data);
	int fmSetPitch(int chan, int pitch);
	int fmSetLevel(int chan, int lvl);

	void bufferedWriteReg(uint8 part, uint8 regAddress, uint8 value);
	
	uint8 _fmChanPlaying;
	uint8 _fmChanNote[6];
	int16 _fmChanPitch[6];

	uint8 *_fmSaveReg[2];
	uint8 *_fmInstruments;
	
	void pcmReset();
	int pcmKeyOn(int chan, int note, int velo);
	int pcmKeyOff(int chan);
	int pcmChanOff(int chan);
	int pcmSetPanPos(int chan, int mode);
	int pcmSetInstrument(int chan, int instrId);
	int pcmLoadInstrument(int instrId, const uint8 *data);
	int pcmSetPitch(int chan, int pitch);
	int pcmSetLevel(int chan, int lvl);
	void pcmUpdateEnvelopeGenerator(int chan);

	TownsAudio_PcmChannel *_pcmChan;
	uint8 _pcmChanOut;
	uint8 _pcmChanReserved;
	uint8 _pcmChanKeyPressed;
	uint8 _pcmChanEffectPlaying;
	uint8 _pcmChanKeyPlaying;
	
	uint8 _pcmChanNote[8];
	uint8 _pcmChanVelo[8];
	uint8 _pcmChanLevel[8];

	uint8 _numReservedChannels;
	uint8 *_pcmInstruments;
	
	TownsAudio_WaveTable *_waveTables;
	uint8 _numWaveTables;
	uint32 _waveTablesTotalDataSize;

	void pcmCalcPhaseStep(TownsAudio_PcmChannel *p, TownsAudio_WaveTable *w);

	void cdaReset();

	const float _baserate;
	uint32 _timerBase;
	uint32 _tickLength;
	uint32 _timer;

	TownsAudioInterfacePluginDriver *_drv;
	bool _ready;

	static const uint8 _chanFlags[];
	static const uint16 _frequency[];
	static const uint8 _carrier[];
	static const uint8 _fmDefaultInstrument[];
	static const uint16 _pcmPhase1[];
	static const uint16 _pcmPhase2[];	
};

#endif

