// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef MIXER_H
#define MIXER_H

#include "resource.h"
#include <list>
#include <SDL_audio.h>

class Sound;

class Mixer {
public:
  static Mixer *instance();

  void start();

  void playVoice(Sound *s);
  void playSfx(Sound *s);
  void stopSfx(Sound *s);
  void setImuseState(int state);
  void setImuseSeq(int seq);

  Sound *findSfx(const char *filename);
  bool voicePlaying() const;

private:
  Mixer();
  void getAudio(int16_t *data, int numSamples);

  static Mixer *instance_;
  typedef std::list<ResPtr<Sound> > sound_list;
  sound_list voiceSounds_, sfxSounds_;
  ResPtr<Sound> musicSound_, seqSound_;

  friend void mixerCallback(void *userdata, Uint8 *stream, int len);
};

#endif
