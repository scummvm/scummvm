#include        <wind.h>
#include	<conio.h>

#define	C(fgr,bgr)	((bgr<<4)|fgr)



extern	byte	Colors[2][ColorBanks][ColorRegs] = {
 {
//    Frame                 Frame_h            Field                 Field_h           Selected           Selected_h
//---------------------------------------------------------------------------------------------------------------------------------
 { C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY), C(RED,LIGHTGRAY),   C(BLACK,CYAN),      C(RED,CYAN) },        // STD
 { C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY), C(RED,LIGHTGRAY),   C(BLACK,CYAN),      C(RED,CYAN) },        // MNU
 { C(BLACK,CYAN),      C(BLACK,CYAN),      C(BLACK,CYAN),      C(WHITE,CYAN),      C(CYAN,BLACK),      C(BLUE,BLACK) },      // HLP
 { C(WHITE,RED),       C(WHITE,RED),       C(WHITE,RED),       C(YELLOW,RED),      C(WHITE,CYAN),      C(YELLOW,CYAN) },     // WAR
 { C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(BLUE,BLACK),      C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(BLACK,LIGHTGRAY) }  // DSK
 },
 {
//    Frame                 Frame_h            Field                 Field_h           Selected           Selected_h
//---------------------------------------------------------------------------------------------------------------------------------
 { C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(WHITE,BLACK),     C(BLACK,LIGHTGRAY), C(WHITE,BLACK) },     // STD
 { C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(WHITE,BLACK),     C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY) }, // MNU
 { C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY), C(WHITE,BLACK),     C(LIGHTGRAY,BLACK), C(WHITE,BLACK) },     // HLP
 { C(WHITE,BLACK),     C(WHITE,BLACK),     C(LIGHTGRAY,BLACK), C(WHITE,BLACK),     C(BLACK,LIGHTGRAY), C(BLACK,LIGHTGRAY) }, // WAR
 { C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(LIGHTGRAY,BLACK), C(BLACK,LIGHTGRAY) }  // DSK
 } };
