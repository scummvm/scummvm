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

#ifndef ICB_PSX_PCDEFINES_H
#define ICB_PSX_PCDEFINES_H

#include "common/types.h"
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

// make our own equivalents
typedef struct MATRIX {
	short m[3][3]; /* 3x3 rotation matrix */
	short pad;
	int32 t[3]; /* transfer vector */
	MATRIX() { pad = 0; }
} MATRIX;

typedef struct MATRIXPC {
	int m[3][3]; /* 3x3 rotation matrix */
	int pad;
	int32 t[3]; /* transfer vector */
	MATRIXPC() { pad = 0; }
} MATRIXPC;

/* int32 word type 3D vector */
typedef struct VECTOR {
	int32 vx, vy;
	int32 vz, pad;
	VECTOR() { pad = 0; }
} VECTOR;

/* short word type 3D vector */
typedef struct SVECTOR {
	short vx, vy;
	short vz, pad;
} SVECTOR;

/* short word type 3D vector - PC version */
typedef struct SVECTORPC {
	int vx, vy;
	int vz, pad;
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
