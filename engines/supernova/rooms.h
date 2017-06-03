#ifndef ROOMS_H
#define ROOMS_H

#include "msn_def.h"

namespace Supernova {
/* --- ROOM 0 ----------------------------------- */

room roomlist0[ROOMNUM0] =
{
	{
		"",{0},NIL,NIL,NIL,
		{
			{"Keycard","Die Keycard f�r deine Schr�nke.",KEYCARD,
			TAKE+CARRIED+COMBINABLE,255,255,0},
			{"Taschenmesser","Es ist nicht mehr das sch�rfste.",KNIFE,
			TAKE+CARRIED+COMBINABLE,255,255,0},
			{"Armbanduhr",&description,WATCH,TAKE+COMBINABLE+CARRIED,255,255,8},
			{"Discman","Es ist eine \"Mad Monkeys\"-CD darin.",
			DISCMAN,TAKE+COMBINABLE,255,255,0}
		}
	}
};


room roomlist1[ROOMNUM1] =
{
/* --- Spaceship --- */

	{
		"017",{1,0,0,0,1},&sb_corridor,NIL,NIL,
		{
			{"Luke",&description,HATCH1,OPEN+EXIT,0,6,1,CABIN_L1,15},
			{"Luke",&description,0,OPEN+EXIT,1,7,2,CABIN_L2,10},
			{"Luke",&description,0,OPEN+EXIT,2,8,3,CABIN_L3,5},
			{"Luke",&description,0,OPEN+EXIT+OPENED,9,3,4,CABIN_R3,9},
			{"Luke",&description,0,OPEN+EXIT,4,10,5,CABIN_R2,14},
			{"Luke",&description,0,OPEN+EXIT,5,11,6,CABIN_R1,19},
			{"Luke",&description,0,OPEN+CLOSED+EXIT,12,12,0,AIRLOCK,2},
			{"Knopf","Er geh�rt zu der gro�en Luke.",BUTTON,PRESS,13,13,0},
			{"Leiter",&description,0,0,14,14,0},
			{"Ausgang",&description,0,EXIT,15,15,0,HALL,22}
		}
	},
	{
		"015",{1},&sb_hall,NIL,NIL,
		{
			{"Luke","Sie f�hrt ins Cockpit.",0,OPEN+EXIT,4,5,1,COCKPIT,10},
			{"Luke","Sie f�hrt zur K�che.",KITCHEN_HATCH,
			OPEN+EXIT,0,0,0,NIL,1},
			{"Luke","Sie f�hrt zu den Tiefschlafkammern.",0,
			OPEN+CLOSED+EXIT,1,1,2,SLEEP,8},
			{"Schlitz","Es ist ein Keycard-Leser.",SLEEP_SLOT,COMBINABLE,2,2,0},
			{"Leiter",&description,0,0,3,3,0},
			{"Gang",&description,0,EXIT,6,6,0,CORRIDOR,19}
		}
	},
	{
		"033",{1},&sb_sleep,&anim_sleep,&once_sleep,
		{
			{"Luke","Dies ist eine der Tiefschlafkammern.",CABINS,0,0,0,0},
			{"Luke","Dies ist eine der Tiefschlafkammern.",CABIN,0,1,1,0},
			{"Computer",&description,COMPUTER,0,2,2,0},
			{"Ausgang",&description,0,EXIT,255,255,0,HALL,22}
		}
	},
	{
		"009",{1},&sb_cockpit,&anim_cockpit,&once_cockpit,
		{
			{"Instrumente","Hmm, sieht ziemlich kompliziert aus.",
			INSTRUMENTS,0,2,2,0},
			{"Monitor",&description,MONITOR,0,0,0,0},
			{"Monitor","Dieser Monitor sagt dir nichts.",0,1,1,0},
			{"Ausgang",&description,0,EXIT,255,255,0,HALL,22}
		}
	},
	{
		"034",{1,0,0,0,0,0,1},&sb_airlock,NIL,&once_airlock,
		{
			{"Luke","Das ist eine Luke !!!",0,
			EXIT+OPEN+OPENED+CLOSED,0,0,0,CORRIDOR,10},
			{"Luke","Dies ist eine Luke !!!",0,
			EXIT+OPEN+CLOSED,1,1,0,HOLD,14},
			{"Knopf",&description,BUTTON1,PRESS,2,2,0},
			{"Knopf",&description,BUTTON2,PRESS,3,3,0},
			{"Helm","Es ist der Helm zum Raumanzug.",HELMET,TAKE,4,4,7},
			{"Raumanzug","Der einzige Raumanzug, den die|anderen hiergelassen haben ...",
			SUIT,TAKE,5,5,8},
			{"Versorgung","Es ist der Versorgungsteil zum Raumanzug.",
			LIFESUPPORT,TAKE,6,6,9},
			{"Druckmesser",&description,MANOMETER,0,7,7,0}
		}
	},
	{
		"024",{1},&sb_hold,NIL,&once_hold,
		{
			{"",&description,HOLD_WIRE,COMBINABLE,255,255,0},
			{"Schrott","Da ist eine L�sterklemme dran, die|noch ganz brauchbar aussieht.|Ich nehme sie mit.",
			SCRAP_LK,0,4,4,0},
			{"L�sterklemme",&description,TERMINALSTRIP,COMBINABLE,255,255,0},
			{"Schrott","Junge, Junge! Die Explosion hat ein|ganz sch�nes Durcheinander angerichtet.",
			0,0,5,5,0},
			{"Reaktor","Das war einmal der Reaktor.",0,0,6,6,0},
			{"D�se",&description,0,0,7,7,0},
			{"blauer K�rbis","Keine Ahnung, was das ist.",0,0,8,8,0},
			{"Luke",&description,LANDINGMOD_OUTERHATCH,EXIT+OPEN,1,2,2,LANDINGMODULE,6},
			{"Landef�hre","Sie war eigentlich f�r Bodenuntersuchungen|auf Arsano 3 gedacht.",
			0,0,0,0,0},
			{"Ausgang",&description,0,EXIT,255,255,0,AIRLOCK,22},
			{"Luke","Sie f�hrt nach drau�en.",
			OUTERHATCH_TOP,EXIT+OPEN+OPENED,3,3,0,GENERATOR,8},
			{"Generator","Er versorgt das Raumschiff mit Strom.",
			GENERATOR_TOP,EXIT,12,12,0,GENERATOR,8}
		}
	},
	{
		"025",{1},&sb_landingmod,NIL,NIL,
		{
			{"Steckdose",&description,LANDINGMOD_SOCKET,COMBINABLE,1,1,0},
			{"Knopf","Es ist ein Sicherheitsknopf.|Er kann nur mit einem spitzen|Gegenstand gedr�ckt werden.",
			LANDINGMOD_BUTTON,PRESS+COMBINABLE,2,2,0},
			{"Monitor",&description,LANDINGMOD_MONITOR,0,3,3,0},
			{"Tastatur",&description,KEYBOARD,0,4,4,0},
			{"",&description,LANDINGMOD_WIRE,COMBINABLE,255,255,0},
			{"Luke",&description,LANDINGMOD_HATCH,EXIT+OPEN+OPENED+COMBINABLE,
			0,0,1+128,HOLD,10}
		}
	},
	{
		"018",{1,0,0,0,0,1},&sb_generator,NIL,NIL,
		{
			{"langes Kabel mit Stecker",&description,GENERATOR_WIRE,COMBINABLE,255,255,0},
			{"leere Kabelrolle",&description,0,UNNECESSARY,255,255,0},
			{"Keycard","Hey, das ist die Keycard des Commanders!|Er mu� sie bei dem �berst�rzten|Aufbruch verloren haben.",
			KEYCARD2,COMBINABLE+TAKE,12,12,5+128},
			{"Seil",&description,GENERATOR_ROPE,COMBINABLE,255,255,0},
			{"Luke","Sie f�hrt nach drau�en.",
			OUTERHATCH,EXIT+OPEN,1,2,1,OUTSIDE,22},
			{"Luke",&description,0,OPEN+CLOSED,3,3,0},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,4,4,0},
			{"Klappe",&description,TRAP,OPEN,5,6,2},
			{"Leitung",&description,0,0,7,7,0},
			{"Spannungmessger�t",&description,VOLTMETER,0,9,9,0},
			{"Klemme",&description,CLIP,COMBINABLE,8,8,0},
			{"Leitung","Sie f�hrt vom Generator zum Spannungmessger�t.",
			SHORT_WIRE,COMBINABLE,10,10,0},
			{"Leiter",&description,LADDER,EXIT,0,0,0,HOLD,1}
		}
	},
	{
		"004",{1},NIL,NIL,NIL,
		{
			{"Luke",&description,0,EXIT,0,0,0,GENERATOR,3},
			{"Seil",&description,0,UNNECESSARY,255,255,0}
		}
	},
	{
		"022",{1,1,1},NIL,NIL,NIL,
		{
			{"Bild","Manche Leute haben schon|einen komischen Geschmack.",0,
			UNNECESSARY,5,5,0},
			{"Zeichenger�te",
			"Auf dem Zettel sind lauter|unverst�ndliche Skizzen und Berechnungen.|(Jedenfalls f�r dich unverst�ndlich.)",
			0,UNNECESSARY,6,6,0},
			{"Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",0,
			UNNECESSARY,7,7,0},
			{"Luke",&description,0,OPEN+OPENED+EXIT,3,3,15+128,CORRIDOR,5},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,0,0,0},
			{"Schrank",&description,0,OPEN+CLOSED,1,1,0},
			{"Fach",&description,0,OPEN+CLOSED,2,2,0},
			{"Steckdose",&description,SOCKET,COMBINABLE,4,4,0},
			{"Toilette",&description,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22}
		}
	},
	{
		"022",{1,0,0,1,1,1},NIL,NIL,NIL,
		{
			{"Schachspiel","Es macht wohl Spa�, an|der Decke Schach zu spielen.",
			0,UNNECESSARY,11,11,0},
			{"Tennisschl�ger","Fliegt Boris Becker auch mit?",0,UNNECESSARY,8,8,0},
			{"Tennisball","Toll!",0,UNNECESSARY,9,9,0},
			{"Luke",&description,0,OPEN+OPENED+EXIT,3,3,15+128,CORRIDOR,5},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,0,0,0},
			{"Schrank",&description,0,OPEN+CLOSED,1,1,0},
			{"Fach",&description,0,OPEN+CLOSED,2,2,0},
			{"Steckdose",&description,SOCKET,COMBINABLE,4,4,0},
			{"Toilette",&description,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22}
		}
	},
	{
		"022",{1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},&sb_cabin_r3,NIL,NIL,
		{
			{"Schachspiel","Dein Magnetschachspiel. Schach war|schon immer deine Leidenschaft.",
			CHESS,TAKE+COMBINABLE,12,12,7+128},
			{"Bett","Das ist dein Bett. Toll, nicht wahr?",0,0,13,13,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_K1,COMBINABLE,27,27,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_K2,COMBINABLE,28,28,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_K3,COMBINABLE,29,29,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_K4,COMBINABLE,30,30,0},
			{"Fach","Das ist eins deiner drei F�cher.",SHELF1,
			OPEN+CLOSED,14,18,9},
			{"Alben","Deine Briefmarkensammlung.",0,UNNECESSARY,14,14,0},
			{"Fach","Das ist eins deiner drei F�cher.",SHELF2,
			OPEN+CLOSED,15,19,10},
			{"Seil","Es ist ungef�hr 10 m lang und 4 cm dick.",ROPE,
			TAKE+COMBINABLE,15,15,12},
			{"Schrank","Das ist dein Schrank.",SHELF3,OPEN+CLOSED,16,17,11},
			{"Krimskram","Es ist nichts brauchbares dabei.",0,UNNECESSARY,20,20,0},
			{"Kleider","Es sind Standard-Weltraum-Klamotten.",0,UNNECESSARY,21,21,0},
			{"Unterw�sche",&description,0,UNNECESSARY,22,22,0},
			{"Str�mpfe",&description,0,UNNECESSARY,23,23,0},
			{"Fach","Das ist eins deiner drei F�cher.",SHELF4,
			OPEN+CLOSED,24,25,13},
			{"Buch","Es ist|\"Per Anhalter durch die Galaxis\"|von Douglas Adams.",
			BOOK,TAKE,26,26,14},
			{"Discman","Es ist eine \"Mad Monkeys\"-CD darin.",
			DISCMAN,TAKE+COMBINABLE,33,33,16},
			{"Luke",&description,0,OPEN+EXIT,3,3,15+128,CORRIDOR,5},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,0,0,0},
			{"Schrank",&description,0,OPEN+CLOSED,1,1,0},
			{"Fach",&description,0,OPEN+CLOSED,2,2,0},
			{"Steckdose",&description,SOCKET,COMBINABLE,4,4,0},
			{"Toilette",&description,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22}
		}
	},
	{
		"021",{1,1,1,1,1,1},NIL,NIL,NIL,
		{
			{"Bild","Herb!",0,UNNECESSARY,5,5,0},
			{"Bild","Toll!",0,UNNECESSARY,6,6,0},
			{"Bild","Genial!",0,UNNECESSARY,7,7,0},
			{"Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",0,
			UNNECESSARY,8,8,0},
			{"Bild","Es scheint noch nicht fertig zu sein.",0,UNNECESSARY,9,9,0},
			{"Stift","Ein Kugelschreiber.",PEN,TAKE+COMBINABLE,10,10,5+128},
			{"Luke",&description,0,OPEN+OPENED+EXIT,3,3,24+128,CORRIDOR,9},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,0,0,0},
			{"Schrank",&description,0,OPEN+CLOSED,1,1,0},
			{"Fach",&description,0,OPEN+CLOSED,2,2,0},
			{"Steckdose",&description,SOCKET,COMBINABLE,4,4,0},
			{"Toilette",&description,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22}
		}
	},
	{
		"021",{1,0,0,0,0,0,1,1,1,1,0,0,1},&sb_cabin_l3,NIL,NIL,
		{
			{"Poster","Ein Poster von \"Big Boss\".",0,UNNECESSARY,11,11,0},
			{"Poster","Ein Poster von \"Rock Desaster\".",0,UNNECESSARY,12,12,0},
			{"Box",&description,0,0,13,13,0},
			{"Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",0,
			UNNECESSARY,14,14,0},
			{"Schallplatte","Die Platte ist von \"Big Boss\".",
			RECORD,TAKE+COMBINABLE,15,15,8+128},
			{"Schallplattenst�nder","Du hast jetzt keine Zeit, in|der Plattensammlung rumzust�bern.",
			0,UNNECESSARY,16,16,0},
			{"Knopf",&description,TURNTABLE_BUTTON,PRESS,22,22,0},
			{"Plattenspieler","Sieht aus, als k�me|er aus dem Museum.",
			TURNTABLE,UNNECESSARY+COMBINABLE,17,17,0},
			{"Leitung",&description,WIRE,COMBINABLE,18,18,0},
			{"Leitung",&description,WIRE2,COMBINABLE,19,19,0},
			{"Stecker",&description,PLUG,COMBINABLE,20,20,0},
			{"Luke",&description,0,OPEN+OPENED+EXIT,3,3,24+128,CORRIDOR,9},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,0,0,0},
			{"Schrank",&description,0,OPEN+CLOSED,1,1,0},
			{"Fach",&description,0,OPEN+CLOSED,2,2,0},
			{"Steckdose",&description,SOCKET,COMBINABLE,4,4,0},
			{"Toilette",&description,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22}
		}
	},
	{
		"021",{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},&sb_cabin_l2,NIL,NIL,
		{
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_KL1,COMBINABLE,31,31,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_KL2,COMBINABLE,32,32,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_KL3,COMBINABLE,33,33,0},
			{"Schlitz","Es ist ein Keycard-Leser.",SLOT_KL4,COMBINABLE,45,45,0},
			{"Schrank",&description,SHELF_L1,OPEN+CLOSED,25,26,17},
			{"Pistole","Es ist keine Munition drin.",PISTOL,TAKE,39,39,20},
			{"Fach",&description,SHELF_L2,OPEN+CLOSED,27,28,18},
			{"B�cher","Lauter wissenschaftliche B�cher.",0,UNNECESSARY,40,40,0},
			{"Fach",&description,SHELF_L3,OPEN+CLOSED,29,30,19},
			{"Kabelrolle","Da sind mindestens zwanzig Meter drauf.",
			SPOOL,TAKE+COMBINABLE,41,41,21},
			{"Fach",&description,SHELF_L4,OPEN+CLOSED,43,44,22},
			{"Buch",&description,BOOK2,TAKE,46,46,23},
			{"Unterw�sche","Ich habe keine Lust, in|der Unterw�sche des|Commanders rumzuw�hlen.",
			0,UNNECESSARY,34,34,0},
			{"Unterw�sche","Ich habe keine Lust, in|der Unterw�sche des|Commanders rumzuw�hlen.",
			0,UNNECESSARY,35,35,0},
			{"Kleider",&description,0,UNNECESSARY,36,36,0},
			{"Krimskram","Es ist nichts brauchbares dabei.",0,UNNECESSARY,37,37,0},
			{"Krimskram","Es ist nichts brauchbares dabei.",0,UNNECESSARY,38,38,0},
			{"Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",0,
			UNNECESSARY,23,23,0},
			{"Toilette",&description,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22},
			{"Luke",&description,0,OPEN+OPENED+EXIT,3,3,24+128,CORRIDOR,9},
			{"Schlitz","Es ist ein Keycard-Leser.",0,COMBINABLE,0,0,0},
			{"Schrank",&description,0,OPEN+CLOSED,1,1,0},
			{"Fach",&description,0,OPEN+CLOSED,2,2,0},
			{"Steckdose",&description,SOCKET,COMBINABLE,4,4,0},
			{"Ordner","Darauf steht \"Dienstanweisungen|zur Mission Supernova\".|Es steht nichts wichtiges drin.",
			0,UNNECESSARY,49,49,0}
		}
	},
	{
		"023",{1},&sb_bathroom,NIL,NIL,
		{
			{"Klo","Ein Klo mit Saugmechanismus.",TOILET,0,0,0,0},
			{"Dusche",&description,SHOWER,0,1,1,0},
			{"Ausgang",&description,BATHROOM_EXIT,EXIT,255,255,0,0,22}
		}
	}
};


/* --- Rooms ------------------------------------ */

room roomlist2[ROOMNUM2] =
{

/* --- Arsano --- */

	{
		"012",{1},&sb_rocks,NIL,&once_rocks,
		{
			{"Seil",&description,0,UNNECESSARY+EXIT,0,0,0,GENERATOR,12},
			{"Stein",&description,STONE,0,1,1,0},
			{"Stein",&description,0,0,2,2,0},
			{"Loch","Es scheint eine H�hle zu sein.",0,0,255,255,0,CAVE,1}
		}
	},
	{
		"012",{0},NIL,NIL,NIL,
		{
			{"Ausgang","Hier bist du gerade hergekommen.",
			0,EXIT,255,255,0,ROCKS,22},
			{"Ausgang",&description,0,EXIT,255,255,0,MEETUP,2}
		}
	},
	{
		"037",{1},&sb_meetup,&anim_meetup,&once_meetup,
		{
			{"H�hle",&description,0,EXIT,255,255,0,CAVE,22},
			{"Schild","Diese Schrift kannst du nicht lesen.",MEETUP_SIGN,0,0,0,0},
			{"Eingang",&description,DOOR,EXIT,1,1,0,ENTRANCE,7},
			{"Stern",&description,STAR,0,2,2,0},
			{"Raumschiff",&description,SPACESHIPS,COMBINABLE,3,3,0},
			{"Raumschiff",&description,SPACESHIP,COMBINABLE,4,4,0}
		}
	},
	{
		"010",{1},&sb_entrance,&anim_entrance,NIL,
		{
			{"Portier","Du siehst doch selbst, wie er aussieht.",
			PORTER,TALK,0,0,0},
			{"T�r",&description,0,EXIT+OPEN+CLOSED,1,1,0,0,5},
			{"Schild","Diese Schrift kannst du nicht lesen.",
			KITCHEN_SIGN,0,2,2,0},
			{"Kaugummi",&description,SCHNUCK,TAKE,255,255,10+128},
			{"Gummib�rchen",&description,SCHNUCK,TAKE,255,255,11+128},
			{"Schokokugel",&description,SCHNUCK,TAKE,255,255,12+128},
			{"�berraschungsei",&description,EGG,TAKE,255,255,13+128},
			{"Lakritz",&description,SCHNUCK,TAKE,255,255,14+128},
			{"Tablette","Die Plastikh�lle zeigt einen|Mund mit einer Sprechblase. Was|darin steht, kannst du nicht lesen.",
			PILL,TAKE,255,255,0},
			{"Schlitz",&description,CAR_SLOT,COMBINABLE,6,6,0},
			{"Automat","Sieht aus wie ein Kaugummiautomat.",0,0,5,5,0},
			{"Toilette","Die Toiletten sind denen|auf der Erde sehr �hnlich.",
			ARS_BATHROOM,0,255,255,0},
			{"Knopf",&description,BATHROOM_BUTTON,PRESS,3,3,0},
			{"Schild","Diese Schrift kannst du nicht lesen.",BATHROOM_SIGN,0,4,4,0},
			{"Treppe",&description,STAIRCASE,EXIT,8,8,0,REST,3},
			{"Ausgang",&description,MEETUP_EXIT,EXIT,255,255,0,MEETUP,22},
			{"M�nzen","Es sind seltsame|K�pfe darauf abgebildet.",
			COINS,TAKE+COMBINABLE,255,255,0},
			{"Tablettenh�lle","Darauf steht:\"Wenn Sie diese|Schrift jetzt lesen k�nnen,|hat die Tablette gewirkt.\"",
			PILL_HULL,TAKE,255,255,0}
		}
	},
	{
		"028",{1},NIL,&anim_remaining,NIL,
		{
			{"Treppe",&description,0,EXIT,0,0,0,ENTRANCE,17},
			{"Stuhl",&description,0,EXIT,1,1,0,ROGER,2},
			{"Schuhe","Wie ist der denn mit|Schuhen hier reingekommen?",0,0,2,2,0}
		}
	},
	{
		"029",{1},&sb_roger,&anim_roger,&once_roger,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,REST,19},
			{"Froschgesicht",&description,ROGER_W,TALK,0,0,0},
			{"Gekritzel","\"Mr Spock was here\"",0,0,3,3,0},
			{"Brieftasche",&description,WALLET,TAKE,1,1,4},
			{"Speisekarte","\"Heute empfehlen wir:|Fonua Opra mit Ulk.\"",0,UNNECESSARY,2,2,0},
			{"Tasse","Sie enth�lt eine gr�nliche Fl�ssigkeit.",CUP,UNNECESSARY,4,4,0},
			{"Schachspiel",&description,0,UNNECESSARY,255,255,0},
			{"10-Buckazoid-Schein","Nicht gerade sehr viel Geld.",
			0,TAKE+COMBINABLE,255,255,0},
			{"Keycard von Roger",&description,KEYCARD_R,TAKE+COMBINABLE,255,255,0}
		}
	},
	{
		"019",{1},&sb_glider,&anim_glider,NIL,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,MEETUP,15},
			{"Knopf",&description,GLIDER_BUTTON1,PRESS,0,0,0},
			{"Knopf",&description,GLIDER_BUTTON2,PRESS,1,1,0},
			{"Knopf",&description,GLIDER_BUTTON3,PRESS,2,2,0},
			{"Knopf",&description,GLIDER_BUTTON4,PRESS,3,3,0},
			{"Keycard von Roger",&description,GLIDER_KEYCARD,TAKE+COMBINABLE,255,255,0},
			{"Schlitz",&description,GLIDER_SLOT,COMBINABLE,4,4,0},
			{"Fach",&description,0,OPEN,5,6,6},
			{"Tastatur",&description,GLIDER_BUTTONS,0,7,7,0},
			{"Anzeige","Hmm, seltsame Anzeigen.",GLIDER_DISPLAY,0,8,8,0},
			{"Instrumente","Hmm, seltsame Anzeigen.",GLIDER_INSTRUMENTS,0,9,9,0}
		}
	},
	{
		"038",{1},&sb_meetup2,NIL,&once_meetup2,
		{
			{"Roger W.",&description,ROGER_W,TALK,255,255,0},
			{"Raumschiff",&description,SPACESHIP,COMBINABLE,255,255,0},
			{"H�hle",&description,0,EXIT,255,255,0,CAVE,22}
		}
	},
	{
		"039",{1},&sb_meetup3,NIL,NIL,
		{
			{"Ufo","Der Eingang scheint offen zu sein.",UFO,EXIT,0,0,0,0,3},
			{"Stern",&description,STAR,0,1,1,0},
			{"H�hle",&description,0,EXIT,255,255,0,CAVE,22}
		}
	}
};
/* --- Rooms ------------------------------------ */

room roomlist2[ROOMNUM2] =
{

/* --- Arsano --- */

	{
		"012",{1},&sb_rocks,NIL,&once_rocks,
		{
			{"Seil",&description,0,UNNECESSARY+EXIT,0,0,0,GENERATOR,12},
			{"Stein",&description,STONE,0,1,1,0},
			{"Stein",&description,0,0,2,2,0},
			{"Loch","Es scheint eine Hhle zu sein.",0,0,255,255,0,CAVE,1}
		}
	},
	{
		"012",{0},NIL,NIL,NIL,
		{
			{"Ausgang","Hier bist du gerade hergekommen.",
			0,EXIT,255,255,0,ROCKS,22},
			{"Ausgang",&description,0,EXIT,255,255,0,MEETUP,2}
		}
	},
	{
		"037",{1},&sb_meetup,&anim_meetup,&once_meetup,
		{
			{"Hhle",&description,0,EXIT,255,255,0,CAVE,22},
			{"Schild","Diese Schrift kannst du nicht lesen.",MEETUP_SIGN,0,0,0,0},
			{"Eingang",&description,DOOR,EXIT,1,1,0,ENTRANCE,7},
			{"Stern",&description,STAR,0,2,2,0},
			{"Raumschiff",&description,SPACESHIPS,COMBINABLE,3,3,0},
			{"Raumschiff",&description,SPACESHIP,COMBINABLE,4,4,0}
		}
	},
	{
		"010",{1},&sb_entrance,&anim_entrance,NIL,
		{
			{"Portier","Du siehst doch selbst, wie er aussieht.",
			PORTER,TALK,0,0,0},
			{"Tr",&description,0,EXIT+OPEN+CLOSED,1,1,0,0,5},
			{"Schild","Diese Schrift kannst du nicht lesen.",
			KITCHEN_SIGN,0,2,2,0},
			{"Kaugummi",&description,SCHNUCK,TAKE,255,255,10+128},
			{"Gummibrchen",&description,SCHNUCK,TAKE,255,255,11+128},
			{"Schokokugel",&description,SCHNUCK,TAKE,255,255,12+128},
			{"berraschungsei",&description,EGG,TAKE,255,255,13+128},
			{"Lakritz",&description,SCHNUCK,TAKE,255,255,14+128},
			{"Tablette","Die Plastikhlle zeigt einen|Mund mit einer Sprechblase. Was|darin steht, kannst du nicht lesen.",
			PILL,TAKE,255,255,0},
			{"Schlitz",&description,CAR_SLOT,COMBINABLE,6,6,0},
			{"Automat","Sieht aus wie ein Kaugummiautomat.",0,0,5,5,0},
			{"Toilette","Die Toiletten sind denen|auf der Erde sehr hnlich.",
			ARS_BATHROOM,0,255,255,0},
			{"Knopf",&description,BATHROOM_BUTTON,PRESS,3,3,0},
			{"Schild","Diese Schrift kannst du nicht lesen.",BATHROOM_SIGN,0,4,4,0},
			{"Treppe",&description,STAIRCASE,EXIT,8,8,0,REST,3},
			{"Ausgang",&description,MEETUP_EXIT,EXIT,255,255,0,MEETUP,22},
			{"Mnzen","Es sind seltsame|Kpfe darauf abgebildet.",
			COINS,TAKE+COMBINABLE,255,255,0},
			{"Tablettenhlle","Darauf steht:\"Wenn Sie diese|Schrift jetzt lesen knnen,|hat die Tablette gewirkt.\"",
			PILL_HULL,TAKE,255,255,0}
		}
	},
	{
		"028",{1},NIL,&anim_remaining,NIL,
		{
			{"Treppe",&description,0,EXIT,0,0,0,ENTRANCE,17},
			{"Stuhl",&description,0,EXIT,1,1,0,ROGER,2},
			{"Schuhe","Wie ist der denn mit|Schuhen hier reingekommen?",0,0,2,2,0}
		}
	},
	{
		"029",{1},&sb_roger,&anim_roger,&once_roger,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,REST,19},
			{"Froschgesicht",&description,ROGER_W,TALK,0,0,0},
			{"Gekritzel","\"Mr Spock was here\"",0,0,3,3,0},
			{"Brieftasche",&description,WALLET,TAKE,1,1,4},
			{"Speisekarte","\"Heute empfehlen wir:|Fonua Opra mit Ulk.\"",0,UNNECESSARY,2,2,0},
			{"Tasse","Sie enthlt eine grnliche Flssigkeit.",CUP,UNNECESSARY,4,4,0},
			{"Schachspiel",&description,0,UNNECESSARY,255,255,0},
			{"10-Buckazoid-Schein","Nicht gerade sehr viel Geld.",
			0,TAKE+COMBINABLE,255,255,0},
			{"Keycard von Roger",&description,KEYCARD_R,TAKE+COMBINABLE,255,255,0}
		}
	},
	{
		"019",{1},&sb_glider,&anim_glider,NIL,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,MEETUP,15},
			{"Knopf",&description,GLIDER_BUTTON1,PRESS,0,0,0},
			{"Knopf",&description,GLIDER_BUTTON2,PRESS,1,1,0},
			{"Knopf",&description,GLIDER_BUTTON3,PRESS,2,2,0},
			{"Knopf",&description,GLIDER_BUTTON4,PRESS,3,3,0},
			{"Keycard von Roger",&description,GLIDER_KEYCARD,TAKE+COMBINABLE,255,255,0},
			{"Schlitz",&description,GLIDER_SLOT,COMBINABLE,4,4,0},
			{"Fach",&description,0,OPEN,5,6,6},
			{"Tastatur",&description,GLIDER_BUTTONS,0,7,7,0},
			{"Anzeige","Hmm, seltsame Anzeigen.",GLIDER_DISPLAY,0,8,8,0},
			{"Instrumente","Hmm, seltsame Anzeigen.",GLIDER_INSTRUMENTS,0,9,9,0}
		}
	},
	{
		"038",{1},&sb_meetup2,NIL,&once_meetup2,
		{
			{"Roger W.",&description,ROGER_W,TALK,255,255,0},
			{"Raumschiff",&description,SPACESHIP,COMBINABLE,255,255,0},
			{"Hhle",&description,0,EXIT,255,255,0,CAVE,22}
		}
	},
	{
		"039",{1},&sb_meetup3,NIL,NIL,
		{
			{"Ufo","Der Eingang scheint offen zu sein.",UFO,EXIT,0,0,0,0,3},
			{"Stern",&description,STAR,0,1,1,0},
			{"Hhle",&description,0,EXIT,255,255,0,CAVE,22}
		}
	}
};


/* --- Rooms ------------------------------------ */

room roomlist3[ROOMNUM3] =
{

/* --- Axacuss --- */

	{
		"043",{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		&sb_cell,&anim_cell,&once_cell,
		{
			{"Knopf",&description,CELL_BUTTON,PRESS,1,1,0},
			{"T�r",&description,CELL_DOOR,EXIT+OPEN+CLOSED,0,0,31+128,CORRIDOR4,1},
			{"Tablett","Es ist irgendein Fra� und|etwas zu Trinken darauf.",
			TRAY,UNNECESSARY,255,255,0},
			{"Stange","Es scheint eine Lampe zu sein.",0,COMBINABLE,3,3,0},
			{"Augen","Es ist nur ein Bild.",0,0,4,4,0},
			{"Leitung",&description,CELL_WIRE,COMBINABLE+TAKE,6,6,0},
			{"Steckdose","Sieht etwas anders aus als auf der Erde.",
			SOCKET,COMBINABLE,5,5,0},
			{"Metallblock","Er ist ziemlich schwer.",
			MAGNET,TAKE+COMBINABLE,255,255,30},
			{"Roboter","Den hast du erledigt.",0,0,255,255,0},
			{"Tisch","Ein kleiner Metalltisch.",CELL_TABLE,COMBINABLE,2,2,0}
		}
	},
	{
		"016",{1,0,0,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,1},
		NIL,NIL,&once_corridor,
		{
			{"Ausgang",&description,0,EXIT,2,2,0,GUARD3,2},
			{"Ausgang",&description,0,EXIT,3,3,0,CORRIDOR2,22}
		}
	},
	{
		"016",{1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1},
		NIL,NIL,&once_corridor,
		{
			{"Ausgang",&description,0,EXIT,2,2,0,CORRIDOR1,2},
			{"Ausgang",&description,0,EXIT,3,3,0,CORRIDOR3,22},
			{"Ausgang",&description,0,EXIT,1,1,0,CORRIDOR4,14}
		}
	},
	{
		"016",{1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
		NIL,NIL,&once_corridor,
		{
			{"Ausgang",&description,0,EXIT,2,2,0,CORRIDOR2,2}
		}
	},
	{
		"016",{1,1,1,0,0,0,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1},
		&sb_corridor4,NIL,&once_corridor4,
		{
			{"Ausgang",&description,0,EXIT,0,0,0,CORRIDOR2,10},
			{"Ausgang",&description,0,EXIT,1,1,0,GUARD,14},
			{"Zellent�r","Hier warst du eingesperrt.",
			DOOR,EXIT+OPEN+OPENED+CLOSED,7,7,0,CELL,16},
			{"Laptop",&description,NEWSPAPER,TAKE,6,6,8},
			{"Armbanduhr",&description,WATCH,TAKE+COMBINABLE,255,255,8},
			{"Tisch",&description,TABLE,COMBINABLE,5,5,0}
		}
	},
	{
		"016",{1,0,0,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1},
		&sb_corridor5,NIL,&once_corridor,
		{
			{"Ausgang",&description,DOOR,EXIT,2,2,0,0,2},
			{"Ausgang",&description,0,EXIT,3,3,0,CORRIDOR6,22}
		}
	},
	{
		"016",{1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1},
		&sb_corridor6,NIL,&once_corridor,
		{
			{"Ausgang",&description,0,EXIT,2,2,0,CORRIDOR5,2},
			{"Ausgang",&description,0,EXIT,3,3,0,CORRIDOR7,22},
			{"T�r",&description,DOOR,OPEN+CLOSED,255,255,0,CORRIDOR8,13}
		}
	},
	{
		"016",{1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1},
		NIL,NIL,&once_corridor,
		{
			{"Ausgang",&description,0,EXIT,2,2,0,CORRIDOR6,2},
			{"Ausgang",&description,0,EXIT,3,3,0,GUARD,22}
		}
	},
	{
		"016",{1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1},
		&sb_corridor8,NIL,&once_corridor,
		{
			{"T�r",&description,DOOR,EXIT+OPEN,0,0,0,CORRIDOR6,10},
			{"Ausgang",&description,0,EXIT,3,3,0,BCORRIDOR,22}
		}
	},
	{
		"016",{1,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1},
		&sb_corridor9,NIL,&once_corridor,
		{
			{"Ausgang",&description,0,EXIT,2,2,0,BCORRIDOR,2},
			{"T�r",&description,DOOR,EXIT+OPEN,0,0,0,GUARD,10}
		}
	},
	{
		"006",{1,0,0,1},&sb_bgang,NIL,&once_bgang,
		{
			{"S�ule",&description,PILLAR1,0,4,4,0},
			{"S�ule",&description,PILLAR2,0,5,5,0},
			{"Ausgang",&description,0,EXIT,6,6,0,CORRIDOR8,2},
			{"Ausgang",&description,0,EXIT,7,7,0,CORRIDOR9,22},
			{"T�r","Auf einem Schild an der T�r steht \"Dr. Alab Hansi\".",
			DOOR1,EXIT+OPEN+CLOSED+OCCUPIED,0,0,1,OFFICE_L1,6},
			{"T�r","Auf einem Schild an der T�r steht \"Saval Lun\".",
			DOOR2,EXIT+OPEN+CLOSED+OCCUPIED,1,1,2,OFFICE_L2,16},
			{"T�r","Auf einem Schild an der T�r steht \"Prof. Dr. Ugnul Tschabb\".",
			DOOR3,EXIT+OPEN+OPENED,2,2,3,OFFICE_R1,8},
			{"T�r","Auf einem Schild an der T�r steht \"Alga Hurz Li\".",
			DOOR4,EXIT+OPEN+CLOSED+OCCUPIED,3,3,4,OFFICE_R2,18}
		}
	},
	{
		"040",{1},&sb_guard,NIL,NIL,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,CORRIDOR4,21},
			{"Gang",&description,0,EXIT,3,3,0,CORRIDOR7,5},
			{"T�r",&description,DOOR,EXIT+OPEN,1,1,6,CORRIDOR9,3},
			{"Axacussaner","Du m��test ihn irgendwie ablenken.",
			GUARD,TALK,0,0,0},
			{"Bild","Komisches Bild.",0,0,2,2,0},
			{"Karte","Darauf steht: \"Generalkarte\".",
			MASTERKEYCARD,TAKE+COMBINABLE,255,255,1}
		}
	},
	{
		"042",{1},&sb_guard3,NIL,NIL,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,CORRIDOR1,22},
			{"T�r",&description,0,EXIT+OPEN+CLOSED,0,0,0,0,20},
			{"T�r",&description,0,EXIT+OPEN+CLOSED,1,1,0,0,15},
			{"T�r",&description,DOOR,EXIT+OPEN,2,2,11,OFFICE_L,0},
			{"Lampe",&description,LAMP,COMBINABLE,3,3,0},
			{"Axacussaner",&description,GUARD,TALK,5,5,0},
			{"Bild","Seltsam!",0,0,4,4,0}
		}
	},
	{
		"007",{1,0,1,0,0,0,0,1,0,1,0,0,0,0,0,0,1},
		&sb_office1,NIL,NIL,
		{
			{"T�r",&description,DOOR,EXIT+OPEN+OPENED,6,6,9,BCORRIDOR,9},
			{"Computer",&description,COMPUTER,COMBINABLE,4,4,0},
			{"Geld","Es sind 500 Xa.",TICKETS,TAKE,255,255,0},
			{"Schlie�fach","Es hat ein elektronisches Zahlenschlo�.",
			LOCKER,OPEN+CLOSED,5,5,0},
			{"Brief",&description,LETTER,UNNECESSARY,3,3,0}
		}
	},
	{
		"007",{1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
		&sb_office2,NIL,NIL,
		{
			{"T�r",&description,DOOR,EXIT+OPEN+OPENED,6,6,9,BCORRIDOR,9},
			{"Computer",&description,COMPUTER,COMBINABLE,4,4,0},
			{"W�rfel","Sonderbar!",0,0,0,0,0},
			{"Bild","Affenstark!",0,0,1,1,0},
			{"Komisches Ding","Wundersam!",0,UNNECESSARY,2,2,0}
		}
	},
	{
		"008",{1,1,0,1},&sb_office3,NIL,NIL,
		{
			{"T�r",&description,DOOR,EXIT+OPEN+OPENED,0,0,3,BCORRIDOR,5},
			{"Computer",&description,COMPUTER,COMBINABLE,4,4,0},
			{"Bild","Es ist ein Axacussanerkopf auf dem Bild.",0,UNNECESSARY,1,1,0},
			{"Bild","Es ist ein Axacussanerkopf auf dem Bild.",PAINTING,UNNECESSARY,2,2,0},
			{"Pflanze",&description,0,UNNECESSARY,3,3,0},
			{NIL,&description,MONEY,TAKE+COMBINABLE,255,255,0}
		}
	},
	{
		"008",{1,0,1,1},&sb_office4,NIL,NIL,
		{
			{"T�r",&description,DOOR,EXIT+OPEN+OPENED,0,0,3,BCORRIDOR,5},
			{"Computer",&description,COMPUTER,COMBINABLE,4,4,0},
			{"Figur","Stark!",0,UNNECESSARY,6,6,0},
			{"Pflanze","Sie ist den Pflanzen auf der Erde sehr �hnlich.",
			0,UNNECESSARY,5,5,0}
		}
	},
	{
		"007",{1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
		&sb_office5,NIL,&once_office5,
		{
			{"T�r",&description,DOOR,EXIT+OPEN+OPENED,6,6,17,GUARD3,9},
			{"Computer","Er funktioniert nicht.",COMPUTER,COMBINABLE,4,4,0},
			{"Graffiti","Seltsamer B�roschmuck!",0,0,7,7,0},
			{"Geld","Es sind 350 Xa.",TICKETS,TAKE,8,8,0}
		}
	},
	{
		"003",{1},&sb_elevator,NIL,NIL,
		{
			{"Knopf",&description,BUTTON1,PRESS,0,0,0},
			{"Knopf",&description,BUTTON2,PRESS,1,1,0},
			{"Ausgang",&description,DOOR,EXIT,255,255,0,0,22},
			{"Dschungel","Lauter B�ume.",JUNGLE,0,255,255,0,STATION,2}
		}
	},
	{
		"005",{1},&sb_station,NIL,NIL,
		{
			{"Schild",&description,STATION_SIGN,0,0,0,0},
			{"T�r",&description,DOOR,EXIT+OPEN+CLOSED,1,1,0,0,7}
		}
	},
	{
		"032",{1,1},&sb_sign,NIL,NIL,
		{
			{"Ausgang",&description,0,EXIT,255,255,0,STATION,22},
			{"Schlitz",&description,SLOT,COMBINABLE,0,0,0}
		}
	}
};

}
#endif // ROOMS_H
