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

// NOT USED AT THE MOMENT
#include "ultima/ultima6/core/nuvie_defs.h"
#include "Sample.h"

namespace Ultima {
namespace Ultima6 {

Sample::Sample() {
	m_pSample = NULL;
	m_Channel = -1;
}

Sample::~Sample() {
	if (m_pSample != NULL) {
		Mix_FreeChunk(m_pSample);
		m_pSample = NULL;
	}
};

bool Sample::SetVolume(uint8 volume) {
	/*
	int ret;
	if (m_Channel == -1) return false;
	ret=Mix_Playing(m_Channel);
	if (ret==0) {
	    m_Channel = -1;
	    return false;
	}
	ret=Mix_Volume(m_Channel,(int)(volume*128.0f));
	if (ret) return false;
	*/
	return true;
}

bool Sample::Init(const char *filename) {
	if (filename == NULL) return false;
	m_Filename = filename;
	m_pSample = Mix_LoadWAV(filename);
	if (m_pSample == NULL) return false;
	return true;
}

bool Sample::Play(bool looping) {
	if (m_pSample == NULL) return false;
	m_Channel = Mix_PlayChannel(-1, m_pSample, looping ? -1 : 0);
	if (m_Channel == -1) return false;
	return SetVolume(1.0f);
}

bool Sample::Stop() {
	int ret;
	if (m_Channel == -1) return false;
	ret = Mix_HaltChannel(m_Channel);
	if (ret) return false;
	return true;
}

bool Sample::FadeOut(float seconds) {
	int ret;
	if (m_Channel == -1) return false;
	ret = Mix_FadeOutChannel(m_Channel, (int)(seconds * 1000.0f));
	if (ret) return false;
	return true;
}

} // End of namespace Ultima6
} // End of namespace Ultima
