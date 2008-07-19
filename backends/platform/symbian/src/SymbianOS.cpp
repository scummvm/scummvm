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
 * $URL$
 * $Id$
 */

#include <eikenv.h> // for CEikonEnv::Static() @ Symbian::FatalError()
#include <sdlapp.h> // for CSDLApp::GetExecutablePathCStr() @ Symbian::GetExecutablePath()
	
#include "backends/fs/symbian/symbian-fs-factory.h"
#include "backends/platform/symbian/src/SymbianOS.h"
#include "backends/platform/symbian/src/SymbianActions.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "gui/Actions.h"
#include "gui/Key.h"
#include "gui/message.h"
#include "sound/mixer_intern.h"
#include "..\..\sdl\main.cpp"

#ifdef SAMPLES_PER_SEC_8000 // the GreanSymbianMMP format cannot handle values for defines :(
  #define SAMPLES_PER_SEC 8000
#else
  #define SAMPLES_PER_SEC 16000
#endif

#define KInputBufferLength 128
// Symbian libc file functionality in order to provide shared file handles
struct TSymbianFileEntry {
	RFile iFileHandle;
	char iInputBuffer[KInputBufferLength];
	TInt iInputBufferLen;
	TInt iInputPos;
};

#define FILE void

////////// extern "C" ///////////////////////////////////////////////////
namespace Symbian {



// Show a simple Symbian Info win with Msg & exit
void FatalError(const char *msg) {
	TPtrC8 msgPtr((const TUint8 *)msg);
	TBuf<512> msg16Bit;
	msg16Bit.Copy(msgPtr);
#ifdef S60
#else
	CEikonEnv::Static()->InfoWinL(_L("ScummVM Fatal Error"), msg16Bit);
#endif
	if (g_system)
		g_system->quit();
}

// make this easily available everywhere
char* GetExecutablePath() {
	return CSDLApp::GetExecutablePathCStr();
}

} // namespace Symbian {

////////// OSystem_SDL_Symbian //////////////////////////////////////////

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Fullscreen", GFX_NORMAL},
	{0, 0, 0}
};

bool OSystem_SDL_Symbian::hasFeature(Feature f) {
	switch(f) {
		case kFeatureFullscreenMode:
		case kFeatureAspectRatioCorrection:
		case kFeatureAutoComputeDirtyRects:
		case kFeatureCursorHasPalette:
#ifdef  USE_VIBRA_SE_PXXX
		case kFeatureVibration:
#endif
			return true;

		default:
			return false;
	}
}

void OSystem_SDL_Symbian::setFeatureState(Feature f, bool enable) {
	switch(f) {
		case kFeatureVirtualKeyboard:
			if (enable) {
			}
			else {

			}
			break;
		case kFeatureDisableKeyFiltering:
			GUI::Actions::Instance()->beginMapping(enable);
			break;
		default:
			OSystem_SDL::setFeatureState(f, enable);
	}
}

FilesystemFactory *OSystem_SDL_Symbian::getFilesystemFactory() {
	return &SymbianFilesystemFactory::instance();
}

OSystem_SDL_Symbian::zoneDesc OSystem_SDL_Symbian::_zones[TOTAL_ZONES] = {
        { 0, 0, 320, 145 },
        { 0, 145, 150, 55 },
        { 150, 145, 170, 55 }
};
OSystem_SDL_Symbian::OSystem_SDL_Symbian() :_channels(0),_stereo_mix_buffer(0) {
}

void OSystem_SDL_Symbian::initBackend() {
	ConfMan.set("extrapath", Symbian::GetExecutablePath());
	ConfMan.setBool("FM_high_quality", false);
#if !defined(S60) || defined(S60V3) // S60 has low quality as default
	ConfMan.setBool("FM_medium_quality", true);
#else
	ConfMan.setBool("FM_medium_quality", false);
#endif
	ConfMan.setInt("joystick_num", 0); // Symbian OS  should have joystick_num set to 0 in the ini file , but uiq devices might refuse opening the joystick
	ConfMan.flushToDisk();

	GUI::Actions::init();

	OSystem_SDL::initBackend();

	// Initialize global key mapping for Smartphones
	GUI::Actions* actions = GUI::Actions::Instance();

	actions->initInstanceMain(this);
	actions->loadMapping();
	initZones();
}

OSystem_SDL_Symbian::~OSystem_SDL_Symbian() {
	delete[] _stereo_mix_buffer;
}

int OSystem_SDL_Symbian::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

const OSystem::GraphicsMode *OSystem_SDL_Symbian::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

// make sure we always go to normal, even if the string might be set wrong!
bool OSystem_SDL_Symbian::setGraphicsMode(const char * /*name*/) {
	// let parent OSystem_SDL handle it
	return OSystem_SDL::setGraphicsMode(getDefaultGraphicsMode());
}

void OSystem_SDL_Symbian::quitWithErrorMsg(const char * /*aMsg*/) {

	CEikonEnv::Static()->AlertWin(_L("quitWithErrorMsg()")) ;

	if (g_system)
		g_system->quit();
}

// Overloaded from SDL_Commmon
void OSystem_SDL_Symbian::quit() {
	delete GUI_Actions::Instance();
	OSystem_SDL::quit();
}

void OSystem_SDL_Symbian::setupMixer() {

	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;

	memset(&desired, 0, sizeof(desired));

	_samplesPerSec = 0;

	if (ConfMan.hasKey("output_rate"))
		_samplesPerSec = ConfMan.getInt("output_rate");

	if (_samplesPerSec <= 0)
		_samplesPerSec = SAMPLES_PER_SEC;

	// Originally, we always used 2048 samples. This loop will produce the
	// same result at 22050 Hz, and should hopefully produce something
	// sensible for other frequencies. Note that it must be a power of two.

	uint32 samples = 0x8000;

	for (;;) {
		if ((1000 * samples) / _samplesPerSec < 100)
			break;
		samples >>= 1;
	}

	desired.freq = _samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
	desired.callback = symbianMixCallback;
	desired.userdata = this;

	// Create the mixer instance
	assert(!_mixer);
	_mixer = new Audio::MixerImpl(this);
	assert(_mixer);

	if (SDL_OpenAudio(&desired, &obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_samplesPerSec = 0;
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		_samplesPerSec = obtained.freq;
		_channels = obtained.channels;
	
		// Need to create mixbuffer for stereo mix to downmix
		if (_channels != 2) {
			_stereo_mix_buffer = new byte [obtained.size*2];//*2 for stereo values
		}
	
		// Tell the mixer that we are ready and start the sound processing
		_mixer->setOutputRate(_samplesPerSec);
		_mixer->setReady(true);
		SDL_PauseAudio(0);
	}
}

/**
 * The mixer callback function, passed on to OSystem::setSoundCallback().
 * This simply calls the mix() method.
 */
void OSystem_SDL_Symbian::symbianMixCallback(void *sys, byte *samples, int len) {
	OSystem_SDL_Symbian *this_ = (OSystem_SDL_Symbian *)sys;
	assert(this_);

	if (!this_->_mixer)
		return;

#if defined (S60) && !defined(S60V3)
	// If not stereo then we need to downmix
	if (this_->_mixer->_channels != 2) {
		this_->_mixer->mixCallback(_stereo_mix_buffer, len * 2);

		int16 *bitmixDst = (int16 *)samples;
		int16 *bitmixSrc = (int16 *)_stereo_mix_buffer;

		for (int loop = len / 2; loop >= 0; loop --) {
			*bitmixDst = (*bitmixSrc + *(bitmixSrc + 1)) >> 1;
			bitmixDst++;
			bitmixSrc += 2;
		}
	} else
#else
	this_->_mixer->mixCallback(samples, len);
#endif
}


/**
 * This is an implementation by the remapKey function
 * @param SDL_Event to remap
 * @param ScumVM event to modify if special result is requested
 * @return true if Common::Event has a valid return status
 */
bool OSystem_SDL_Symbian::remapKey(SDL_Event &ev, Common::Event &event) {
	if (GUI::Actions::Instance()->mappingActive() || ev.key.keysym.sym <= SDLK_UNKNOWN)
		return false;

	for (TInt loop = 0; loop < GUI::ACTION_LAST; loop++) {
		if (GUI::Actions::Instance()->getMapping(loop) == ev.key.keysym.sym &&
			GUI::Actions::Instance()->isEnabled(loop)) {
			// Create proper event instead
			switch(loop) {
			case GUI::ACTION_UP:
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = -1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_DOWN:
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = 1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFT:
				if (ev.type == SDL_KEYDOWN) {
					_km.x_vel = -1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHT:
				if (ev.type == SDL_KEYDOWN) {
					_km.x_vel = 1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP);
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP);
				fillMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_ZONE:
				if (ev.type == SDL_KEYDOWN) {
					int i;

					for (i=0; i < TOTAL_ZONES; i++)
						if (_km.x >= _zones[i].x && _km.y >= _zones[i].y &&
							_km.x <= _zones[i].x + _zones[i].width && _km.y <= _zones[i].y + _zones[i].height
							) {
							_mouseXZone[i] = _km.x;
							_mouseYZone[i] = _km.y;
							break;
						}
						_currentZone++;
						if (_currentZone >= TOTAL_ZONES)
							_currentZone = 0;
						event.type = Common::EVENT_MOUSEMOVE;
						fillMouseEvent(event, _mouseXZone[_currentZone], _mouseYZone[_currentZone]);
						SDL_WarpMouse(event.mouse.x, event.mouse.y);
				}

				return true;
			case GUI::ACTION_MULTI: {
				GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
				// if key code is pause, then change event to interactive or just fall through
				if (key.keycode() == SDLK_PAUSE) {
					event.type = Common::EVENT_PREDICTIVE_DIALOG;
					return true;
					}
				}
			case GUI::ACTION_SAVE:
			case GUI::ACTION_SKIP:
			case GUI::ACTION_SKIP_TEXT:
			case GUI::ACTION_PAUSE:
			case GUI::ACTION_SWAPCHAR:
			case GUI::ACTION_FASTMODE:
			case GUI::ACTION_DEBUGGER: {
					GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
					ev.key.keysym.sym = (SDLKey) key.keycode();
					ev.key.keysym.scancode = 0;
					ev.key.keysym.mod = (SDLMod) key.flags();

					// Translate from SDL keymod event to Scummvm Key Mod Common::Event.
					// This codes is also present in GP32 backend and in SDL backend as a static function
					// Perhaps it should be shared.
					if (key.flags() != 0) {
						event.kbd.flags = 0;

						if (ev.key.keysym.mod & KMOD_SHIFT)
							event.kbd.flags |= Common::KBD_SHIFT;

						if (ev.key.keysym.mod & KMOD_ALT)
							event.kbd.flags |= Common::KBD_ALT;

						if (ev.key.keysym.mod & KMOD_CTRL)
							event.kbd.flags |= Common::KBD_CTRL;
					}

					return false;
				}

			case GUI::ACTION_QUIT:
				{
					GUI::MessageDialog alert("Do you want to quit ?", "Yes", "No");
					if (alert.runModal() == GUI::kMessageOK)
						quit();

					return true;
				}
			}
		}
	}

	return false;
}

void OSystem_SDL_Symbian::setWindowCaption(const char *caption) {
	OSystem_SDL::setWindowCaption(caption);
	check_mappings();
}

void OSystem_SDL_Symbian::check_mappings() {
	if (ConfMan.get("gameid").empty() || GUI::Actions::Instance()->initialized())
		return;

	GUI::Actions::Instance()->initInstanceGame();
}

void OSystem_SDL_Symbian::initZones() {
	int i;

	_currentZone = 0;

	for (i = 0; i < TOTAL_ZONES; i++) {
		_mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2));
		_mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2));
	}
}

FILE*	symbian_fopen(const char* name, const char* mode) {
	TSymbianFileEntry* fileEntry = new TSymbianFileEntry;
	fileEntry->iInputPos = KErrNotFound;

	if (fileEntry != NULL) {
		TInt modeLen = strlen(mode);

		TPtrC8 namePtr((unsigned char*) name, strlen(name));
		TFileName tempFileName;
		tempFileName.Copy(namePtr);

		TInt fileMode = EFileRead;

		if (mode[0] == 'a')
			fileMode = EFileWrite;

		if (!((modeLen > 1 && mode[1] == 'b') || (modeLen > 2 && mode[2] == 'b'))) {
			fileMode |= EFileStreamText;
		}

		if ((modeLen > 1 && mode[1] == '+') || (modeLen > 2 && mode[2] == '+')) {
			fileMode = fileMode| EFileWrite;
		}

		fileMode = fileMode| EFileShareAny;

		switch(mode[0]) {
		case 'a':
			if (fileEntry->iFileHandle.Open(CEikonEnv::Static()->FsSession(), tempFileName, fileMode) != KErrNone) {
				if (fileEntry->iFileHandle.Create(CEikonEnv::Static()->FsSession(), tempFileName, fileMode) != KErrNone) {
					delete fileEntry;
					fileEntry = NULL;
				}
			}
			break;
		case 'r':
			if (fileEntry->iFileHandle.Open(CEikonEnv::Static()->FsSession(), tempFileName, fileMode) != KErrNone) {
				delete fileEntry;
				fileEntry = NULL;
			}
			break;

		case 'w':
			if (fileEntry->iFileHandle.Replace(CEikonEnv::Static()->FsSession(), tempFileName, fileMode) != KErrNone) {
				delete fileEntry;
				fileEntry = NULL;
			}
			break;
		}
	}
	return (FILE*) fileEntry;
}

void symbian_fclose(FILE* handle) {
	((TSymbianFileEntry*)(handle))->iFileHandle.Close();

	delete (TSymbianFileEntry*)(handle);
}

size_t symbian_fread(const void* ptr, size_t size, size_t numItems, FILE* handle) {
	TSymbianFileEntry* entry = ((TSymbianFileEntry*)(handle));
	TUint32 totsize = size*numItems;
	TPtr8 pointer ( (unsigned char*) ptr, totsize);

	// Nothing cached and we want to load at least KInputBufferLength bytes
	if(totsize >= KInputBufferLength) {	
		TUint32 totLength = 0;
		if(entry->iInputPos != KErrNotFound)
		{
			TPtr8 cacheBuffer( (unsigned char*) entry->iInputBuffer+entry->iInputPos, entry->iInputBufferLen - entry->iInputPos, KInputBufferLength);
			pointer.Append(cacheBuffer);
			entry->iInputPos = KErrNotFound;
			totLength+=pointer.Length();
			pointer.Set(totLength+(unsigned char*) ptr, 0, totsize-totLength);
		}

		entry->iFileHandle.Read(pointer);
		totLength+=pointer.Length();

		pointer.Set((unsigned char*) ptr, totLength, totsize);

	}
	else {
		// Nothing in buffer	
		if(entry->iInputPos == KErrNotFound) {
			TPtr8 cacheBuffer( (unsigned char*) entry->iInputBuffer, KInputBufferLength);
			entry->iFileHandle.Read(cacheBuffer);

			if(cacheBuffer.Length() >= totsize) {
				pointer.Copy(cacheBuffer.Left(totsize));
				entry->iInputPos = totsize;
				entry->iInputBufferLen = cacheBuffer.Length();
			}
			else {
				pointer.Copy(cacheBuffer);
				entry->iInputPos = KErrNotFound;
			}

		}
		else {
			TPtr8 cacheBuffer( (unsigned char*) entry->iInputBuffer, entry->iInputBufferLen, KInputBufferLength);

			if(entry->iInputPos+totsize < entry->iInputBufferLen) {
				pointer.Copy(cacheBuffer.Mid(entry->iInputPos, totsize));
				entry->iInputPos+=totsize;
			}
			else {
			
			pointer.Copy(cacheBuffer.Mid(entry->iInputPos, entry->iInputBufferLen-entry->iInputPos));
			cacheBuffer.SetLength(0);
			entry->iFileHandle.Read(cacheBuffer);

			if(cacheBuffer.Length() >= totsize-pointer.Length()) {
					TUint32 restSize = totsize-pointer.Length();
					pointer.Append(cacheBuffer.Left(restSize));
					entry->iInputPos = restSize;
					entry->iInputBufferLen = cacheBuffer.Length();
				}
				else {
					pointer.Append(cacheBuffer);
					entry->iInputPos = KErrNotFound;
				}
			}
		}
	}	

	return pointer.Length()/size;
}

size_t symbian_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle) {
	TPtrC8 pointer( (unsigned char*) ptr, size*numItems);

	((TSymbianFileEntry*)(handle))->iInputPos = KErrNotFound;
	if (((TSymbianFileEntry*)(handle))->iFileHandle.Write(pointer) == KErrNone) {
		return numItems;
	}

	return 0;
}

bool symbian_feof(FILE* handle) {
	TInt pos = 0;
	TSymbianFileEntry* entry = ((TSymbianFileEntry*)(handle));

	if (entry->iFileHandle.Seek(ESeekCurrent, pos) == KErrNone) {

		TInt size = 0;
		if (entry->iFileHandle.Size(size) == KErrNone) {
			if(entry->iInputPos == KErrNotFound && pos == size) 			
				return true;

			if(entry->iInputPos != KErrNotFound && pos == size && entry->iInputPos == entry->iInputBufferLen)
				return true;

			return false;
		}
	}
	return true;
}

long int symbian_ftell(FILE* handle) {
	TInt pos = 0;

	((TSymbianFileEntry*)(handle))->iFileHandle.Seek(ESeekCurrent, pos);

	return pos;
}

int symbian_fseek(FILE* handle, long int offset, int whence) {

	TSeek seekMode = ESeekStart;
	TInt pos = offset;

	switch(whence) {
	case SEEK_SET:
		seekMode = ESeekStart;
		break;
	case SEEK_CUR:
		seekMode = ESeekCurrent;
		break;
	case SEEK_END:
		seekMode = ESeekEnd;
		break;

	}
	
	((TSymbianFileEntry*)(handle))->iInputPos = KErrNotFound;

	return ((TSymbianFileEntry*)(handle))->iFileHandle.Seek(seekMode, pos);
}

void symbian_clearerr(FILE* /*handle*/) {
}

/** Vibration support */
#ifdef  USE_VIBRA_SE_PXXX
void OSystem_SDL_Symbian::initializeVibration() {
	_vibrationApi = SonyEricsson::CVibration::NewL();
}

void OSystem_SDL_Symbian::vibrationOn(int vibraLength) {
	// initialize?
	if (!_vibrationApi) _vibrationApi = SonyEricsson::CVibration::NewL();
	// do it!
	_vibrationApi->VibrationOn(1, 1, vibraLength);
}

void OSystem_SDL_Symbian::vibrationOff() {
	_vibrationApi->VibrationOff();
}

#endif //  USE_SE_PXX_VIBRA

