/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef __PSX_PcDefines_H__
#define __PSX_PcDefines_H__

#include "engines/icb/common/px_rccommon.h"

namespace ICB {

// We'll need a u_char type
typedef unsigned char u_char;

// We'll need a u_short type
typedef unsigned short u_short;

// a u_int32 type
// typedef unsigned long u_int32;

// a uint type
typedef unsigned int u_int;

// another uint type
typedef unsigned int uint;

// a uint32 type
typedef unsigned int uint32;

// a uint16 type
typedef unsigned short uint16;

// a int32 type
typedef signed int int32;

// a int16 type
typedef short int16;

// a uint8 type
typedef unsigned char uint8;

// make our own equivalents
typedef struct MATRIX {
	short m[3][3]; /* 3x3 rotation matrix */
	short pad;
	int32 t[3]; /* transfer vector */
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
	MATRIX() { pad = 0; }
#endif
} MATRIX;

typedef struct MATRIXPC {
	int m[3][3]; /* 3x3 rotation matrix */
	int pad;
	int32 t[3]; /* transfer vector */
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
	MATRIXPC() { pad = 0; }
#endif
} MATRIXPC;

/* int32 word type 3D vector */
typedef struct VECTOR {
	int32 vx, vy;
	int32 vz, pad;
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
	VECTOR() { pad = 0; }
#endif
} VECTOR;

/* short word type 3D vector */
typedef struct SVECTOR {
	short vx, vy;
	short vz, pad;
#if (_PSX == 0)

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
	SVECTOR() { pad = 0; }
	bool operator==(const SVECTOR &v) { return ((v.vx == vx) && (v.vy == vy) && (v.vz == vz)); }
#endif
#endif // #if (_PSX==0)
} SVECTOR;

/* short word type 3D vector - PC version */
typedef struct SVECTORPC {
	int vx, vy;
	int vz, pad;
#if (_PSX == 0)

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
	SVECTORPC() { pad = 0; }
	bool operator==(const SVECTORPC &v) { return ((v.vx == vx) && (v.vy == vy) && (v.vz == vz)); }
#endif
#endif // #if (_PSX==0)
} SVECTORPC;

typedef struct CVECTOR {
	/* color type vector */
	u_char r, g, b, cd;
} CVECTOR;

typedef struct {/* 2D short vector */
	short vx, vy;
} DVECTOR;

typedef unsigned char PACKET;

/* For holding the coordinate frame of the bone animation */
typedef struct _GsCOORDUNIT {
	uint32 flg;
	MATRIX matrix;
	MATRIX workm;
	SVECTOR rot;
	struct _GsCOORDUNIT *super;
} GsCOORDUNIT;

typedef struct {
	MATRIX view;
	GsCOORDUNIT *super;
} GsVIEWUNIT;

typedef struct {
	int32 vpx, vpy, vpz;
	int32 vrx, vry, vrz;
	int32 rz;
	GsCOORDUNIT *super;
} GsRVIEWUNIT;

typedef struct GsUNIT {
	GsCOORDUNIT *coord; /* local dmatrix */
	uint32 *primtop;
} GsUNIT;

typedef struct GsTYPEUNIT {
	uint32 type;
	uint32 *ptr;
} GsTYPEUNIT;

typedef struct GsOT_TAG {
	unsigned p : 24;
	unsigned char num : 8;
} GsOT_TAG;

typedef struct GsOT {
	uint32 length;
	GsOT_TAG *org;
	uint32 offset;
	uint32 point;
	GsOT_TAG *tag;
} GsOT;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
} GsARGUNIT;

typedef struct {
	DVECTOR vec;
	short otz;
	short p;
} GsWORKUNIT;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *primtop;
	SVECTOR *vertop;
	SVECTOR *nortop;
} GsARGUNIT_NORMAL;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *primtop;
	SVECTOR *vertop;
	GsWORKUNIT *vertop2;
	SVECTOR *nortop;
	SVECTOR *nortop2;
} GsARGUNIT_SHARED;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *imagetop;
	uint32 *cluttop;
} GsARGUNIT_IMAGE;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *polytop;
	uint32 *boxtop;
	uint32 *pointtop;
	SVECTOR *nortop;
} GsARGUNIT_GND;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *polytop;
	uint32 *boxtop;
	uint32 *pointtop;
	SVECTOR *nortop;
	uint32 *uvtop;
} GsARGUNIT_GNDT;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *cparam;
	GsCOORDUNIT *coord;
	GsCOORDUNIT *rcoord;
} GsARGUNIT_CAMERA;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *lparam;
	GsCOORDUNIT *coord;
	GsCOORDUNIT *rcoord;
} GsARGUNIT_LIGHT;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *coord_sect;
	int32 *mimepr;
	uint32 mimenum;
	u_short mimeid, reserved;
	uint32 *mime_diff_sect;
} GsARGUNIT_JntMIMe;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	uint32 *coord_sect;
	u_short mimeid, reserved;
	uint32 *mime_diff_sect;
} GsARGUNIT_RstJntMIMe;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	int32 *mimepr;
	uint32 mimenum;
	u_short mimeid, reserved;
	uint32 *mime_diff_sect;
	SVECTOR *orgs_vn_sect;
	SVECTOR *vert_sect;
	SVECTOR *norm_sect;
} GsARGUNIT_VNMIMe;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	u_short mimeid, reserved;
	uint32 *mime_diff_sect;
	SVECTOR *orgs_vn_sect;
	SVECTOR *vert_sect;
	SVECTOR *norm_sect;
} GsARGUNIT_RstVNMIMe;

typedef struct {
	uint32 *primp;
	GsOT *tagp;
	int shift;
	int offset;
	PACKET *out_packetp;
	int32 header_size;
	uint32 *htop;
	uint32 *ctop;
	uint32 *ptop;
} GsARGUNIT_ANIM;

typedef struct {
	short idx;
	u_char sid;
	u_char pad;
} GsSEH;

typedef struct {
	uint32 rewrite_idx;
	u_short size, num;
	u_short ii;
	u_short aframe;
	u_char sid;
	signed char speed;
	u_short srcii;
	short rframe;
	u_short tframe;
	u_short ci, ti;
	u_short start;
	u_char start_sid;
	u_char traveling;
} GsSEQ;

/*
 * GsTYPEUNIT code macro
 */

#define GsUF3 0x00000008  /* flat triangle */
#define GsUFT3 0x00000009 /* texture flat triangle */
#define GsUG3 0x0000000c  /* gour triangle */
#define GsUGT3 0x0000000d /* texture gour triangle */
#define GsUF4 0x00000010  /* flat quad */
#define GsUFT4 0x00000011 /* texture flat quad */
#define GsUG4 0x00000014  /* gour quad */
#define GsUGT4 0x00000015 /* texture gour quad */

#define GsUFF3 0x00020008  /* fog flat triangle */
#define GsUFFT3 0x00020009 /* fog texture flat triangle */
#define GsUFG3 0x0002000c  /* fog gour triangle */
#define GsUFGT3 0x0002000d /* fog texture gour triangle */
#define GsUFF4 0x00020010  /* fog flat quad */
#define GsUFFT4 0x00020011 /* fog texture flat quad */
#define GsUFG4 0x00020014  /* fog gour quad */
#define GsUFGT4 0x00020015 /* fog texture gour quad */

#define GsUCF3 0x0000000a  /* colored flat triangle */
#define GsUCFT3 0x0000000b /* colored texture flat triangle */
#define GsUCG3 0x0000000e  /* colored gour triangle */
#define GsUCGT3 0x0000000f /* colored texture gour triangle */
#define GsUCF4 0x00000012  /* colored flat quad */
#define GsUCFT4 0x00000013 /* colored texture flat quad */
#define GsUCG4 0x00000016  /* colored gour quad */
#define GsUCGT4 0x00000017 /* colored texture gour quad */

#define GsUNF3 0x00040048  /* nonLight flat triangle */
#define GsUNFT3 0x00040049 /* nonLight texture flat triangle */
#define GsUNG3 0x0004004c  /* nonLight gouraud triangle */
#define GsUNGT3 0x0004004d /* nonLight texture gouraud triangle */
#define GsUNF4 0x00040050  /* nonLight flat quad */
#define GsUNFT4 0x00040051 /* nonLight texture flat quad */
#define GsUNG4 0x00040054  /* nonLight gouraud quad */
#define GsUNGT4 0x00040055 /* nonLight texture gouraud quad */

#define GsUDF3 0x00010008  /* div flat triangle */
#define GsUDFT3 0x00010009 /* div texture flat triangle */
#define GsUDG3 0x0001000c  /* div gour triangle */
#define GsUDGT3 0x0001000d /* div texture gour triangle */
#define GsUDF4 0x00010010  /* div flat quad */
#define GsUDFT4 0x00010011 /* div texture flat quad */
#define GsUDG4 0x00010014  /* div gour quad */
#define GsUDGT4 0x00010015 /* div texture gour quad */

#define GsUDFF3 0x00030008  /* div fog flat triangle */
#define GsUDFFT3 0x00030009 /* div fog texture flat triangle */
#define GsUDFG3 0x0003000c  /* div fog gour triangle */
#define GsUDFGT3 0x0003000d /* div fog texture gour triangle */
#define GsUDFF4 0x00030010  /* div fog flat quad */
#define GsUDFFT4 0x00030011 /* div fog texture flat quad */
#define GsUDFG4 0x00030014  /* div fog gour quad */
#define GsUDFGT4 0x00030015 /* div fog texture gour quad */

#define GsUDNF3 0x00050048  /* div nonLight flat triangle */
#define GsUDNFT3 0x00050049 /* div nonLight texture flat triangle */
#define GsUDNG3 0x0005004c  /* div nonLight gouraud triangle */
#define GsUDNGT3 0x0005004d /* div nonLight tex gouraud triangle */
#define GsUDNF4 0x00050050  /* div nonLight flat quad */
#define GsUDNFT4 0x00050051 /* div nonLight texture flat quad */
#define GsUDNG4 0x00050054  /* div nonLight gouraud quad */
#define GsUDNGT4 0x00050055 /* div nonLight tex gouraud quad */

#define GsUSCAL 0x01000000 /* shared calculate vertex and normal */
#define GsUSG3 0x0100000c  /* shared gour triangle */
#define GsUSGT3 0x0100000d /* shared texture gour triangle */
#define GsUSG4 0x01000014  /* shared gour quad */
#define GsUSGT4 0x01000015 /* shared texture gour quad */

#define GsUSTGT3 0x0100020d /* shared tile texture gour triangle */
#define GsUSTGT4 0x01000215 /* shared tile texture gour quad */

#define GsUSFG3 0x0102000c  /* shared fog gour triangle */
#define GsUSFGT3 0x0102000d /* shared fog texture gour triangle */
#define GsUSFG4 0x01020014  /* shared fog gour quad */
#define GsUSFGT4 0x01020015 /* shared fog texture gour quad */

#define GsUSNF3 0x01040048  /* shared nonLight flat tri */
#define GsUSNFT3 0x01040049 /* shared nonLight texture flat tri */
#define GsUSNG3 0x0104004c  /* shared nonLight gour tri */
#define GsUSNGT3 0x0104004d /* shared nonLight texture gour tri */
#define GsUSNF4 0x01040050  /* shared nonLight flat quad */
#define GsUSNFT4 0x01040051 /* shared nonLight texture flat quad */
#define GsUSNG4 0x01040054  /* shared nonLight gour quad */
#define GsUSNGT4 0x01040055 /* shared nonLight texture gour quad */

#define GsUMF3 0x00000018   /* mesh flat tri */
#define GsUMFT3 0x00000019  /* mesh texture flat tri */
#define GsUMG3 0x0000001c   /* mesh gour triangle */
#define GsUMGT3 0x0000001d  /* mesh texture gour triangle */
#define GsUMNF3 0x00040058  /* mesh nonLight flat tri */
#define GsUMNFT3 0x00040059 /* mesh nonLight tex flat tri */
#define GsUMNG3 0x0004005c  /* mesh nonLight gour triangle */
#define GsUMNGT3 0x0004005d /* mesh nonLight tex gour tri */

#define GsUTFT3 0x00000209 /* tile texture flat triangle */
#define GsUTGT3 0x0000020d /* tile texture gour triangle */
#define GsUTFT4 0x00000211 /* tile texture flat quad */
#define GsUTGT4 0x00000215 /* tile texture gour quad */

#define GsUPNF3 0x00040148  /* preset nonLight flat triangle */
#define GsUPNFT3 0x00040149 /* preset nonLight tex flat triangle */
#define GsUPNG3 0x0004014c  /* preset nonLight gouraud triangle */
#define GsUPNGT3 0x0004014d /* preset nonLight tex gour triangle */
#define GsUPNF4 0x00040150  /* preset nonLight flat quad */
#define GsUPNFT4 0x00040151 /* preset nonLight tex flat quad */
#define GsUPNG4 0x00040154  /* preset nonLight gouraud quad */
#define GsUPNGT4 0x00040155 /* preset nonLight tex gour quad */

#define GsUSTPF3 0x00200008   /* semi-trans flat triangle */
#define GsUSTPFT3 0x00200009  /* semi-trans texture flat triangle */
#define GsUSTPG3 0x0020000c   /* semi-trans gour triangle */
#define GsUSTPGT3 0x0020000d  /* semi-trans texture gour triangle */
#define GsUSTPF4 0x00200010   /* semi-trans flat quad */
#define GsUSTPFT4 0x00200011  /* semi-trans texture flat quad */
#define GsUSTPG4 0x00200014   /* semi-trans gour quad */
#define GsUSTPGT4 0x00200015  /* semi-trans texture gour quad */
#define GsUSTPSG3 0x0120000c  /* semi-trans shared gour tri */
#define GsUSTPSGT3 0x0120000d /* semi-trans shared tex gour tri */
#define GsUSTPSG4 0x01200014  /* semi-trans shared gour quad */
#define GsUSTPSGT4 0x01200015 /* semi-trans shared tex gour quad */

#define GsUSTPNF3 0x00240048  /* semi-trans nonLight flat tri */
#define GsUSTPNFT3 0x00240049 /* semi-trans nonLight tex flat tri */
#define GsUSTPNG3 0x0024004c  /* semi-trans nonLight gour tri */
#define GsUSTPNGT3 0x0024004d /* semi-trans nonLight tex gour tri */
#define GsUSTPNF4 0x00240050  /* semi-trans nonLight flat quad */
#define GsUSTPNFT4 0x00240051 /* semi-trans nonLight tex flat quad */
#define GsUSTPNG4 0x00240054  /* semi-trans nonLight gour quad */
#define GsUSTPNGT4 0x00240055 /* semi-trans nonLight tex gour quad */

#define GsUSTPSNF3 0x01240048  /* stp shared nonLight flat tri */
#define GsUSTPSNFT3 0x01240049 /* stp shared nonLight tex flat tri */
#define GsUSTPSNG3 0x0124004c  /* stp shared nonLight gour tri */
#define GsUSTPSNGT3 0x0124004d /* stp shared nonLight tex gour tri */
#define GsUSTPSNF4 0x01240050  /* stp shared nonLight flat quad */
#define GsUSTPSNFT4 0x01240051 /* stp shared nonLight tex flat quad */
#define GsUSTPSNG4 0x01240054  /* stp shared nonLight gour quad */
#define GsUSTPSNGT4 0x01240055 /* stp shared nonLight tex gour quad */

#define GsUADF3 0x00080008  /* active-div flat triangle */
#define GsUADFT3 0x00080009 /* active-div texture flat triangle */
#define GsUADG3 0x0008000c  /* active-div gour triangle */
#define GsUADGT3 0x0008000d /* active-div texture gour triangle */
#define GsUADF4 0x00080010  /* active-div flat quad */
#define GsUADFT4 0x00080011 /* active-div texture flat quad */
#define GsUADG4 0x00080014  /* active-div gour quad */
#define GsUADGT4 0x00080015 /* active-div texture gour quad */

#define GsUADFF3 0x000a0008  /* active-div fog flat tri */
#define GsUADFFT3 0x000a0009 /* active-div fog texture flat tri */
#define GsUADFG3 0x000a000c  /* active-div fog gour tri */
#define GsUADFGT3 0x000a000d /* active-div fog texture gour tri */
#define GsUADFF4 0x000a0010  /* active-div fog flat quad */
#define GsUADFFT4 0x000a0011 /* active-div fog texture flat quad */
#define GsUADFG4 0x000a0014  /* active-div fog gour quad */
#define GsUADFGT4 0x000a0015 /* active-div fog texture gour quad */

#define GsUADNF3 0x000c0048  /* active-div nonLight flat tri */
#define GsUADNFT3 0x000c0049 /* active-div nonLight tex flat tri */
#define GsUADNG3 0x000c004c  /* active-div nonLight gour tri */
#define GsUADNGT3 0x000c004d /* active-div nonLight tex gour tri */
#define GsUADNF4 0x000c0050  /* active-div nonLight flat quad */
#define GsUADNFT4 0x000c0051 /* active-div nonLight tex flat quad */
#define GsUADNG4 0x000c0054  /* active-div nonLight gour quad */
#define GsUADNGT4 0x000c0055 /* active-div nonLight tex gour quad */

#define GsUBF3 0x00100008  /* back-f flat tri */
#define GsUBFT3 0x00100009 /* back-f tex flat tri */
#define GsUBG3 0x0010000c  /* back-f gour tri */
#define GsUBGT3 0x0010000d /* back-f tex gour tri */
#define GsUBF4 0x00100010  /* back-f flat quad */
#define GsUBFT4 0x00100011 /* back-f tex flat quad */
#define GsUBG4 0x00100014  /* back-f gour quad */
#define GsUBGT4 0x00100015 /* back-f tex gour quad */

#define GsUBCF3 0x0010000a  /* back-f colored flat tri */
#define GsUBCFT3 0x0010000b /* back-f colored tex flat tri */
#define GsUBCG3 0x0010000e  /* back-f colored gour tri */
#define GsUBCGT3 0x0010000f /* back-f colored tex gour tri */
#define GsUBCF4 0x00100012  /* back-f colored flat quad */
#define GsUBCFT4 0x00100013 /* back-f colored tex flat quad */
#define GsUBCG4 0x00100016  /* back-f colored gour quad */
#define GsUBCGT4 0x00100017 /* back-f colored tex gour quad */

#define GsUBSTPF3 0x00300008  /* back-f semi-trans flat tri */
#define GsUBSTPFT3 0x00300009 /* back-f semi-trans tex flat tri */
#define GsUBSTPG3 0x0030000c  /* back-f semi-trans gour tri */
#define GsUBSTPGT3 0x0030000d /* back-f semi-trans tex gour tri */
#define GsUBSTPF4 0x00300010  /* back-f semi-trans flat quad */
#define GsUBSTPFT4 0x00300011 /* back-f semi-trans tex flat quad */
#define GsUBSTPG4 0x00300014  /* back-f semi-trans gour quad */
#define GsUBSTPGT4 0x00300015 /* back-f semi-trans tex gour quad */

#define GsUBNF3 0x00140048  /* back-f noLgt flat tri */
#define GsUBNFT3 0x00140049 /* back-f noLgt tex flat tri */
#define GsUBNG3 0x0014004c  /* back-f noLgt gouraud tri */
#define GsUBNGT3 0x0014004d /* back-f noLgt tex gouraud tri */
#define GsUBNF4 0x00140050  /* back-f noLgt flat quad */
#define GsUBNFT4 0x00140051 /* back-f noLgt tex flat quad */
#define GsUBNG4 0x00140054  /* back-f noLgt gouraud quad */
#define GsUBNGT4 0x00140055 /* back-f noLgt tex gouraud quad */

#define GsUBSTPNF3 0x00340048  /* back-f stp noLgt flat tri */
#define GsUBSTPNFT3 0x00340049 /* back-f stp noLgt tex flat tri */
#define GsUBSTPNG3 0x0034004c  /* back-f stp noLgt gour tri */
#define GsUBSTPNGT3 0x0034004d /* back-f stp noLgt tex gour tri */
#define GsUBSTPNF4 0x00340050  /* back-f stp noLgt flat quad */
#define GsUBSTPNFT4 0x00340051 /* back-f stp noLgt tex flat quad */
#define GsUBSTPNG4 0x00340054  /* back-f stp noLgt gour quad */
#define GsUBSTPNGT4 0x00340055 /* back-f stp noLgt tex gour quad */

#define GsUBSNF3 0x01140048  /* back-f shrd noLgt flat tri */
#define GsUBSNFT3 0x01140049 /* back-f shrd noLgt tex flat tri */
#define GsUBSNG3 0x0114004c  /* back-f shrd noLgt gour tri */
#define GsUBSNGT3 0x0114004d /* back-f shrd noLgt tex gour tri */
#define GsUBSNF4 0x01140050  /* back-f shrd noLgt flat quad */
#define GsUBSNFT4 0x01140051 /* back-f shrd noLgt tex flat quad */
#define GsUBSNG4 0x01140054  /* back-f shrd noLgt gour quad */
#define GsUBSNGT4 0x01140055 /* back-f shrd noLgt tex gour quad */

#define GsUBSTPSNF3 0x01340048  /* back-f stp shrd noLgt flat tri */
#define GsUBSTPSNFT3 0x01340049 /* back-f stp shrd noLgt tex flat tri */
#define GsUBSTPSNG3 0x0134004c  /* back-f stp shrd noLgt gour tri */
#define GsUBSTPSNGT3 0x0134004d /* back-f stp shrd noLgt tex gour tri */
#define GsUBSTPSNF4 0x01340050  /* back-f stp shrd noLgt flat quad */
#define GsUBSTPSNFT4 0x01340051 /* back-f stp shrd noLgt tex flat quad*/
#define GsUBSTPSNG4 0x01340054  /* back-f stp shrd noLgt gour quad */
#define GsUBSTPSNGT4 0x01340055 /* back-f stp shrd noLgt tex gour quad*/

#define GsUIMG0 0x02000000 /* image data with no-clut */
#define GsUIMG1 0x02000001 /* image data with clut */

#define GsUGNDF 0x05000000  /* ground flat */
#define GsUGNDFT 0x05000001 /* ground flat texture */

#define GsUSCAL2 0x06000100  /* envmap shared calculate */
#define GsUE1G3 0x0600100c   /* envmap 1D gour tri */
#define GsUE1G4 0x06001014   /* envmap 1D gour quad */
#define GsUE1SG3 0x0600110c  /* envmap 1D shared gour tri */
#define GsUE1SG4 0x06001114  /* envmap 1D shared gour quad */
#define GsUE2LG3 0x0600200c  /* envmap 2D reflect gour tri */
#define GsUE2LG4 0x06002014  /* envmap 2D reflect gour quad */
#define GsUE2RG3 0x0600300c  /* envmap 2D refract gour tri */
#define GsUE2RG4 0x06003014  /* envmap 2D refract gour quad */
#define GsUE2RLG3 0x0600400c /* envmap 2D both gour tri */
#define GsUE2RLG4 0x06004014 /* envmap 2D both gour quad */
#define GsUE2OLG3 0x0600500c /* envmap 2D org+reflect gour tri */
#define GsUE2OLG4 0x06005014 /* envmap 2D org+reflect gour quad */

#define GsVtxMIMe 0x04010020        /* Vertex-MIMe */
#define GsNrmMIMe 0x04010021        /* Normal-MIMe */
#define GsRstVtxMIMe 0x04010028     /* Reset-Vertex-MIMe */
#define GsRstNrmMIMe 0x04010029     /* Reset-Normal-MIMe */
#define GsJntAxesMIMe 0x04010010    /* Joint-Axes-MIMe */
#define GsRstJntAxesMIMe 0x04010018 /* Reset-Joint-Axes-MIMe */
#define GsJntRPYMIMe 0x04010011     /* Joint-RPY-MIMe */
#define GsRstJntRPYMIMe 0x04010019  /* Reset-Joint-RPY-MIMe */

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif
extern uint32 *GsU_00000008(GsARGUNIT *);
extern uint32 *GsU_00000009(GsARGUNIT *);
extern uint32 *GsU_0000000c(GsARGUNIT *);
extern uint32 *GsU_0000000d(GsARGUNIT *);
extern uint32 *GsU_00000010(GsARGUNIT *);
extern uint32 *GsU_00000020(GsARGUNIT *);
extern uint32 *GsU_00000011(GsARGUNIT *);
extern uint32 *GsU_00000014(GsARGUNIT *);
extern uint32 *GsU_00000015(GsARGUNIT *);
extern uint32 *GsU_00020008(GsARGUNIT *);
extern uint32 *GsU_00020009(GsARGUNIT *);
extern uint32 *GsU_0002000c(GsARGUNIT *);
extern uint32 *GsU_0002000d(GsARGUNIT *);
extern uint32 *GsU_00020010(GsARGUNIT *);
extern uint32 *GsU_00020011(GsARGUNIT *);
extern uint32 *GsU_00020014(GsARGUNIT *);
extern uint32 *GsU_00020015(GsARGUNIT *);
extern uint32 *GsU_0000000a(GsARGUNIT *);
extern uint32 *GsU_0000000b(GsARGUNIT *);
extern uint32 *GsU_0000000e(GsARGUNIT *);
extern uint32 *GsU_0000000f(GsARGUNIT *);
extern uint32 *GsU_00000012(GsARGUNIT *);
extern uint32 *GsU_00000013(GsARGUNIT *);
extern uint32 *GsU_00000016(GsARGUNIT *);
extern uint32 *GsU_00000017(GsARGUNIT *);
extern uint32 *GsU_00030008(GsARGUNIT *);
extern uint32 *GsU_00030009(GsARGUNIT *);
extern uint32 *GsU_0003000c(GsARGUNIT *);
extern uint32 *GsU_0003000d(GsARGUNIT *);
extern uint32 *GsU_00030010(GsARGUNIT *);
extern uint32 *GsU_00030011(GsARGUNIT *);
extern uint32 *GsU_00030014(GsARGUNIT *);
extern uint32 *GsU_00030015(GsARGUNIT *);
extern uint32 *GsU_00040048(GsARGUNIT *);
extern uint32 *GsU_00040049(GsARGUNIT *);
extern uint32 *GsU_0004004c(GsARGUNIT *);
extern uint32 *GsU_0004004d(GsARGUNIT *);
extern uint32 *GsU_00040050(GsARGUNIT *);
extern uint32 *GsU_00040051(GsARGUNIT *);
extern uint32 *GsU_00040054(GsARGUNIT *);
extern uint32 *GsU_00040055(GsARGUNIT *);
extern uint32 *GsU_00010008(GsARGUNIT *);
extern uint32 *GsU_00010009(GsARGUNIT *);
extern uint32 *GsU_0001000c(GsARGUNIT *);
extern uint32 *GsU_0001000d(GsARGUNIT *);
extern uint32 *GsU_00010010(GsARGUNIT *);
extern uint32 *GsU_00010011(GsARGUNIT *);
extern uint32 *GsU_00010014(GsARGUNIT *);
extern uint32 *GsU_00010015(GsARGUNIT *);
extern uint32 *GsU_00050048(GsARGUNIT *);
extern uint32 *GsU_00050049(GsARGUNIT *);
extern uint32 *GsU_0005004c(GsARGUNIT *);
extern uint32 *GsU_0005004d(GsARGUNIT *);
extern uint32 *GsU_00050050(GsARGUNIT *);
extern uint32 *GsU_00050051(GsARGUNIT *);
extern uint32 *GsU_00050054(GsARGUNIT *);
extern uint32 *GsU_00050055(GsARGUNIT *);
extern uint32 *GsU_00040058(GsARGUNIT *);
extern uint32 *GsU_00040059(GsARGUNIT *);
extern uint32 *GsU_0004005c(GsARGUNIT *);
extern uint32 *GsU_0004005d(GsARGUNIT *);
extern uint32 *GsU_01000000(GsARGUNIT *);
extern uint32 *GsU_0100000c(GsARGUNIT *);
extern uint32 *GsU_0100000d(GsARGUNIT *);
extern uint32 *GsU_01000014(GsARGUNIT *);
extern uint32 *GsU_01000015(GsARGUNIT *);
extern uint32 *GsU_0102000c(GsARGUNIT *);
extern uint32 *GsU_0102000d(GsARGUNIT *);
extern uint32 *GsU_01020014(GsARGUNIT *);
extern uint32 *GsU_01020015(GsARGUNIT *);
extern uint32 *GsU_01040048(GsARGUNIT *);
extern uint32 *GsU_01040049(GsARGUNIT *);
extern uint32 *GsU_0104004c(GsARGUNIT *);
extern uint32 *GsU_0104004d(GsARGUNIT *);
extern uint32 *GsU_01040050(GsARGUNIT *);
extern uint32 *GsU_01040051(GsARGUNIT *);
extern uint32 *GsU_01040054(GsARGUNIT *);
extern uint32 *GsU_01040055(GsARGUNIT *);
extern uint32 *GsU_00000018(GsARGUNIT *);
extern uint32 *GsU_00000019(GsARGUNIT *);
extern uint32 *GsU_0000001c(GsARGUNIT *);
extern uint32 *GsU_0000001d(GsARGUNIT *);
extern uint32 *GsU_00000209(GsARGUNIT *);
extern uint32 *GsU_0000020d(GsARGUNIT *);
extern uint32 *GsU_00000211(GsARGUNIT *);
extern uint32 *GsU_00000215(GsARGUNIT *);
extern uint32 *GsU_02000000(GsARGUNIT *);
extern uint32 *GsU_02000001(GsARGUNIT *);
extern uint32 *GsU_00040148(GsARGUNIT *);
extern uint32 *GsU_00040149(GsARGUNIT *);
extern uint32 *GsU_0004014c(GsARGUNIT *);
extern uint32 *GsU_0004014d(GsARGUNIT *);
extern uint32 *GsU_00040150(GsARGUNIT *);
extern uint32 *GsU_00040151(GsARGUNIT *);
extern uint32 *GsU_00040154(GsARGUNIT *);
extern uint32 *GsU_00040155(GsARGUNIT *);
extern uint32 *GsU_00200008(GsARGUNIT *);
extern uint32 *GsU_00200009(GsARGUNIT *);
extern uint32 *GsU_0020000c(GsARGUNIT *);
extern uint32 *GsU_0020000d(GsARGUNIT *);
extern uint32 *GsU_00200010(GsARGUNIT *);
extern uint32 *GsU_00200011(GsARGUNIT *);
extern uint32 *GsU_00200014(GsARGUNIT *);
extern uint32 *GsU_00200015(GsARGUNIT *);
extern uint32 *GsU_0120000c(GsARGUNIT *);
extern uint32 *GsU_0120000d(GsARGUNIT *);
extern uint32 *GsU_01200014(GsARGUNIT *);
extern uint32 *GsU_01200015(GsARGUNIT *);
extern uint32 *GsU_00240048(GsARGUNIT *);
extern uint32 *GsU_00240049(GsARGUNIT *);
extern uint32 *GsU_0024004c(GsARGUNIT *);
extern uint32 *GsU_0024004d(GsARGUNIT *);
extern uint32 *GsU_00240050(GsARGUNIT *);
extern uint32 *GsU_00240051(GsARGUNIT *);
extern uint32 *GsU_00240054(GsARGUNIT *);
extern uint32 *GsU_00240055(GsARGUNIT *);
extern uint32 *GsU_01240048(GsARGUNIT *);
extern uint32 *GsU_01240049(GsARGUNIT *);
extern uint32 *GsU_0124004c(GsARGUNIT *);
extern uint32 *GsU_0124004d(GsARGUNIT *);
extern uint32 *GsU_01240050(GsARGUNIT *);
extern uint32 *GsU_01240051(GsARGUNIT *);
extern uint32 *GsU_01240054(GsARGUNIT *);
extern uint32 *GsU_01240055(GsARGUNIT *);
extern uint32 *GsU_00080008(GsARGUNIT *);
extern uint32 *GsU_00080009(GsARGUNIT *);
extern uint32 *GsU_0008000c(GsARGUNIT *);
extern uint32 *GsU_0008000d(GsARGUNIT *);
extern uint32 *GsU_00080010(GsARGUNIT *);
extern uint32 *GsU_00080011(GsARGUNIT *);
extern uint32 *GsU_00080014(GsARGUNIT *);
extern uint32 *GsU_00080015(GsARGUNIT *);
extern uint32 *GsU_000a0008(GsARGUNIT *);
extern uint32 *GsU_000a0009(GsARGUNIT *);
extern uint32 *GsU_000a000c(GsARGUNIT *);
extern uint32 *GsU_000a000d(GsARGUNIT *);
extern uint32 *GsU_000a0010(GsARGUNIT *);
extern uint32 *GsU_000a0011(GsARGUNIT *);
extern uint32 *GsU_000a0014(GsARGUNIT *);
extern uint32 *GsU_000a0015(GsARGUNIT *);
extern uint32 *GsU_000c0048(GsARGUNIT *);
extern uint32 *GsU_000c0049(GsARGUNIT *);
extern uint32 *GsU_000c004c(GsARGUNIT *);
extern uint32 *GsU_000c004d(GsARGUNIT *);
extern uint32 *GsU_000c0050(GsARGUNIT *);
extern uint32 *GsU_000c0051(GsARGUNIT *);
extern uint32 *GsU_000c0054(GsARGUNIT *);
extern uint32 *GsU_000c0055(GsARGUNIT *);
extern uint32 *GsU_0100020d(GsARGUNIT *);
extern uint32 *GsU_01000215(GsARGUNIT *);

extern uint32 *GsU_00100008(GsARGUNIT *);
extern uint32 *GsU_00100009(GsARGUNIT *);
extern uint32 *GsU_0010000a(GsARGUNIT *);
extern uint32 *GsU_0010000b(GsARGUNIT *);
extern uint32 *GsU_0010000c(GsARGUNIT *);
extern uint32 *GsU_0010000d(GsARGUNIT *);
extern uint32 *GsU_0010000e(GsARGUNIT *);
extern uint32 *GsU_0010000f(GsARGUNIT *);
extern uint32 *GsU_00100010(GsARGUNIT *);
extern uint32 *GsU_00100011(GsARGUNIT *);
extern uint32 *GsU_00100012(GsARGUNIT *);
extern uint32 *GsU_00100013(GsARGUNIT *);
extern uint32 *GsU_00100014(GsARGUNIT *);
extern uint32 *GsU_00100015(GsARGUNIT *);
extern uint32 *GsU_00100016(GsARGUNIT *);
extern uint32 *GsU_00100017(GsARGUNIT *);
extern uint32 *GsU_00300008(GsARGUNIT *);
extern uint32 *GsU_00300009(GsARGUNIT *);
extern uint32 *GsU_0030000c(GsARGUNIT *);
extern uint32 *GsU_0030000d(GsARGUNIT *);
extern uint32 *GsU_00300010(GsARGUNIT *);
extern uint32 *GsU_00300011(GsARGUNIT *);
extern uint32 *GsU_00300014(GsARGUNIT *);
extern uint32 *GsU_00300015(GsARGUNIT *);
extern uint32 *GsU_00140048(GsARGUNIT *);
extern uint32 *GsU_00140049(GsARGUNIT *);
extern uint32 *GsU_0014004c(GsARGUNIT *);
extern uint32 *GsU_0014004d(GsARGUNIT *);
extern uint32 *GsU_00140050(GsARGUNIT *);
extern uint32 *GsU_00140051(GsARGUNIT *);
extern uint32 *GsU_00140054(GsARGUNIT *);
extern uint32 *GsU_00140055(GsARGUNIT *);
extern uint32 *GsU_00340048(GsARGUNIT *);
extern uint32 *GsU_00340049(GsARGUNIT *);
extern uint32 *GsU_0034004c(GsARGUNIT *);
extern uint32 *GsU_0034004d(GsARGUNIT *);
extern uint32 *GsU_00340050(GsARGUNIT *);
extern uint32 *GsU_00340051(GsARGUNIT *);
extern uint32 *GsU_00340054(GsARGUNIT *);
extern uint32 *GsU_00340055(GsARGUNIT *);
extern uint32 *GsU_01140048(GsARGUNIT *);
extern uint32 *GsU_01140049(GsARGUNIT *);
extern uint32 *GsU_0114004c(GsARGUNIT *);
extern uint32 *GsU_0114004d(GsARGUNIT *);
extern uint32 *GsU_01140050(GsARGUNIT *);
extern uint32 *GsU_01140051(GsARGUNIT *);
extern uint32 *GsU_01140054(GsARGUNIT *);
extern uint32 *GsU_01140055(GsARGUNIT *);
extern uint32 *GsU_01340048(GsARGUNIT *);
extern uint32 *GsU_01340049(GsARGUNIT *);
extern uint32 *GsU_0134004c(GsARGUNIT *);
extern uint32 *GsU_0134004d(GsARGUNIT *);
extern uint32 *GsU_01340050(GsARGUNIT *);
extern uint32 *GsU_01340051(GsARGUNIT *);
extern uint32 *GsU_01340054(GsARGUNIT *);
extern uint32 *GsU_01340055(GsARGUNIT *);

extern uint32 *GsU_0020000a(GsARGUNIT *);
extern uint32 *GsU_0020000b(GsARGUNIT *);
extern uint32 *GsU_0020000e(GsARGUNIT *);
extern uint32 *GsU_0020000f(GsARGUNIT *);
extern uint32 *GsU_00200012(GsARGUNIT *);
extern uint32 *GsU_00200013(GsARGUNIT *);
extern uint32 *GsU_00200016(GsARGUNIT *);
extern uint32 *GsU_00200017(GsARGUNIT *);
extern uint32 *GsU_0030000a(GsARGUNIT *);
extern uint32 *GsU_0030000b(GsARGUNIT *);
extern uint32 *GsU_0030000e(GsARGUNIT *);
extern uint32 *GsU_0030000f(GsARGUNIT *);
extern uint32 *GsU_00300012(GsARGUNIT *);
extern uint32 *GsU_00300013(GsARGUNIT *);
extern uint32 *GsU_00300016(GsARGUNIT *);
extern uint32 *GsU_00300017(GsARGUNIT *);

extern uint32 *GsU_0100000e(GsARGUNIT *);
extern uint32 *GsU_0100000f(GsARGUNIT *);
extern uint32 *GsU_01000016(GsARGUNIT *);
extern uint32 *GsU_01000017(GsARGUNIT *);
extern uint32 *GsU_0120000e(GsARGUNIT *);
extern uint32 *GsU_0120000f(GsARGUNIT *);
extern uint32 *GsU_01200016(GsARGUNIT *);
extern uint32 *GsU_01200017(GsARGUNIT *);

extern uint32 *GsU_0002000a(GsARGUNIT *);
extern uint32 *GsU_0002000b(GsARGUNIT *);
extern uint32 *GsU_0002000e(GsARGUNIT *);
extern uint32 *GsU_0002000f(GsARGUNIT *);
extern uint32 *GsU_00020012(GsARGUNIT *);
extern uint32 *GsU_00020013(GsARGUNIT *);
extern uint32 *GsU_00020016(GsARGUNIT *);
extern uint32 *GsU_00020017(GsARGUNIT *);
extern uint32 *GsU_0102000e(GsARGUNIT *);
extern uint32 *GsU_0102000f(GsARGUNIT *);
extern uint32 *GsU_01020016(GsARGUNIT *);
extern uint32 *GsU_01020017(GsARGUNIT *);

extern uint32 *GsU_00000000(GsARGUNIT *);

extern uint32 *GsU_05000000(GsARGUNIT *);
extern uint32 *GsU_05000001(GsARGUNIT *);

/* camera */
extern uint32 *GsU_07000100(GsARGUNIT *);
extern uint32 *GsU_07010100(GsARGUNIT *);
extern uint32 *GsU_07020100(GsARGUNIT *);
extern uint32 *GsU_07030100(GsARGUNIT *);

/* light */
extern uint32 *GsU_07000200(GsARGUNIT *);
extern uint32 *GsU_07010200(GsARGUNIT *);
extern uint32 *GsU_07020200(GsARGUNIT *);
extern uint32 *GsU_07030200(GsARGUNIT *);

/* update driver */
extern uint32 *GsU_03000000(GsARGUNIT_ANIM *);

/* interpolation driver */
extern int GsU_03000001(GsARGUNIT_ANIM *);
extern int GsU_03000002(GsARGUNIT_ANIM *);
extern int GsU_03000003(GsARGUNIT_ANIM *);
extern int GsU_03000009(GsARGUNIT_ANIM *);
extern int GsU_0300000a(GsARGUNIT_ANIM *);
extern int GsU_0300000b(GsARGUNIT_ANIM *);
extern int GsU_03000010(GsARGUNIT_ANIM *);
extern int GsU_03000011(GsARGUNIT_ANIM *);
extern int GsU_03000012(GsARGUNIT_ANIM *);
extern int GsU_03000013(GsARGUNIT_ANIM *);
extern int GsU_03000019(GsARGUNIT_ANIM *);
extern int GsU_0300001a(GsARGUNIT_ANIM *);
extern int GsU_0300001b(GsARGUNIT_ANIM *);
extern int GsU_03000020(GsARGUNIT_ANIM *);
extern int GsU_03000021(GsARGUNIT_ANIM *);
extern int GsU_03000022(GsARGUNIT_ANIM *);
extern int GsU_03000023(GsARGUNIT_ANIM *);
extern int GsU_03000029(GsARGUNIT_ANIM *);
extern int GsU_0300002a(GsARGUNIT_ANIM *);
extern int GsU_0300002b(GsARGUNIT_ANIM *);
extern int GsU_03000030(GsARGUNIT_ANIM *);
extern int GsU_03000031(GsARGUNIT_ANIM *);
extern int GsU_03000032(GsARGUNIT_ANIM *);
extern int GsU_03000033(GsARGUNIT_ANIM *);
extern int GsU_03000039(GsARGUNIT_ANIM *);
extern int GsU_0300003a(GsARGUNIT_ANIM *);
extern int GsU_0300003b(GsARGUNIT_ANIM *);
extern int GsU_03000100(GsARGUNIT_ANIM *);
extern int GsU_03000111(GsARGUNIT_ANIM *);
extern int GsU_03000112(GsARGUNIT_ANIM *);
extern int GsU_03000119(GsARGUNIT_ANIM *);
extern int GsU_0300011a(GsARGUNIT_ANIM *);
extern int GsU_03000901(GsARGUNIT_ANIM *);
extern int GsU_03000902(GsARGUNIT_ANIM *);
extern int GsU_03000909(GsARGUNIT_ANIM *);
extern int GsU_0300090a(GsARGUNIT_ANIM *);
extern int GsU_03000910(GsARGUNIT_ANIM *);
extern int GsU_03000911(GsARGUNIT_ANIM *);
extern int GsU_03000912(GsARGUNIT_ANIM *);
extern int GsU_03000919(GsARGUNIT_ANIM *);
extern int GsU_0300091a(GsARGUNIT_ANIM *);
extern int GsU_03001010(GsARGUNIT_ANIM *);
extern int GsU_03001011(GsARGUNIT_ANIM *);
extern int GsU_03001012(GsARGUNIT_ANIM *);
extern int GsU_03001013(GsARGUNIT_ANIM *);
extern int GsU_03001019(GsARGUNIT_ANIM *);
extern int GsU_0300101a(GsARGUNIT_ANIM *);
extern int GsU_0300101b(GsARGUNIT_ANIM *);
extern int GsU_03001020(GsARGUNIT_ANIM *);
extern int GsU_03001021(GsARGUNIT_ANIM *);
extern int GsU_03001022(GsARGUNIT_ANIM *);
extern int GsU_03001023(GsARGUNIT_ANIM *);
extern int GsU_03001029(GsARGUNIT_ANIM *);
extern int GsU_0300102a(GsARGUNIT_ANIM *);
extern int GsU_0300102b(GsARGUNIT_ANIM *);
extern int GsU_03001030(GsARGUNIT_ANIM *);
extern int GsU_03001031(GsARGUNIT_ANIM *);
extern int GsU_03001032(GsARGUNIT_ANIM *);
extern int GsU_03001033(GsARGUNIT_ANIM *);
extern int GsU_03001039(GsARGUNIT_ANIM *);
extern int GsU_0300103a(GsARGUNIT_ANIM *);
extern int GsU_0300103b(GsARGUNIT_ANIM *);
extern int GsU_03001111(GsARGUNIT_ANIM *);
extern int GsU_03001112(GsARGUNIT_ANIM *);
extern int GsU_03001119(GsARGUNIT_ANIM *);
extern int GsU_0300111a(GsARGUNIT_ANIM *);
extern int GsU_03001910(GsARGUNIT_ANIM *);
extern int GsU_03001911(GsARGUNIT_ANIM *);
extern int GsU_03001912(GsARGUNIT_ANIM *);
extern int GsU_03001919(GsARGUNIT_ANIM *);
extern int GsU_0300191a(GsARGUNIT_ANIM *);
extern int GsU_03002010(GsARGUNIT_ANIM *);
extern int GsU_03002011(GsARGUNIT_ANIM *);
extern int GsU_03002012(GsARGUNIT_ANIM *);
extern int GsU_03002013(GsARGUNIT_ANIM *);
extern int GsU_03002019(GsARGUNIT_ANIM *);
extern int GsU_0300201a(GsARGUNIT_ANIM *);
extern int GsU_0300201b(GsARGUNIT_ANIM *);
extern int GsU_03002020(GsARGUNIT_ANIM *);
extern int GsU_03002021(GsARGUNIT_ANIM *);
extern int GsU_03002022(GsARGUNIT_ANIM *);
extern int GsU_03002023(GsARGUNIT_ANIM *);
extern int GsU_03002029(GsARGUNIT_ANIM *);
extern int GsU_0300202a(GsARGUNIT_ANIM *);
extern int GsU_0300202b(GsARGUNIT_ANIM *);
extern int GsU_03002030(GsARGUNIT_ANIM *);
extern int GsU_03002031(GsARGUNIT_ANIM *);
extern int GsU_03002032(GsARGUNIT_ANIM *);
extern int GsU_03002033(GsARGUNIT_ANIM *);
extern int GsU_03002039(GsARGUNIT_ANIM *);
extern int GsU_0300203a(GsARGUNIT_ANIM *);
extern int GsU_0300203b(GsARGUNIT_ANIM *);
extern int GsU_03002111(GsARGUNIT_ANIM *);
extern int GsU_03002112(GsARGUNIT_ANIM *);
extern int GsU_03002119(GsARGUNIT_ANIM *);
extern int GsU_0300211a(GsARGUNIT_ANIM *);
extern int GsU_03002910(GsARGUNIT_ANIM *);
extern int GsU_03002911(GsARGUNIT_ANIM *);
extern int GsU_03002912(GsARGUNIT_ANIM *);
extern int GsU_03002919(GsARGUNIT_ANIM *);
extern int GsU_0300291a(GsARGUNIT_ANIM *);
extern int GsU_03003010(GsARGUNIT_ANIM *);
extern int GsU_03003011(GsARGUNIT_ANIM *);
extern int GsU_03003012(GsARGUNIT_ANIM *);
extern int GsU_03003013(GsARGUNIT_ANIM *);
extern int GsU_03003019(GsARGUNIT_ANIM *);
extern int GsU_0300301a(GsARGUNIT_ANIM *);
extern int GsU_0300301b(GsARGUNIT_ANIM *);
extern int GsU_03003020(GsARGUNIT_ANIM *);
extern int GsU_03003021(GsARGUNIT_ANIM *);
extern int GsU_03003022(GsARGUNIT_ANIM *);
extern int GsU_03003023(GsARGUNIT_ANIM *);
extern int GsU_03003029(GsARGUNIT_ANIM *);
extern int GsU_0300302a(GsARGUNIT_ANIM *);
extern int GsU_0300302b(GsARGUNIT_ANIM *);
extern int GsU_03003030(GsARGUNIT_ANIM *);
extern int GsU_03003031(GsARGUNIT_ANIM *);
extern int GsU_03003032(GsARGUNIT_ANIM *);
extern int GsU_03003033(GsARGUNIT_ANIM *);
extern int GsU_03003039(GsARGUNIT_ANIM *);
extern int GsU_0300303a(GsARGUNIT_ANIM *);
extern int GsU_0300303b(GsARGUNIT_ANIM *);
extern int GsU_03003111(GsARGUNIT_ANIM *);
extern int GsU_03003112(GsARGUNIT_ANIM *);
extern int GsU_03003119(GsARGUNIT_ANIM *);
extern int GsU_0300311a(GsARGUNIT_ANIM *);
extern int GsU_03003910(GsARGUNIT_ANIM *);
extern int GsU_03003911(GsARGUNIT_ANIM *);
extern int GsU_03003912(GsARGUNIT_ANIM *);
extern int GsU_03003919(GsARGUNIT_ANIM *);
extern int GsU_0300391a(GsARGUNIT_ANIM *);
extern int GsU_03004010(GsARGUNIT_ANIM *);
extern int GsU_03004011(GsARGUNIT_ANIM *);
extern int GsU_03004012(GsARGUNIT_ANIM *);
extern int GsU_03004013(GsARGUNIT_ANIM *);
extern int GsU_03004019(GsARGUNIT_ANIM *);
extern int GsU_0300401a(GsARGUNIT_ANIM *);
extern int GsU_0300401b(GsARGUNIT_ANIM *);
extern int GsU_03004020(GsARGUNIT_ANIM *);
extern int GsU_03004021(GsARGUNIT_ANIM *);
extern int GsU_03004022(GsARGUNIT_ANIM *);
extern int GsU_03004023(GsARGUNIT_ANIM *);
extern int GsU_03004029(GsARGUNIT_ANIM *);
extern int GsU_0300402a(GsARGUNIT_ANIM *);
extern int GsU_0300402b(GsARGUNIT_ANIM *);
extern int GsU_03004030(GsARGUNIT_ANIM *);
extern int GsU_03004031(GsARGUNIT_ANIM *);
extern int GsU_03004032(GsARGUNIT_ANIM *);
extern int GsU_03004033(GsARGUNIT_ANIM *);
extern int GsU_03004039(GsARGUNIT_ANIM *);
extern int GsU_0300403a(GsARGUNIT_ANIM *);
extern int GsU_0300403b(GsARGUNIT_ANIM *);
extern int GsU_03004111(GsARGUNIT_ANIM *);
extern int GsU_03004112(GsARGUNIT_ANIM *);
extern int GsU_03004119(GsARGUNIT_ANIM *);
extern int GsU_0300411a(GsARGUNIT_ANIM *);
extern int GsU_03004910(GsARGUNIT_ANIM *);
extern int GsU_03004911(GsARGUNIT_ANIM *);
extern int GsU_03004912(GsARGUNIT_ANIM *);
extern int GsU_03004919(GsARGUNIT_ANIM *);
extern int GsU_0300491a(GsARGUNIT_ANIM *);
extern int GsU_03005010(GsARGUNIT_ANIM *);
extern int GsU_03005011(GsARGUNIT_ANIM *);
extern int GsU_03005012(GsARGUNIT_ANIM *);
extern int GsU_03005013(GsARGUNIT_ANIM *);
extern int GsU_03005019(GsARGUNIT_ANIM *);
extern int GsU_0300501a(GsARGUNIT_ANIM *);
extern int GsU_0300501b(GsARGUNIT_ANIM *);
extern int GsU_03005020(GsARGUNIT_ANIM *);
extern int GsU_03005021(GsARGUNIT_ANIM *);
extern int GsU_03005022(GsARGUNIT_ANIM *);
extern int GsU_03005023(GsARGUNIT_ANIM *);
extern int GsU_03005029(GsARGUNIT_ANIM *);
extern int GsU_0300502a(GsARGUNIT_ANIM *);
extern int GsU_0300502b(GsARGUNIT_ANIM *);
extern int GsU_03005030(GsARGUNIT_ANIM *);
extern int GsU_03005031(GsARGUNIT_ANIM *);
extern int GsU_03005032(GsARGUNIT_ANIM *);
extern int GsU_03005033(GsARGUNIT_ANIM *);
extern int GsU_03005039(GsARGUNIT_ANIM *);
extern int GsU_0300503a(GsARGUNIT_ANIM *);
extern int GsU_0300503b(GsARGUNIT_ANIM *);
extern int GsU_03005111(GsARGUNIT_ANIM *);
extern int GsU_03005112(GsARGUNIT_ANIM *);
extern int GsU_03005119(GsARGUNIT_ANIM *);
extern int GsU_0300511a(GsARGUNIT_ANIM *);
extern int GsU_03005910(GsARGUNIT_ANIM *);
extern int GsU_03005911(GsARGUNIT_ANIM *);
extern int GsU_03005912(GsARGUNIT_ANIM *);
extern int GsU_03005919(GsARGUNIT_ANIM *);
extern int GsU_0300591a(GsARGUNIT_ANIM *);
extern int GsU_03010110(GsARGUNIT_ANIM *);
extern int GsU_03010111(GsARGUNIT_ANIM *);
extern int GsU_03010112(GsARGUNIT_ANIM *);
extern int GsU_03010121(GsARGUNIT_ANIM *);
extern int GsU_03010122(GsARGUNIT_ANIM *);
extern int GsU_03010141(GsARGUNIT_ANIM *);
extern int GsU_03010142(GsARGUNIT_ANIM *);
extern int GsU_03010171(GsARGUNIT_ANIM *);
extern int GsU_03010172(GsARGUNIT_ANIM *);
extern int GsU_03010182(GsARGUNIT_ANIM *);
extern int GsU_03010210(GsARGUNIT_ANIM *);
extern int GsU_03010211(GsARGUNIT_ANIM *);
extern int GsU_03010212(GsARGUNIT_ANIM *);
extern int GsU_03010221(GsARGUNIT_ANIM *);
extern int GsU_03010222(GsARGUNIT_ANIM *);
extern int GsU_03010241(GsARGUNIT_ANIM *);
extern int GsU_03010242(GsARGUNIT_ANIM *);
extern int GsU_03010271(GsARGUNIT_ANIM *);
extern int GsU_03010272(GsARGUNIT_ANIM *);
extern int GsU_03010310(GsARGUNIT_ANIM *);
extern int GsU_03010311(GsARGUNIT_ANIM *);
extern int GsU_03010312(GsARGUNIT_ANIM *);
extern int GsU_03010321(GsARGUNIT_ANIM *);
extern int GsU_03010322(GsARGUNIT_ANIM *);
extern int GsU_03010341(GsARGUNIT_ANIM *);
extern int GsU_03010342(GsARGUNIT_ANIM *);
extern int GsU_03010371(GsARGUNIT_ANIM *);
extern int GsU_03010372(GsARGUNIT_ANIM *);

/* envmap driver */
extern uint32 *GsU_06000100(GsARGUNIT *sp);
extern uint32 *GsU_0600100c(GsARGUNIT *sp);
extern uint32 *GsU_06001014(GsARGUNIT *sp);
extern uint32 *GsU_0600110c(GsARGUNIT *sp);
extern uint32 *GsU_06001114(GsARGUNIT *sp);
extern uint32 *GsU_0600200c(GsARGUNIT *sp);
extern uint32 *GsU_06002014(GsARGUNIT *sp);
extern uint32 *GsU_0600300c(GsARGUNIT *sp);
extern uint32 *GsU_06003014(GsARGUNIT *sp);
extern uint32 *GsU_0600400c(GsARGUNIT *sp);
extern uint32 *GsU_06004014(GsARGUNIT *sp);
extern uint32 *GsU_0600500c(GsARGUNIT *sp);
extern uint32 *GsU_06005014(GsARGUNIT *sp);

/* MIMe driver */
extern uint32 *GsU_04010020(GsARGUNIT *);
extern uint32 *GsU_04010021(GsARGUNIT *);
extern uint32 *GsU_04010028(GsARGUNIT *);
extern uint32 *GsU_04010029(GsARGUNIT *);
extern uint32 *GsU_04010010(GsARGUNIT *);
extern uint32 *GsU_04010018(GsARGUNIT *);
extern uint32 *GsU_04010011(GsARGUNIT *);
extern uint32 *GsU_04010019(GsARGUNIT *);

extern GsCOORDUNIT *GsMapCoordUnit(uint32 *, uint32 *);
extern uint32 *GsGetHeadpUnit(void);
extern int GsScanUnit(uint32 *, GsTYPEUNIT *, GsOT *, uint32 *);
extern void GsMapUnit(uint32 *);
extern void GsSortUnit(GsUNIT *, GsOT *, uint32 *);
extern void GsGetLwUnit(GsCOORDUNIT *, MATRIX *);
extern void GsGetLsUnit(GsCOORDUNIT *, MATRIX *);
extern void GsGetLwsUnit(GsCOORDUNIT *, MATRIX *, MATRIX *);
extern int GsSetViewUnit(GsVIEWUNIT *);
extern int GsSetRefViewUnit(GsRVIEWUNIT *);
extern int GsSetRefViewLUnit(GsRVIEWUNIT *);
extern uint32 *GsScanAnim(uint32 *, GsTYPEUNIT *);
extern int32 GsLinkAnim(GsSEQ **, uint32 *);

/* for MIMe */
extern void GsInitRstVtxMIMe(uint32 *primtop, uint32 *hp);
extern void GsInitRstNrmMIMe(uint32 *primtop, uint32 *hp);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif

//-=- Definitions -=-//
#ifndef ONE
#define ONE 4096
#endif
#define PSX_SCREEN_WIDTH 512
#define PSX_SCREEN_HEIGHT 240

//-=- Macros -=-//
#define __nint__(x) (((x) > 0) ? int((x) + 0.5) : int((x)-0.5))

#define getTPage(tp, abr, x, y) ((((tp)&0x3) << 7) | (((abr)&0x3) << 5) | (((y)&0x100) >> 4) | (((x)&0x3ff) >> 6) | (((y)&0x200) << 2))

#define getClut(x, y) ((y << 6) | ((x >> 4) & 0x3f))

} // End of namespace ICB

#endif // __PSX_PcDefines_H__
