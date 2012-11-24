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

#ifndef HOPKINS_OBJECTS_H
#define HOPKINS_OBJECTS_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/str.h"
#include "hopkins/globals.h"

namespace Hopkins {

struct SpriteItem {
	int field0;
	const byte *spriteData;
	Common::Point spritePos;
	int spriteY;
	int fieldC;
	int fieldE;
	int spriteIndex;
	int field12;
	int field14;
	byte *field1C;
	int field20;
	int field22;
	int field24;
	int field26;
	int field28;
	int field2A;
	int field2C;
	int field2E;
	int field30;
	int field32;
	int field34;
	int field36;
};

class HopkinsEngine;

class ObjectsManager {
private:
	HopkinsEngine *_vm;

public:
	SpriteItem Sprite[6];
	int PRIORITY;
	int old_cadx, old_cady, old_cadi;
	int cadx, cady, cadi;
	int SL_X, SL_Y;
	int I_old_x, I_old_y;
	int g_old_x, g_old_y;
	int FLAG_VISIBLE_EFFACE;
	byte *SL_SPR;
	byte *SL_SPR2;
	byte *sprite_ptr;
	const byte *S_old_spr; 
	bool PERSO_ON;
	bool SL_FLAG;
	int SL_MODE;
	bool FLAG_VISIBLE;
	int DESACTIVE_CURSOR;
	bool BOBTOUS;
	int my_anim;
	int NUMZONE;
	int ARRET_PERSO_FLAG;
	int ARRET_PERSO_NUM;
	int FORCEZONE;
	int CHANGEVERBE;
	int verbe;
	int Vold_taille;
	bool SPEED_FLAG;
	int SPEED_X, SPEED_Y;
	int SPEED_IMAGE;
	byte *SPEED_PTR;
	int DERLIGNE;
	int A_ANIM;
	int MA_ANIM;
	int MA_ANIM1;
	int A_DEPA;
	int MAX_DEPA;
	int MAX_DEPA1;
	int CH_TETE;
	int T_RECTIF;
	bool DESACTIVE;
	bool DEUXPERSO;
	int PERX, PERY;
	int PERI;
	int RECALL;
	int PTAILLE;
	int PEROFX;
	int PEROFY;
	int OBSSEUL;
	int NVVERBE;
	int NVZONE;
	int S_old_ani; 
	int S_old_ret;
	int nouveau_x, nouveau_y;
	int nouveau_sens;
	int nouveau_anim;
public:
	ObjectsManager();
	void setParent(HopkinsEngine *vm);

	byte *CHANGE_OBJET(int objIndex);
	byte *CAPTURE_OBJET(int objIndex, int mode);
	void DELETE_OBJET(int objIndex);

	int Get_Largeur(const byte *objectData, int idx);
	int Get_Hauteur(const byte *objectData, int idx);
	int sprite_alone(const byte *objectData, byte *sprite, int objIndex);
	byte *DEL_FICHIER_OBJ();

	byte *CHARGE_SPRITE(const Common::String &file);
	void set_offsetxy(byte *data, int idx, int xp, int yp, bool isSize);
	int get_offsetx(const byte *spriteData, int spriteIndex, bool isSize);
	int get_offsety(const byte *spriteData, int spriteIndex, bool isSize);
	void AFF_SPRITES();

	int capture_mem_sprite(const byte *objectData, byte *sprite, int objIndex);
	int AJOUTE_OBJET(int objIndex);

	void INIT_BOB();
	void BOB_ZERO(int idx);
	void DEF_BOB(int idx);
	void BOB_VISU(int idx);
	void BOB_OFF(int idx);
	void BOB_OFFSET(int idx, int v);
	void BOB_ADJUST(int idx, int v);
	void BOB_OFFSETY(int idx, int v);
	void SCBOB(int idx);
	void CALCUL_BOB(int idx);

	void VERIFCACHE();
	void DEF_SPRITE(int idx);
	void DEF_CACHE(int idx);
	void CALCUL_SPRITE(int idx);
	int AvantTri(TriMode triMode, int index, int priority);
	void AFF_BOB_ANIM();
	void AFF_VBOB();

	int XSPR(int idx);
	int YSPR(int idx);
	void SPRITE_NOW(const byte *spriteData, int a2, int a3, int a4, int a5, int a6, int a7, int a8);

	void CLEAR_SPR();
	void SPRITE_ON(int idx);
	void SPRITE(const byte *spriteData, int xp, int yp, int idx, int spriteIndex, int a6, int a7, int a8, int a9);
	void SPRITE2(const byte *spriteData, int idx, byte *a3, int a4, int a5);
	void SPRITE_OFF(int idx);
	void SPRITE_GEL(int idx);
	int SXSPR(int idx);
	int SYSPR(int idx);
	int POSISPR(int idx);
	void SETPOSISPR(int idx, int a2);
	void SETXSPR(int idx, int xp);
	void SETANISPR(int idx, int spriteIndex);
	void SETYSPR(int idx, int yp);
	void SETTAILLESPR(int idx, int a2);
	void SETFLIPSPR(int idx, int a2);

	void VERIFZONE();
	void GOHOME();
	void GOHOME2();
	void CHARGE_OBSTACLE(const Common::String &file);
	void CHARGE_ZONE(const Common::String &file);
	void CARRE_ZONE();
	void PLAN_BETA();
	void BTGAUCHE();
	void PARADISE();
	void CLEAR_ECRAN();
	
	void CHANGE_TETE(PlayerCharacter a1, PlayerCharacter a2);
	void VERIFTAILLE();
	void PACOURS_PROPRE(int16 *a1);
	int16 *PARC_VOITURE(int a1, int a2, int a3, int a4);
	void VERBEPLUS();
	void BTDROITE();
	int MZONE();
	void PARAMCADRE(int a1);
	void OBJETPLUS(int a1);
	void VALID_OBJET(int a1);
	void OPTI_OBJET();
	void SPECIAL_JEU();
	void BOB_VIVANT(int a1);
	void VBOB(byte *src, int idx, int xp, int yp, int frameIndex);
	void VBOB_OFF(int idx);
	void ACTION_DOS(int idx);
	void ACTION_DROITE(int idx);
	void Q_DROITE(int idx);
	void ACTION_FACE(int idx);
	void Q_GAUCHE(int idx);
	void ACTION_GAUCHE(int idx);

	void ZONE_ON(int idx);
	void ZONE_OFF(int idx);
	void OPTI_ONE(int a1, int a2, int a3, int a4);
	void AFFICHE_SPEED1(byte *speedData, int xp, int yp, int img);
	int BOBPOSI(int idx);
	void BOBANIM_ON(int idx);
	void BOBANIM_OFF(int idx);
	void SET_BOBPOSI(int idx, int a2);
	int BOBX(int idx);
	int BOBY(int idx);
	int BOBA(int idx);

	void INILINK(const Common::String &file);
	void SPECIAL_INI(const Common::String &a1);
	void OPTI_BOBON(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
	void SCI_OPTI_ONE(int a1, int a2, int a3, int a4);
	void VERBE_OFF(int idx, int a2);
	void VERBE_ON(int idx, int a2);
	int CALC_PROPRE(int idx);
	int colision(int xp, int yp);

	void ACTION(const byte *spriteData, const Common::String &a2, int a3, int a4, int a5, int a6);
	void SPACTION(byte *a1, const Common::String &animationSeq, int a3, int a4, int a5, int a6);
	void SPACTION1(byte *a1, const Common::String &a2, int a3, int a4, int a5);
	void TEST_FORET(int a1, int a2, int a3, int a4, int a5, int a6);
	void BLOQUE_ANIMX(int idx, int a2);

	/**
	 * Game scene control method
	 */
	void PERSONAGE(const Common::String &backgroundFile, const Common::String &linkFile, 
			const Common::String &animFile, const Common::String &s4, int v);

	/**
	 * Game scene control method
	 */
	void PERSONAGE2(const Common::String &backgroundFile, const Common::String &linkFile, 
			const Common::String &animFile, const Common::String &s4, int v);
};

} // End of namespace Hopkins

#endif /* HOPKINS_OBJECTS_H */
