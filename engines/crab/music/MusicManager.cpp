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


#include "crab/music/MusicManager.h"
#include "crab/ScreenSettings.h"
#include "crab/XMLDoc.h"

namespace Crab {

using namespace pyrodactyl::music;

//------------------------------------------------------------------------
// Purpose: Clear stored data
//------------------------------------------------------------------------
void MusicManager::freeMusic() {
	warning("STUB: MusicManager::freeMusic()");

#if 0
	Mix_FreeMusic(bg.track);
#endif

}

void MusicManager::freeChunk() {
	warning("STUB: MusicManager::freeChunk()");

#if 0
	for (auto i = effect.begin(); i != effect.end(); ++i)
		Mix_FreeChunk(i->second);
#endif

}

//------------------------------------------------------------------------
// Purpose: Play or queue music
//------------------------------------------------------------------------
void MusicManager::playMusic(const MusicKey &id) {
	warning("STUB: MusicManager::playMusic()");

#if 0
	if (bg.id != id) {
		XMLDoc track_list(g_engine->_filePath->sound_music);
		if (track_list.ready()) {
			rapidxml::xml_node<char> *node = track_list.doc()->first_node("music");
			for (auto n = node->first_node(); n != NULL; n = n->next_sibling()) {
				rapidxml::xml_attribute<char> *att = n->first_attribute("id");
				if (att != NULL && id == StringToNumber<MusicKey>(att->value())) {
					bg.load(n);
					break;
				}
			}
		}

		if (bg.track != NULL)
			Mix_FadeInMusic(bg.track, -1, bg.fade_in_duration);
	}
#endif

}

//------------------------------------------------------------------------
// Purpose: Play or queue sound effects
//------------------------------------------------------------------------
void MusicManager::playEffect(const ChunkKey &id, const int &loops) {
	warning("STUB: MusicManager::playEffect()");

#if 0
	if (effect.count(id) > 0)
		Mix_PlayChannel(-1, effect[id], loops);
#endif

}

//------------------------------------------------------------------------
// Purpose: Initialize the music subsystem (currently SDL_mixer) and load sound effects
//------------------------------------------------------------------------
bool MusicManager::load(rapidxml::xml_node<char> *node) {
	warning("STUB: MusicManager::load()");

#if 0
	// Initialize music parameters
	int volume_mus = 100, volume_eff = 100;

	if (nodeValid("sound", node)) {
		rapidxml::xml_node<char> *volnode = node->first_node("sound");
		loadNum(volume_mus, "music", volnode);
		loadNum(volume_eff, "effects", volnode);
		loadNum(freq, "frequency", volnode);
		loadNum(channels, "channels", volnode);
		loadNum(chunksize, "chunk_size", volnode);
	}

	// Start up audio
	const int audioflags = MIX_INIT_OGG;
	if ((Mix_Init(audioflags) & audioflags) != audioflags) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize audio mixer", "Please install libsdl2_mixer", NULL);
		return false;
	}

	if (Mix_OpenAudio(freq, MIX_DEFAULT_FORMAT, channels, chunksize) == -1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to open audio channels", "Please check settings", NULL);
		return false;
	}

	// Set the volume from the settings
	VolEffects(volume_eff);
	VolMusic(volume_mus);

	// Load sound effects
	XMLDoc track_list(g_engine->_filePath->sound_effect);
	if (track_list.ready()) {
		rapidxml::xml_node<char> *tnode = track_list.doc()->first_node("effects");
		if (nodeValid(tnode)) {
			loadNum(notify, "notify", tnode);
			loadNum(rep_inc, "rep_inc", tnode);
			loadNum(rep_dec, "rep_dec", tnode);

			for (auto n = tnode->first_node(); n != NULL; n = n->next_sibling()) {
				rapidxml::xml_attribute<char> *id = n->first_attribute("id"), *path = n->first_attribute("path");
				if (id != NULL && path != NULL)
					effect[StringToNumber<ChunkKey>(id->value())] = Mix_LoadWAV(path->value());
			}
		}
	}
#endif

	return true;
}

void MusicManager::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	warning("STUB: MusicManager::saveState()");

#if 0
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "sound");
	child->append_attribute(doc.allocate_attribute("music", gStrPool->Get(Mix_VolumeMusic(-1))));
	child->append_attribute(doc.allocate_attribute("effects", gStrPool->Get(Mix_Volume(0, -1))));
	child->append_attribute(doc.allocate_attribute("frequency", gStrPool->Get(freq)));
	child->append_attribute(doc.allocate_attribute("channels", gStrPool->Get(channels)));
	child->append_attribute(doc.allocate_attribute("chunk_size", gStrPool->Get(chunksize)));
	root->append_node(child);
#endif
}

} // End of namespace Crab
