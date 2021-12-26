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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/blitters.h"
#include "saga2/fta.h"
#include "saga2/mouseimg.h"
#include "saga2/panel.h"

namespace Saga2 {

enum {
	gaugeWidth = 46,
	gaugeImageWidth = gaugeWidth + 2,
    gaugeImageHeight = 5,
    gaugeOneThird = gaugeWidth / 3,
    gaugeTwoThirds = gaugeWidth * 2 / 3
};

extern gFont *mainFont;

extern uint8 *closeBx1ImageData;
extern uint8 *closeBx2ImageData;
extern uint8 *usePtrImageData;
extern uint8 *xPointerImageData;
extern uint8 *arrowImageData;
extern uint8 *grabPtrImageData;
extern uint8 *attakPtrImageData;
extern uint8 *centerActorIndicatorImageData;
extern uint8 *pgUpImageData;
extern uint8 *pgDownImageData;
extern uint8 *pgLeftImageData;
extern uint8 *pgRightImageData;
extern uint8 *autoWalkImageData;
extern uint8 *gaugeImageData;


/* ===================================================================== *
   Globals
 * ===================================================================== */

const int maxMouseTextLen = 80;

static char mouseText[maxMouseTextLen] = { "" };  //  Current mouse text string

static StaticPoint16 mouseImageOffset = {0, 0};           //  Hotspot on mouse image

static gPixelMap  *mouseImage = nullptr;  //  Current mouse cursor image

static gPixelMap *textImage = nullptr;   //  Current mouse text image
static gPixelMap *combinedImage = nullptr;  //   Combine mouse text
//  and image

static int textImageCenteredCol;       //  The pixel column in the text
//  image to be centered under
//  the mouse pointer

static int gaugeNumerator,             //  Current mouse gauge values
       gaugeDenominator;

static uint8 gaugeGrayMap[7] =
{ 0x00, 0x18, 0x14, 0x12, 0x16, 0x17, 0x0E };   //  Gray

static uint8 gaugeColorMapArray[3][7] = {
	{ 0x00, 0x18, 0xE2, 0xD1, 0xE5, 0xE7, 0xC9 },   //  Green
	{ 0x00, 0x18, 0x5C, 0x5A, 0x5E, 0x60, 0x49 },   //  Yellow
	{ 0x00, 0x18, 0x83, 0x81, 0x86, 0x72, 0x39 },   //  Red
};

static struct MouseCurosrs {
	uint8 **data;
	int w;
	int h;
} mouseCursorData[] = {
	{ &closeBx1ImageData,	12, 12 },
	{ &closeBx2ImageData,	12, 12 },
	{ &usePtrImageData,		15, 15 },
	{ &xPointerImageData,	15, 15 },
	{ &arrowImageData,		11, 17 },
	{ &grabPtrImageData,	16, 13 },
	{ &attakPtrImageData,	23, 23 },
	{ &centerActorIndicatorImageData, 9, 9 },
	{ &pgUpImageData,		16, 16 },
	{ &pgDownImageData,		16, 16 },
	{ &pgLeftImageData,		16, 16 },
	{ &pgRightImageData,	16, 16 },
	{ &autoWalkImageData,	15, 15 },
	{ &gaugeImageData,		gaugeImageWidth, gaugeImageHeight }
};

gStaticImage *mouseCursors[kMouseMax];

static uint8 gaugeImageBuffer[gaugeImageWidth * gaugeImageHeight];
static gPixelMap *gaugeImage = nullptr;

static bool showGauge = false;

/* ===================================================================== *
   Functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Create a new bitmap of several images stacked on top of each other

void initCursors() {
	for (int i = 0; i < kMouseMax; i++)
		mouseCursors[i] = new gStaticImage(mouseCursorData[i].w, mouseCursorData[i].h, *mouseCursorData[i].data);

	mouseImage = mouseCursors[kMouseArrowImage];
	gaugeImage = new gPixelMap;
	gaugeImage->size.x = gaugeImageWidth;
	gaugeImage->size.y = gaugeImageHeight;
	gaugeImage->data = gaugeImageBuffer;

	textImage = new gPixelMap;
	combinedImage = new gPixelMap;
}

void freeCursors() {
	for (int i = 0; i < kMouseMax; i++)
		delete mouseCursors[i];

	delete gaugeImage;
	delete textImage;
	delete combinedImage;
}

void createStackedImage(gPixelMap **newImage, int *newImageCenter, gPixelMap **imageArray, int *imageCenterArray, int images) {
	assert(images != 0);

	if (*newImage)
		delete *newImage;

	*newImage = new gPixelMap;

	(*newImage)->size.x = 0;
	(*newImage)->size.y = 0;
	*newImageCenter = 0;

	for (int i = 0; i < images; i++) {
		if (imageCenterArray[i] > *newImageCenter)
			*newImageCenter = imageCenterArray[i];
	}

	for (int i = 0; i < images; i++) {
		int16  rightImageBoundary;

		(*newImage)->size.y += imageArray[i]->size.y;

		rightImageBoundary = *newImageCenter + (imageArray[i]->size.x - imageCenterArray[i]);

		if (rightImageBoundary > (*newImage)->size.x)
			(*newImage)->size.x = rightImageBoundary;
	}

	(*newImage)->size.y += images - 1;

	int newImageBytes = (*newImage)->bytes();

	(*newImage)->data = (uint8 *)malloc(newImageBytes) ;

	memset((*newImage)->data, 0, newImageBytes);

	int newImageRow = 0;
	for (int i = 0; i < images; i++) {
		gPixelMap *currentImage = imageArray[i];

		TBlit((*newImage), currentImage, *newImageCenter - imageCenterArray[i], newImageRow);

		newImageRow += currentImage->size.y + 1;
	}
}

//-----------------------------------------------------------------------
//	Dispose of an image created with createStackedImage

inline void disposeStackedImage(gPixelMap **image) {
	if (*image) {
		free((*image)->data);
		(*image)->data = nullptr;

		delete *image;
		*image = nullptr;
	}
}

//-----------------------------------------------------------------------
//  Combine the mouse image and the mouse text image into a single
//  pixel map and reset the global mouse cursor to use the new combined
//	image.

void cleanupMousePointer() {
	if (combinedImage->data != nullptr)
		disposeStackedImage(&combinedImage);
}

void setupMousePointer() {
	int  imageIndex = 1;
	gPixelMap  *imageArray[3];
	int imageCenterArray[3];
	Point16 imageOffset;            //  mouse image offset
	int combinedImageCenter;

	imageArray[0] = mouseImage;
	imageCenterArray[0] = mouseImage->size.x / 2;

	if (mouseText[0] != '\0') {
		imageArray[imageIndex] = textImage;
		imageCenterArray[imageIndex] = textImageCenteredCol;
		imageIndex++;
	}

	if (showGauge) {
		imageArray[imageIndex] = gaugeImage;
		imageCenterArray[imageIndex] = gaugeImage->size.x / 2;
		imageIndex++;
	}

	if (combinedImage->data != nullptr)
		disposeStackedImage(&combinedImage);

	createStackedImage(&combinedImage, &combinedImageCenter, imageArray, imageCenterArray, imageIndex);

	imageOffset.x = combinedImageCenter - mouseImage->size.x / 2;
	imageOffset.y = 0;

	//  Set the combined image as the new mouse cursor
	g_vm->_pointer->hide();
	g_vm->_pointer->setImage(*combinedImage, mouseImageOffset.x - imageOffset.x, mouseImageOffset.y - imageOffset.y);
	g_vm->_pointer->show();
}

//-----------------------------------------------------------------------
//	Setup a new mouse cursor image

void setMouseImage(int id, int16 x, int16 y) {
	setMouseImage(*mouseCursors[id], x, y);
}

void setMouseImage(gPixelMap &img, int16 x, int16 y) {
	if (mouseImage != &img
	        ||  mouseImageOffset.x != x
	        ||  mouseImageOffset.y != y) {
		mouseImage = &img;
		mouseImageOffset.x = x;
		mouseImageOffset.y = y;

		setupMousePointer();
	}
}

//-----------------------------------------------------------------------
//	Dispose of old text

inline void disposeText() {
	mouseText[0] = '\0';

	//  Free the memory previously allocated to hold the text image
	//  bitmap
	if (textImage == nullptr)
		return;

	if (textImage->data != nullptr) {
		free(textImage->data);
		textImage->data = nullptr;
	}
}

//-----------------------------------------------------------------------
//	Build a new text image

void setNewText(char *text) {
	//  Dispose of old text image
	disposeText();

	Common::strlcpy(mouseText, text, maxMouseTextLen);

	//  Compute the size of the text bitmap
	textImage->size.y = mainFont->height + 2;
	textImage->size.x = TextWidth(mainFont, text, -1, 0) + 2;

	//  Allocate a new buffer for the text image bitmap
	int16 textImageBytes = textImage->bytes();

	textImage->data = (uint8 *)malloc(textImageBytes);
	memset(textImage->data, 0, textImageBytes);

	gPort textImagePort;  //  gPort used to draw text onto bitmap

	//  Intialize the text image port
	textImagePort.setMap(textImage);
	textImagePort.setMode(drawModeReplace);
	textImagePort.setColor(11);
	textImagePort.setOutlineColor(24);
	textImagePort.setFont(mainFont);
	textImagePort.setStyle(textStyleOutline);
	textImagePort.moveTo(1, 1);

	//  Draw the text
	textImagePort.drawText(text, -1);

	//  Compute relative position of text
	Point16 mousePos;
	g_vm->_pointer->getImageCurPos(mousePos);

	int mouseImageCenter = mousePos.x + mouseImageOffset.x + mouseImage->size.x / 2;
	textImageCenteredCol = textImage->size.x / 2;
	if (mouseImageCenter - textImageCenteredCol < 5) {
		textImageCenteredCol = mouseImageCenter - 5;
	} else if (mouseImageCenter + (textImage->size.x - textImageCenteredCol) >= screenWidth - 5) {
		textImageCenteredCol = textImage->size.x - ((screenWidth - 5) - mouseImageCenter);
	}
}

//-----------------------------------------------------------------------
//  Setup a new text string to display on the mouse cursor

void setMouseText(char *text) {
	if (text != nullptr) {
		if (strcmp(text, mouseText) == 0) return;

		setNewText(text);
		setupMousePointer();
	} else if (mouseText[0] != '\0') {
		disposeText();
		setupMousePointer();
	}
}

//  Formatted version of setMouseText, for gTools use...

void setMouseTextF(char *format, ...) {
	if (format == nullptr) {
		setMouseText(nullptr);
		g_vm->_toolBase->mouseHintSet = true;
	} else {
		char        lineBuf[128];
		va_list     argptr;

		va_start(argptr, format);
		vsprintf(lineBuf, format, argptr);
		va_end(argptr);

		setMouseText(lineBuf);
		g_vm->_toolBase->mouseHintSet = true;
	}
}

//-----------------------------------------------------------------------
//	Set the gauge value for the mouse pointer (automatically turns
//	gauge on if necessary ).

void setMouseGauge(int numerator, int denominator) {
	assert(denominator != 0);
	assert(numerator <= denominator);

	if (showGauge && numerator == gaugeNumerator && denominator == gaugeDenominator)
		return;

	gaugeNumerator = numerator;
	gaugeDenominator = denominator;

	int gaugePos = (numerator * gaugeWidth + denominator - 1) / denominator;

	uint8 *gaugeColorMap;

	if (gaugePos <= gaugeOneThird)
		gaugeColorMap = gaugeColorMapArray[2];
	else if (gaugePos <= gaugeTwoThirds)
		gaugeColorMap = gaugeColorMapArray[1];
	else
		gaugeColorMap = gaugeColorMapArray[0];

	int gaugeImageIndex = 0;

	for (int y = 0; y < gaugeImageHeight; y++) {
		for (int x = 0; x < gaugeImageWidth; x++) {
			uint8 *gaugeMap = x < gaugePos + 1 ? gaugeColorMap : gaugeGrayMap;

			gaugeImageBuffer[gaugeImageIndex] = gaugeMap[mouseCursors[kMouseGaugeImage]->data[gaugeImageIndex]];

			gaugeImageIndex++;
		}
	}

	showGauge = true;

	setupMousePointer();
}

//-----------------------------------------------------------------------
//	Turn off the gauge on the mouse pointer

void clearMouseGauge() {
	showGauge = false;

	setupMousePointer();
}

} // end of namespace Saga2
