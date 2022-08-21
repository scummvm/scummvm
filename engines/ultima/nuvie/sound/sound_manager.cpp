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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/sound/adplug/emu_opl.h"
#include "ultima/nuvie/sound/song_adplug.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/sound/adplug/emu_opl.h"
#include "ultima/nuvie/sound/adlib_sfx_manager.h"
#include "ultima/nuvie/sound/pc_speaker_sfx_manager.h"
#include "ultima/nuvie/sound/towns_sfx_manager.h"
#include "ultima/nuvie/sound/custom_sfx_manager.h"
#include "audio/mixer.h"
#include "common/algorithm.h"

namespace Ultima {
namespace Nuvie {

struct ObjSfxLookup { // obj sfx lookup
	uint16 obj_n;
	SfxIdType sfx_id;
};

#define SOUNDMANANGER_OBJSFX_TBL_SIZE 5

static const ObjSfxLookup u6_obj_lookup_tbl[] = {
	{OBJ_U6_FOUNTAIN, NUVIE_SFX_FOUNTAIN},
	{OBJ_U6_FIREPLACE, NUVIE_SFX_FIRE},
	{OBJ_U6_CLOCK, NUVIE_SFX_CLOCK},
	{OBJ_U6_PROTECTION_FIELD, NUVIE_SFX_PROTECTION_FIELD},
	{OBJ_U6_WATER_WHEEL, NUVIE_SFX_WATER_WHEEL}
};



bool SoundManager::g_MusicFinished;

void musicFinished() {
	SoundManager::g_MusicFinished = true;
}

SoundManager::SoundManager(Audio::Mixer *mixer) : _mixer(mixer) {
	m_pCurrentSong = NULL;
	m_CurrentGroup = "";
	g_MusicFinished = true;

	audio_enabled = false;
	music_enabled = false;
	sfx_enabled = false;

	m_Config = NULL;
	m_SfxManager = NULL;

	opl = NULL;
}

SoundManager::~SoundManager() {
	// Stop all mixing
	_mixer->stopAll();

	//thanks to wjp for this one
	while (!m_Songs.empty()) {
		delete *(m_Songs.begin());
		m_Songs.erase(m_Songs.begin());
	}
	while (!m_Samples.empty()) {
		delete *(m_Samples.begin());
		m_Samples.erase(m_Samples.begin());
	}

	delete opl;

	for (IntCollectionMap::iterator it = m_ObjectSampleMap.begin(); it != m_ObjectSampleMap.end(); ++it)
		delete it->_value;
	for (IntCollectionMap::iterator it = m_TileSampleMap.begin(); it != m_TileSampleMap.end(); ++it)
		delete it->_value;
	for (StringCollectionMap::iterator it = m_MusicMap.begin(); it != m_MusicMap.end(); ++it)
		delete it->_value;

	delete m_SfxManager;
}

bool SoundManager::nuvieStartup(Configuration *config) {
	Std::string config_key;
	Std::string music_style;
	Std::string music_cfg_file; //full path and filename to music.cfg
	Std::string sound_dir;
	Std::string sfx_style;
	bool val;

	m_Config = config;

	m_Config->value("config/mute", val, false);
	audio_enabled = !val;
	m_Config->value("config/GameType", game_type);
	m_Config->value("config/audio/stop_music_on_group_change", stop_music_on_group_change, true);

	/*  if(audio_enabled == false) // commented out to allow toggling
	     {
	      music_enabled = false;
	      sfx_enabled = false;
	      music_volume = 0;
	      sfx_volume = 0;
	      mixer = NULL;
	      return false;
	     }*/

	m_Config->value("config/music_mute", val, false);
	music_enabled = !val;
	m_Config->value("config/sfx_mute", val, false);
	sfx_enabled = !val;

	int volume;

	m_Config->value("config/music_volume", volume, Audio::Mixer::kMaxChannelVolume);
	music_volume = clamp(volume, 0, 255);

	m_Config->value("config/sfx_volume", volume, Audio::Mixer::kMaxChannelVolume);
	sfx_volume = clamp(volume, 0, 255);

	config_key = config_get_game_key(config);
	config_key.append("/music");
	config->value(config_key, music_style, "native");

	config_key = config_get_game_key(config);
	config_key.append("/sfx");
	config->value(config_key, sfx_style, "native");

	config_key = config_get_game_key(config);
	config_key.append("/sounddir");
	config->value(config_key, sound_dir, "");

	if (game_type == NUVIE_GAME_U6) { // FM-Towns speech
		config->value("config/speech_mute", val, false);
		speech_enabled = !val;
	} else {
		speech_enabled = false;
	}

	if (!initAudio()) {
		return false;
	}

//  if(music_enabled)  // commented out to allow toggling
	{
		if (music_style == "native") {
			if (game_type == NUVIE_GAME_U6)
				LoadNativeU6Songs(); //FIX need to handle MD & SE music too.
		} else if (music_style == "custom")
			LoadCustomSongs(sound_dir);
		else
			DEBUG(0, LEVEL_WARNING, "Unknown music style '%s'\n", music_style.c_str());

		musicPlayFrom("random");
	}

//  if(sfx_enabled)  // commented out to allow toggling
	{
		//LoadObjectSamples(sound_dir);
		//LoadTileSamples(sound_dir);
		LoadSfxManager(sfx_style);
	}

	return true;
}

bool SoundManager::initAudio() {
	opl = new CEmuopl(_mixer->getOutputRate(), true, true);
	return true;
}

bool SoundManager::LoadNativeU6Songs() {
	Song *song;

	string filename;

	config_get_path(m_Config, "brit.m", filename);
	song = new SongAdPlug(_mixer, opl);
// loadSong(song, filename.c_str());
	loadSong(song, filename.c_str(), "Rule Britannia");
	groupAddSong("random", song);

	config_get_path(m_Config, "forest.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Wanderer (Forest)");
	groupAddSong("random", song);

	config_get_path(m_Config, "stones.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Stones");
	groupAddSong("random", song);

	config_get_path(m_Config, "ultima.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Ultima VI Theme");
	groupAddSong("random", song);

	config_get_path(m_Config, "engage.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Engagement and Melee");
	groupAddSong("combat", song);

	config_get_path(m_Config, "hornpipe.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Captain Johne's Hornpipe");
	groupAddSong("boat", song);

	config_get_path(m_Config, "gargoyle.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Audchar Gargl Zenmur");
	groupAddSong("gargoyle", song);

	config_get_path(m_Config, "dungeon.m", filename);
	song = new SongAdPlug(_mixer, opl);
	loadSong(song, filename.c_str(), "Dungeon");
	groupAddSong("dungeon", song);

	return true;
}

bool SoundManager::LoadCustomSongs(string sound_dir) {
	char seps[] = ";\r\n";
	char *token1;
	char *token2;
	char *sz;
	NuvieIOFileRead niof;
	Song *song;
	Std::string scriptname;
	Std::string filename;

	build_path(sound_dir, "music.cfg", scriptname);

	if (niof.open(scriptname) == false)
		return false;

	sz = (char *)niof.readAll();

	if (sz == NULL)
		return false;

	token1 = strtok(sz, seps);
	for (; (token1 != NULL) && ((token2 = strtok(NULL, seps)) != NULL) ; token1 = strtok(NULL, seps)) {
		build_path(sound_dir, token2, filename);

		song = (Song *)SongExists(token2);
		if (song == NULL) {
			song = new Song;
			if (!loadSong(song, filename.c_str()))
				continue; //error loading song
		}

		if (groupAddSong(token1, song))
			DEBUG(0, LEVEL_DEBUGGING, "%s : %s\n", token1, token2);
	}

	free(sz);

	return true;
}

bool SoundManager::loadSong(Song *song, const char *filename) {
	if (song->Init(filename)) {
		m_Songs.push_back(song);       //add it to our global list
		return true;
	} else {
		DEBUG(0, LEVEL_ERROR, "could not load %s\n", filename);
	}

	return false;
}

// (SB-X)
bool SoundManager::loadSong(Song *song, const char *filename, const char *title) {
	if (loadSong(song, filename) == true) {
		song->SetName(title);
		return true;
	}
	return false;
}

bool SoundManager::groupAddSong(const char *group, Song *song) {
	if (song != NULL) {
		//we have a valid song
		SoundCollection *psc;
		Common::HashMap <Common::String, SoundCollection * >::iterator it;
		it = m_MusicMap.find(group);
		if (it == m_MusicMap.end()) {
			//is there already a collection for this entry?
			psc = new SoundCollection();	// no, create a new sound collection
			psc->m_Sounds.push_back(song);	// add this sound to the collection
			m_MusicMap[group] = psc;		// insert this pair into the map
		} else {
			psc = (*it)._value;				// yes, get the existing
			psc->m_Sounds.push_back(song);	// add this sound to the collection
		}
	}

	return true;
}

/*
bool SoundManager::LoadObjectSamples (string sound_dir)
{
  char seps[] = ";\r\n";
  char *token1;
  char *token2;
  NuvieIOFileRead niof;
  char *sz;
  string samplename;
  string scriptname;

  build_path(sound_dir, "obj_samples.cfg", scriptname);

  if(niof.open (scriptname) == false)
	return false;

  sz = (char *) niof.readAll ();

  token1 = strtok (sz, seps);

  while ((token1 != NULL) && ((token2 = strtok (NULL, seps)) != NULL))
	{
	  int id = atoi (token1);
	  DEBUG(0,LEVEL_DEBUGGING,"%d : %s\n", id, token2);
	  Sound *ps;
	  ps = SampleExists (token2);
	  if (ps == NULL)
		{
		  Sample *s;
		  s = new Sample;
		  build_path(sound_dir, token2, samplename);
		  if (!s->Init (samplename.c_str ()))
			{
			  DEBUG(0,LEVEL_ERROR,"could not load %s\n", samplename.c_str ());
			}
		  ps = s;
		  m_Samples.push_back (ps);     //add it to our global list
		}
	  if (ps != NULL)
		{                       //we have a valid sound
		  SoundCollection *psc;
		  Common::HashMap < int, SoundCollection * >::iterator it;
		  it = m_ObjectSampleMap.find (id);
		  if (it == m_ObjectSampleMap.end ())
			{                   //is there already a collection for this entry?
			  psc = new SoundCollection;        //no, create a new sound collection
			  psc->m_Sounds.push_back (ps);     //add this sound to the collection
			  m_ObjectSampleMap.insert (Std::make_pair (id, psc));      //insert this pair into the map
			}
		  else
			{
			  psc = (*it).second;       //yes, get the existing
			  psc->m_Sounds.push_back (ps);     //add this sound to the collection
			}
		}
	  token1 = strtok (NULL, seps);
	}
  return true;
};

bool SoundManager::LoadTileSamples (string sound_dir)
{
  char seps[] = ";\r\n";
  char *token1;
  char *token2;
  NuvieIOFileRead niof;
  char *sz;
  string samplename;
  string scriptname;

  build_path(sound_dir, "tile_samples.cfg", scriptname);

  if(niof.open (scriptname) == false)
	{
	 DEBUG(0,LEVEL_ERROR,"opening %s\n",scriptname.c_str());
	 return false;
	}

  sz = (char *) niof.readAll ();

  token1 = strtok (sz, seps);

  while ((token1 != NULL) && ((token2 = strtok (NULL, seps)) != NULL))
	{
	  int id = atoi (token1);
	  DEBUG(0,LEVEL_DEBUGGING,"%d : %s\n", id, token2);
	  Sound *ps;
	  ps = SampleExists (token2);
	  if (ps == NULL)
		{
		  Sample *s;
		  s = new Sample;
		  build_path(sound_dir, token2, samplename);
		  if (!s->Init (samplename.c_str ()))
			{
			  DEBUG(0,LEVEL_ERROR,"could not load %s\n", samplename.c_str ());
			}
		  ps = s;
		  m_Samples.push_back (ps);     //add it to our global list
		}
	  if (ps != NULL)
		{                       //we have a valid sound
		  SoundCollection *psc;
		  Common::HashMap < int, SoundCollection * >::iterator it;
		  it = m_TileSampleMap.find (id);
		  if (it == m_TileSampleMap.end ())
			{                   //is there already a collection for this entry?
			  psc = new SoundCollection;        //no, create a new sound collection
			  psc->m_Sounds.push_back (ps);     //add this sound to the collection
			  m_TileSampleMap.insert (Std::make_pair (id, psc));        //insert this pair into the map
			}
		  else
			{
			  psc = (*it).second;       //yes, get the existing
			  psc->m_Sounds.push_back (ps);     //add this sound to the collection
			}
		}
	  token1 = strtok (NULL, seps);
	}
  return true;
};
*/
bool SoundManager::LoadSfxManager(string sfx_style) {
	if (m_SfxManager != NULL) {
		return false;
	}

	if (sfx_style == "native") {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			if (has_fmtowns_support(m_Config)) {
				sfx_style = "towns";
			} else {
				sfx_style = "pcspeaker";
			}
			break;
		case NUVIE_GAME_MD :
		case NUVIE_GAME_SE :
			sfx_style = "adlib";
			break;
		}
	}

	if (sfx_style == "pcspeaker") {
		m_SfxManager = new PCSpeakerSfxManager(m_Config, _mixer);
	}
	if (sfx_style == "adlib") {
		m_SfxManager = new AdLibSfxManager(m_Config, _mixer);
	} else if (sfx_style == "towns") {
		m_SfxManager = new TownsSfxManager(m_Config, _mixer);
	} else if (sfx_style == "custom") {
		m_SfxManager = new CustomSfxManager(m_Config, _mixer);
	}
//FIXME what to do if unknown sfx_style is entered in config file.
	return true;
}

void SoundManager::musicPlayFrom(string group) {
	if (!music_enabled || !audio_enabled)
		return;
	if (m_CurrentGroup != group) {
		if (stop_music_on_group_change)
			g_MusicFinished = true;
		m_CurrentGroup = group;
	}
}

void SoundManager::musicPause() {
//Mix_PauseMusic();
	if (m_pCurrentSong != NULL) {
		m_pCurrentSong->Stop();
	}
}

/* don't call if audio or music is disabled */
void SoundManager::musicPlay() {
// Mix_ResumeMusic();

// (SB-X) Get a new song if stopped.
	if (m_pCurrentSong == NULL)
		m_pCurrentSong = RequestSong(m_CurrentGroup);

	if (m_pCurrentSong != NULL) {
		m_pCurrentSong->Play();
		m_pCurrentSong->SetVolume(music_volume);
	}

}

void SoundManager::musicPlay(const char *filename, uint16 song_num) {
	string path;

	if (!music_enabled || !audio_enabled)
		return;

	config_get_path(m_Config, filename, path);
	SongAdPlug *song = new SongAdPlug(_mixer, opl);
	song->Init(path.c_str(), song_num);

	musicStop();
	m_pCurrentSong = song;
	m_CurrentGroup = "";
	musicPlay();
}

// (SB-X) Stop the current song so a new song will play when resumed.
void SoundManager::musicStop() {
	musicPause();
	m_pCurrentSong = NULL;
}

Std::list < SoundManagerSfx >::iterator SoundManagerSfx_find(Std::list < SoundManagerSfx >::iterator first, Std::list < SoundManagerSfx >::iterator last, const SfxIdType &value) {
	for (; first != last; first++) {
		if ((*first).sfx_id == value)
			break;
	}
	return first;
}

void SoundManager::update_map_sfx() {
	unsigned int i;
	uint16 x, y;
	uint8 l;

	if (sfx_enabled == false)
		return;

	string next_group = "";
	Player *p = Game::get_game()->get_player();
	MapWindow *mw = Game::get_game()->get_map_window();

	vector < SfxIdType >currentlyActiveSounds;
	Common::HashMap < SfxIdType, float >volumeLevels;

	p->get_location(&x, &y, &l);

	//m_ViewableTiles

	//get a list of all the sounds
	for (i = 0; i < mw->m_ViewableObjects.size(); i++) {
		//DEBUG(0,LEVEL_DEBUGGING,"%d %s",mw->m_ViewableObjects[i]->obj_n,Game::get_game()->get_obj_manager()->get_obj_name(mw->m_ViewableObjects[i]));
		SfxIdType sfx_id = RequestObjectSfxId(mw->m_ViewableObjects[i]->obj_n); //does this object have an associated sound?
		if (sfx_id != NUVIE_SFX_NONE) {
			//calculate the volume
			uint16 ox = mw->m_ViewableObjects[i]->x;
			uint16 oy = mw->m_ViewableObjects[i]->y;
			float dist = sqrtf((float)(x - ox) * (x - ox) + (float)(y - oy) * (y - oy));
			float vol = (8.0f - dist) / 8.0f;
			if (vol < 0)
				vol = 0;
			//sp->SetVolume(vol);
			//need the map to adjust volume according to number of active elements
			Common::HashMap < SfxIdType, float >::iterator it;
			it = volumeLevels.find(sfx_id);
			if (it != volumeLevels.end()) {
				if (volumeLevels[sfx_id] < vol)
					volumeLevels[sfx_id] = vol;
			} else {
				volumeLevels[sfx_id] = vol;
			}
			//add to currently active list
			currentlyActiveSounds.push_back(sfx_id);
		}
	}
	/*
	for (i = 0; i < mw->m_ViewableTiles.size(); i++)
	  {
	    Sound *sp = RequestTileSound (mw->m_ViewableTiles[i].t->tile_num);        //does this object have an associated sound?
	    if (sp != NULL)
	      {
	        //calculate the volume
	        short ox = mw->m_ViewableTiles[i].x - 5;
	        short oy = mw->m_ViewableTiles[i].y - 5;
	//                      DEBUG(0,LEVEL_DEBUGGING,"%d %d\n",ox,oy);
	        float dist = sqrtf ((float) (ox) * (ox) + (float) (oy) * (oy));
	//                      DEBUG(0,LEVEL_DEBUGGING,"%s %f\n",sp->GetName().c_str(),dist);
	        float vol = (7.0f - (dist - 1)) / 7.0f;
	        if (vol < 0)
	          vol = 0;
	        //sp->SetVolume(vol);
	        //need the map to adjust volume according to number of active elements
	        Common::HashMap < Sound *, float >::iterator it;
	        it = volumeLevels.find (sp);
	        if (it != volumeLevels.end ())
	          {
	            float old = volumeLevels[sp];
	//                              DEBUG(0,LEVEL_DEBUGGING,"old:%f new:%f\n",old,vol);
	            if (old < vol)
	              {
	                volumeLevels[sp] = vol;
	              }
	          }
	        else
	          {
	            volumeLevels.insert (Std::make_pair (sp, vol));
	          }
	        //add to currently active list
	        currentlyActiveSounds.push_back (sp);
	      }
	  }
	  */
	//DEBUG(1,LEVEL_DEBUGGING,"\n");
	//is this sound new? - activate it.
	for (i = 0; i < currentlyActiveSounds.size(); i++) {
		Std::list < SoundManagerSfx >::iterator it;
		it = SoundManagerSfx_find(m_ActiveSounds.begin(), m_ActiveSounds.end(), currentlyActiveSounds[i]);          //is the sound already active?
		if (it == m_ActiveSounds.end()) {
			//this is a new sound, add it to the active list
			//currentlyActiveSounds[i]->Play (true);
			//currentlyActiveSounds[i]->SetVolume (0);
			SoundManagerSfx sfx;
			sfx.sfx_id = currentlyActiveSounds[i];
			if (m_SfxManager->playSfxLooping(sfx.sfx_id, &sfx.handle, 0)) {
				m_ActiveSounds.push_back(sfx);//currentlyActiveSounds[i]);
			}
		}
	}
	//is this sound old? - deactivate it
	Std::list < SoundManagerSfx >::iterator it;
	it = m_ActiveSounds.begin();
	while (it != m_ActiveSounds.end()) {
		Std::vector<SfxIdType>::iterator fit;
		SoundManagerSfx sfx = (*it);
		fit = Common::find(currentlyActiveSounds.begin(), currentlyActiveSounds.end(), sfx.sfx_id);          //is the sound in the new active list?
		if (fit == currentlyActiveSounds.end()) {
			//its not, stop this sound from playing.
			//sfx_id->Stop ();
			_mixer->stopHandle(sfx.handle);
			it = m_ActiveSounds.erase(it);
		} else {
			_mixer->setChannelVolume(sfx.handle, (uint8)(volumeLevels[sfx.sfx_id] * (sfx_volume / 255.0f) * 255.0f));
			it++;
		}
	}
}

void SoundManager::update() {
	if (music_enabled && audio_enabled && g_MusicFinished) {
		g_MusicFinished = false;
		if (m_pCurrentSong != NULL) {
			m_pCurrentSong->Stop();
		}

		if (m_CurrentGroup.length() > 0)
			m_pCurrentSong = SoundManager::RequestSong(m_CurrentGroup);

		if (m_pCurrentSong) {
			DEBUG(0, LEVEL_INFORMATIONAL, "assigning new song! '%s'\n", m_pCurrentSong->GetName().c_str());
			if (!m_pCurrentSong->Play(false)) {
				DEBUG(0, LEVEL_ERROR, "play failed!\n");
			}
			m_pCurrentSong->SetVolume(music_volume);
		}
	}

}


Sound *SoundManager::SongExists(string name) {
	Std::list < Sound * >::iterator it;
	for (it = m_Songs.begin(); it != m_Songs.end(); ++it) {
		if ((*it)->GetName() == name)
			return *it;
	}

	return NULL;
}

Sound *SoundManager::SampleExists(string name) {
	Std::list < Sound * >::iterator it;
	for (it = m_Samples.begin(); it != m_Samples.end(); ++it) {
		if ((*it)->GetName() == name)
			return *it;
	}

	return NULL;
}

Sound *SoundManager::RequestTileSound(int id) {
	Common::HashMap < int, SoundCollection * >::iterator it;
	it = m_TileSampleMap.find(id);
	if (it != m_TileSampleMap.end()) {
		SoundCollection *psc;
		psc = (*it)._value;
		return psc->Select();
	}
	return NULL;
}

Sound *SoundManager::RequestObjectSound(int id) {
	Common::HashMap < int, SoundCollection * >::iterator it;
	it = m_ObjectSampleMap.find(id);
	if (it != m_ObjectSampleMap.end()) {
		SoundCollection *psc;
		psc = (*it)._value;
		return psc->Select();
	}
	return NULL;
}

uint16 SoundManager::RequestObjectSfxId(uint16 obj_n) {
	uint16 i = 0;
	for (i = 0; i < SOUNDMANANGER_OBJSFX_TBL_SIZE; i++) {
		if (u6_obj_lookup_tbl[i].obj_n == obj_n) {
			return u6_obj_lookup_tbl[i].sfx_id;
		}
	}

	return NUVIE_SFX_NONE;
}

Sound *SoundManager::RequestSong(string group) {
	Common::HashMap<Common::String, SoundCollection * >::iterator it;
	it = m_MusicMap.find(group);
	if (it != m_MusicMap.end()) {
		SoundCollection *psc;
		psc = (*it)._value;
		return psc->Select();
	}
	return NULL;
}

Audio::SoundHandle SoundManager::playTownsSound(Std::string filename, uint16 sample_num) {
	FMtownsDecoderStream *stream = new FMtownsDecoderStream(filename, sample_num);
	Audio::SoundHandle handle;
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, stream, -1);

	return handle;
}

bool SoundManager::isSoundPLaying(Audio::SoundHandle handle) {
	return _mixer->isSoundHandleActive(handle);
}

bool SoundManager::playSfx(uint16 sfx_id, bool async) {
	if (m_SfxManager == NULL || audio_enabled == false || sfx_enabled == false)
		return false;

	if (async) {
		if (m_SfxManager->playSfx(sfx_id, sfx_volume)) {
			uint32 duration = m_SfxManager->getLastSfxDuration();

			TimedEffect *timer = new TimedEffect();

			AsyncEffect *e = new AsyncEffect(timer);
			timer->start_timer(duration);
			e->run();

			return true;
		}
	} else {
		return m_SfxManager->playSfx(sfx_id, sfx_volume);
	}

	return false;
}

void SoundManager::set_audio_enabled(bool val) {
	audio_enabled = val;
	if (audio_enabled && music_enabled)
		musicPlay();
	else
		musicStop();
}

void SoundManager::set_music_enabled(bool val) {
	music_enabled = val;
	if (audio_enabled && music_enabled)
		musicPlay();
	else
		musicStop();
}

void SoundManager::set_speech_enabled(bool val) {
	speech_enabled = val;
	// FIXME - stop speech
}

} // End of namespace Nuvie
} // End of namespace Ultima
