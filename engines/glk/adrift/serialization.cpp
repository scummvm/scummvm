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

#include "glk/adrift/serialization.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_char NEWLINE = '\n';
static const sc_char CARRIAGE_RETURN = '\r';
static const sc_char NUL = '\0';

enum { BUFFER_SIZE = 4096 };

void SaveSerializer::save() {
	const sc_var_setref_t vars = gs_get_vars(_game);
	const sc_prop_setref_t bundle = gs_get_bundle(_game);
	sc_vartype_t vt_key[3];
	sc_int index_, var_count;

	// Write the _game name
	vt_key[0].string = "Globals";
	vt_key[1].string = "GameName";
	writeString(prop_get_string(bundle, "S<-ss", vt_key));

	/* Write the counts of rooms, objects, etc. */
	writeInt(gs_room_count(_game));
	writeInt(gs_object_count(_game));
	writeInt(gs_task_count(_game));
	writeInt(gs_event_count(_game));
	writeInt(gs_npc_count(_game));

	/* Write the score and player information. */
	writeInt(_game->score);
	writeInt(gs_playerroom(_game) + 1);
	writeInt(gs_playerparent(_game));
	writeInt(gs_playerposition(_game));

	/* Write player gender. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "PlayerGender";
	writeInt(prop_get_integer(bundle, "I<-ss", vt_key));

	/*
	 * Write encumbrance details. The player limits are constant for a given
	 * _game, and can be extracted from properties.  The current sizes and
	 * weights can also be recalculated from held objects, so we don't maintain
	 * them in the _game.  We can write constants here, then, and ignore
	 * the values on restoring.  Note however that if the Adrift Runner is
	 * relying on these values, this may give it problems with one of our saved
	 * games.
	 */
	writeInt(90);
	writeInt(0);
	writeInt(90);
	writeInt(0);

	/* Save rooms information. */
	for (index_ = 0; index_ < gs_room_count(_game); index_++)
		writeBool(gs_room_seen(_game, index_));

	/* Save objects information. */
	for (index_ = 0; index_ < gs_object_count(_game); index_++) {
		writeInt(gs_object_position(_game, index_));
		writeBool(gs_object_seen(_game, index_));
		writeInt(gs_object_parent(_game, index_));
		if (gs_object_openness(_game, index_) != 0)
			writeInt(gs_object_openness(_game, index_));

		if (gs_object_state(_game, index_) != 0)
			writeInt(gs_object_state(_game, index_));

		writeBool(gs_object_unmoved(_game, index_));
	}

	/* Save tasks information. */
	for (index_ = 0; index_ < gs_task_count(_game); index_++) {
		writeBool(gs_task_done(_game, index_));
		writeBool(gs_task_scored(_game, index_));
	}

	/* Save events information. */
	for (index_ = 0; index_ < gs_event_count(_game); index_++) {
		sc_int startertype, task;

		/* Get starter task, if any. */
		vt_key[0].string = "Events";
		vt_key[1].integer = index_;
		vt_key[2].string = "StarterType";
		startertype = prop_get_integer(bundle, "I<-sis", vt_key);
		if (startertype == 3) {
			vt_key[2].string = "TaskNum";
			task = prop_get_integer(bundle, "I<-sis", vt_key);
		}
		else
			task = 0;

		/* Save event details. */
		writeInt(gs_event_time(_game, index_));
		writeInt(task);
		writeInt(gs_event_state(_game, index_) - 1);
		if (task > 0)
			writeBool(gs_task_done(_game, task - 1));
		else
			writeBool(FALSE);
	}

	/* Save NPCs information. */
	for (index_ = 0; index_ < gs_npc_count(_game); index_++) {
		sc_int walk;

		writeInt(gs_npc_location(_game, index_));
		writeBool(gs_npc_seen(_game, index_));
		for (walk = 0; walk < gs_npc_walkstep_count(_game, index_); walk++)
			writeIntSpecial(gs_npc_walkstep(_game, index_, walk));
	}

	/* Save each variable. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	for (index_ = 0; index_ < var_count; index_++) {
		const sc_char *name;
		sc_int var_type;

		vt_key[1].integer = index_;

		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Type";
		var_type = prop_get_integer(bundle, "I<-sis", vt_key);

		switch (var_type) {
		case TAFVAR_NUMERIC:
			writeInt(var_get_integer(vars, name));
			break;

		case TAFVAR_STRING:
			writeString(var_get_string(vars, name));
			break;

		default:
			sc_fatal("ser_save_game: unknown variable type, %ld\n", var_type);
		}
	}

	/* Save timing information. */
	writeUint(var_get_elapsed_seconds(vars));

	/* Save turns count. */
	writeUint((sc_uint)_game->turns);

	/*
	 * Flush the last buffer contents, and drop the callback and opaque
	 * references.
	 */
	flush(TRUE);
	_callback = NULL;
	_opaque = NULL;
}

void SaveSerializer::flush(sc_bool is_final) {
	if (is_final) {
		_callback(_opaque, _buffer.getData(), _buffer.size());
	}
}

void SaveSerializer::writeChar(sc_char character) {
	// Validate the buffer hasn't exceeded the maximum allowable size
	if (_buffer.size() == BUFFER_SIZE)
		sc_error("Ran out of serialization buffer");

	// Add to the buffer
	_buffer.writeByte(character);
}

void SaveSerializer::write(const sc_char *buffer, sc_int length) {
	// Add each character to the buffer
	for (int idx = 0; idx < length; ++idx)
		writeChar(buffer[idx]);
}

void SaveSerializer::writeString(const sc_char *string) {
	// Write string, followed by DOS style end-of-line
	write(string, strlen(string));
	writeChar(CARRIAGE_RETURN);
	writeChar(NEWLINE);
}

void SaveSerializer::writeInt(sc_int value) {
	Common::String s = Common::String::format("%ld", value);
	writeString(s.c_str());
}

void SaveSerializer::writeIntSpecial(sc_int value) {
	Common::String s = Common::String::format("% ld ", value);
	writeString(s.c_str());
}

void SaveSerializer::writeUint(sc_uint value) {
	Common::String s = Common::String::format("%lu", value);
	writeString(s.c_str());
}

void SaveSerializer::writeBool(sc_bool boolean) {
	// Write a 1 for TRUE, 0 for FALSE
	writeString(boolean ? "1" : "0");
}

/*--------------------------------------------------------------------------*/

/* TAS input file line counter. */
static sc_tafref_t ser_tas = NULL;
static sc_int ser_tasline = 0;

/* Restore error jump buffer. */
static jmp_buf ser_tas_error;
static sc_var_setref_t new_vars;  /* For setjmp safety */
static sc_gameref_t new_game;     /* For setjmp safety */

bool LoadSerializer::load() {
	const sc_filterref_t filter = gs_get_filter(_game);
	const sc_prop_setref_t bundle = gs_get_bundle(_game);
	sc_vartype_t vt_key[3];
	sc_int index_, var_count;
	const sc_char *gamename;

	/* Create a TAF (TAS) reference from callbacks, for reader functions. */
	ser_tas = taf_create_tas(_callback, _opaque);
	if (!ser_tas)
		return FALSE;

	/* Reset line counter for error messages. */
	ser_tasline = 1;

	new_game = NULL;
	new_vars = NULL;

	/* Set up error handling jump buffer, and handle errors. */
	if (setjmp(ser_tas_error) != 0) {
		/* Destroy any temporary _game and variables. */
		if (new_game)
			gs_destroy(new_game);
		if (new_vars)
			var_destroy(new_vars);

		/* Destroy the TAF (TAS) file and return fail status. */
		taf_destroy(ser_tas);
		ser_tas = NULL;
		return FALSE;
	}

	/*
	 * Read the _game name, and compare with the one in the _game.  Fail if
	 * they don't match exactly.  A tighter check than this would perhaps be
	 * preferable, say, something based on the TAF file header, but this isn't
	 * in the save file format.
	 */
	vt_key[0].string = "Globals";
	vt_key[1].string = "GameName";
	gamename = prop_get_string(bundle, "S<-ss", vt_key);
	if (strcmp(readString(), gamename) != 0)
		longjmp(ser_tas_error, 1);

	/* Read and verify the counts in the saved _game. */
	if (readInt() != gs_room_count(_game)
		|| readInt() != gs_object_count(_game)
		|| readInt() != gs_task_count(_game)
		|| readInt() != gs_event_count(_game)
		|| readInt() != gs_npc_count(_game))
		longjmp(ser_tas_error, 1);

	/* Create a variables set and _game to restore into. */
	new_vars = var_create(bundle);
	new_game = gs_create(new_vars, bundle, filter);
	var_register_game(new_vars, new_game);

	/* All set to load TAF (TAS) data into the new _game. */

	/* Restore the score and player information. */
	new_game->score = readInt();
	gs_set_playerroom(new_game, readInt() - 1);
	gs_set_playerparent(new_game, readInt());
	gs_set_playerposition(new_game, readInt());

	/* Skip player gender. */
	(void)readInt();

	/* Skip encumbrance details, not currently maintained by the _game. */
	(void)readInt();
	(void)readInt();
	(void)readInt();
	(void)readInt();

	/* Restore rooms information. */
	for (index_ = 0; index_ < gs_room_count(new_game); index_++)
		gs_set_room_seen(new_game, index_, readBool());

	/* Restore objects information. */
	for (index_ = 0; index_ < gs_object_count(new_game); index_++) {
		sc_int openable, currentstate;

		/* Bypass mutators for position and parent.  Fix later? */
		new_game->objects[index_].position = readInt();
		gs_set_object_seen(new_game, index_, readBool());
		new_game->objects[index_].parent = readInt();

		vt_key[0].string = "Objects";
		vt_key[1].integer = index_;
		vt_key[2].string = "Openable";
		openable = prop_get_integer(bundle, "I<-sis", vt_key);
		gs_set_object_openness(new_game, index_,
			openable != 0 ? readInt() : 0);

		vt_key[2].string = "CurrentState";
		currentstate = prop_get_integer(bundle, "I<-sis", vt_key);
		gs_set_object_state(new_game, index_,
			currentstate != 0 ? readInt() : 0);

		gs_set_object_unmoved(new_game, index_, readBool());
	}

	/* Restore tasks information. */
	for (index_ = 0; index_ < gs_task_count(new_game); index_++) {
		gs_set_task_done(new_game, index_, readBool());
		gs_set_task_scored(new_game, index_, readBool());
	}

	/* Restore events information. */
	for (index_ = 0; index_ < gs_event_count(new_game); index_++) {
		sc_int startertype, task;

		/* Restore first event details. */
		gs_set_event_time(new_game, index_, readInt());
		task = readInt();
		gs_set_event_state(new_game, index_, readInt() + 1);

		/* Verify and restore the starter task, if any. */
		if (task > 0) {
			vt_key[0].string = "Events";
			vt_key[1].integer = index_;
			vt_key[2].string = "StarterType";
			startertype = prop_get_integer(bundle, "I<-sis", vt_key);
			if (startertype != 3)
				longjmp(ser_tas_error, 1);

			/* Restore task state. */
			gs_set_task_done(new_game, task - 1, readBool());
		}
		else
			(void)readBool();
	}

	/* Restore NPCs information. */
	for (index_ = 0; index_ < gs_npc_count(new_game); index_++) {
		sc_int walk;

		gs_set_npc_location(new_game, index_, readInt());
		gs_set_npc_seen(new_game, index_, readBool());
		for (walk = 0; walk < gs_npc_walkstep_count(new_game, index_); walk++)
			gs_set_npc_walkstep(new_game, index_, walk, readInt());
	}

	/* Restore each variable. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	for (index_ = 0; index_ < var_count; index_++) {
		const sc_char *name;
		sc_int var_type;

		vt_key[1].integer = index_;

		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Type";
		var_type = prop_get_integer(bundle, "I<-sis", vt_key);

		switch (var_type) {
		case TAFVAR_NUMERIC:
			var_put_integer(new_vars, name, readInt());
			break;

		case TAFVAR_STRING:
			var_put_string(new_vars, name, readString());
			break;

		default:
			sc_fatal("ser_load_game: unknown variable type, %ld\n", var_type);
		}
	}

	/* Restore timing information. */
	var_set_elapsed_seconds(new_vars, readUint());

	/* Restore turns count. */
	new_game->turns = (sc_int)readUint();

	/*
	 * Resources tweak -- set requested to match those in the current _game
	 * so that they remain unchanged by the gs_copy() of new_game onto
	 * _game.  This way, both the requested and the active resources in the
	 * _game are unchanged by restore.
	 */
	new_game->requested_sound = _game->requested_sound;
	new_game->requested_graphic = _game->requested_graphic;

	/*
	 * Quitter tweak -- set the quit jump buffer in the new _game to be the
	 * same as the current one, so that it remains unchanged by gs_copy().  The
	 * one in the new _game is still the unset one from gs_create().
	 */
	memcpy(&new_game->quitter, &_game->quitter, sizeof(_game->quitter));

	/*
	 * If we got this far, we successfully restored the _game from the file.
	 * As our final act, copy the new _game onto the old one.
	 */
	new_game->temporary = _game->temporary;
	new_game->undo = _game->undo;
	gs_copy(_game, new_game);

	/* Done with the temporary _game and variables. */
	gs_destroy(new_game);
	var_destroy(new_vars);

	/* Done with TAF (TAS) file; destroy it and return successfully. */
	taf_destroy(ser_tas);
	ser_tas = NULL;
	return TRUE;
}

const sc_char *LoadSerializer::readString() {
	const sc_char *string;

	/* Get the next line, and complain if absent. */
	string = taf_next_line(ser_tas);
	if (!string) {
		sc_error("readString: out of TAS data at line %ld\n", ser_tasline);
		longjmp(ser_tas_error, 1);
	}

	ser_tasline++;
	return string;
}

sc_int LoadSerializer::readInt() {
	const sc_char *string;
	sc_int value;

	// Get line, and scan for a single integer; return it
	string = readString();
	if (sscanf(string, "%ld", &value) != 1) {
		sc_error("readInt: invalid integer at line %ld\n", ser_tasline - 1);
		longjmp(ser_tas_error, 1);
	}

	return value;
}

sc_uint LoadSerializer::readUint() {
	const sc_char *string;
	sc_uint value;

	// Get line, and scan for a single integer; return it
	string = readString();
	if (sscanf(string, "%lu", &value) != 1) {
		sc_error("readUint: invalid integer at line %ld\n", ser_tasline - 1);
		longjmp(ser_tas_error, 1);
	}

	return value;
}

sc_bool LoadSerializer::readBool(void) {
	const sc_char *string;
	sc_uint value;

	// Get line, and scan for a single integer; check it's a valid-looking flag, and return it.
	string = readString();
	if (sscanf(string, "%lu", &value) != 1) {
		sc_error("readBool:"
		         " invalid boolean at line %ld\n", ser_tasline - 1);
		longjmp(ser_tas_error, 1);
	}
	if (value != 0 && value != 1) {
		sc_error("readBool:"
		         " warning: suspect boolean at line %ld\n", ser_tasline - 1);
	}

	return value != 0;
}

} // End of namespace Adrift
} // End of namespace Glk
