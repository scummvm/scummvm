/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include <stdafx.h>
#include "player.h"

#include "renderer.h"
#include "channel.h"
#include "Chunk_type.h"
#include "rect.h"
#include "blitter.h"

#include <assert.h>
#include <stdlib.h> // for atoi
#include <stdio.h> // for FILE, fopen, fclose, fread, fseek, ftell
#include <string.h> // for strchr, strrchr
#include <ctype.h> // for isdigit

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

const int WAIT = 100;

/*!	@brief parser and map of string resources

	This class implements a parser for the string resource format of SMUSH animations.
	It then allows the player to get the string corresponding to a particular identifier.
	
	@bug some of The Dig strings are not completely parsed (in titles)
*/

const int MAX_STRINGS = 200;

class StringResource {
private:
	struct {
		int id;
		char * string;
	} _strings[MAX_STRINGS];
	int _nbStrings;
	int _lastId;
	char * _lastString;
public:
	StringResource() : _nbStrings(0), _lastId(-1) {};
	~StringResource() {
		for(int i = 0; i < _nbStrings; i++) {
			delete []_strings[i].string;
		}
	}
	/*!	@brief parse the given buffer

		@param buffer the buffer that contain the resource (in lucasart format)
		@param length the length of the buffer

		@return \c true if the parsing went fine, \c false otherwise
	*/
	bool init(char * buffer, int length) {
		debug(9, "parsing string resources...");
		char * def_start = strchr(buffer, '#');
		while(def_start != NULL) {
			char * def_end = strchr(def_start, '\n');
			assert(def_end != NULL); // def_end is just before the start of the string [def_start,def_end] correspond to the definition text
			char * id_end = def_end;
			while(id_end >= def_start && !isdigit(*(id_end-1))) id_end--;
			assert(id_end > def_start);
			char * id_start = id_end;
			while(isdigit(*(id_start - 1))) id_start--;
			// [id_start-id_end] is the id number
			char idstring[32];
			memcpy(idstring, id_start, id_end - id_start);
			idstring[id_end - id_start] = 0;
			int id = atoi(idstring);
			//~ assert(id != LONG_MIN && id != 0 && id != LONG_MAX);
			char * data_start = def_end;
			while(*data_start == '\n' || *data_start == '\r') data_start++;
			char * data_end = data_start;
			while(1) {
				if(data_end[-2] == '\r' && data_end[1] == '\n' && data_end[-1] == '\n' && data_end[0] == '\r')
					break;
				data_end++;
				if(data_end >= buffer + length) {
					data_end = buffer + length;
					break;
				}
			}
			data_end -= 2;
			assert(data_end > data_start);
			char * value = new char[data_end - data_start + 1];
			assert(value);
			memcpy(value, data_start, data_end - data_start);
			value[data_end - data_start] = 0;
#ifdef DEBUG
			debug(9, "Inserting (%s)%d == \"%s\"", idstring, id, value);
#endif				
			_strings[_nbStrings].id = id;
			_strings[_nbStrings].string = value;
			_nbStrings ++;
			def_start = strchr(data_end + 2, '#');
		}
		return true;
	}
	/*!	@brief extract a string

		@param id the resource identifier

		@return the corresponding string.
	*/
	const char * get(int id) {
		if(id == _lastId) return _lastString;
		for(int i = 0; i < _nbStrings; i++)
		{
			if(_strings[i].id == id) {
				_lastId = id;
				_lastString = _strings[i].string;
				return _strings[i].string;
			}
		}
		warning("invalid string id : %d", id);
		_lastId = -1;
		_lastString = "unknown string";
		return _lastString;
	}
};

void SmushPlayer::show(const char * p) {
	if(strcmp(p, "subtitles") == 0)
		_subtitles = true;
	else if(strcmp(p, "bgmusic") == 0)
		_bgmusic = true;
	else if(strcmp(p, "voices") == 0)
		_voices = true;
	else {
		int id = atoi(p);
		if(id < 0 || id > 36) error("invalid parameter to show");
		_skips[id] = true;
	}
}

void SmushPlayer::hide(const char * p) {
	if(strcmp(p, "subtitles") == 0)
		_subtitles = false;
	else if(strcmp(p, "bgmusic") == 0)
		_bgmusic = false;
	else if(strcmp(p, "voices") == 0)
		_voices = false;
	else {
		int id = atoi(p);
		if(id < 0 || id > 36) error("invalid parameter to hide");
		_skips[id] = false;
	}
}

SmushPlayer::SmushPlayer(Renderer * renderer, bool wait, bool sound) :
							_version(-1),
							_secondaryVersion(0),
							_soundFrequency(0),
							_nbframes(0),
							_mixer(0),
							_renderer(renderer),
							_strings(0),
							_frameSize(-1, -1),
							_frame(0),
							_outputSound(sound),
							_wait(wait),
							_alreadyInit(false),
							_codec37Called(false),
							_skipNext(false),
							_subtitles(true),
							_bgmusic(true),
							_voices(true) {
	_fr[0] = _fr[1] = _fr[2] = _fr[3] = 0;
	assert(_renderer != 0);
}

SmushPlayer::~SmushPlayer() {
	clean();
	//~ if(_mixer) delete _mixer;
}

void SmushPlayer::updatePalette(void) { 
	_renderer->setPalette(_pal); 
}

void SmushPlayer::clean() {
	if(_strings) 
		delete _strings;
	if(_fr[0]) delete _fr[0];
	if(_fr[1]) delete _fr[1];
	if(_fr[2]) delete _fr[2];
	if(_fr[3]) delete _fr[3];
}

void SmushPlayer::checkBlock(const Chunk & b, Chunk::type type_expected, unsigned int min_size) {
	if(type_expected != b.getType()) {
		error("Chunk type is different from expected : %d != %d", b.getType(), type_expected);
	}
	if(min_size > b.getSize()) {
		error( "Chunk size is inferior than minimum required size : %d < %d", b.getSize(), min_size);
	}
}

void SmushPlayer::handleSoundBuffer(int track_id, int index, int max_frames, int flags, int vol, int bal, Chunk & b, int size) {
	debug(6, "smush_player::handleSoundBuffer(%d)", track_id);
	if(!_voices && (flags & 128) == 128) return;
	if(!_bgmusic && (flags & 64) == 64) return;
	_Channel * c = _mixer->findChannel(track_id);
	if(c == 0) {
		c = new SaudChannel(track_id,  _soundFrequency);
		_mixer->addChannel(c);
	}
	if(index == 0)
		c->setParameters(max_frames, flags, vol, bal);
	else
		c->checkParameters(index, max_frames, flags, vol, bal);
	c->appendData(b, size);
}

void SmushPlayer::handleSoundFrame(Chunk & b) {
	checkBlock(b, TYPE_PSAD);
	debug(6, "SmushPlayer::handleSoundFrame()");
	if(!_outputSound) return;
	int track_id = b.getWord();
	int index = b.getWord();
	int max_frames = b.getWord();
	int flags = b.getWord();
	int vol = b.getByte();
	int bal = b.getChar();
#ifdef DEBUG
	if(index == 0) {
		debug(5, "track_id == %d, max_frames == %d, %d, %d, %d", track_id, max_frames, flags, vol, bal);
	}
#endif
	int size = b.getSize() - 10;
	handleSoundBuffer(track_id, index, max_frames, flags, vol, bal, b, size);
}

void SmushPlayer::handleSkip(Chunk & b) {
	checkBlock(b, TYPE_SKIP, 4);
	int code = b.getDword();
	debug(6, "SmushPlayer::handleSkip(%d)", code);
	if(code >= 0 && code < 37)
		_skipNext = _skips[code];
	else
		_skipNext = true;
}

void SmushPlayer::handleStore(Chunk & b) {
	checkBlock(b, TYPE_STOR, 4);
	debug(6, "SmushPlayer::handleStore()");
}

void SmushPlayer::handleFetch(Chunk & b) {
	checkBlock(b, TYPE_FTCH, 6);
	debug(6, "SmushPlayer::handleFetch()");
}

void SmushPlayer::handleImuseBuffer(int track_id, int index, int nbframes, int size, int unk1, int unk2, Chunk & b, int bsize) {
	_Channel * c = _mixer->findChannel(track_id);
	if(c == 0) {
		c = new ImuseChannel(track_id, _soundFrequency);
		_mixer->addChannel(c);
	}
	if(index == 0)
		c->setParameters(nbframes, size, unk1, unk2);
	else
		c->checkParameters(index, nbframes, size, unk1, unk2);
	c->appendData(b, bsize);
}

void SmushPlayer::handleImuseAction8(Chunk & b, int flags, int unknown, int track_id) {
	assert(flags == 46 && unknown == 0);
	int unknown2 = b.getWord();
	track_id |= unknown2 << 16;
	int index = b.getWord();
	int nbframes = b.getWord();
	int size = b.getDword();
	int bsize = b.getSize() - 18;
	handleImuseBuffer(track_id, index, nbframes, size, unknown, unknown2, b, bsize);
}

void SmushPlayer::handleImuseAction(Chunk & b) {
	checkBlock(b, TYPE_IACT, 8);
	debug(6, "SmushPlayer::handleImuseAction()");
	if(!_outputSound) return;
	int code = b.getWord();
	int flags = b.getWord();
	int unknown = b.getShort();
	int track_id = b.getWord();
#ifdef DEBUG
	debug(5, "handleImuseAction(%d, %d, %d, %d)", code, flags, unknown, track_id);
#endif
	switch(code) {
		case 8:
			handleImuseAction8(b, flags, unknown, track_id);
			break;
#ifdef DEBUG
		default: {
				debug(9, "%5.5d %d %8.8d %4.4d", track_id, flags, unknown);
			}
#endif
	}
}

void SmushPlayer::handleTextResource(Chunk & b) {
	checkBlock(b, TYPE_TRES, 18); 
	int pos_x = b.getShort();
	int pos_y = b.getShort();
	int flags = b.getShort();
	int left = b.getShort();
	int top = b.getShort();
	int width = b.getShort();
	int height = b.getShort();
	int unk2 = b.getWord();
	int string_id = b.getWord();
	debug(6, "SmushPlayer::handleTextResource(%d)", string_id);
	if(!_strings) return;

	// if subtitles disabled and bit 3 is set, then do not draw
	if((!_subtitles) && ((flags & 8) == 8)) return;
	const char * str = _strings->get(string_id);

	FontRenderer * fr = _fr[0];
	int color = 15;
	while(*str == '/') str++; // For Full Throttle text resources
	while(str[0] == '^') {
		switch(str[1]) {
		case 'f':
			{
#if 0
				// This cause trouble if the next character is a digit.
				int id = atoi(str+2);
#else
				// assume ASCII like character set...
				int id = str[3] - '0';
#endif
				str += 4;
				fr = _fr[id]; 
			} break;
		case 'c':
			{
				//~ int id = atoi(str+2);
				color = str[4] - '0' + 10 *(str[3] - '0');
				str += 5;
			} break;
		default:
			error("invalid escape code in text string");
		}
	}
	assert(fr != 0);
	fr->setColor(color);
	if(!_curBuffer) { _curBuffer = _renderer->lockFrame(_frame); }
	if(flags == 0 || flags == 4) {
		fr->drawStringAbsolute(str, _curBuffer, _frameSize, pos_x, pos_y);
	} else {
		fr->drawStringCentered(str, _curBuffer, _frameSize, max(pos_y, top), left, width, pos_x);
	}
}

void SmushPlayer::readPalette(Palette & out, Chunk & in) {
	unsigned char buffer[768];
	in.read(buffer, 768);
	out = Palette(buffer);
}

void SmushPlayer::handleDeltaPalette(Chunk & b) {
	checkBlock(b, TYPE_XPAL);
	debug(6, "SmushPlayer::handleDeltaPalette()");
	if(b.getSize() == 768 * 3 + 4) {
		int unk1, num;
		unk1 = b.getWord();
		num = b.getWord();
		for(int i = 0; i < 768; i++) {
			_deltaPal[i] = b.getWord();
		}
		readPalette(_pal, b);
		updatePalette();
	} else if(b.getSize() == 6) {
		int unk1, num, unk2;
		unk1 = b.getWord();
		num = b.getWord();
		unk2 = b.getWord();
		for(int i = 0; i < 256; i++) {
			_pal[i].delta(_deltaPal + 3 * i);
		}
		updatePalette();
	} else {
		error("wrong size for DeltaPalette");
	}
}

void SmushPlayer::handleNewPalette(Chunk & b) {
	checkBlock(b, TYPE_NPAL, 768);
	debug(6, "SmushPlayer::handleNewPalette()");
	readPalette(_pal, b);
	updatePalette();
}

void SmushPlayer::decodeCodec(Chunk & b, const Rect & r, Decoder & codec) {
	assert(_curBuffer);
	Blitter blit(_curBuffer, _frameSize, r);
	codec.decode(blit, b);
}

void SmushPlayer::initSize(const Rect & r, bool always, bool transparent) {
	if(_codec37Called) _alreadyInit = true;
		
	if(!_alreadyInit || _frameSize.getX() < r.right() || _frameSize.getY() < r.bottom() || always) {
		if(_curBuffer) {
			_renderer->unlockFrame();
			_curBuffer = 0;
		}
		_frameSize = r.bottomRight();
		_renderer->initFrame(_frameSize);
	}

	if(_curBuffer) {
		_renderer->unlockFrame();
		_curBuffer = 0;
	}

	_curBuffer = _renderer->lockFrame(_frame);
	if(!_alreadyInit && transparent) {
		memset(_curBuffer, 0, _frameSize.getX()*_frameSize.getY());
	}

	_codec1.initSize(_frameSize, r);
	_codec37.initSize(_frameSize, r);
	_codec44.initSize(_frameSize, r);
	_codecd.initSize(_frameSize, r);
	_alreadyInit = true;
}

void SmushPlayer::handleFrameObject(Chunk & b) {
	checkBlock(b, TYPE_FOBJ, 14);
	if(_skipNext) {
		_skipNext = false;
		return;
	}
	int codec = b.getWord();
	debug(6, "SmushPlayer::handleFrameObject(%d)", codec);
	unsigned short left = b.getWord();
	unsigned short top = b.getWord();
	unsigned short width = b.getWord();
	unsigned short height = b.getWord();
	Rect r(left, top, left + width, top + height);
	unsigned short data[2];
	data[1] = b.getWord();
	data[0] = b.getWord();
#ifdef DEBUG
	debug(5, "Frame pos : %d, %d", left, top);
	debug(5, "Frame size : %dx%d", width, height);
	debug(5, "Codec : %d", codec);
#endif
	switch (codec) {
	case 3:
	case 1:
		initSize(r, false, true);
		decodeCodec(b, r, _codec1);
		break;
	case 37:
		assert(left == 0 && top == 0);
		initSize(r, true, false);
		decodeCodec(b, r, _codec37);
		_codec37Called = true;
		break;
	case 47:
		initSize(r, false, true);
		decodeCodec(b, r, _codecd);
		break;
	case 21:
	case 44:
		initSize(r, true, true);
		decodeCodec(b, r, _codec44);
		break;
	default:
		error("Invalid codec for frame object : %d", (int)codec);
	}
}

void SmushPlayer::handleFrame(Chunk & b) {
	checkBlock(b, TYPE_FRME);
	debug(6, "SmushPlayer::handleFrame(%d)", _frame);
	_alreadyInit = false;
	_skipNext = false;

	while(!b.eof()) {
		Chunk * sub = b.subBlock();
		if(sub->getSize() & 1) b.seek(1);
		switch(sub->getType()) {
			case TYPE_NPAL:
				handleNewPalette(*sub);
				break;
			case TYPE_FOBJ:
				handleFrameObject(*sub);
				break;
			case TYPE_PSAD:
				handleSoundFrame(*sub);
				break;
			case TYPE_TRES:
				handleTextResource(*sub);
				break;
			case TYPE_XPAL:
				handleDeltaPalette(*sub);
				break;
			case TYPE_IACT:
				handleImuseAction(*sub);
				break;
			case TYPE_STOR:
				handleStore(*sub);
				break;
			case TYPE_FTCH:
				handleFetch(*sub);
				break;
			case TYPE_SKIP:
				handleSkip(*sub);
				break;
			default:
				error("Unknown frame subChunk found : %s, %d", Chunk::ChunkString(sub->getType()), sub->getSize());
		}
		delete sub;
	}
	if(_curBuffer) {
		_renderer->unlockFrame();
		_curBuffer = 0;
	}
	if(_outputSound)
		_mixer->handleFrame();
#ifdef DEBUG
	debug(5, "===================END OF FRAME========================");
#endif
	_renderer->flipFrame();
	if(_wait)
		_renderer->wait(WAIT);
	_frame++;
}

void SmushPlayer::handleAnimHeader(Chunk & b) {
	checkBlock(b, TYPE_AHDR, 774);
	debug(6, "SmushPlayer::handleAnimHeader()");
	_version = b.getWord();
	_nbframes = b.getWord();
	int unknown = b.getWord();
#ifdef DEBUG
	debug(5, "SMUSH HEADER : version == %d, nbframes == %d, unknown == %d", _version, _nbframes, unknown);
#else
	unknown = unknown;
#endif
	_renderer->startDecode(_fname, _version, _nbframes);
	readPalette(_pal, b);
	updatePalette();
	if(_version == 1) {
		_soundFrequency = 22050;
	}
	if(_version == 2) {
		_secondaryVersion = b.getDword();
		int unknown2 = b.getDword();
		_soundFrequency = b.getDword();
#ifdef DEBUG
		debug(5, "SMUSH HEADER : secondary version == %d, unknown2 == %d, sound frequency == %d", _secondaryVersion, unknown2, _soundFrequency);
		int i = 0, c;
		while(!b.eof()) {
			c = b.getByte();
			if(c) debug(9, "SMUSH HEADER : remaining bytes : %d == %d", i, c);
			i++;
		}
#else
		unknown2 = unknown2;
#endif
		if(_secondaryVersion != 10 && _secondaryVersion != 0 && _secondaryVersion != 12 && _secondaryVersion != 15 && _secondaryVersion != 14)
			error("Wrong secondary version number for SMUSH animation");
		if(_soundFrequency != 0 && _soundFrequency != 11025 && _soundFrequency != 22050)
			error("Wrong _sound_frequency number for SMUSH animation");
	} else if(_version > 2) {
		error("Wrong primary version number for SMUSH animation");
	}
	if(_outputSound && _soundFrequency) {
		if(_soundFrequency != 22050) _soundFrequency = 22050;
		_mixer = _renderer->getMixer();
		if(_mixer) {
			_mixer->init();
		} else {
			_outputSound = false;
		}
	}
}

static StringResource * getStrings(const char * file, bool is_encoded) {
	debug(7, "trying to read text ressources from %s", file);
	FILE * is;
	is = fopen(file, "rb");
	if(is == NULL) return 0;
	fseek(is, 0, SEEK_END);
	int length = ftell(is);
	fseek(is, 0, SEEK_SET);
	char * filebuffer = new char [length + 1];
	assert(filebuffer);
	fread (filebuffer, length, 1, is);
	filebuffer[length] = 0;
	fclose(is);
	if(is_encoded) {
		static const int ETRS_HEADER_LENGTH = 16;
		assert(length > ETRS_HEADER_LENGTH);
		Chunk::type type = READ_BE_UINT32(filebuffer);
		if(type != TYPE_ETRS) error("invalid type for file"); // mem leak !!!
		char * old = filebuffer;
		filebuffer = new char[length - ETRS_HEADER_LENGTH];
		for(int i = ETRS_HEADER_LENGTH; i < length; i++)
			filebuffer[i - ETRS_HEADER_LENGTH] = old[i] ^ 0xCC;
		delete []old;
		length -= ETRS_HEADER_LENGTH;
	}
	StringResource * sr = new StringResource;
	assert(sr);
	sr->init(filebuffer, length);
	delete []filebuffer;
	return sr;
}

bool SmushPlayer::readString(const char * file, bool & ft) {
	const char * i = strrchr(file, '.');
	if(i == NULL) error("invalid filename : %s", file);
	char fname[260];
	memcpy(fname, file, i - file);
	strcpy(fname + (i - file), ".trs");
	if((_strings = getStrings(fname, false)) != 0) {
		ft = true;
		return true;
	}
	i = strrchr(file, '\\');
	if(i == NULL) i = strrchr(file, '/');
	else {
		char * j = strrchr(file, '/');
		if(j > i) i = j;
	}
	if(i == NULL) error("invalid filename : %s", file);

	memcpy(fname, file, i - file + 1);
	strcpy(fname + (i - file + 1), "digtxt.trs");
	if((_strings = getStrings(fname, true)) != 0) {
		ft = false;
		return true;
	}
	return false;
}

static FontRenderer * loadFont(const char * file, bool original = false) {
#ifdef DEBUG
	debug(5, "loading font from \"%s\"", file);
#endif
	FontRenderer * fr = new FontRenderer(original);
	SmushPlayer p(fr, false, false);
	p.play(file);
	return fr;
}

bool SmushPlayer::play(const char * file) {
#ifdef DEBUG
	debug(5, "start of animation : %s", file);
#endif	
	char * i = strrchr(file, '\\');
	if(i == NULL)
	{
		i = strrchr(file, '/');
	} else {
		char * j = strrchr(i, '/');
		if(j != NULL)
			i = j;
	}
	char directory[260];
	if(i != NULL) {
		strcpy(directory, file);
		directory[i-file] = 0;
		//! @todo remove this...
		_fname = strdup(i);
	} else {
		directory[0] = 0;
		_fname = strdup(file);
	}
	clean();
	
	if(_wait) {
		bool isFullthrottle;
		if(!readString(file, isFullthrottle))
			warning("unable to read text information for \"%s\"", file);
		if(_strings) {
			if(isFullthrottle) {
				if(strcmp(directory, "") == 0) {
					strcpy(directory, "../data/");
				} else {
					char * i = strrchr(directory, '\\');
					char * j = strrchr(directory, '/');
					if(j > i) i = j;
					if(i == NULL) {
						strcpy(directory, "data/");
					} else {
						*i = 0;
						strcat(directory, "/data/");
					}
				}
				char file[260];
				strcpy(file, directory); strcat(file, "scummfnt.nut");
				_fr[0] = loadFont(file, true);
				strcpy(file, directory); strcat(file, "titlfnt.nut");
				_fr[2] = loadFont(file, true);
			} else {
				for(int i = 0; i < 4; i++) {
					char file[260];
					sprintf(file, "%s/font%d.nut",directory, i);
					_fr[i] = loadFont(file, i != 0);
				}
			}
		}
	}
	FileChunk base = FileChunk(file);

	checkBlock(base, TYPE_ANIM); 

	while(!base.eof()) {
		Chunk * sub = base.subBlock();
		switch(sub->getType()) {
			case TYPE_AHDR:
				handleAnimHeader(*sub);
				break;
			case TYPE_FRME:
				handleFrame(*sub);
				break;
			default:
				error("Unknown Chunk found : %d, %d", sub->getType(), sub->getSize());
		}
		delete sub;
		if(_renderer->prematureClose())
			break;
	}
#ifdef DEBUG
	debug(5, "end of animation");
#endif	
	if(_outputSound) {
		_mixer->stop();
	}
	return true;
}
