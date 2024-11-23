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



#include "base/version.h"

#include "common/util.h"
#include "common/system.h"
#include "common/endian.h"

#include "audio/mixer.h"

#include "scumm/imuse/imuse.h"
#include "scumm/imuse/imuse_internal.h"
#include "scumm/imuse/instrument.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"

namespace Scumm {

////////////////////////////////////////
//
//  IMuseInternal implementation
//
////////////////////////////////////////

IMuseInternal::IMuseInternal(ScummEngine *vm, MidiDriverFlags sndType, bool nativeMT32) :
	_vm(vm),
	_native_mt32(nativeMT32),
	_newSystem(vm && vm->_game.id == GID_SAMNMAX),
	_dynamicChanAllocation(vm && (vm->_game.id != GID_MONKEY2 && vm->_game.id != GID_INDY4)), // For the non-iMuse games that (unfortunately) run on this player we need to pretend we're on the more modern version
	_midi_adlib(nullptr),
	_midi_native(nullptr),
	_sysex(nullptr),
	_paused(false),
	_initialized(false),
	_tempoFactor(0),
	_player_limit(ARRAYSIZE(_players)),
	_recycle_players(false),
	_queue_end(0),
	_queue_pos(0),
	_queue_sound(0),
	_queue_adding(0),
	_queue_marker(0),
	_queue_cleared(0),
	_master_volume(0),
	_music_volume(0),
	_music_volume_eff(0),
	_trigger_count(0),
	_snm_trigger_index(0),
	_soundType(sndType),
	_lowLevelVolumeControl(sndType == MDT_MACINTOSH),
	_game_id(vm ? vm->_game.id : 0),
	_mutex(vm ? vm->_mixer->mutex() : _dummyMutex) {
	memset(_channel_volume, 0, sizeof(_channel_volume));
	memset(_channel_volume_eff, 0, sizeof(_channel_volume_eff));
	memset(_volchan_table, 0, sizeof(_volchan_table));
}

IMuseInternal::~IMuseInternal() {
	// Do just enough stuff inside the mutex to
	// make sure any MIDI timing threads won't
	// interrupt us, and then do the rest outside
	// the mutex.
	{
		Common::StackLock lock(_mutex);
		_initialized = false;
		stopAllSounds_internal();
	}

	if (_midi_adlib) {
		_midi_adlib->close();
		delete _midi_adlib;
		_midi_adlib = nullptr;
	}

	if (_midi_native) {
		_midi_native->close();
		delete _midi_native;
		_midi_native = nullptr;
	}
}

byte *IMuseInternal::findStartOfSound(int sound, int ct) {
	int32 size, pos;

	static const uint32 id[] = {
		MKTAG('M', 'T', 'h', 'd'),
		MKTAG('F', 'O', 'R', 'M'),
		MKTAG('M', 'D', 'h', 'd'),
		MKTAG('M', 'D', 'p', 'g')
	};

	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;

	if (ptr == nullptr) {
		debug(1, "IMuseInternal::findStartOfSound(): Sound %d doesn't exist", sound);
		return nullptr;
	}

	// Check for old-style headers first, like 'RO'
	int trFlag = (kMThd | kFORM);
	if (ptr[0] == 'R' && ptr[1] == 'O' && ptr[2] != 'L')
		return ct == trFlag ? ptr : nullptr;
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return ct == trFlag ? ptr + 4 : nullptr;

	ptr += 4;
	//size = READ_BE_UINT32(ptr);
	ptr += 4;

	// Okay, we're looking for one of those things: either
	// an 'MThd' tag (for SMF), or a 'FORM' tag (for XMIDI).
	size = 48; // Arbitrary; we should find our tag within the first 48 bytes of the resource
	pos = 0;
	while (pos < size) {
		for (int i = 0; i < ARRAYSIZE(id); ++i) {
			if ((ct & (1 << i)) && (READ_BE_UINT32(ptr + pos) == id[i]))
				return ptr + pos;
		}
		++pos; // We could probably iterate more intelligently
	}

	if (ct == (kMThd | kFORM))
		debug(3, "IMuseInternal::findStartOfSound(): Failed to align on sound %d", sound);

	return nullptr;
}

bool IMuseInternal::isMT32(int sound) {
	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;
	if (ptr == nullptr)
		return false;

	uint32 tag = READ_BE_UINT32(ptr);
	switch (tag) {
	case MKTAG('A', 'D', 'L', ' '):
	case MKTAG('A', 'S', 'F', 'X'): // Special AD class for old AdLib sound effects
	case MKTAG('S', 'P', 'K', ' '):
		return false;

	case MKTAG('A', 'M', 'I', ' '): // MI2 Amiga
		return false;

	case MKTAG('R', 'O', 'L', ' '): // Roland LAPC/MT-32/CM32L track, but FOA Amiga and and DOTT Demo Mac also use this resource type
		return _soundType != MDT_AMIGA && _soundType != MDT_MACINTOSH;

	case MKTAG('M', 'A', 'C', ' '): // Occurs in the Mac version of FOA and MI2
		return false;

	case MKTAG('G', 'M', 'D', ' '):
		return false;

	case MKTAG('M', 'I', 'D', 'I'): // Occurs in Sam & Max
		// HE games use Roland music
		if (ptr[8] == 'H' && ptr[9] == 'S')
			return true;
		else
			return false;

	default:
		break;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[0] == 'R' && ptr[1] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return false;

	error("Unknown music type: '%s'", tag2str(tag));

	return false;
}

bool IMuseInternal::isMIDI(int sound) {
	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;
	if (ptr == nullptr)
		return false;

	uint32 tag = READ_BE_UINT32(ptr);
	switch (tag) {
	case MKTAG('A', 'D', 'L', ' '):
	case MKTAG('A', 'S', 'F', 'X'): // Special AD class for old AdLib sound effects
	case MKTAG('S', 'P', 'K', ' '):
		return false;

	case MKTAG('A', 'M', 'I', ' '): // Amiga (return true, since the driver is initalized as native midi)
		return true;

	case MKTAG('R', 'O', 'L', ' '): // Roland LAPC/MT-32/CM32L track
		return true;

	case MKTAG('M', 'A', 'C', ' '): // Occurs in the Mac version of FOA and MI2
		return true;

	case MKTAG('G', 'M', 'D', ' '):
	case MKTAG('M', 'I', 'D', 'I'): // Occurs in Sam & Max
		return true;

	default:
		break;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[0] == 'R' && ptr[1] == 'O')
		return true;
	// Euphony tracks show as 'SO' and have equivalent properties to 'ADL'
	// FIXME: Right now we're pretending it's GM.
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return true;

	error("Unknown music type: '%s'", tag2str(tag));

	return false;
}

bool IMuseInternal::supportsPercussion(int sound) {
	byte *ptr = g_scumm->_res->_types[rtSound][sound]._address;
	if (ptr == nullptr)
		return false;

	uint32 tag = READ_BE_UINT32(ptr);
	switch (tag) {
	case MKTAG('A', 'D', 'L', ' '):
	case MKTAG('A', 'S', 'F', 'X'): // Special AD class for old AdLib sound effects
	case MKTAG('S', 'P', 'K', ' '):
		return false;

	case MKTAG('A', 'M', 'I', ' '): // MI2 Amiga
		return false;

	case MKTAG('R', 'O', 'L', ' '): // Roland LAPC/MT-32/CM32L track, but also used by INDY4 Amiga and DOTT Demo Mac (but the latter does support percussion).
		return _soundType != MDT_AMIGA;

	case MKTAG('M', 'A', 'C', ' '): // Occurs in the Mac version of FOA and MI2. The early Mac imuse system doesn't support percussion.
		return false;

	case MKTAG('G', 'M', 'D', ' '): // DOTT
	case MKTAG('M', 'I', 'D', 'I'): // Sam & Max
		return true;				// This is correct for Mac, too. The later Mac imuse system does have a percussion channel.

	default:
		break;
	}

	// Old style 'RO' has equivalent properties to 'ROL'
	if (ptr[0] == 'R' && ptr[1] == 'O')
		return true;
	if (ptr[4] == 'S' && ptr[5] == 'O')
		return true;

	error("Unknown music type: '%s'", tag2str(tag));

	return false;
}

MidiDriver *IMuseInternal::getBestMidiDriver(int sound) {
	MidiDriver *driver = nullptr;

	if (isMIDI(sound)) {
		if (_midi_native) {
			driver = _midi_native;
		} else {
			// Route it through AdLib anyway.
			driver = _midi_adlib;
		}
	} else {
		driver = _midi_adlib;
	}
	return driver;
}

Player *IMuseInternal::allocate_player(byte priority) {
	Player *player = _players, *best = nullptr;
	int i;
	byte bestpri = 255;

	for (i = _player_limit; i != 0; i--, player++) {
		if (!player->isActive())
			return player;
		if (player->getPriority() < bestpri) {
			best = player;
			bestpri = player->getPriority();
		}
	}

	if (bestpri < priority || _recycle_players)
		return best;

	debug(1, "Denying player request");
	return nullptr;
}

void IMuseInternal::init_players() {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		player->_se = this;
		player->clear(); // Used to just set _active to false
	}
}

void IMuseInternal::init_parts() {
	Part *part;
	int i;

	for (i = 0, part = _parts; i != ARRAYSIZE(_parts); i++, part++) {
		part->init(_native_mt32);
		part->_se = this;
		part->_slot = i;
	}
}

////////////////////////////////////////
//
// IMuse mixin interface methods
//
////////////////////////////////////////

void IMuseInternal::on_timer(MidiDriver *midi) {
	Common::StackLock lock(_mutex);
	if (_paused || !_initialized)
		return;

	// CD version only
	if (_game_id == GID_SAMNMAX && strcmp(_vm->_game.variant, "Floppy"))
		musicVolumeReduction(midi);

	if (midi == _midi_native || !_midi_native)
		handleDeferredCommands(midi);
	sequencer_timers(midi);
}

void IMuseInternal::pause(bool paused) {
	Common::StackLock lock(_mutex);
	if (_paused == paused)
		return;
	int vol = _music_volume_eff;
	if (paused)
		_music_volume_eff = 0;
	update_volumes();
	_music_volume_eff = vol;

	// Fix for Bug #1263. The MT-32 apparently fails
	// sometimes to respond to a channel volume message
	// (or only uses it for subsequent note events).
	// The result is hanging notes on pause. Reportedly
	// happens in the original distro, too. To fix that,
	// just send AllNotesOff to the channels.
	if (_midi_native && _native_mt32) {
		for (int i = 0; i < 16; ++i)
			_midi_native->send(123 << 8 | 0xB0 | i);
	}

	_paused = paused;
}

static void syncWithSerializer(Common::Serializer &s, CommandQueue &cq) {
	s.syncArray(cq.array, 8, Common::Serializer::Uint16LE, VER(23));
}

static void syncWithSerializer(Common::Serializer &s, ImTrigger &it) {
	s.syncAsSint16LE(it.sound, VER(54));
	s.syncAsByte(it.id, VER(54));
	s.syncAsUint16LE(it.expire, VER(54));
	s.syncArray(it.command, 8, Common::Serializer::Uint16LE, VER(54));
}

void IMuseInternal::saveLoadIMuse(Common::Serializer &s, ScummEngine *scumm, bool fixAfterLoad) {
	Common::StackLock lock(_mutex);

	int i;

	s.syncAsByte(_queue_end, VER(8));
	s.syncAsByte(_queue_pos, VER(8));
	s.syncAsUint16LE(_queue_sound, VER(8));
	s.syncAsByte(_queue_adding, VER(8));
	s.syncAsByte(_queue_marker, VER(8));
	s.syncAsByte(_queue_cleared, VER(8));
	s.syncAsByte(_master_volume, VER(8));
	s.syncAsUint16LE(_trigger_count, VER(8));
	s.syncAsUint16LE(_snm_trigger_index, VER(54));
	s.syncArray(_channel_volume, 8, Common::Serializer::Uint16LE, VER(8));
	s.syncArray(_volchan_table, 8, Common::Serializer::Uint16LE, VER(8));
	s.syncArray(_cmd_queue, ARRAYSIZE(_cmd_queue), syncWithSerializer);
	s.syncArray(_snm_triggers, ARRAYSIZE(_snm_triggers), syncWithSerializer);

	// The players
	for (i = 0; i < ARRAYSIZE(_players); ++i)
		_players[i].saveLoadWithSerializer(s);

	// The parts
	for (i = 0; i < ARRAYSIZE(_parts); ++i)
		_parts[i].saveLoadWithSerializer(s);

	{
		// Load/save the instrument definitions, which were revamped with V11.
		Part *part = &_parts[0];
		if (s.getVersion() >= VER(11)) {
			for (i = ARRAYSIZE(_parts); i; --i, ++part) {
				part->_instrument.saveLoadWithSerializer(s);
			}
		} else {
			for (i = ARRAYSIZE(_parts); i; --i, ++part)
				part->_instrument.clear();
		}
	}

	// VolumeFader has been replaced with the more generic ParameterFader.
	s.skip(13 * 8, VER(8), VER(16));

	// Normally, we have to fix up the data structures after loading a
	// saved game. But there are cases where we don't. For instance, The
	// Macintosh version of Monkey Island 1 used to convert the Mac0 music
	// resources to General MIDI and play it through iMUSE as a rough
	// approximation. Now it has its own player, but old savegame still
	// have the iMUSE data in them. We have to skip that data, using a
	// dummy iMUSE object, but since the resource is no longer recognizable
	// to iMUSE, the fixup fails hard. So yes, this is a bit of a hack.

	if (s.isLoading() && fixAfterLoad) {
		// Load all sounds that we need
		fix_players_after_load(scumm);
		fix_parts_after_load();
		setImuseMasterVolume(_master_volume);

		if (_midi_native)
			reallocateMidiChannels(_midi_native);
		if (_midi_adlib)
			reallocateMidiChannels(_midi_adlib);
	}
}

bool IMuseInternal::get_sound_active(int sound) const {
	Common::StackLock lock(_mutex);
	return getSoundStatus_internal(sound, false) != 0;
}

int32 IMuseInternal::doCommand(int numargs, int a[]) {
	Common::StackLock lock(_mutex);
	return doCommand_internal(numargs, a);
}


uint32 IMuseInternal::property(int prop, uint32 value) {
	Common::StackLock lock(_mutex);
	switch (prop) {
	case IMuse::PROP_TEMPO_BASE:
		// This is a specified as a percentage of normal
		// music speed. The number must be an integer
		// ranging from 50 to 200(for 50% to 200% normal speed).
		if (value >= 50 && value <= 200)
			_tempoFactor = value;
		break;

	case IMuse::PROP_LIMIT_PLAYERS:
		if (value > 0 && value <= ARRAYSIZE(_players))
			_player_limit = (int)value;
		break;

	case IMuse::PROP_RECYCLE_PLAYERS:
		_recycle_players = (value != 0);
		break;

	case IMuse::PROP_QUALITY:
		if (_midi_native)
			_midi_native->property(IMuse::PROP_QUALITY, value);
		break;

	case IMuse::PROP_MUSICVOLUME:
	case IMuse::PROP_SFXVOLUME:
		if (_midi_native && _lowLevelVolumeControl)
			_midi_native->property(prop, value);
		break;

	default:
		break;
	}

	return 0;
}

void IMuseInternal::addSysexHandler(byte mfgID, sysexfunc handler) {
	// TODO: Eventually support multiple sysEx handlers and pay
	// attention to the client-supplied manufacturer ID.
	Common::StackLock lock(_mutex);
	_sysex = handler;
}

void IMuseInternal::startSoundWithNoteOffset(int sound, int offset) {
	Common::StackLock lock(_mutex);
	startSound_internal(sound, offset);
}

////////////////////////////////////////
//
// MusicEngine interface methods
//
////////////////////////////////////////

void IMuseInternal::setMusicVolume(int vol) {
	if (_lowLevelVolumeControl) {
		property(IMuse::PROP_MUSICVOLUME, vol);
		return;
	}

	Common::StackLock lock(_mutex);
	if (vol > 255)
		vol = 255;
	if (_music_volume == vol)
		return;

	_music_volume = vol;
	_music_volume_eff = _music_volume;

	vol = _master_volume * _music_volume_eff / 255;
	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
}

void IMuseInternal::setSfxVolume(int vol) {
	// This is supported only for drivers that can distinguish music from sound effects at the driver or emulator level.
	// The imuse engine in its early version does not have volume groups. It simply (and successfully for the more relevant
	// targets) relies on sound effects not being played through the imuse engine.
	if (_lowLevelVolumeControl)
		property(IMuse::PROP_SFXVOLUME, vol);
}

void IMuseInternal::startSound(int sound) {
	Common::StackLock lock(_mutex);
	startSound_internal(sound);
}

void IMuseInternal::stopSound(int sound) {
	Common::StackLock lock(_mutex);
	stopSound_internal(sound);
}

void IMuseInternal::stopAllSounds() {
	Common::StackLock lock(_mutex);
	stopAllSounds_internal();
}

int IMuseInternal::getSoundStatus(int sound) const {
	Common::StackLock lock(_mutex);
	return getSoundStatus_internal(sound, true);
}

int IMuseInternal::getMusicTimer() {
	Common::StackLock lock(_mutex);
	int best_time = 0;
	const Player *player = _players;
	for (int i = ARRAYSIZE(_players); i; i--, player++) {
		if (player->isActive()) {
			int timer = player->getMusicTimer();
			if (timer > best_time)
				best_time = timer;
		}
	}
	return best_time;
}

////////////////////////////////////////
//
// Internal versions of the IMuse and
// MusicEngine base class methods.
// These methods assume the appropriate
// mutex locks have already been set,
// and may also have slightly different
// semantics than the interface methods.
//
////////////////////////////////////////

bool IMuseInternal::startSound_internal(int sound, int offset) {
	// Do not start a sound if it is already set to start on an ImTrigger
	// event. This fixes carnival music problems where a sound has been set
	// to trigger at the right time, but then is started up immediately
	// anyway, only to be restarted later when the trigger occurs.
	//
	// However, we have to make sure the sound with the trigger is actually
	// playing, otherwise the music may stop when Sam and Max are thrown
	// out of Bumpusville, because entering the mansion sets up a trigger
	// for a sound that isn't necessarily playing. This is somewhat related
	// to bug #1090.

	int i;
	ImTrigger *trigger = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trigger) {
		if (trigger->sound && trigger->id && trigger->command[0] == 8 && trigger->command[1] == sound && getSoundStatus_internal(trigger->sound, true))
			return false;
	}

	byte *ptr = findStartOfSound(sound);
	if (!ptr) {
		debug(2, "IMuseInternal::startSound(): Couldn't find sound %d", sound);
		return false;
	}

	// Check which MIDI driver this track should use.
	// If it's NULL, it ain't something we can play.
	MidiDriver *driver = getBestMidiDriver(sound);
	if (!driver)
		return false;

	// If the requested sound is already playing, start it over
	// from scratch. This was originally a hack to prevent Sam & Max
	// iMuse messiness while upgrading the iMuse engine, but it
	// is apparently necessary to deal with fade-and-restart
	// race conditions that were observed in MI2. Reference
	// Bug #385 and Patch #8038 (which was reversed to fix
	// an FOA regression: Bug #488).
	Player *player = findActivePlayer(sound);
	if (!player) {
		ptr = findStartOfSound(sound, IMuseInternal::kMDhd);
		player = allocate_player(ptr ? (READ_BE_UINT32(&ptr[4]) && ptr[10] ? ptr[10] : 128) : 128);
	}

	if (!player)
		return false;

	// WORKAROUND: This is to work around a problem at the Dino Bungie
	// Memorial.
	//
	// There are three pieces of music involved here:
	//
	// 80 - Main theme (looping)
	// 81 - Music when entering Rex's and Wally's room (not looping)
	// 82 - Music when listening to Rex or Wally
	//
	// When entering, tune 81 starts, tune 80 is faded down (not out) and
	// a trigger is set in tune 81 to fade tune 80 back up.
	//
	// When listening to Rex or Wally, tune 82 is started, tune 81 is faded
	// out and tune 80 is faded down even further.
	//
	// However, when tune 81 is faded out its trigger will cause tune 80 to
	// fade back up, resulting in two tunes being played simultaneously at
	// full blast. It's no use trying to keep tune 81 playing at volume 0.
	// It doesn't loop, so eventually it will terminate on its own.
	//
	// I don't know how the original interpreter handled this - or even if
	// it handled it at all - but it looks like sloppy scripting to me. Our
	// workaround is to clear the trigger if the player listens to Rex or
	// Wally before tune 81 has finished on its own.

	if (_game_id == GID_SAMNMAX && sound == 82 && getSoundStatus_internal(81, false))
		ImClearTrigger(81, 1);

	// WORKAROUND for monkey2 bug #1410 / Scabb Island
	//
	// Tunes involved:
	// 100 - Captain Dread's map
	// 101 - Woodtick
	// 107 - Map of Scabb Island
	//
	// If you go from Woodtick to the map of Scabb Island tune 107 is added as
	// trigger on 101 and 101 moves to an outro and stop (triggering start
	// of 107). Then at Captain Dread tune 107 is stopped and 100 is started.
	//
	// If you go quickly enough, the trigger occurs not at the Scabb Island
	// map but at Captain Dread causing tune 107 not to be stopped.
	// So we prevent starting 107 if 100 is already running.
	if (_game_id == GID_MONKEY2 && (sound == 107) && (getSoundStatus_internal(100, true) == 1))
		return false;

	// WORKAROUND: In some cases 107 is running and doesn't get killed at Dread's map
	if (_game_id == GID_MONKEY2 && (sound == 100) && (getSoundStatus_internal(107, true) == 1))
		IMuseInternal::stopSound_internal(107);

	// WORKAROUND for monkey2 bug #1410 / Booty Island
	//
	// Tunes involved
	// 100 - Captain Dread's map
	// 113 - Guard Kiosk / Mardi Grass
	// 115 - Map of Booty Island / Ville de la Booty
	// 118 - Ville de la Booty
	//
	// When you enter the Guard Kiosk tune 113 is added as trigger on song
	// 115. Then if you leave, 113 is stopped and 115 is started again.
	// If you leave quickly enough, the trigger occurs on the map and tune
	// 113 will not stop.
	// We kill 113 on entry of one of the other locations (Captain Dread
	// or Ville de la Booty) because tune 115 is not always started.
	if (_game_id == GID_MONKEY2 && (sound == 100 || sound == 115 || sound == 118) && (getSoundStatus_internal(113, true) == 1))
		IMuseInternal::stopSound_internal(113);

	player->clear();
	player->setOffsetNote(offset);
	return player->startSound(sound, driver);
}

int IMuseInternal::stopSound_internal(int sound) {
	int r = -1;
	Player *player = findActivePlayer(sound);
	if (player) {
		player->clear();
		r = 0;
	}
	return r;
}

int IMuseInternal::stopAllSounds_internal() {
	clear_queue();
	Player *player = _players;
	for (int i = ARRAYSIZE(_players); i; i--, player++) {
		if (player->isActive())
			player->clear();
	}
	return 0;
}

int IMuseInternal::getSoundStatus_internal(int sound, bool ignoreFadeouts) const {
	if (_game_id != GID_MONKEY2) {
		// The whole fadeout checking / ignoring is not present in any of the original
		// drivers, but necessary as a WORKAROUND for a bug in Monkey Island 2 that also
		// happens with the original interpreter (bug no. 385: "No music if room
		// transition is too fast"). The bug is caused by sloppy scripting, but probably
		// wouldn't ever be seen on machines of that era, when the loading time for a
		// room change would take longer than the fadeout time.
		// Since the code is objectively wrong and the workaround is not known to be
		// needed elsewhere, we restrict it to Monkey Island 2.
		ignoreFadeouts = false;
	}

	const Player *player = _players;
	for (int i = ARRAYSIZE(_players); i; i--, player++) {
		if (player->isActive() && (!ignoreFadeouts || !player->isFadingOut())) {
			if (sound == -1)
				return player->getID();
			else if (player->getID() == (uint16)sound)
				return 1;
		}
	}
	return (sound == -1) ? 0 : get_queue_sound_status(sound);
}

int32 IMuseInternal::doCommand_internal(int a, int b, int c, int d, int e, int f, int g, int h) {
	int args[8];
	args[0] = a;
	args[1] = b;
	args[2] = c;
	args[3] = d;
	args[4] = e;
	args[5] = f;
	args[6] = g;
	args[7] = h;
	return doCommand_internal(8, args);
}

int32 IMuseInternal::doCommand_internal(int numargs, int a[]) {
	if (numargs < 1)
		return -1;

	int i;
	byte cmd = a[0] & 0xFF;
	byte param = a[0] >> 8;
	Player *player = nullptr;

	if (!_initialized && (cmd || param))
		return -1;

	{
		Common::String string = "doCommand - ";
		string += Common::String::format("%d (%d/%d)", a[0], (int)param, (int)cmd);
		for (i = 1; i < numargs; ++i)
			string += Common::String::format(", %d", a[i]);
		debugC(DEBUG_IMUSE, "%s", string.c_str());
	}

	if (param == 0) {
		switch (cmd) {
		case 6:
			if (a[1] > 127)
				return -1;
			else {
				debug(0, "IMuse doCommand(6) - setImuseMasterVolume (%d)", a[1]);
				return setImuseMasterVolume((a[1] << 1) | (a[1] ? 0 : 1)); // Convert from 0-127 to 0-255
			}
		case 7:
			debug(0, "IMuse doCommand(7) - getMasterVolume (%d)", a[1]);
			return _master_volume / 2; // Convert from 0-255 to 0-127
		case 8:
			return startSound_internal(a[1]) ? 0 : -1;
		case 9:
			return stopSound_internal(a[1]);
		case 10: // FIXME: Sam and Max - Not sure if this is correct
			return stopAllSounds_internal();
		case 11:
			return stopAllSounds_internal();
		case 12:
			// Sam & Max: Player-scope commands
			player = findActivePlayer(a[1]);
			if (player == nullptr)
				return -1;

			switch (a[3]) {
			case 6:
				// Set player volume.
				return player->setVolume(a[4]);
			default:
				error("IMuseInternal::doCommand(12) unsupported sub-command %d", a[3]);
			}
			return -1;
		case 13:
			return getSoundStatus_internal(a[1], true);
		case 14:
			// Sam and Max: Parameter fade
			player = findActivePlayer(a[1]);
			if (player)
				return player->addParameterFader(a[3], a[4], a[5]);
			return -1;

		case 15:
			// Sam & Max: Set hook for a "maybe" jump
			player = findActivePlayer(a[1]);
			if (player) {
				player->setHook(0, a[3], 0);
				return 0;
			}
			return -1;
		case 16:
			debug(0, "IMuse doCommand(16) - set_volchan (%d, %d)", a[1], a[2]);
			return set_volchan(a[1], a[2]);
		case 17:
			if (_game_id != GID_SAMNMAX) {
				debug(0, "IMuse doCommand(17) - set_channel_volume (%d, %d)", a[1], a[2]);
				return set_channel_volume(a[1], a[2]);
			} else {
				if (a[4]) {
					int b[16];
					memset(b, 0, sizeof(b));
					for (i = 0; i < numargs; ++i)
						b[i] = a[i];
					return ImSetTrigger(b[1], b[3], b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11]);
				} else {
					return ImClearTrigger(a[1], a[3]);
				}
			}
		case 18:
			if (_game_id != GID_SAMNMAX) {
				return set_volchan_entry(a[1], a[2]);
			} else {
				// Sam & Max: ImCheckTrigger.
				// According to Mike's notes to Ender,
				// this function returns the number of triggers
				// associated with a particular player ID and
				// trigger ID.
				a[0] = 0;
				for (i = 0; i < ARRAYSIZE(_snm_triggers); ++i) {
					if (_snm_triggers[i].sound == a[1] && _snm_triggers[i].id &&
					        (a[3] == -1 || _snm_triggers[i].id == a[3])) {
						++a[0];
					}
				}
				return a[0];
			}
		case 19:
			// Sam & Max: ImClearTrigger
			// This should clear a trigger that's been set up
			// with ImSetTrigger(cmd == 17). Seems to work....
			return ImClearTrigger(a[1], a[3]);
		case 20:
			// Sam & Max: Deferred Command
			addDeferredCommand(a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			return 0;
		case 2:
		case 3:
			return 0;
		default:
			error("doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		}
	} else if (param == 1) {
		Part *part = nullptr;
		if ((1 << cmd) & 0x783FFF) {
			player = findActivePlayer(a[1]);
			if (!player)
				return -1;
			if (_newSystem && cmd == 5) {
				assert(a[3] >= 0 && a[3] <= 15);
				part = player->getPart(a[2]);
				if (!part)
					return -1;
			} else if (((1 << cmd) & (1 << 11 | 1 << 22))) {
				assert(a[2] >= 0 && a[2] <= 15);
				part = player->getPart(a[2]);
				if (!part)
					return -1;
			}
		}

		switch (cmd) {
		case 0:
			if (_game_id == GID_SAMNMAX) {
				if (a[3] == 1) // Measure number
					return ((player->getBeatIndex() - 1) >> 2) + 1;
				else if (a[3] == 2) // Beat number
					return player->getBeatIndex();
				return -1;
			} else {
				return player->getParam(a[2], a[3]);
			}
		case 1:
			if (_game_id == GID_SAMNMAX) {
				// FIXME: Could someone verify this?
				//
				// This jump instruction is known to be used in
				// the following cases:
				//
				// 1) Going anywhere on the USA map
				// 2) Winning the Wak-A-Rat game
				// 3) Losing or quitting the Wak-A-Rat game
				// 4) Conroy hitting Max with a golf club
				//
				// For all these cases the position parameters
				// are always the same: 2, 1, 0, 0.
				//
				// 5) When leaving the bigfoot party. The
				//    position parameters are: 3, 4, 300, 0
				// 6) At Frog Rock, when the UFO appears. The
				//    position parameters are: 10, 4, 400, 1
				//
				// The last two cases used to be buggy, so I
				// have made a change to how the last two
				// position parameters are handled. I still do
				// not know if it's correct, but it sounds
				// good to me at least.

				debug(0, "doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d)", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
				player->jump(a[3] - 1, (a[4] - 1) * 4 + a[5], a[6] + ((a[7] * player->getTicksPerBeat()) >> 2));
			} else
				player->setPriority(a[2]);
			return 0;
		case 2:
			return player->setVolume(a[2]);
		case 3:
			if (_newSystem)
				player->setSpeed(a[3]);
			else
				player->setPan(a[2]);
			return 0;
		case 4:
			return player->setTranspose(a[2], a[3]);
		case 5:
			if (_newSystem) {
				assert(part);
				part->volControlSensitivity(a[4]);
			} else {
				player->setDetune(a[2]);
			}
			return 0;
		case 6:
			// WORKAROUND for bug #2242. When playing the
			// "flourishes" as Rapp's body appears from his ashes,
			// MI2 sets up triggers to pause the music, in case the
			// animation plays too slowly, and then the music is
			// manually unpaused for the next part of the music.
			//
			// In ScummVM, the animation finishes slightly too
			// quickly, and the pause command is run *after* the
			// unpause command. So we work around it by ignoring
			// all attempts at pausing this particular sound.
			//
			// I could have sworn this wasn't needed after the
			// recent timer change, but now it looks like it's
			// still needed after all.
			if (_game_id != GID_MONKEY2 || player->getID() != 183 || a[2] != 0) {
				player->setSpeed(a[2]);
			}
			return 0;
		case 7:
			return player->jump(a[2], a[3], a[4]) ? 0 : -1;
		case 8:
			return player->scan(a[2], a[3], a[4]);
		case 9:
			return player->setLoop(a[2], a[3], a[4], a[5], a[6]) ? 0 : -1;
		case 10:
			player->clearLoop();
			return 0;
		case 11:
			assert(part);
			part->set_onoff(a[3] != 0);
			return 0;
		case 12:
			return player->setHook(a[2], a[3], a[4]);
		case 13:
			return player->addParameterFader(ParameterFader::pfVolume, a[2], a[3]);
		case 14:
			return enqueue_trigger(a[1], a[2]);
		case 15:
			return enqueue_command(a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
		case 16:
			return clear_queue();
		case 19:
			return player->getParam(a[2], a[3]);
		case 20:
			return player->setHook(a[2], a[3], a[4]);
		case 21:
			return -1;
		case 22:
			assert(part);
			part->volume(a[3]);
			return 0;
		case 23:
			return query_queue(a[1]);
		case 24:
			return 0;
		default:
			error("doCommand(%d [%d/%d], %d, %d, %d, %d, %d, %d, %d) unsupported", a[0], param, cmd, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			return -1;
		}
	}

	return -1;
}

// mixin end

void IMuseInternal::sequencer_timers(MidiDriver *midi) {
	Player *player = _players;
	int i;
	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && player->getMidiDriver() == midi) {
			player->onTimer();
		}
	}
}

void IMuseInternal::handle_marker(uint id, byte data) {
	if ((_queue_end == _queue_pos) || (_queue_adding && _queue_sound == id && data == _queue_marker))
		return;

	uint16 *p = _cmd_queue[_queue_end].array;
	if (p[0] != TRIGGER_ID || id != p[1] || data != p[2])
		return;

	_trigger_count--;
	_queue_cleared = false;
	_queue_end = (_queue_end + 1) % ARRAYSIZE(_cmd_queue);

	while (_queue_end != _queue_pos && _cmd_queue[_queue_end].array[0] == COMMAND_ID && !_queue_cleared) {
		p = _cmd_queue[_queue_end].array;
		doCommand_internal(p[1], p[2], p[3], p[4], p[5], p[6], p[7], 0);
		_queue_end = (_queue_end + 1) % ARRAYSIZE(_cmd_queue);
	}
}

int IMuseInternal::get_channel_volume(uint a) {
	if (a < 8)
		return _channel_volume_eff[a];
	return (_master_volume * _music_volume_eff / 255) / 2;
}

Part *IMuseInternal::allocate_part(byte pri, MidiDriver *midi) {
	Part *part, *best = nullptr;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (!part->_player)
			return part;
		if (pri >= part->_pri_eff) {
			pri = part->_pri_eff;
			best = part;
		}
	}

	if (best) {
		best->uninit();
		reallocateMidiChannels(midi);
	} else {
		debug(1, "Denying part request");
	}
	return best;
}

int IMuseInternal::get_queue_sound_status(int sound) const {
	const uint16 *a;
	int i, j;

	j = _queue_pos;
	i = _queue_end;

	while (i != j) {
		a = _cmd_queue[i].array;
		if (a[0] == COMMAND_ID && a[1] == 8 && a[2] == (uint16)sound)
			return 2;
		i = (i + 1) % ARRAYSIZE(_cmd_queue);
	}

	for (i = 0; i < ARRAYSIZE(_deferredCommands); ++i) {
		if (_deferredCommands[i].time_left && _deferredCommands[i].a == 8 &&
		        _deferredCommands[i].b == sound) {
			return 2;
		}
	}

	return 0;
}

int IMuseInternal::set_volchan(int sound, int volchan) {
	int r;
	int i;
	int num;
	Player *player, *best, *sameid;

	r = get_volchan_entry(volchan);
	if (r == -1)
		return -1;

	if (r >= 8) {
		player = findActivePlayer(sound);
		if (player && player->_vol_chan != (uint16)volchan) {
			player->_vol_chan = volchan;
			player->setVolume(player->getVolume());
			return 0;
		}
		return -1;
	} else {
		best = nullptr;
		num = 0;
		sameid = nullptr;
		for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
			if (player->isActive()) {
				if (player->_vol_chan == (uint16)volchan) {
					num++;
					if (!best || player->getPriority() <= best->getPriority())
						best = player;
				} else if (player->getID() == (uint16)sound) {
					sameid = player;
				}
			}
		}
		if (sameid == nullptr)
			return -1;
		if (best != nullptr && num >= r)
			best->clear();
		sameid->_vol_chan = volchan;
		sameid->setVolume(sameid->getVolume());
		return 0;
	}
}

int IMuseInternal::clear_queue() {
	_queue_adding = false;
	_queue_cleared = true;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
	return 0;
}

int IMuseInternal::enqueue_command(int a, int b, int c, int d, int e, int f, int g) {
	uint16 *p;
	uint i;

	i = _queue_pos;

	if (i == _queue_end)
		return -1;

	if (a == -1) {
		_queue_adding = false;
		_trigger_count++;
		return 0;
	}

	p = _cmd_queue[_queue_pos].array;
	p[0] = COMMAND_ID;
	p[1] = a;
	p[2] = b;
	p[3] = c;
	p[4] = d;
	p[5] = e;
	p[6] = f;
	p[7] = g;

	i = (i + 1) % ARRAYSIZE(_cmd_queue);

	if (_queue_end != i) {
		_queue_pos = i;
		return 0;
	} else {
		_queue_pos = (i - 1) % ARRAYSIZE(_cmd_queue);
		return -1;
	}
}

int IMuseInternal::query_queue(int param) {
	switch (param) {
	case 0: // Get trigger count
		return _trigger_count;
	case 1: // Get trigger type
		if (_queue_end == _queue_pos)
			return -1;
		return _cmd_queue[_queue_end].array[1];
	case 2: // Get trigger sound
		if (_queue_end == _queue_pos)
			return 0xFF;
		return _cmd_queue[_queue_end].array[2];
	default:
		return -1;
	}
}

int IMuseInternal::setImuseMasterVolume(uint vol) {
	if (vol > 255)
		vol = 255;
	if (_master_volume == vol)
		return 0;
	_master_volume = vol;
	vol = _master_volume * _music_volume_eff / 255;
	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * vol / 255;
	}
	if (!_paused)
		update_volumes();
	return 0;
}

int IMuseInternal::enqueue_trigger(int sound, int marker) {
	uint16 *p;
	uint pos;

	pos = _queue_pos;

	p = _cmd_queue[pos].array;
	p[0] = TRIGGER_ID;
	p[1] = sound;
	p[2] = marker;

	pos = (pos + 1) % ARRAYSIZE(_cmd_queue);
	if (_queue_end == pos) {
		_queue_pos = (pos - 1) % ARRAYSIZE(_cmd_queue);
		return -1;
	}

	_queue_pos = pos;
	_queue_adding = true;
	_queue_sound = sound;
	_queue_marker = marker;
	return 0;
}

int32 IMuseInternal::ImSetTrigger(int sound, int id, int a, int b, int c, int d, int e, int f, int g, int h) {
	// Sam & Max: ImSetTrigger.
	// Sets a trigger for a particular player and
	// marker ID, along with doCommand parameters
	// to invoke at the marker. The marker is
	// represented by MIDI SysEx block 00 xx(F7)
	// where "xx" is the marker ID.
	uint16 oldest_trigger = 0;
	ImTrigger *oldest_ptr = nullptr;

	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trig) {
		if (!trig->id)
			break;
		// We used to only compare 'id' and 'sound' here, but at least
		// at the Dino Bungie Memorial that causes the music to stop
		// after getting the T-Rex tooth. See bug #1429.
		if (trig->id == id && trig->sound == sound && trig->command[0] == a)
			break;

		// The wraparound if trig->expire > _snm_trigger_index is intentional
		uint16 diff = _snm_trigger_index - trig->expire;

		if (!oldest_ptr || oldest_trigger < diff) {
			oldest_ptr = trig;
			oldest_trigger = diff;
		}
	}

	// If we didn't find a trigger, see if we can expire one.
	if (!i) {
		if (!oldest_ptr)
			return -1;
		trig = oldest_ptr;
	}

	trig->id = id;
	trig->sound = sound;
	trig->expire = (++_snm_trigger_index & 0xFFFF);
	trig->command[0] = a;
	trig->command[1] = b;
	trig->command[2] = c;
	trig->command[3] = d;
	trig->command[4] = e;
	trig->command[5] = f;
	trig->command[6] = g;
	trig->command[7] = h;

	// If the command is to start a sound, stop that sound if it's already playing.
	// This fixes some carnival music problems.
	// NOTE: We ONLY do this if the sound that will trigger the command is actually
	// playing. Otherwise, there's a problem when exiting and re-entering the
	// Bumpusville mansion. Ref Bug #1090.
	if (trig->command[0] == 8 && getSoundStatus_internal(trig->command[1], true) && getSoundStatus_internal(sound, true))
		stopSound_internal(trig->command[1]);
	return 0;
}

int32 IMuseInternal::ImClearTrigger(int sound, int id) {
	int count = 0;
	int i;
	ImTrigger *trig = _snm_triggers;
	for (i = ARRAYSIZE(_snm_triggers); i; --i, ++trig) {
		if ((sound == -1 || trig->sound == sound) && trig->id && (id == -1 || trig->id == id)) {
			trig->sound = trig->id = 0;
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int32 IMuseInternal::ImFireAllTriggers(int sound) {
	if (!sound)
		return 0;
	int count = 0;
	int i;
	for (i = 0; i < ARRAYSIZE(_snm_triggers); ++i) {
		if (_snm_triggers[i].sound == sound) {
			_snm_triggers[i].sound = _snm_triggers[i].id = 0;
			doCommand_internal(8, _snm_triggers[i].command);
			++count;
		}
	}
	return (count > 0) ? 0 : -1;
}

int IMuseInternal::set_channel_volume(uint chan, uint vol) {
	if (chan >= 8 || vol > 127)
		return -1;

	_channel_volume[chan] = vol;
	_channel_volume_eff[chan] = _master_volume * _music_volume_eff * vol / 255 / 255;
	update_volumes();
	return 0;
}

void IMuseInternal::update_volumes() {
	Player *player;
	int i;

	for (i = ARRAYSIZE(_players), player = _players; i != 0; i--, player++) {
		if (player->isActive())
			player->setVolume(player->getVolume());
	}
}

void IMuseInternal::musicVolumeReduction(MidiDriver *midi) {
	int curVol;
	int curEffVol;
	int factor = 2; // The music volume variables are 0-255, and we need 0-127

	if (_paused)
		return;

	_musicVolumeReductionTimer += midi->getBaseTempo();
	while (_musicVolumeReductionTimer >= MUS_REDUCTION_TIMER_TICKS) {
		_musicVolumeReductionTimer -= MUS_REDUCTION_TIMER_TICKS;
		curVol = _music_volume / factor;

		if (_vm->_sound->speechIsPlaying())
			curVol = (90 * curVol) >> 7;

		curEffVol = _music_volume_eff / factor;

		// The reduction curve is pretty slow, but running
		// the original through a debugger shows the same behavior...
		if (curEffVol > curVol)
			_music_volume_eff = (curEffVol - 1) * factor;

		if (curEffVol < curVol)
			_music_volume_eff = (curEffVol + 1) * factor;
	}

	for (uint i = 0; i < ARRAYSIZE(_channel_volume); i++) {
		_channel_volume_eff[i] = _channel_volume[i] * (_master_volume * _music_volume_eff / 255) / 255;
	}

	update_volumes();
}

int IMuseInternal::set_volchan_entry(uint a, uint b) {
	if (a >= 8)
		return -1;
	_volchan_table[a] = b;
	return 0;
}

int HookDatas::query_param(int param, byte chan) {
	switch (param) {
	case 18:
		return _jump[0];
	case 19:
		return _transpose;
	case 20:
		return _part_onoff[chan];
	case 21:
		return _part_volume[chan];
	case 22:
		return _part_program[chan];
	case 23:
		return _part_transpose[chan];
	default:
		return -1;
	}
}

int HookDatas::set(byte cls, byte value, byte chan) {
	switch (cls) {
	case 0:
		if (value != _jump[0]) {
			_jump[1] = _jump[0];
			_jump[0] = value;
		}
		break;
	case 1:
		_transpose = value;
		break;
	case 2:
		if (chan < 16)
			_part_onoff[chan] = value;
		else if (chan == 16)
			memset(_part_onoff, value, 16);
		break;
	case 3:
		if (chan < 16)
			_part_volume[chan] = value;
		else if (chan == 16)
			memset(_part_volume, value, 16);
		break;
	case 4:
		if (chan < 16)
			_part_program[chan] = value;
		else if (chan == 16)
			memset(_part_program, value, 16);
		break;
	case 5:
		if (chan < 16)
			_part_transpose[chan] = value;
		else if (chan == 16)
			memset(_part_transpose, value, 16);
		break;
	default:
		return -1;
	}
	return 0;
}

Player *IMuseInternal::findActivePlayer(int id) {
	int i;
	Player *player = _players;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive() && player->getID() == (uint16)id)
			return player;
	}
	return nullptr;
}

int IMuseInternal::get_volchan_entry(uint a) {
	if (a < 8)
		return _volchan_table[a];
	return -1;
}

IMuseInternal *IMuseInternal::create(ScummEngine *vm, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver, MidiDriverFlags sndType, bool nativeMT32) {
	IMuseInternal *i = new IMuseInternal(vm, sndType, nativeMT32);
	i->initialize(vm->_system, nativeMidiDriver, adlibMidiDriver);
	return i;
}

int IMuseInternal::initialize(OSystem *syst, MidiDriver *native_midi, MidiDriver *adlib_midi) {
	int i;

	_midi_native = native_midi;
	_midi_adlib = adlib_midi;
	if (native_midi != nullptr) {
		_timer_info_native.imuse = this;
		_timer_info_native.driver = native_midi;
		initMidiDriver(&_timer_info_native);
	}
	if (adlib_midi != nullptr) {
		_timer_info_adlib.imuse = this;
		_timer_info_adlib.driver = adlib_midi;
		initMidiDriver(&_timer_info_adlib);
	}

	if (!_tempoFactor)
		_tempoFactor = 100;
	_master_volume = 255;

	if (_lowLevelVolumeControl)
		_music_volume = _music_volume_eff = 255;

	for (i = 0; i != 8; i++)
		_channel_volume[i] = _channel_volume_eff[i] = _volchan_table[i] = 127;

	init_players();
	init_queue();
	init_parts();

	_initialized = true;

	return 0;
}

void IMuseInternal::initMidiDriver(TimerCallbackInfo *info) {
	// Open MIDI driver
	int result = info->driver->open();
	if (result)
		error("IMuse initialization - %s", MidiDriver::getErrorName(result));

	// Connect to the driver's timer
	info->driver->setTimerCallback(info, &IMuseInternal::midiTimerCallback);
}

void IMuseInternal::init_queue() {
	_queue_adding = false;
	_queue_pos = 0;
	_queue_end = 0;
	_trigger_count = 0;
}

void IMuseInternal::handleDeferredCommands(MidiDriver *midi) {
	uint32 advance = midi->getBaseTempo();

	DeferredCommand *ptr = &_deferredCommands[0];
	int i;
	for (i = ARRAYSIZE(_deferredCommands); i; --i, ++ptr) {
		if (!ptr->time_left)
			continue;
		if (ptr->time_left <= advance) {
			doCommand_internal(ptr->a, ptr->b, ptr->c, ptr->d, ptr->e, ptr->f, 0, 0);
			ptr->time_left = advance;
		}
		ptr->time_left -= advance;
	}
}

// "time" is interpreted as hundredths of a second.
// FIXME: Is that correct?
// We convert it to microseconds before prceeding
void IMuseInternal::addDeferredCommand(int time, int a, int b, int c, int d, int e, int f) {
	DeferredCommand *ptr = &_deferredCommands[0];
	int i;
	for (i = ARRAYSIZE(_deferredCommands); i; --i, ++ptr) {
		if (!ptr->time_left)
			break;
	}

	if (i) {
		ptr->time_left = time * 10000;
		ptr->a = a;
		ptr->b = b;
		ptr->c = c;
		ptr->d = d;
		ptr->e = e;
		ptr->f = f;
	}
}

void IMuseInternal::fix_parts_after_load() {
	Part *part;
	int i;

	for (i = ARRAYSIZE(_parts), part = _parts; i != 0; i--, part++) {
		if (part->_player)
			part->fix_after_load();
	}
}

// Only call this routine from the main thread,
// since it uses getResourceAddress
void IMuseInternal::fix_players_after_load(ScummEngine *scumm) {
	Player *player = _players;
	int i;

	for (i = ARRAYSIZE(_players); i != 0; i--, player++) {
		if (player->isActive()) {
			scumm->getResourceAddress(rtSound, player->getID());
			player->fixAfterLoad();
		}
	}
}

////////////////////////////////////////
//
// Some more IMuseInternal stuff
//
////////////////////////////////////////

void IMuseInternal::midiTimerCallback(void *data) {
	TimerCallbackInfo *info = (TimerCallbackInfo *)data;
	info->imuse->on_timer(info->driver);
}

MidiChannel *IMuseInternal::allocateChannel(MidiDriver *midi, byte prio) {
	MidiChannel *mc = midi->allocateChannel();
	if (mc)
		return mc;

	Part *best = nullptr;
	for (Part *part = _parts; part < &_parts[ARRAYSIZE(_parts)]; ++part) {
		if (!part->_percussion && part->_mc && part->_mc->device() == midi && part->_pri_eff <= prio) {
			prio = part->_pri_eff;
			best = part;
		}
	}

	if (best) {
		best->off();
		suspendPart(best);
		mc = midi->allocateChannel();
	}

	return mc;
}

bool IMuseInternal::reassignChannelAndResumePart(MidiChannel *mc) {
	while (!_waitingPartsQueue.empty()) {
		Part *part = _waitingPartsQueue.remove_at(0);
		if (part->_player) {
			part->_mc = mc;
			part->sendAll();
			return true;
		}
	}

	return false;
}

void IMuseInternal::suspendPart(Part *part) {
	if (_waitingPartsQueue.empty()) {
		_waitingPartsQueue.push_back(part);
		return;
	}

	for (Common::Array<Part*>::iterator it = _waitingPartsQueue.begin(); it != _waitingPartsQueue.end(); ++it) {
		if ((*it)->_pri_eff > part->_pri_eff)
			continue;
		_waitingPartsQueue.insert(it, part);
		return;
	}
}

void IMuseInternal::removeSuspendedPart(Part *part) {
	for (Common::Array<Part*>::iterator it = _waitingPartsQueue.begin(); it != _waitingPartsQueue.end(); ++it) {
		if (*it != part)
			continue;
		_waitingPartsQueue.erase(it);
		return;
	}
}

void IMuseInternal::reallocateMidiChannels(MidiDriver *midi) {
	Part *part, *hipart;
	int i;
	byte hipri, lopri;
	Part *lopart;

	if (!_dynamicChanAllocation)
		return;

	while (true) {
		hipri = 0;
		hipart = nullptr;
		for (i = 32, part = _parts; i; i--, part++) {
			if (part->_player && part->_player->getMidiDriver() == midi &&
			        !part->_percussion && part->_on &&
			        !part->_mc && part->_pri_eff >= hipri) {
				hipri = part->_pri_eff;
				hipart = part;
			}
		}

		if (!hipart)
			return;

		if ((hipart->_mc = midi->allocateChannel()) == nullptr) {
			lopri = 255;
			lopart = nullptr;
			for (i = 32, part = _parts; i; i--, part++) {
				if (part->_mc && part->_mc->device() == midi && part->_pri_eff <= lopri) {
					lopri = part->_pri_eff;
					lopart = part;
				}
			}

			if (lopart == nullptr || lopri >= hipri)
				return;
			lopart->off();

			if ((hipart->_mc = midi->allocateChannel()) == nullptr)
				return;
		}
		hipart->sendAll();
	}
}

void IMuseInternal::setGlobalInstrument(byte slot, byte *data) {
	if (slot < 32) {
		if (_soundType == MDT_PCSPK)
			_global_instruments[slot].pcspk(data);
		else
			_global_instruments[slot].adlib(data);
	}
}

void IMuseInternal::copyGlobalInstrument(byte slot, Instrument *dest) {
	if (slot >= 32)
		return;

	// Both the AdLib code and the PC Speaker code use an all zero instrument
	// as default in the original, thus we do the same.
	// PC Speaker instrument size is 23, while AdLib instrument size is 30.
	// Thus we just use a 30 byte instrument data array as default.
	const byte defaultInstr[30] = { 0 };

	if (_global_instruments[slot].isValid()) {
		// In case we have a valid instrument set up, copy it to the part.
		_global_instruments[slot].copy_to(dest);
	} else if (_soundType == MDT_PCSPK) {
		debug(0, "Trying to use non-existent global PC Speaker instrument %d", slot);
		dest->pcspk(defaultInstr);
	} else {
		debug(0, "Trying to use non-existent global AdLib instrument %d", slot);
		dest->adlib(defaultInstr);
	}
}



/**
 * IMuseInternal factory creation method.
 * This method provides a means for creating an IMuse
 * implementation without requiring that the details
 * of that implementation be exposed to the client
 * through a header file. This allows the internals
 * of the implementation to be changed and updated
 * without requiring a recompile of the client code.
 */
IMuse *IMuse::create(ScummEngine *vm, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver, MidiDriverFlags sndType, bool nativeMT32) {
	IMuseInternal *engine = IMuseInternal::create(vm, nativeMidiDriver, adlibMidiDriver, sndType, nativeMT32);
	return engine;
}

} // End of namespace Scumm
