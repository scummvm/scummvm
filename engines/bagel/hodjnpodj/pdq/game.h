/*****************************************************************
 *
 *  game.h
 *
 *  HISTORY
 *
 *      1.00      03/14/94     BCW     Created this file
 *
 *  MODULE DESCRIPTION:
 *
 *      Public header for GAME.CPP
 *   
 *  RELEVANT DOCUMENTATION:
 *
 *
 *
 *  PORTABILITY ISSUES:
 *   
 *
 *   
 ****************************************************************/
#ifndef GAME_H
#define GAME_H

#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/error.h"

#define MAX_PLENGTH   	25
#define MAX_PLENGTH_S 	30

#define MAX_TURNS       3           // Max Turn Count when playing the metagame

// Easter Egg info:
#define	BIRD_X				43
#define BIRD_Y				241
#define	BIRD_DX				94
#define	BIRD_DY				44
#define	NUM_BIRD_CELS		20
#define	BIRD_SLEEP			40

#define	HORSE1_X			158
#define HORSE1_Y			146
#define	HORSE1_DX			85
#define	HORSE1_DY			126
#define	NUM_HORSE1_CELS		16
#define	HORSE1_SLEEP		100

#define	HORSE2_X			240
#define HORSE2_Y			188
#define	HORSE2_DX			127
#define	HORSE2_DY			249
#define	NUM_HORSE2_CELS		14
#define	HORSE2_SLEEP		100

#define	FLOWER_X			578
#define FLOWER_Y			246
#define	FLOWER_DX			47
#define	FLOWER_DY			64
#define	NUM_FLOWER_CELS		21
#define	FLOWER_SLEEP		200

#define	BIRD_ANIM		".\\ART\\BIRD.BMP"
#define	HORSE1_ANIM		".\\ART\\HORSE1.BMP"
#define	HORSE2_ANIM		".\\ART\\HORSE2.BMP"
#define	FLOWER_ANIM		".\\ART\\FLOWER.BMP"

#define WAV_REVEAL      ".\\SOUND\\REVEAL.WAV"
#define WAV_BADGUESS    ".\\SOUND\\BADGUESS.WAV"
#define WAV_GAMEOVER    ".\\SOUND\\GAMEOVER.WAV"
#define WAV_YOUWIN      ".\\SOUND\\YOUWIN.WAV"
#define WAV_HORSE1      ".\\SOUND\\HORSE1.WAV"
#define	WAV_HORSE2		".\\SOUND\\HORSE2.WAV"
#define	WAV_BIRD		".\\SOUND\\BIRD.WAV"
#define	WAV_FLOWER		".\\SOUND\\WEIRD.WAV"

#define MID_SOUNDTRACK  ".\\SOUND\\THGESNG.MID"

/*
* prototypes
*/
	ERROR_CODE  InitGame(HWND, CDC *);
	ERROR_CODE  StartGame(CDC *);
	VOID        GameStopTimer(VOID);
	ERROR_CODE  GameStartTimer(VOID);
	VOID        GamePauseTimer(VOID);
	VOID        GameResumeTimer(VOID);
	ERROR_CODE  EndGame(CDC *);
	VOID        WinGame(VOID);
	ERROR_CODE  RepaintSpriteList(CDC *);
	BOOLEAN     CheckUserGuess(const CHAR *);
	VOID CALLBACK GetGameParams(CWnd *);
	VOID        GameGetScore(UINT *, UINT *, UINT *, UINT *);

#endif /* GAME_H */
