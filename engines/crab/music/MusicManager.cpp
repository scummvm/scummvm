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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "common/config-manager.h"
#include "crab/crab.h"
#include "crab/GameParam.h"
#include "crab/ScreenSettings.h"
#include "crab/XMLDoc.h"
#include "crab/music/MusicManager.h"

namespace Crab {

using namespace pyrodactyl::music;

//------------------------------------------------------------------------
// Purpose: Clear stored data
//------------------------------------------------------------------------
void MusicManager::freeMusic() {
	delete _musicHandle;
}

void MusicManager::freeChunk() {
	for (auto &i : _effects) {
		i._value->_file.close();
		delete i._value->_handle;
		delete i._value->_stream;
		delete i._value;
	}

	_effects.clear();
}

//------------------------------------------------------------------------
// Purpose: Play or queue music
//------------------------------------------------------------------------
void MusicManager::playMusic(const MusicKey &id) {
	if (_bg._id != id) {
		XMLDoc trackList(g_engine->_filePath->_soundMusic);
		if (trackList.ready()) {
			rapidxml::xml_node<char> *node = trackList.doc()->first_node("music");
			for (auto n = node->first_node(); n != nullptr; n = n->next_sibling()) {
				rapidxml::xml_attribute<char> *att = n->first_attribute("id");
				if (att != nullptr && id == stringToNumber<MusicKey>(att->value())) {
					if (g_system->getMixer()->isSoundHandleActive(*_musicHandle))
						g_system->getMixer()->stopHandle(*_musicHandle);
					_bg.reset();
					_bg.load(n);
					break;
				}
			}
		}

		if (_bg._track != nullptr)
			g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, _musicHandle, _bg._track, (int)_bg._id);
	}
}

//------------------------------------------------------------------------
// Purpose: Play or queue sound effects
//------------------------------------------------------------------------
void MusicManager::playEffect(const ChunkKey &id, const int &loops) {
	// I am not sure if the game uses a value of more than 0 anywhere.
	// For now error out in case loops > 0.
	assert(loops == 0);

	if (_effects.contains(id)) {
		EffectAudio *audio = _effects[id];
		audio->_stream->rewind();
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, audio->_handle, audio->_stream, id, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}
}

//------------------------------------------------------------------------
// Purpose: Initialize the music subsystem (currently SDL_mixer) and load sound effects
//------------------------------------------------------------------------
bool MusicManager::load(rapidxml::xml_node<char> *node) {
	_musicHandle = new Audio::SoundHandle();

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	int volumeEff = mute ? 0 : ConfMan.getInt("sfx_volume");
	int volumeMus = mute ? 0 : ConfMan.getInt("music_volume");

	if (nodeValid("sound", node)) {
		rapidxml::xml_node<char> *volnode = node->first_node("sound");
		/* Originally the game loads the music and sound effect at the default value of 100 from the settings file.
			However, we get these values from the ConfMan, so skip reading these values from settings
		*/

		//loadNum(_volumeMus, "music", volnode);
		//loadNum(_volumeEff, "effects", volnode);

		loadNum(_freq, "frequency", volnode);
		loadNum(_channels, "channels", volnode);
		loadNum(_chunksize, "chunk_size", volnode);
	}

	// Set the volume from the settings
	volEffects(volumeEff);
	volMusic(volumeMus);

	// Load sound effects
	XMLDoc trackList(g_engine->_filePath->_soundEffect);
	if (trackList.ready()) {
		rapidxml::xml_node<char> *tnode = trackList.doc()->first_node("effects");
		if (nodeValid(tnode)) {
			loadNum(_notify, "notify", tnode);
			loadNum(_repInc, "rep_inc", tnode);
			loadNum(_repDec, "rep_dec", tnode);

			for (auto n = tnode->first_node(); n != nullptr; n = n->next_sibling()) {
				rapidxml::xml_attribute<char> *id = n->first_attribute("id"), *path = n->first_attribute("path");
				if (id != nullptr && path != nullptr) {
					EffectAudio *audio = new EffectAudio();
					audio->_file.open(path->value());
					audio->_handle = new Audio::SoundHandle();
					audio->_stream = Audio::makeWAVStream(&audio->_file, DisposeAfterUse::NO);
					_effects[stringToNumber<ChunkKey>(id->value())] = audio;
				}
			}
		}
	}

	return true;
}

// Function is not needed as of now, keeping it incase its needed in future.
void MusicManager::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
#if 0
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "sound");
	child->append_attribute(doc.allocate_attribute("music", g_engine->_stringPool->Get(Mix_VolumeMusic(-1))));
	child->append_attribute(doc.allocate_attribute("effects", g_engine->_stringPool->Get(Mix_Volume(0, -1))));
	child->append_attribute(doc.allocate_attribute("frequency", g_engine->_stringPool->Get(freq)));
	child->append_attribute(doc.allocate_attribute("channels", g_engine->_stringPool->Get(channels)));
	child->append_attribute(doc.allocate_attribute("chunk_size", g_engine->_stringPool->Get(chunksize)));
	root->append_node(child);
#endif
}

} // End of namespace Crab
