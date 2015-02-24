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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_LABFUN_H
#define LAB_LABFUN_H

#include "lab/stddefines.h"
#include "lab/parsetypes.h"
#include "common/file.h"
#include "common/savefile.h"

#include "engines/savestate.h"

namespace Lab {

/* Direction defines */
#define NORTH   0
#define SOUTH   1
#define EAST    2
#define WEST    3

struct Image;
struct TextFont;
struct Gadget;

struct SaveGameHeader {
	byte version;
	SaveStateDescriptor desc;
	uint16 roomNumber;
	uint16 direction;
};

/*----------------------------*/
/*------ From Audioi.c -------*/
/*----------------------------*/

bool initAudio();
void freeAudio();
bool musicBufferEmpty();
void playMusicBlock(void *Ptr, uint32 Size, uint16 BufferNum, uint16 SampleSpeed);
uint16 getPlayingBufferCount();
void updateSoundBuffers();
void flushAudio();
void playSoundEffect(uint16 SampleSpeed, uint16 Volume, uint32 Length, bool flush, void *Data);




/*----------------------------*/
/*----- From graphics.c ------*/
/*----------------------------*/

/* Reads in pictures */

bool readPict(const char *filename, bool PlayOnce);

bool readMusic(const char *filename);

byte *readPictToMem(const char *filename, uint16 x, uint16 y);

/* Window text stuff */

uint32 flowText(void *font,  /* the TextAttr pointer */
                int16 spacing,          /* How much vertical spacing between the lines */
                uint16 pencolor,         /* pen number to use for text */
                uint16 backpen,          /* the background color */
                bool fillback,                /* Whether to fill the background */
                bool centerh,                 /* Whether to center the text horizontally */
                bool centerv,                 /* Whether to center the text vertically */
                bool output,                  /* Whether to output any text */
                uint16 x1,               /* Cords */
                uint16 y1, uint16 x2, uint16 y2, const char *text); /* The text itself */

uint32 flowTextToMem(Image *DestIm, void *font,     /* the TextAttr pointer */
                     int16 spacing,          /* How much vertical spacing between the lines */
                     uint16 pencolor,         /* pen number to use for text */
                     uint16 backpen,          /* the background color */
                     bool fillback,                /* Whether to fill the background */
                     bool centerh,                 /* Whether to center the text horizontally */
                     bool centerv,                 /* Whether to center the text vertically */
                     bool output,                  /* Whether to output any text */
                     uint16 x1,               /* Cords */
                     uint16 y1, uint16 x2, uint16 y2, const char *text); /* The text itself */

void drawMessage(const char *text);

void longDrawMessage(const char *text);

bool readFont(char *filename, void *font, void *data);

/* The Wipes */

void doWipe(uint16 WipeType, CloseDataPtr *CPtr, char *filename);


/* Double Buffer stuff */

void newFlipViews(void *scrPtr, uint16 *newpal, uint16 numcolors);

void flipViews(void *scrPtr);




/*----------------------------*/
/*----- From Interface.c -----*/
/*----------------------------*/

Gadget *addGadButton(uint16 x, uint16 y, void *UpImage, void *DownImage, uint16 id);

void gadgetsOnOff(void *gptr, void *win, int32 num, bool on);

/*----------------------*/
/*---- From Intro.c ----*/
/*----------------------*/

void introSequence();



/*----------------------*/
/*----- From Lab.c -----*/
/*----------------------*/

void eatMessages();

bool setUpScreens();

void drawPanel();

bool quitPlaying();



/*---------------------------*/
/*------ From LabFile.c -----*/
/*---------------------------*/

uint32 sizeOfFile(const char *name);

/* Buffer a whole file */

byte **isBuffered(const char *fileName);

byte **openFile(const char *name);

void readBlock(void *Buffer, uint32 Size, byte **File);

char readChar(char **File);

void skip(byte **File, uint32 skip);

void resetBuffer();

bool initBuffer(uint32 BufSize, bool IsGraphicsMem);

void freeBuffer();


/* Functions that borrow memory from the buffer */

bool allocFile(void **Ptr, uint32 Size, const char *fileName);

void *stealBufMem(int32 Size);

void freeAllStolenMem();


/* Read chunks of a file */


Common::File *openPartial(const char *name);

void closePartial(int32 File);


/*---------------------------*/
/*------ From LabText.c -----*/
/*---------------------------*/

bool initLabText();

void freeLabText();

void decrypt(byte *text);



/*---------------------------*/
/*----- From LabMusic.c -----*/
/*---------------------------*/

#define MAXBUFFERS         5L

class Music {
public:
	Music();

	byte **newOpen(const char *name);
	bool initMusic();
	void freeMusic();
	void fillUpMusic(bool doit);
	void updateMusic();
	void checkMusic();
	void newCheckMusic();
	void closeMusic();
	void setMusic(bool on);
	void restartBackMusic();
	void pauseBackMusic();
	void changeMusic(const char *newmusic);
	void resetMusic();

	bool _winmusic, _doNotFileFlushAudio;
	bool _turnMusicOn;
	bool _musicOn;

private:
	void fillbuffer(byte *musicBuffer);
	void startMusic(bool startatbegin);

	Common::File *_file;
	Common::File *_tFile;
	bool _musicPaused;

	bool _tMusicOn;
	uint32 _tLeftInFile;
	uint32 _leftinfile;
};


extern Music *g_music;

/*---------------------------*/
/*----- From LabSets.c ------*/
/*---------------------------*/

class LargeSet {
public:
    LargeSet(uint16 last);

    ~LargeSet();

    bool in(uint16 element);

    void inclElement(uint16 element);

    void exclElement(uint16 element);

    bool readInitialConditions(const char *fileName);

public:
    uint16 _lastElement;
    uint16 *_array;
};

/*---------------------------*/
/*----- From Machine.c ------*/
/*---------------------------*/

uint16 scaleX(uint16 x);

uint16 scaleY(uint16 y);

uint16 VGAScaleX(uint16 x);

uint16 VGAScaleY(uint16 y);

int16 VGAScaleXs(int16 x);

int16 VGAScaleYs(int16 y);

uint16 SVGACord(uint16 cord);

uint16 VGAUnScaleX(uint16 x);

uint16 VGAUnScaleY(uint16 y);

char *translateFileName(const char *filename);



/*---------------------------*/
/*-------- From Map.c -------*/
/*---------------------------*/

void fade(bool fadein, uint16 res);

void setAmigaPal(uint16 *pal, uint16 numcolors);

char *getText(const char *filename);

bool getFont(const char *filename, TextFont *textfont);

void readImage(byte **buffer, Image **im);

void doMap(uint16 CurRoom);

void doJournal();

void doNotes();

void doWestPaper();

void doMonitor(char *background, char *textfile, bool isinteractive, uint16 x1, uint16 y1, uint16 x2, uint16 y2);

bool saveRestoreGame();



/*--------------------------*/
/*----- From saveGame.c ----*/
/*--------------------------*/

bool saveGame(uint16 RoomNum, uint16 Direction, uint16 Quarters, int slot, Common::String desc);
bool loadGame(uint16 *RoomNum, uint16 *Direction, uint16 *Quarters, int slot);

bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header);

/*--------------------------*/
/*----- From Special.c -----*/
/*--------------------------*/

void showCombination(const char *filename);

void mouseCombination(uint16 x, uint16 y);

void showTile(const char *filename, bool showsolution);

void mouseTile(uint16 x, uint16 y);

void inner_main();

} // End of namespace Lab

#endif /* LAB_LABFUN_H */
