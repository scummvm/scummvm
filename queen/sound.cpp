/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/file.h"
#include "common/util.h"
#include "queen/input.h"
#include "queen/resource.h"
#include "queen/sound.h"

#define	SB_HEADER_SIZE	110
#define	STOP_MUSIC	-1

namespace Queen {

#ifdef USE_VORBIS
// These are wrapper functions to allow using a File object to
// provide data to the OggVorbis_File object.

struct file_info {
	File *file;
	int start, curr_pos;
	size_t len;
};

static size_t read_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	file_info *f = (file_info *) datasource;
	int result;

	nmemb *= size;
	if (f->curr_pos > (int) f->len)
		nmemb = 0;
	else if (nmemb > f->len - f->curr_pos)
		nmemb = f->len - f->curr_pos;
	result = f->file->read(ptr, nmemb);
	if (result == -1) {
		f->curr_pos = f->file->pos() - f->start;
		return (size_t) -1;
	} else {
		f->curr_pos += result;
		return result / size;
	}
}

static int seek_wrap(void *datasource, ogg_int64_t offset, int whence) {
	file_info *f = (file_info *) datasource;

	if (whence == SEEK_SET)
		offset += f->start;
	else if (whence == SEEK_END) {
		offset += f->start + f->len;
		whence = SEEK_SET;
	}

	f->file->seek(offset, whence);
	f->curr_pos = f->file->pos() - f->start;
	return f->curr_pos;
}

static int close_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	f->file->close();
	delete f;
	return 0;
}

static long tell_wrap(void *datasource) {
	file_info *f = (file_info *) datasource;

	return f->curr_pos;
}

static ov_callbacks g_File_wrap = {
	read_wrap, seek_wrap, close_wrap, tell_wrap
};
#endif

Sound::Sound(SoundMixer *mixer, Input *input, Resource  *resource) : 
  _mixer(mixer), _input(input), _resource(resource), _lastOverride(0), _currentSong(0), _sfxHandle(0) {
}

Sound::~Sound() {
}

Sound *Sound::giveSound(SoundMixer *mixer, Input *input, Resource *resource, uint8 compression) {
	switch(compression) {
		case COMPRESSION_NONE:
				return new SBSound(mixer, input, resource);
				break;
		case COMPRESSION_MP3:
				#ifndef USE_MAD
					warning("Using MP3 compressed datafile, but MP3 support not compiled in");
					return new SilentSound(mixer, input, resource);
				#else
					return new MP3Sound(mixer, input, resource);

				#endif
				break;
		case COMPRESSION_OGG:
				#ifndef USE_VORBIS
					warning("Using OGG compressed datafile, but OGG support not compiled in");
					return new SilentSound(mixer, input, resource);
				#else
					return new OGGSound(mixer, input, resource);
				#endif
				break;
		default:
				warning("Unknown compression type");
				return new SilentSound(mixer, input, resource);
	}
}

void Sound::waitSfxFinished() {
	while((volatile PlayingSoundHandle)_sfxHandle != 0)
		_input->delay(10);
}

void Sound::playSong(int16 songNum) {
	if (songNum == STOP_MUSIC) {
		//stop the music
		//_fanfareCount = _fluteCount = 0;
		//_oldName[0] = 0;
		return;
	}

	// Remap some songs because there is less amiga music
	// and we want lastoverride to work properly
	switch(songNum) {
		case 1:
		case 2:
			songNum = 39;
			break;

		case 37:
		case 52:
		case 196:
			songNum = 90;
			break;

		case 38:
		case 89:
			songNum = 3;
			break;

		case 24:
		case 158:
			songNum = 117;
			break;

		case 71:
		case 72:
		case 73:
		case 75:
			songNum = 133;
			break;

		case 203:
			songNum = 67;
			break;

		case 145:
			songNum = 140;
			break;

		case 53:
		case 204:
			songNum = 44;
			break;

		case 136:
		case 142:
		case 179:
			songNum = 86;
			break;

		case 101:
		case 102:
		case 143:
			songNum = 188;
			break;

		case 65:
		case 62:
			songNum = 69;
			break;

		case 118:
		case 119:
			songNum = 137;
			break;

		case 130:
		case 131:
			songNum = 59;
			break;

		case 174:
		case 175:
			songNum = 57;
			break;

		case 171:
		case 121:
			songNum = 137;
			break;

		case 138:
		case 170:
		case 149:
			songNum = 28;
			break;

		case 122:
		case 180:
		case 83:
		case 98:
			songNum = 83;
			break;

		case 20:
		case 33:
			songNum = 34;
			break;

		case 29:
		case 35:
			songNum = 36;
			break;

		case 7:
		case 9:
		case 10:
			songNum = 11;
			break;

		case 110:
			songNum = 94;
			break;

		case 111:
			songNum = 95;
			break;

		case 30:
			songNum = 43;
			break;

		case 76:
			songNum = 27;
			break;

		case 194:
		case 195:
			songNum = 32;
			break;
	}

	// Sound effects and volume changes exit before setting lastoverride
	// 04-19-95 01:31pm Fix sfx bug
	if(_lastOverride != 32 && _lastOverride != 44) {
		switch(songNum) {
			// Volume normal
			case 5:
				warning("Sound::playSong() -> alter_current_volume() not implemented yet");
    				//alter_current_volume();
    				return;
	
			// Volume soft
			case 15:
    				//volumecontrol(-1,(20*VOLUME)/100);
    				return;
		
			// Volume medium
			case 14:
    				//volumecontrol(-1,(32*VOLUME)/100);
    				return;

			// Open door
			case 25:
				sfxPlay("116BSSSS");
				return;

			// Close door
			case 26:
    				sfxPlay("105ASSSS");
				return;

			// Light switch
			case 56:
				sfxPlay("27SSSSSS");
				return;

			// Hydraulic doors open
			case 57:
				sfxPlay("96SSSSSS");
				return;

			// Hydraulic doors close
			case 58:
				sfxPlay("97SSSSSS");
				return;

			// Metallic door slams
			case 59:
				sfxPlay("105SSSSS");
				return;

			// Oracle rezzes in
			case 63:
				sfxPlay("132SSSSS");
				return;

			// Cloth slide 1
			case 27:
				sfxPlay("135SSSSS");
				return;
	
			// Splash
			case 83:
				sfxPlay("18SSSSSS");
				return;

			// Agression enhancer
			case 85:
				sfxPlay("138BSSSS");
				return;
		
			// Dino Ray
			case 68:
				sfxPlay("138SSSSS");
				return;

			// Dino transformation
			case 140:
				sfxPlay("55BSSSSS");
				return;

			// Experimental laser
			case 141:
				sfxPlay("55SSSSSS");
				return;

			// Plane hatch open
			case 94:
				sfxPlay("3SSSSSSS");
				return;

			// Plane hatch close
			case 95:
				sfxPlay("4SSSSSSS");
				return;

			// Oracle rezzes out
			case 117:
				sfxPlay("70SSSSSS");
				return;

			// Dino horn
			case 124:
				sfxPlay("103SSSSS");
				return;

			// Punch
			case 127:
				sfxPlay("128SSSSS");
				return;

			// Body hits ground
			case 128:
				sfxPlay("129SSSSS");
				return;

			// Explosion
			case 137:
				sfxPlay("88SSSSSS");
				return;
	
			// Stone door grind 1
			case 86:
				sfxPlay("1001SSSS");
				return;

			// Stone door grind 2
			case 188:
				sfxPlay("1002SSSS");
				return;

			// Cloth slide 2
			case 28:
				sfxPlay("1005SSSS");
				return;

			// Rattle bars
			case 151:
				sfxPlay("115SSSSS");
				return;

			// Door dissolves
			case 152:
				sfxPlay("56SSSSSS");
				return;

			// Altar slides
			case 153:
				sfxPlay("85SSSSSS");
				return;

			// Pull lever
			case 166:
				sfxPlay("1008SSSS");
				return;

			// Zap Frank
			case 182:
				sfxPlay("1023SSSS");
				return;

			// Splorch
			case 69:
				sfxPlay("137ASSSS");
				return;

			// Robot laser
			case 70:
				sfxPlay("61SSSSSS");
				return;

			// Pick hits stone
			case 133:
				sfxPlay("71SSSSSS");
				return;

			// Press button
			case 165:
				sfxPlay("1007SSSS");
				return;
		}
	}


	// Exit if trying to play a song that's already playing
	// and instruments are already loaded i.e. last song wasn't -1
	if (songNum == _lastOverride /* && oldName[0] != 0 */ )

	// Play songs
	switch(songNum) {
		// Hotel
		case 39:
			//goSong("hotel",1);
			break;

		case 19:
			//goSong("hotel",3);
			break;

		case 34:
			//goSong("hotel",2);
			break;

		case 36:
			//goSong("hotel",4);
			//fanfarerestore=lastoverride;
			//fanfarecount=60;
			break;

		// Jungle
		case 40:
			//goSong("jung",1);
			//fanfarerestore=lastoverride;
			//fanfarecount=80;
			//flutecount=100;
			break;

		case 3:
			//goSong("jung",2);
			//flutecount=100;
			break;

		// Temple
		case 54:
			//goSong("temple",1);
			break;

		case 12:
			//goSong("temple",2);
			break;

		case 11:
			//goSong("temple",3);
			break;

		case 31:
			//goSong("temple",4);
			//fanfarerestore=lastoverride;
			//fanfarecount=80;
			break;

		// Floda
		case 41:
			//goSong("floda",4);
			//fanfarerestore=lastoverride;
			//fanfarecount=60;
			break;

		case 13:
			//goSong("floda",3);
			break;

		case 16:
			//goSong("floda",1);
			break;
			
		case 17:
			//goSong("floda",2);
			break;

		case 43:
			//goSong("floda",5);
			break;

		// Prequel (End credits)
		case 67:
			//goSong("title",1);
			// 05-19-95 03:24pm Removed to fix A500 Bug
			//fanfarerestore=90;
			//fanfarecount=550;
			break;

			// Title (Intro credits)
		case 88:
			//goSong("title",1);
			break;

		// Valley
		case 90:
			//goSong("awestruk",1);
			break;

		// Confrontation
		case 91:
			//goSong("'jungle'",1);
			break;

		// Frank
		case 46:
			//goSong("frank",1);
			break;

		// Trader Bob
		case 6:
			//goSong("bob",1);
			break;

		// Azura
		case 44:
			//goSong("azura",1);
			break;

		// Amazon Fortress
		case 21:
			//goSong("fort",1);
			break;

		// Rocket
		case 32:
			//goSong("rocket",1);
			break;

		// Robot
		case 92:
			//goSong("robot",1);
			break;

		// Song not handled by amiga
		default:
			return;
	}


	_lastOverride = _currentSong = songNum;

}


int SBSound::playSound(byte *sound, uint32 size) {
	byte flags = 0 | SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	return _mixer->playRaw(&_sfxHandle, sound, size, 11025, flags);
}

void SBSound::sfxPlay(const char *base) {
	char name[13];
	strcpy(name, base);
	//alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");

	waitSfxFinished();
	
	if (_resource->exists(name)) 
		playSound(_resource->loadFileMalloc(name, SB_HEADER_SIZE), _resource->fileSize(name) - SB_HEADER_SIZE);
}

#ifdef USE_MAD
void MP3Sound::sfxPlay(const char *base) {
	char name[13];
	strcpy(name, base);
	//alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");
	
	waitSfxFinished();

	if (_resource->exists(name)) 
		_mixer->playMP3(&_sfxHandle, _resource->giveCompressedSound(name), _resource->fileSize(name));
}
#endif

#ifdef USE_VORBIS
void OGGSound::sfxPlay(const char *base) {
	char name[13];
	strcpy(name, base);
	//alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");

	waitSfxFinished();
	
	if (_resource->exists(name)) {
		OggVorbis_File *oggFile = new OggVorbis_File;
		file_info *f = new file_info;

		f->file = _resource->giveCompressedSound(name);
		f->start = _resource->fileOffset(name);
		f->len = _resource->fileSize(name);
		f->curr_pos = 0;

		if (ov_open_callbacks((void *)f, oggFile, NULL, 0, g_File_wrap) < 0) {
			delete oggFile;
			delete f;
		} else
			_mixer->playVorbis(&_sfxHandle, oggFile, 0, false);
	}
}
#endif

} //End of namespace Queen
