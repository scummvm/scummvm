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

#include "stdafx.h"
#include "mixer.h"
#include "sound.h"
#include "debug.h"
#include <cstdlib>
#include <SDL_audio.h>

class AudioLock {
public:
  AudioLock() { SDL_LockAudio(); }
  ~AudioLock() { SDL_UnlockAudio(); }
};

struct imuseTableEntry {
  int stateNum;
  const char *filename;
};

static const imuseTableEntry grimMusicTable[] = {
  { 1001, "1001 - Manny's Office.IMC" },
  { 1002, "1002 - Mr. Frustration.IMC" },
  { 1003, "1002 - Mr. Frustration.IMC" },
  { 1004, "1002 - Mr. Frustration.IMC" },
  { 1005, "1002 - Mr. Frustration.IMC" },
  { 1006, "1002 - Mr. Frustration.IMC" },
  { 1007, "1002 - Mr. Frustration.IMC" },
  { 1008, "1008 - Domino's Office.IMC" },
  { 1009, "1009 - Copal's Office.IMC" },
  { 1010, "1010 - Ledge.IMC" },
  { 1011, "1011 - Roof.IMC" },
  { 1020, "1020 - Tube Room.IMC" },
  { 1021, "1021 - Brennis.IMC" },
  { 1022, "1022 - Lobby.IMC" },
  { 1023, "1023 - Packing Room.IMC" },
  { 1030, "1030 - Garage.IMC" },
  { 1031, "1031 - Glottis' Shop.IMC" },
  { 1032, "1030 - Garage.IMC" },
  { 1040, "1040 - Festival Wet.IMC" },
  { 1041, "1041 - Festival Dry.IMC" },
  { 1042, "1041 - Festival Dry.IMC" },
  { 1043, "1041 - Festival Dry.IMC" },
  { 1044, "1040 - Festival Wet.IMC" },
  { 1050, "1050 - Headquarters.IMC" },
  { 1060, "1060 - Real World.IMC" },
  { 1070, "1070 - Stump Room.IMC" },
  { 1071, "1071 - Signpost Room.IMC" },
  { 1072, "1072 - Navigation.IMC" },
  { 1073, "1071 - Signpost Room.IMC" },
  { 1074, "1074 - Bone Wagon.IMC" },
  { 1075, "1075 - Spider's Eye.IMC" },
  { 1076, "1076 - Spider Room.IMC" },
  { 1077, "1077 - Tree Pump Amb.IMC" },
  { 1078, "1078 - Tree Pump.IMC" },
  { 1079, "1071 - Signpost Room.IMC" },
  { 1080, "1080 - Beaver Room Lobby.IMC" },
  { 1081, "1081 - Beaver Dam.IMC" },
  { 1082, "1083 - Beaver Room.IMC" },
  { 1083, "1083 - Beaver Room.IMC" },
  { 1084, "1084 - Foggy Cactus.IMC" },
  { 1085, "1085 - Rubamat Exterior.IMC" },
  { 1086, "1086 - Blue Hector.IMC" },
  { 1100, "1109 - Cafe Exterior.IMC" },
  { 1101, "1101 - Cafe Office.IMC" },
  { 1102, "1102 - Cafe Intercom.IMC" },
  { 1103, "1103 - Coat Check.IMC" },
  { 1104, "1104 - Lupe.IMC" },
  { 1105, "1106 - Glottis Noodle.IMC" },
  { 1106, "1106 - Glottis Noodle.IMC" },
  { 1107, "1101 - Cafe Office.IMC" },
  { 1108, "1108 - Casino Interior.IMC" },
  { 1109, "1109 - Cafe Exterior.IMC" },
  { 1110, "1110 - Cafe Ledge.IMC" },
  { 1111, "1108 - Casino Interior.IMC" },
  { 1112, "1112 - Rusty Sans Vox.IMC" },
  { 1120, "1120 - Elevator Station.IMC" },
  { 1121, "1122 - Blue Exterior.IMC" },
  { 1122, "1122 - Blue Exterior.IMC" },
  { 1123, "1123 - Blue Casket Ins.IMC" },
  { 1124, "1124 - Blue Casket Amb.IMC" },
  { 1125, "1125 - Smooth Hector.IMC" },
  { 1126, "1122 - Blue Exterior.IMC" },
  { 1127, "1127 - Limbo Dock.IMC" },
  { 1128, "1128 - Limbo Talk.IMC" },
  { 1129, "1129 - Limbo Poem.IMC" },
  { 1130, "1130 - Dry Dock.IMC" },
  { 1131, "1131 - Dry Dock Strike.IMC" },
  { 1132, "1132 - Lighthouse Ext.IMC" },
  { 1133, "1133 - Lola's Last.IMC" },
  { 1140, "1140 - Police Station.IMC" },
  { 1141, "1141 - Police Interior.IMC" },
  { 1142, "1141 - Police Interior.IMC" },
  { 1143, "1143 - Morgue.IMC" },
  { 1144, "1140 - Police Station.IMC" },
  { 1145, "1145 - Bridge Blimp.IMC" },
  { 1146, "1146 - LOL Security Ext.IMC" },
  { 1147, "1147 - LOL Security Int.IMC" },
  { 1148, "1148 - Carla's Life.IMC" },
  { 1149, "1149 - Bomb.IMC" },
  { 1150, "1150 - Track Stairs.IMC" },
  { 1151, "1151 - Track Stairs.IMC" },
  { 1152, "1152 - Track Stairs.IMC" },
  { 1153, "1153 - Track Base.IMC" },
  { 1154, "1154 - Kitty Hall.IMC" },
  { 1155, "1155 - Sanspoof.IMC" },
  { 1156, "1156 - Kitty Stables.IMC" },
  { 1160, "1160 - High Roller Hall.IMC" },
  { 1161, "1161 - High Roller Lnge.IMC" },
  { 1162, "1162 - Glottis Gambling.IMC" },
  { 1163, "1163 - Max's Office.IMC" },
  { 1164, "1125 - Hector Steps Out.IMC" },
  { 1165, "1125 - Hector Steps Out.IMC" },
  { 1166, "1125 - Hector Steps Out.IMC" },
  { 1167, "1167 - Dillopede Elev.IMC" },
  { 1168, "1168 - Dillopede Elev.IMC" },
  { 1169, "1169 - Dillopede Elev.IMC" },
  { 1170, "1170 - Extendo Bridge.IMC" },
  { 1171, "1170 - Extendo Bridge.IMC" },
  { 1172, "1170 - Extendo Bridge.IMC" },
  { 1173, "1173 - Scrimshaw Int.IMC" },
  { 1174, "1174 - Scrim Sleep.IMC" },
  { 1180, "1180 - Note to Manny.IMC" },
  { 1181, "1155 - Sanspoof.IMC" },
  { 1190, "1106 - Glottis Noodle.IMC" },
  { 1191, "1106 - Glottis Noodle.IMC" },
  { 1201, "1201 - Lola Zapata.IMC" },
  { 1202, "1202 - Inside the Lola.IMC" },
  { 1203, "1203 - Engine Room.IMC" },
  { 1204, "1204 - Porthole.IMC" },
  { 1205, "1204 - Porthole.IMC" },
  { 1210, "1210 - Sunken Lola.IMC" },
  { 1211, "1211 - Pearl Crater Sub.IMC" },
  { 1220, "1220 - Miner's Room.IMC" },
  { 1221, "1221 - Miner's Room.IMC" },
  { 1222, "1222 - Exterior Airlock.IMC" },
  { 1223, "1223 - Factory Hub.IMC" },
  { 1224, "1224 - Foreman's Office.IMC" },
  { 1230, "1230 - Vault Door.IMC" },
  { 1231, "1231 - Outer Vault.IMC" },
  { 1232, "1232 - Inner Vault.IMC" },
  { 1233, "1233 - Ashtray Room.IMC" },
  { 1234, "1234 - Ashtray Scary.IMC" },
  { 1235, "1235 - Ashtray Pensive.IMC" },
  { 1236, "1236 - Domino's Room.IMC" },
  { 1240, "1240 - Conveyor Under.IMC" },
  { 1241, "1240 - Conveyor Under.IMC" },
  { 1242, "1241 - Crane Intro.IMC" },
  { 1243, "1243 - Anchor Room.IMC" },
  { 1244, "1244 - Glottis Hanging.IMC" },
  { 1245, "1245 - End of the World.IMC" },
  { 1246, "1246 - End World Later.IMC" },
  { 1247, "1241 - Crane Intro.IMC" },
  { 1250, "1250 - Upper Beach.IMC" },
  { 1251, "1250 - Upper Beach.IMC" },
  { 1252, "1252 - Lower Beach Boat.IMC" },
  { 1253, "1253 - Lamancha Sub.IMC" },
  { 1254, "1254 - Crane Later.IMC" },
  { 1301, "1301 - Temple Gate.IMC" },
  { 1302, "1301 - Temple Gate.IMC" },
  { 1303, "1303 - Truck Depot.IMC" },
  { 1304, "1304 - Mayan Train Sta.IMC" },
  { 1305, "1305 - Mayan Workshop.IMC" },
  { 1306, "1306 - Mayan Train Pad.IMC" },
  { 1307, "1307 - Mechanic's Kitch.IMC" },
  { 1310, "1310 - Jello Bomb.IMC" },
  { 1311, "1310 - Jello Bomb.IMC" },
  { 1312, "1125 - Smooth Hector.IMC" },
  { 1313, "1125 - Smooth Hector.IMC" },
  { 1314, "1125 - Smooth Hector.IMC" },
  { 1315, "1122 - Blue Exterior.IMC" },
  { 1316, "1122 - Blue Exterior.IMC" },
  { 1317, "1122 - Blue Exterior.IMC" },
  { 1318, "1332 - Hector's Foyer.IMC" },
  { 1319, "1319 - Florist Video.IMC" },
  { 1320, "1320 - New LSA HQ.IMC" },
  { 1321, "1321 - LSA Sewer.IMC" },
  { 1322, "1321 - LSA Sewer.IMC" },
  { 1323, "1323 - Sewer Maze.IMC" },
  { 1324, "1324 - Albinozod.IMC" },
  { 1325, "1325 - Florist Shop.IMC" },
  { 1326, "1326 - Florist Shop Int.IMC" },
  { 1327, "1327 - Florist OK.IMC" },
  { 1328, "1323 - Sewer Maze.IMC" },
  { 1329, "1329 - Theater Backstag.IMC" },
  { 1330, "1330 - Lemans Lobby.IMC" },
  { 1331, "1330 - Lemans Lobby.IMC" },
  { 1332, "1332 - Hector's Foyer.IMC" },
  { 1333, "1333 - Brennis Talk.IMC" },
  { 1334, "1334 - Albino Trap.IMC" },
  { 1340, "1342 - Neon Ledge.IMC" },
  { 1350, "1350 - Meadow Flowers.IMC" },
  { 1351, "1351 - Meadow.IMC" },
  { 1352, "1352 - Car Head.IMC" },
  { 1353, "1353 - Greenhouse Appr.IMC" },
  { 1354, "1354 - Game Ending.IMC" },
  { 1355, "1355 - Shootout.IMC" },
  { 1400, "1400 - Start Credits.IMC" },
  { 1401, "1401 - Smooth Hector.IMC" },
  { 2001, "2001 - Climb Rope.IMC" },
  { 2010, "2010 - Glottis OK.IMC" },
  { 2020, "2020 - Reap Bruno.IMC" },
  { 2030, "2030 - Ledgepeckers.IMC" },
  { 2050, "2050 - Glottis Heart.IMC" },
  { 2055, "2055 - Slingshot Bone.IMC" },
  { 2060, "2060 - Glott Tree Fall.IMC" },
  { 2070, "2070 - Beaver Fly.IMC" },
  { 2071, "2071 - Beaver Sink.IMC" },
  { 2080, "2080 - Meet Velasco.IMC" },
  { 2140, "2140 - Ooo Bonewagon.IMC" },
  { 2141, "2141 - Ooo Meche.IMC" },
  { 2155, "2155 - Find Detector.IMC" },
  { 2156, "2156 - Glott Drink Wine.IMC" },
  { 2157, "2157 - Glott No Wine.IMC" },
  { 2161, "2161 - Raoul Appears.IMC" },
  { 2162, "2162 - Raoul KO.IMC" },
  { 2163, "2163 - Raoul Dissed.IMC" },
  { 2165, "2165 - Fake Tix.IMC" },
  { 2180, "2180 - Befriend Commies.IMC" },
  { 2186, "2186 - Nick Punchout.IMC" },
  { 2200, "2200 - Year 3 Iris.IMC" },
  { 2210, "2210 - Hit Men.IMC" },
  { 2230, "2230 - Open Vault.IMC" },
  { 2235, "2235 - Dead Tix.IMC" },
  { 2240, "2240 - Sprinkler.IMC" },
  { 2250, "2250 - Crane Track.IMC" },
  { 2255, "2255 - Crane Fall.IMC" },
  { 2300, "2300 - Yr 4 Iris.IMC" },
  { 2301, "2301 - Pop Bruno Casket.IMC" },
  { 2310, "2310 - Rocket Idea.IMC" },
  { 2320, "2320 - Jello Suspense.IMC" },
  { 2325, "2325 - Lumbago Lemo.IMC" },
  { 2327, "2327 - Breath Mint.IMC" },
  { 2330, "2330 - Pigeon Fly.IMC" },
  { 2340, "2340 - Coffee On Boys.IMC" },
  { 2350, "2350 - Sprout Aha.IMC" },
  { 2360, "2360 - Chowchilla Bye.IMC" },
  { 2370, "2370 - Salvador Death.IMC" },
  { 2399, "2399 - End Credits.IMC" }
};

Mixer *Mixer::instance_ = NULL;

Mixer *Mixer::instance() {
  if (instance_ == NULL)
    instance_ = new Mixer;
  return instance_;
}

void mixerCallback(void *userdata, Uint8 *stream, int len) {
  Mixer *m = static_cast<Mixer *>(userdata);
  int16 *samples = reinterpret_cast<int16 *>(stream);
  m->getAudio(samples, len / 2);
}

Mixer::Mixer() :
  musicSound_(NULL), seqSound_(NULL)
{
}

void Mixer::start() {
  Sound::init();

  SDL_AudioSpec desired;
  memset(&desired, 0, sizeof(desired)); // for valgrind cleanness
  desired.freq = 22050;
  desired.format = AUDIO_S16SYS;
  desired.channels = 2;
  desired.samples = 2048;
  desired.callback = mixerCallback;
  desired.userdata = this;
  SDL_OpenAudio(&desired, NULL);
  SDL_PauseAudio(0);
}

void Mixer::playVoice(Sound *s) {
  AudioLock l;

  s->reset();
  voiceSounds_.push_back(s);
}

void Mixer::playSfx(Sound *s) {
  AudioLock l;

  s->reset();
  sfxSounds_.push_back(s);
}

void Mixer::stopSfx(Sound *s) {
  AudioLock l;

  for (sound_list::iterator i = sfxSounds_.begin();
       i != sfxSounds_.end(); ) {
    if (*i == s)
      i = sfxSounds_.erase(i);
    else
      i++;
  }
}

void Mixer::stopVoice(Sound *s) {
  AudioLock l;

  for (sound_list::iterator i = voiceSounds_.begin();
       i != voiceSounds_.end(); ) {
    if (*i == s)
      i = voiceSounds_.erase(i);
    else
      i++;
  }
}

static int compareStates(const void *p1, const void *p2) {
  const imuseTableEntry *e1 = static_cast<const imuseTableEntry *>(p1);
  const imuseTableEntry *e2 = static_cast<const imuseTableEntry *>(p2);
  return e1->stateNum - e2->stateNum;
}

void Mixer::setImuseState(int state) {
  Sound *newSound = NULL;

  if (state != 1000) {
    imuseTableEntry key;
    key.stateNum = state;
    const imuseTableEntry *e = static_cast<imuseTableEntry *>
      (std::bsearch(&key, grimMusicTable,
		    sizeof(grimMusicTable) / sizeof(grimMusicTable[0]),
		    sizeof(grimMusicTable[0]), compareStates));
    if (e == NULL) {
      warning("Unknown IMuse state %d\n", state);
      return;
    }

    newSound = ResourceLoader::instance()->loadSound(e->filename);
    if (newSound == NULL) {
      warning("Could not find music file %s\n", e->filename);
      return;
    }
  }

  AudioLock l;
  if (newSound != musicSound_) {
    if (newSound != NULL)
      newSound->reset();
    musicSound_ = newSound;
  }
}

void Mixer::setImuseSeq(int state) {
  Sound *newSound = NULL;

  if (state != 2000) {
    imuseTableEntry key;
    key.stateNum = state;
    const imuseTableEntry *e = static_cast<imuseTableEntry *>
      (std::bsearch(&key, grimMusicTable,
		    sizeof(grimMusicTable) / sizeof(grimMusicTable[0]),
		    sizeof(grimMusicTable[0]), compareStates));
    if (e == NULL) {
      warning("Unknown IMuse state %d\n", state);
      return;
    }

    Sound *newSound = ResourceLoader::instance()->loadSound(e->filename);
    if (newSound == NULL) {
      warning("Could not find music file %s\n", e->filename);
      return;
    }
  }

  AudioLock l;
  if (newSound != seqSound_) {
    if (newSound != NULL)
      newSound->reset();
    seqSound_ = newSound;
  }
}

Sound *Mixer::findSfx(const char *filename) {
  AudioLock l;

  for (sound_list::iterator i = sfxSounds_.begin();
       i != sfxSounds_.end(); i++) {
    if (std::strcmp((*i)->filename(), filename) == 0)
      return *i;
  }
  return NULL;
}

bool Mixer::voicePlaying() const {
  AudioLock l;

  return ! voiceSounds_.empty();
}

void Mixer::getAudio(int16 *data, int numSamples) {
  memset(data, 0, numSamples * 2);
  for (sound_list::iterator i = voiceSounds_.begin();
       i != voiceSounds_.end(); ) {
    (*i)->mix(data, numSamples);
    if ((*i)->done())
      i = voiceSounds_.erase(i);
    else
      i++;
  }
  for (sound_list::iterator i = sfxSounds_.begin();
       i != sfxSounds_.end(); ) {
    (*i)->mix(data, numSamples);
    if ((*i)->done())
      i = sfxSounds_.erase(i);
    else
      i++;
  }
  if (seqSound_ != NULL) {
    seqSound_->mix(data, numSamples);
    if (seqSound_->done())
      seqSound_ = NULL;
  }
  else if (musicSound_ != NULL) {
    musicSound_->mix(data, numSamples);
    if (musicSound_->done())
      musicSound_->reset();
  }
}
