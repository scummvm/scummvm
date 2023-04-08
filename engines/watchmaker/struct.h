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

#ifndef WATCHMAKER_STRUCT_H
#define WATCHMAKER_STRUCT_H

#include "watchmaker/t3d.h"
#include "watchmaker/types.h"
#include "watchmaker/sysdef.h"
#include "common/stream.h"
#include "watchmaker/message.h"

namespace Watchmaker {

class SerializableAsset {
public:
	virtual ~SerializableAsset() {}
	virtual void loadFromStream(Common::SeekableReadStream &stream) = 0;
};

template<uint32 strSize>
class SerializableString : public SerializableAsset {
	char str[strSize] = {};
public:
	char *c_str() {
		return str;
	}
	void loadFromStream(Common::SeekableReadStream &stream) override {
		stream.read(str, strSize);
	};
};

template<typename T>
void loadArrayFromStream(Common::SeekableReadStream &stream, T array[], uint32 size) {
	for (uint32 i = 0; i < size; i++) {
		array[i].loadFromStream(stream);
	}
}

template<>
inline void loadArrayFromStream(Common::SeekableReadStream &stream, uint8 array[], uint32 size) {
	for (uint32 i = 0; i < size; i++) {
		array[i] = stream.readByte();
	}
}

template<>
inline void loadArrayFromStream(Common::SeekableReadStream &stream, uint16 array[], uint32 size) {
	for (uint32 i = 0; i < size; i++) {
		array[i] = stream.readUint16LE();
	}
}

template<>
inline void loadArrayFromStream(Common::SeekableReadStream &stream, int32 array[], uint32 size) {
	for (uint32 i = 0; i < size; i++) {
		array[i] = stream.readSint32LE();
	}
}

template<typename T, uint32 arraySize>
class SerializableArray : public SerializableAsset {
	T _array[arraySize]; // Can't do initializer here, as GCC 4.8 crashes in that case
public:
	SerializableArray() : _array{} {}
	T &operator[](int index) {
		return _array[index];
	};
	T *rawArray() {
		return _array;
	}
	uint32 size() const {
		return arraySize;
	}
	void loadFromStream(Common::SeekableReadStream &stream) override {
		loadArrayFromStream(stream, _array, arraySize);
	}
};

template<typename T>
class SerializableDynamicArray : public SerializableAsset {
	T *_array = nullptr;
	uint32 _size = 0;
public:
	SerializableDynamicArray() : SerializableDynamicArray(0) {} // TODO
	SerializableDynamicArray(uint32 size) : _size(size) {
		_array = new T[size] {};
	}
	~SerializableDynamicArray() {
		delete[] _array;
	}
	uint32 size() const {
		return _size;
	}
	T &operator[](int index) {
		return _array[index];
	};
	void loadFromStream(Common::SeekableReadStream &stream) override {
		loadArrayFromStream(stream, _array, _size);
	}
};

struct SRoom : public SerializableAsset {
	uint8 name[T3D_NAMELEN];                   // nome della stanza
	char desc[64];                            // descrizione della stanza
	uint16 flags;                               // DONE | EXTRA
	uint16 env;                                 // EAX preset environment number
	uint16 music;                               // Music for this room
	SerializableArray<uint16, MAX_OBJS_IN_ROOM>  objects;           // oggetti nella stanza rivelabili dal mouse
	SerializableArray<uint16, MAX_SOUNDS_IN_ROOM>  sounds;          // sounds per room
	SerializableArray<uint16, MAX_ANIMS_IN_ROOM>  anims;            // animazioni per room
	SerializableArray<uint16, MAX_ACTIONS_IN_ROOM>  actions;        // azioni per room

	void loadFromStream(Common::SeekableReadStream &stream) override {
		stream.read(name, T3D_NAMELEN);
		stream.read(desc, 64);
		flags = stream.readUint16LE();
		env = stream.readUint16LE();
		music = stream.readUint16LE();
		objects.loadFromStream(stream);
		sounds.loadFromStream(stream);
		anims.loadFromStream(stream);
		actions.loadFromStream(stream);
	}
};

struct SObject : public SerializableAsset {
	uint16 name;                            // frase nome
	SerializableArray<uint16, MAX_PLAYERS> examine;            // frase esamina
	SerializableArray<uint16, MAX_PLAYERS> action;             // frase azione
	SerializableArray<uint16, MAX_OBJ_USER_SENTS> text;        // Frasi user
	SerializableArray<uint16, MAX_PLAYERS> anim;               // animazione su action
	SerializableArray<uint16, MAX_PLAYERS> anim2;              // animazione aggiunta (su examine)
	uint8  room;                            // Stanza in cui si trova l'oggetto
	uint8  goroom;                          // Se direzione num stanza - se persona num dialog
	uint8  ninv;                            // ptr inventario
	uint16 flags;                           // EXAMINE | EXAMINEACT | ROOM | PERSON | TAKE | USEWITH | EXTRA | EXTRA2 | DONE | ON | HIDE
	uint8  pos;                             // 0 se no position
	SerializableArray<SerializableArray<uint8, T3D_NAMELEN>, MAX_OBJ_MESHLINKS>  meshlink_int = {};// link -> oggetto mesh (nome mesh) -> da rimuovere
	uint8 *meshlink[MAX_OBJ_MESHLINKS];

	void loadFromStream(Common::SeekableReadStream &stream) override {
		name = stream.readUint16LE();
		examine.loadFromStream(stream);
		action.loadFromStream(stream);
		text.loadFromStream(stream);
		anim.loadFromStream(stream);
		anim2.loadFromStream(stream);
		room = stream.readByte();
		goroom = stream.readByte();
		ninv = stream.readByte();
		flags = stream.readUint16LE();
		pos = stream.readByte();
		meshlink_int.loadFromStream(stream);
		// HACK:
		for (int i = 0; i < MAX_OBJ_MESHLINKS; i++) {
			meshlink[i] = meshlink_int.rawArray()[i].rawArray();
		}
	}
};

struct SInvObject : public SerializableAsset {
	uint16 name;                            // Nome Oggetto in inventario
	SerializableArray<uint16, MAX_PLAYERS> examine;            // Frase se si esamina
	SerializableArray<uint16, MAX_PLAYERS> action;             // Frase se si opera
	uint16 flags;                           // USEWITH | EXTRA | EXTRA2 | DONE | ON | WITHI | WITHO
	SerializableArray<uint16, MAX_ICON_USER_SENTS> text;       // Frasi user
	uint16 uwobj;                           // usewithobj automatico
	SerializableArray<uint16, MAX_PLAYERS> anim;               // animazione su usewith
	SerializableArray<uint16, MAX_PLAYERS> anim2;              // animazione su usewith
	SerializableArray<uint8, T3D_NAMELEN> meshlink;            // link -> mesh

	void loadFromStream(Common::SeekableReadStream &stream) override {
		name = stream.readUint16LE();
		examine.loadFromStream(stream);
		action.loadFromStream(stream);
		flags = stream.readUint16LE();
		text.loadFromStream(stream);
		uwobj = stream.readUint16LE();
		anim.loadFromStream(stream);
		anim2.loadFromStream(stream);
		meshlink.loadFromStream(stream);
	}
};

struct SAtFrame : public SerializableAsset {
	uint8 type;                             // ATFTEXT, ATFSND, ATFEVENT
	uint8 anim;                             // solo se subanim e' attiva
	int16 nframe;                          // 10000 se attraversa portale
	uint16 index;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		type = stream.readByte();
		anim = stream.readByte();
		nframe = stream.readSint16LE();
		index = stream.readUint16LE();
	}
};

struct SAnim : public SerializableAsset {
	SerializableArray<SerializableArray<uint8, T3D_NAMELEN>, MAX_SUBANIMS> meshlink; // link -> anim mesh    (nome mesh) -> da rimuovere
	SerializableArray<SerializableArray<uint8, T3D_NAMELEN>, MAX_SUBANIMS> name; // nome animazione
	SerializableArray<uint8, T3D_NAMELEN>  RoomName;           // nome stanza destinazione
	SerializableArray<SAtFrame, MAX_ATFRAMES> atframe;  // atframe
	uint32 flags;
	int16 active;                          // indece tra le animazioni attive
	int32 obj;                             // obj a cui e' collegato
	uint8  pos;                             // posizione
	uint8  cam;                             // camera per l'azione

	void loadFromStream(Common::SeekableReadStream &stream) override {
		meshlink.loadFromStream(stream);
		name.loadFromStream(stream);
		RoomName.loadFromStream(stream);
		atframe.loadFromStream(stream);
		flags = stream.readUint32LE();
		active = stream.readSint16LE();
		obj = stream.readSint32LE();
		pos = stream.readByte();
		cam = stream.readByte();
	}
};

struct SSound : public SerializableAsset {
	char  name[T3D_NAMELEN];
	uint8 flags;
	SerializableArray<SerializableArray<uint8, T3D_NAMELEN>, MAX_SOUND_MESHLINKS> meshlink;
	uint32 ConeInside, ConeOutside;
	int32 ConeOutsideVolume;
	t3dF32 MinDist, MaxDist;
	int32 Angle;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		stream.read(name, T3D_NAMELEN);
		flags = stream.readByte();
		meshlink.loadFromStream(stream);
		ConeInside = stream.readUint32LE();
		ConeOutside = stream.readUint32LE();
		ConeOutsideVolume = stream.readSint32LE();
		MinDist = stream.readFloatLE();
		MaxDist = stream.readFloatLE();
		Angle = stream.readSint32LE();
	}
};

struct SMusic : public SerializableAsset {
	char  name[MAX_SUB_MUSICS][T3D_NAMELEN];
	SerializableArray<int32, MAX_SUB_MUSICS> vol;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		for (int i = 0; i < MAX_SUB_MUSICS; i++) {
			stream.read(name[i], T3D_NAMELEN);
		}
		vol.loadFromStream(stream);
	}
};

struct SDlgMenu : public SerializableAsset {                           // struttura di appoggio per riempiere i DlgItem
	uint8 parent, on;
	uint16 titolo;                          // indice della sentence di titolo

	void loadFromStream(Common::SeekableReadStream &stream) override {
		parent = stream.readByte();
		on = stream.readByte();
		titolo = stream.readUint16LE();
	}
};

struct SItemCommand : public SerializableAsset {
	uint8 com;                              // comando
	uint16 param1;                          // parametro 1
	uint16 param2;                          // parametro 2

	void loadFromStream(Common::SeekableReadStream &stream) override {
		com = stream.readByte();
		param1 = stream.readUint16LE();
		param2 = stream.readUint16LE();
	}
};

struct SDlgItem : public SerializableAsset {
	SerializableArray<SerializableArray<SItemCommand, MAX_IC_PER_DLG_ITEM>, MAX_PLAYERS> item;                 // lista di comandi per personaggio

	void loadFromStream(Common::SeekableReadStream &stream) override {
		item.loadFromStream(stream);
	}
};

struct SDialog : public SerializableAsset {
	uint16 flags;                                                               // flags del dialogo - per ora non serve
	int32 obj;                                                                 // Personaggio con cui fa il dialogo
	SerializableArray<int32, MAX_ALTERNATES> AltPosSco;                                           // Scostamento posizione alternate
	SerializableArray<int32, MAX_ALTERNATES> AltCamSco;                                           // Scostamento camere alternate
	SerializableArray<SerializableArray<SerializableArray<int32, 2>, MAX_ALT_ANIMS>, MAX_ALTERNATES> AltAnims; // Rimappatura anim alternative dialoghi
	SerializableArray<uint16, MAX_DLG_MENUS> ItemIndex;                                            // indice delle liste di comandi

	void loadFromStream(Common::SeekableReadStream &stream) override {
		flags = stream.readUint16LE();
		obj = stream.readSint32LE();
		AltPosSco.loadFromStream(stream);
		AltCamSco.loadFromStream(stream);
		AltAnims.loadFromStream(stream);
		ItemIndex.loadFromStream(stream);
	}
};

struct SDiaryItem : public SerializableAsset {
	uint16 rand;
	uint16 loop;
	uint16 loopc;
	uint16 bnd, saved_bnd;
	uint16 cur;
	uint16 on;
	SerializableArray<uint16, MAX_ANIMS_PER_DIARY_ITEM> anim;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		rand = stream.readUint16LE();
		loop = stream.readUint16LE();
		loopc = stream.readUint16LE();
		bnd = stream.readUint16LE();
		saved_bnd = stream.readUint16LE();
		cur = stream.readUint16LE();
		on = stream.readUint16LE();
		anim.loadFromStream(stream);
	}
};

struct SDiary : public SerializableAsset {
	uint16 startt, endt;
	int32 room;
	uint16 obj;
	uint16 cur;
	uint16 end_hideobj; //se il bit piu' alto e' settato vuol dire che il diario e' stato lanciato almeno una volta e quindi questa variabile e' valida solo per oggetti da 0..32767
	SerializableArray<SDiaryItem, MAX_DIARY_ITEMS> item;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		startt = stream.readUint16LE();
		endt = stream.readUint16LE();
		room = stream.readSint32LE();
		obj = stream.readUint16LE();
		cur = stream.readUint16LE();
		end_hideobj = stream.readUint16LE();
		item.loadFromStream(stream);
	}
};

struct SPlayerStand {
	uint8 pos = 0;
	int32 an = 0;
	int32 cr = 0;
	uint16 bnd = 0;
	Common::String roomName;
	SPlayerStand() = default;
	SPlayerStand(Common::SeekableReadStream &stream) {
		pos = stream.readByte();
		an = stream.readSint32LE();
		cr = stream.readSint32LE();
		bnd = stream.readUint16LE();
	}
};

struct SPDALog : public SerializableAsset {
	int32 time = 0;
	SerializableString<MAX_STRING_LEN> info;
	SerializableArray<int32, MAX_PDA_INFO> text;
	SerializableArray<SerializableString<MAX_TEXT_CHARS>, MAX_PDA_LINES *MAX_PDA_INFO> formatted;
	int32 menu_appartenenza = 0, menu_creato = 0;
	int32 flags = 0;
	int32 lines = 0;
	int32 dx = 0, dy = 0;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		time = stream.readSint32LE();
		info.loadFromStream(stream);
		text.loadFromStream(stream);
		formatted.loadFromStream(stream);
		menu_appartenenza = stream.readSint32LE();
		menu_creato = stream.readSint32LE();
		flags = stream.readSint32LE();
		lines = stream.readSint32LE();
		dx = stream.readSint32LE();
		dy = stream.readSint32LE();
	}
};

struct SString {
	char text[MAX_STRING_LEN] = {};
	uint16  x = 0, y = 0, dx = 0;

	SString() = default;
	SString(const char *text, uint16 x = 0, uint16 y = 0, uint16 dx = 0) : x(x), y(y), dx(dx) {
		if (text != nullptr) {
			memcpy(this->text, text, strlen(text));
		}
	}
};

struct SRect {
	int32  x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	SRect() = default;
	SRect(int32 x1, int32 y1, int32 x2, int32 y2): x1(x1), y1(y1), x2(x2), y2(y2) {}
};

struct SD3DRect {
	int32 px = 0, py = 0, dx = 0, dy = 0;
	uint8 r = 0, g = 0, b = 0, a = 0;
};
struct SD3DTriangle {
	int32 x1 = 0, y1 = 0, x2 = 0, y2 = 0, x3 = 0, y3 = 0;
	uint8 r = 0, g = 0, b = 0, a = 0;
};
struct SD3DBitmap {
	int32 tnum = 0;
	int32 px = 0, py = 0, dx = 0, dy = 0;
	int32 rtype = 0;
};
struct SDDBitmap {
	int32 tnum = 0;
	int32 px = 0, py = 0, ox = 0, oy = 0, dx = 0, dy = 0;
};

enum class FontKind;
struct SDDText {
	SDDText() = default;
	SDDText(const char *text, FontKind font, FontColor color, int32 tnum) : font(font),
																			color(color),
																			tnum(tnum) {
		Common::strlcpy(this->text, text, sizeof(this->text));
	}
	char text[MAX_STRING_LEN] = {};
	FontKind font; // TODO: Move elsewhere so we can initalize
	FontColor color;
	int32 tnum = 0;
};

struct SScript {
	uint16 firstframe = 0;
	uint8 flags = 0;                    // 0 - BREAKABLE - Se si interrompe con la pressione dei tasti del mouse
};                                      // 1 - DIALOGEXITNOANIM - Se lo script e' lanciato da un dialogo all'ultima battuta esce dal dialogo senza anim di link e di default

struct SGameOptions {
	uint8  sound_on = 0;
	uint8  sound_volume = 0;
	uint8  music_on = 0;
	uint8  music_volume = 0;
	uint8  speech_on = 0;
	uint8  speech_volume = 0;
	uint8  subtitles_on = 0;
};


// MESSAGGI PER LO SCHEDULER
struct message {
	EventClass classe;                       // message classe
	uint8 event = 0;                         // message name
	uint16 flags = 0;                        // message priority | NOWAIT per script

	uint8 bparam = 0;
	int16 wparam1 = 0;                     // t3dS16 parameter 1
	int16 wparam2 = 0;                     // t3dS16 parameter 2
	union {
		int32 lparam[3];               // long parameter
		t3dF32 fparam[3];               // float parameter
	};
	message() = default;
	message(EventClass classe, uint8 event, uint16 flags) : classe(classe), event(event), flags(flags) {}
};

struct pqueue {
	uint8   head = 0, tail = 0, len = 0;
	message *event[MAX_MESSAGES] = {};
};


// STRUTTURE PER TITOLI DI CODA
struct SCreditsName : public SerializableAsset {
	char name[64] = {};
	uint16 role = 0;
	uint8 flags = 0;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		stream.read(name, 64);
		role = stream.readUint16LE();
		flags = stream.readByte();
	}
};

struct SCreditsRole : public SerializableAsset {
	char role[48];
	uint8 flags = 0;

	void loadFromStream(Common::SeekableReadStream &stream) override {
		stream.read(role, 48);
		flags = stream.readByte();
	}
};

struct STitoliCoda {
	char *s = nullptr;        //puntatore alla stringa
	int32 px = 0, py = 0;  //posizione
	int32 dx = 0, dy = 0;  //dimensione
	int32 tnum = 0;    //bitmap

	uint32 time = 0;    //quando deve apparire (solo nel caso degli StaticCredits)
};

struct SRoomInfo {
	char name[64] = {};            //nome della stanza
	char fullstring[64 + 16] = {}; //stringa con ora
	int32 px = 0, py = 0;         //posizione
	int32 dx = 0, dy = 0;         //dimensione totale
	int32 _dx = 0, _dy = 0;       //dimensione della stringa attuale
	int32 tnum = 0;               //bitmap
	char *letter_ptr = nullptr;    //puntatore alla lettera corrente
	int16 t_next_letter = 0;      //quando deve apparire la nuova lettera
	FontKind f;     //il font
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_STRUCT_H
