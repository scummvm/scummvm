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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_GLOBALS_H
#define HOPKINS_GLOBALS_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

struct ZonePItem {
	int field0;
	int field2;
	int field4;
	int field6;
	int field7;
	int field8;
	int field9;
	int fieldA;
	int fieldB;
	int fieldC;
	int fieldD;
	int fieldE;
	int fieldF;
	int field10;
	int field12;
	int field16;
};

struct CarreZoneItem {
	int field0;
	int field2;
	int field4;
	int field6;
	int field8;
	int fieldA;
	int fieldC;
	int fieldE;
};

struct BqeAnimItem {
	byte *data;
	int field4;
};

struct BankItem {
	byte *data;
	int8 field4;
	Common::String filename1;
	int fileHeader;
	int field1A;
	uint32 field1C;
	Common::String filename2;
};

struct ListeItem {
	int field0;
	int field2;
	int field4;
	int width;
	int height;
	int fieldA;
};

struct Liste2Item {
	int field0;
	int xp;
	int yp;
	int width;
	int height;
};

struct BobItem {
	int field0;
	byte *field4;
	int field8;
	int fieldA;
	int fieldC;
	int fieldE;
	int field10;
	int field12;
	int field14;
	int field16;
	byte *field18;
	int field1C;
	int field1E;
	int field20;
	int field22;
	int field24;
	int field26;
	int field28;
	int field2A;
	int field2C;
	byte *field30;
	int field34;
	int field36;
	int field38;
	int field3A;
	int field3C;
	int field3E;
	int field40;
	int field42;
	int field44;
	int field46;
	int field48;
	int field4A;
};

struct BlAnimItem {
	uint32 v1;
	uint32 v2;
};

struct VBobItem {
	byte *field0;
	int field4;
	int field6;
	int field8;
	int fieldA;
	int fieldC;
	int fieldE;
	byte *field10;
	int field14;
	int field16;
	int field18;
	byte *field1C;
};

struct ObjetWItem {
	byte field0;
	byte field1;
	byte field2;
	byte field3;
	byte field4;
	byte field5;
	byte field6;
	byte field7;
};

struct BlocItem {
	uint16 field0;
	int x1;
	int y1;
	int x2;
	int y2;
};

struct TriItem {
	int field0;
	int field2;
	int field4;
	int field6;
};

struct CacheItem {
	int field0;
	int field2;
	int field4;
	int field6;
	int field8;
	int fieldA;
	byte *fieldC;
	int field10;
	int field12;
	int field14;
};

struct SegmentItem {
	int field0;
	int field2;
	int field4;
};

struct HopkinsItem {
	int field0;
	int field2;
};

struct Sauvegarde1 {
	int field0;
	int field1;
	int field2;
	int field3;
	int field4;
};

enum SauvegardeOffset {
	svField1 = 1
	, svField2 = 2
	, svField3 = 3
	, svField4 = 4
	, svField5 = 5
	, svField6 = 6
	, svField8 = 8
	, svField9 = 9
	, svField10 = 10
	, svField13 = 13
	, svField80 = 80
	, svField94 = 94
	, svField95 = 95
	, svField121 = 121
	, svField122 = 122
	, svField132 = 132
	, svField133 = 133
	, svField170 = 170
	, svField171 = 171
	, svField172 = 172
	, svField173 = 173
	, svField176 = 176
	, svField180 = 180
	, svField181 = 181
	, svField182 = 182
	, svField183 = 183
	, svField184 = 184
	, svField185 = 185
	, svField186 = 186
	, svField187 = 187
	, svField188 = 188
	, svField189 = 189
	, svField190 = 190
	, svField191 = 191
	, svField192 = 192
	, svField193 = 193
	, svField194 = 194
	, svField228 = 228
	, svField231 = 231
	, svField253 = 253
	, svField261 = 261
	, svField270 = 270
	, svField300 = 300
	, svField318 = 318
	, svField320 = 320
	, svField333 = 333
	, svField338 = 338
	, svField339 = 339
	, svField340 = 340
	, svField341 = 341
	, svField349 = 349
	, svField352 = 352
	, svField353 = 353
	, svField354 = 354
	, svField355 = 355
	, svField356 = 356
	, svField357 = 357
	, svField399 = 399
	, svField401 = 401
	, svField1300 = 1300
};

// TODO: Sauvegrade1 fields should really be mapped into data array
struct Sauvegarde {
	byte data[0x802];
	Sauvegarde1 field360;
	Sauvegarde1 field370;
	Sauvegarde1 field380;
};

class HopkinsEngine;

/**
 * Engine Globals
 */
class Globals {
private:
	HopkinsEngine *_vm;
public:
	byte *CACHE_BANQUE[6];
	ZonePItem ZONEP[106];
	CarreZoneItem CarreZone[101];
	BqeAnimItem Bqe_Anim[35];
	BankItem Bank[8];
	BobItem Bob[36];
	ListeItem Liste[6];
	Liste2Item Liste2[35];
	BlAnimItem BL_ANIM[30];
	VBobItem VBob[35];
	ObjetWItem ObjetW[300];
	BlocItem BLOC[250];
	CacheItem Cache[25];
	SegmentItem Segment[101];
	int BOBZONE[105];
	bool BOBZONE_FLAG[105];
	int STAILLE[500];
	int16 super_parcours[32002];
	int Param[2100];
	HopkinsItem Hopkins[70];
	int FR;
	int SVGA;
	int internet;
	int PUBEXIT;
	bool XFULLSCREEN;
	int XSETMODE;
	int XZOOM;
	bool XFORCE16;
	bool XFORCE8;
	bool CARD_SB;
	int PERSO_TYPE;
	int vitesse;
	int INSTALL_TYPE;
	Common::String HOPIMAGE;
	Common::String HOPANIM;
	Common::String HOPLINK;
	Common::String HOPSAVE;
	Common::String HOPSOUND;
	Common::String HOPMUSIC;
	Common::String HOPVOICE;
	Common::String HOPANM;
	Common::String HOPSEQ;
	Common::String FICH_ZONE;
	Common::String FICH_TEXTE;
	Common::String NFICHIER;
	Common::String REP_SPR;
	int SOUNDVOL;
	int MUSICVOL;
	int VOICEVOL;
	bool SOUNDOFF;
	bool MUSICOFF;
	bool VOICEOFF;
	bool CENSURE;
	int lItCounter;
	int lOldItCounter;
	int g_old_anim;
	int g_old_sens, g_old_sens2;
	int last_sens;
	byte *police;
	int police_l;
	int police_h;
	byte *TETE;
	byte *ICONE;
	byte *BUF_ZONE;
	byte *Winventaire;
	byte *texte_tmp;
	int texte_long;
	int TEXTE_FORMATE;
	byte *SPRITE_ECRAN;
	Sauvegarde *SAUVEGARDE;
	byte *BUFFERTAPE;
	byte *essai0;
	byte *essai1;
	byte *essai2;
	byte *Bufferobjet;
	int INVENTAIRE[36];
	TriItem Tri[51];
	byte *inventaire2;
	byte *GESTE;
	int OBJET_EN_COURS;
	int NUM_FICHIER_OBJ;
	int nbrligne;
	int largeur_boite;
	byte *INVENTAIRE_OBJET;
	int hauteur_boite;
	int GESTE_FLAG;
	byte *FORETSPR;
	int FORET;
	byte *COUCOU;
	byte *chemin;
	byte *cache_souris;
	byte *BufLig;
	byte *Bufferdecor;
	byte *ADR_FICHIER_OBJ;
	byte *PERSO;
	bool redraw;
	int OBJL, OBJH;
	int Nouv_objet;
	int HELICO;
	uint32 CAT_POSI;
	uint32 CAT_TAILLE;
	bool CAT_FLAG;
	int iRegul;
	bool BPP_NOAFF;
	int SORTIE;
	int PLANX, PLANY;
	int PLANI;
	int PASSWORD;
	int NOSPRECRAN;
	int ECRAN;
	int OLD_ECRAN;
	int Max_Propre;
	int Max_Propre_Gen;
	int Max_Ligne_Long;
	int Max_Perso_Y;
	bool DESACTIVE_INVENT;
	bool FLAG_VISIBLE;
	bool netscape;
	bool NOMARCHE;
	int NBBLOC;
	bool NO_VISU;
	byte *OPTION_SPR;
	bool OPTION_FLAG;
	int opt_anm;
	int opt_vitesse;
	int opt_scrtype;
	int opt_scrspeed;
	int opt_sound;
	int opt_voice;
	int opt_music;
	int opt_txt;
	int NBTRI;
	int AFFLI;
	int AFFIVBL;
	int NOT_VERIF;
	bool CACHEFLAG;
	int AFFINVEN;
	bool NOPARLE;
	bool PLAN_FLAG;
	int GOACTION;
	int NECESSAIRE;
	int Compteur;
	int ACTION_SENS;
	int SegmentEnCours;

	int STOP_BUG;
	bool NO_OFFSET;
	int MAX_COMPTE;

	int force_to_data_0;
	int oldzone_46;
	int old_x1_65;
	int old_y1_66;
	int old_x2_67;
	int old_zone_68;
	int old_x_69, old_y_70;
	int compteur_71;
	int zozo_73;
	int old_y2_68;
	int old_z_69;
	int j_104;

	Globals();
	~Globals();
	void setParent(HopkinsEngine *vm);

	void setConfig();
	void clearAll();
	void HOPKINS_DATA();
	void INIT_ANIM();
	void INIT_VBOB();
	void CLEAR_VBOB();
	void CHARGE_OBJET();
	byte *dos_malloc2(int count);
	byte *dos_free2(byte *p);
	byte *LIBERE_FICHIER(byte *p);

	void RESET_CACHE();
	void CACHE_ON(); 
	void CACHE_OFF(int v1 = 0);
	void CACHE_SUB(int idx); 
	void CACHE_ADD(int idx);
	void CHARGE_CACHE(const Common::String &file);
	void B_CACHE_OFF(int idx);
};

// Global null pointer
extern byte *g_PTRNUL;

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
