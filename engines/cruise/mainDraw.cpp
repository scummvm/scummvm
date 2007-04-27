/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * $URL$
 * $Id$
 *
 */

#include "cruise/cruise_main.h"
#include "polys.h"

namespace Cruise {

int currentTransparent;

struct drawVar1Struct
{
	struct drawVar1Struct* next;
	short int field_2;
	short int field_4;
	short int field_6;
	short int field_8;
	objectStruct* field_A;
};

typedef struct drawVar1Struct drawVar1Struct;

drawVar1Struct drawVar1;


void mainDraw6(void)
{
	drawVar1Struct* pCurrent = drawVar1.next;

	while(pCurrent)
	{
		drawVar1Struct* next = pCurrent->next;

		if(pCurrent->field_6 == 5)
		{
			Op_7Sub(pCurrent->field_2, pCurrent->field_4, pCurrent->field_8);
		}
		else
		{
			setObjectPosition(pCurrent->field_2, pCurrent->field_4, pCurrent->field_6, pCurrent->field_8);
		}

		if(pCurrent->field_A->nextAnimDelay < 0)
		{
			objectParamsQuery params;

			getMultipleObjectParam(pCurrent->field_2, pCurrent->field_4, &params);

			pCurrent->field_A->currentAnimDelay = params.var6-1;
		}

		free(pCurrent);

		pCurrent = next;
	}
}

void flipScreen(void)
{
	uint8* swapPtr;

	swapPtr = gfxModuleData.pPage00;
	gfxModuleData.pPage00 = gfxModuleData.pPage10;
	gfxModuleData.pPage10 = swapPtr;

	gfxModuleData_flipScreen();

	/*memcpy(globalAtariScreen, gfxModuleData.pPage00, 16000);
	convertAtariToRaw(gfxModuleData.pPage00,globalScreen,200,320);*/
}

int spriteX1;
int spriteX2;
int spriteY1;
int spriteY2;

char* polyOutputBuffer;

void pixel(int x, int y, char color)
{
	if(x >= 0 && x < 320 && y >= 0 && y < 200)
		polyOutputBuffer[320*y+x] = color;
}

// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
void getPolyData(int fileIndex, int X, int Y, int *outScale, int *outY, int *outX, char **outDataPtr, int scale, char* dataPtr)
{
	if(*(uint16*) dataPtr == 0)
	{
		uint16   newFileIndex;
		uint16   newX;
		char* newDataPtr;			// this one is quite useless
		uint16   newY;

		dataPtr      += 2;
		newFileIndex  = *(uint16*) dataPtr;
		flipShort(&newFileIndex);

		dataPtr += 2;
		newX     = *(uint16*) dataPtr;
		flipShort(&newX);

		newDataPtr = dataPtr;		// useless

		newY = *(uint16*) (newDataPtr + 2);
		flipShort(&newY);

		newFileIndex += fileIndex;

		if(newFileIndex >= 0)
		{
			if(filesDatabase[newFileIndex].resType == 0 && filesDatabase[newFileIndex].subData.ptr)
			{
				dataPtr = (char*)filesDatabase[newFileIndex].subData.ptr;
			}
		}

		scale = -scale;
		X    -= newX;
		Y    -= newY;
	}

	*outDataPtr = dataPtr;
	*outX       = X;
	*outY       = Y;
	*outScale   = scale;
}

int upscaleValue(int value, int scale)
{
	return (((value * scale) << 8) / 2);
}

int m_flipLeftRight;
int m_useSmallScale;
int m_lowerX;
int m_lowerY;
int m_coordCount;
int m_first_X;
int m_first_Y;
int m_scaleValue;
int m_current_X;
int m_current_Y;
int m_color;

int16 polyBuffer[512];
int16 polyBuffer2[512];
int16 polyBuffer3[404];
int16 polyBuffer4[512];

// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
void getPolySize(int positionX, int positionY, int scale, int sizeTable[4], unsigned char* dataPtr)
{
	int upperBorder;
	int lowerBorder;
	m_flipLeftRight = 0;

	if(scale < 0) // flip left right
	{
		m_flipLeftRight = 1;
		scale           = -scale;
	}

	// X1

	upperBorder = *(dataPtr + 3);

	if(m_flipLeftRight)
	{
		upperBorder = -upperBorder;
	}
	
	upperBorder  = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;
	upperBorder  = -upperBorder;
	lowerBorder  =  upperBorder;

	// X2

	upperBorder  = *(dataPtr + 1);
	upperBorder -= *(dataPtr + 3);

	if(m_flipLeftRight)
	{
		upperBorder = -upperBorder;
	}

	upperBorder  = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;

	if(upperBorder < lowerBorder)	// exchange borders if lower > upper
	{
		int temp    = upperBorder;
		upperBorder = lowerBorder;
		lowerBorder = temp;
	}

	sizeTable[0] = lowerBorder;		// left
	sizeTable[1] = upperBorder;		// right

	// Y1

	upperBorder  = *(dataPtr + 4);
	upperBorder  = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;
	upperBorder  = -upperBorder;
	lowerBorder  = upperBorder;

	// Y2

	upperBorder  = *(dataPtr + 2);
	upperBorder -= *(dataPtr + 4);
	upperBorder  = (upscaleValue(upperBorder, scale) + 0x8000) >> 16;

	if(upperBorder < lowerBorder)	// exchange borders if lower > upper
	{
		int temp    = upperBorder;
		upperBorder = lowerBorder;
		lowerBorder = temp;
	}

	sizeTable[2] = lowerBorder;		// bottom
	sizeTable[3] = upperBorder;		// top
}

void blitPolyMode1(char* dest, char* ptr, int16* buffer, char color)
{
}

void blitPolyMode2(char* dest, int16* buffer, char color)
{
}

int polySize1;
int polySize2;
int polySize3;
int polySize4;

int polyVar1;
int16* polyVar2;

void drawPolySub(void)
{
	int i;

	for(i = 0; i < polyVar1; i++)
	{
		line(polyBuffer4[i * 2], polyBuffer4[i * 2 + 1], polyBuffer4[(i + 1) * 2], polyBuffer4[(i + 1) * 2 + 1], m_color & 0xF);
	}

	fillpoly(polyBuffer4, polyVar1, m_color & 0xF);
}

char* drawPolyMode1(char* dataPointer, int linesToDraw)
{
	int index;
	int16* pBufferDest;

	polyVar1    = linesToDraw;
	pBufferDest = &polyBuffer4[polyVar1 * 2];
	index       = *(dataPointer++);

	polySize1 = polySize2 = pBufferDest[-2] = pBufferDest[-2 + linesToDraw * 2] = polyBuffer2[index * 2];
	polySize1 = polySize2 = pBufferDest[-1] = pBufferDest[-1 + linesToDraw * 2] = polyBuffer2[(index * 2) + 1];

	linesToDraw--;

	pBufferDest -= 2;

	polyVar2 = pBufferDest;

	do
	{
		int value;

		index = *(dataPointer++);
		value = pBufferDest[-2] = pBufferDest[-2 + polyVar1 * 2] = polyBuffer2[index * 2];
		
		if(value < polySize1)
		{
			polySize1 = value;
		}
		if(value > polySize2)
		{
			polySize2 = value;
		}

		value = pBufferDest[-1] = pBufferDest[-1 + polyVar1 * 2] = polyBuffer2[(index * 2) + 1];
		
		if(value > polySize4)
		{
			polySize4 = value;
		}
		if(value < polySize3)
		{
			polySize3 = value;
			polyVar2 = pBufferDest - 4;
		}
		pBufferDest -= 2;

	} while(--linesToDraw);

	drawPolySub();

	return dataPointer;
}

char* drawPolyMode2(char* dataPointer, int linesToDraw)
{
	int index;
	int16* pBufferDest;

	pBufferDest = polyBuffer4;
	polyVar1    = linesToDraw;
	polyVar2    = polyBuffer4;
	index       = *(dataPointer++);

	polySize1 = polySize2 = pBufferDest[0] = pBufferDest[linesToDraw * 2]     = polyBuffer2[index * 2];
	polySize1 = polySize2 = pBufferDest[1] = pBufferDest[linesToDraw * 2 + 1] = polyBuffer2[(index * 2) + 1];

	linesToDraw--;

	pBufferDest += 2;

	do
	{
		int value;

		index = *(dataPointer++);
		value = pBufferDest[0] = pBufferDest[polyVar1 * 2] = polyBuffer2[index * 2];
		
		if(value < polySize1)
		{
			polySize1 = value;
		}
		if(value > polySize2)
		{
			polySize2 = value;
		}

		value = pBufferDest[1] = pBufferDest[polyVar1 * 2 + 1] = polyBuffer2[(index * 2) + 1];

		if(value > polySize4)
		{
			polySize4 = value;
		}
		if(value < polySize3)
		{
			polySize3 = value;
			polyVar2 = pBufferDest;
		}

		pBufferDest += 2;

	} while(--linesToDraw);

	drawPolySub();

	return dataPointer;
}

// this function builds the poly model and then calls the draw functions (OLD: mainDrawSub1Sub5)
void buildPolyModel(int positionX, int positionY, int scale, char* ptr2, char* destBuffer, char* dataPtr)
{
	int counter				   = 0;						// numbers of coordinates to process
	int startX				   = 0;						// first X in model
	int startY				   = 0;						// first Y in model
	int x					   = 0;						// current X
	int y					   = 0;						// current Y
	int offsetXinModel         = 0;						// offset of the X value in the model
	int offsetYinModel         = 0;						// offset of the Y value in the model
	unsigned char* dataPointer = (unsigned char*)dataPtr;
	int16* ptrPoly_1_Buf         = polyBuffer;
	int16* ptrPoly_2_Buf;
	polyOutputBuffer		   = destBuffer;			// global

	m_flipLeftRight = 0;
	m_useSmallScale = 0;
	m_lowerX        = *(dataPointer + 3);
	m_lowerY        = *(dataPointer + 4);

	if(scale < 0)
	{
		scale           = -scale;		// flip left right
		m_flipLeftRight = 1;
	}

	if(scale < 0x180)					// If scale is smaller than 384
	{
		m_useSmallScale = 1;
		m_scaleValue    = scale << 1;	// double scale
	}
	else
	{
		m_scaleValue = scale;
	}

	dataPointer += 5;

	m_coordCount = (*(dataPointer++)) + 1;	// original uses +1 here but its later substracted again, we could skip it
	m_first_X    =  *(dataPointer++);
	m_first_Y    =  *(dataPointer++);
	startX		 = m_lowerX - m_first_X;
	startY		 = m_lowerY - m_first_Y;

	if(m_useSmallScale)
	{
		startX >>= 1;
		startY >>= 1;
	}
	
	if(m_flipLeftRight)
	{
		startX = -startX;
	}

	/*
		NOTE:

		The original code continues here with using X, Y instead of startX and StartY.
	
		Original code:
		positionX -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
		positionY -= (upscaleValue(startX, m_scaleValue) + 0x8000) >> 16;
	*/
	
	// get coordinates from data

	startX = positionX - ((upscaleValue(startX, m_scaleValue) + 0x8000) >> 16);
	startY = positionY - ((upscaleValue(startY, m_scaleValue) + 0x8000) >> 16);
	
	ptrPoly_1_Buf[0] = 0;
	ptrPoly_1_Buf[1] = 0;
	ptrPoly_1_Buf   += 2;
	counter          = m_coordCount - 1 - 1;	// skip the first pair, we already have the values

	do
	{
		x = *(dataPointer++) - m_first_X;
		y = *(dataPointer++) - m_first_Y;
		
		if(m_useSmallScale)					// shrink all coordinates by factor 2 if a scale smaller than 384 is used
		{
			x >>= 1;
			y >>= 1;
		}

		ptrPoly_1_Buf[0] = offsetXinModel - x;
		ptrPoly_1_Buf++;
		offsetXinModel = x;
		
		ptrPoly_1_Buf[0] = -(offsetYinModel - y);
		ptrPoly_1_Buf++;
		offsetYinModel = y;

	} while(--counter);

	// scale and adjust coordinates with offset (using two polybuffers by doing that)

	/*
		NOTE: Is there a need for having two, a scaled and unscaled polybuffer?
	*/
	
	ptrPoly_2_Buf = polyBuffer;
	ptrPoly_1_Buf = polyBuffer2;
	counter       = m_coordCount - 1;		// reset counter // process first pair two
	m_current_X   = 0;
	m_current_Y   = 0;

	do
	{
		x = ptrPoly_2_Buf[0];
		
		if(m_flipLeftRight == 0)
		{
			 x = -x;
		}

		//////////////////

		m_current_X     += upscaleValue(x, m_scaleValue);
		ptrPoly_1_Buf[0] = ((m_current_X + 0x8000) >> 16) + startX;			// adjust X value with start offset

		m_current_Y     += upscaleValue(ptrPoly_2_Buf[1], m_scaleValue);
		ptrPoly_1_Buf[1] = ((m_current_Y + 0x8000) >> 16) + startY;			// adjust Y value with start offset
		
		/////////////////

		ptrPoly_1_Buf += 2;
		ptrPoly_2_Buf += 2;

	} while(--counter);

	// position of the dataPointer is m_coordCount * 2

	do
	{
		int linesToDraw = *dataPointer++;

		if(linesToDraw > 1)							// if value not zero
		{
			uint16 minimumScale;

			m_color      = *dataPointer;			// color
			dataPointer += 2;

			minimumScale = *(uint16*) (dataPointer);
			dataPointer += 2;

			flipShort(&minimumScale);

			if(minimumScale > scale)				// if the scale is too small, for the model to be drawn ...
			{
				dataPointer += linesToDraw;			// ... skip ahead
			}
			else
			{
				if(m_flipLeftRight)
				{
					dataPointer = (unsigned char*)drawPolyMode1((char*)dataPointer, linesToDraw);
				}
				else
				{
					dataPointer = (unsigned char*)drawPolyMode2((char*)dataPointer, linesToDraw);
				}

				if(destBuffer)
				{
					if(ptr2)
					{
						blitPolyMode1(destBuffer, ptr2, polyBuffer3, m_color & 0xF);
					}
					else
					{
						blitPolyMode2(destBuffer, polyBuffer3, m_color & 0xF);
					}
				}
			}
		}
		else
		{
			dataPointer += 4;
		}
	} while(*dataPointer != 0xFF);
}

// draw poly sprite (OLD: mainDrawSub1)
void mainDrawPolygons(int fileIndex, objectStruct* pObject, int X, int scale, int Y, char* destBuffer, char* dataPtr)
{
	int   newX;
	int   newY;
	int   newScale;
	char* newDataPtr;
	char* ptr2;				// unused
	int   var_8;			// unused

	int sizeTable[4];		// 0 = left, 1 = right, 2 = bottom, 3 = top

	// this function checks if the dataPtr is not 0, else it retrives the data for X, Y, scale and DataPtr again (OLD: mainDrawSub1Sub1)
	getPolyData(fileIndex, X, Y, &newScale, &newY, &newX, &newDataPtr, scale, dataPtr);
	
	// this function fills the sizeTable for the poly (OLD: mainDrawSub1Sub2)
	getPolySize(newX, newY, newScale, sizeTable, (unsigned char*)newDataPtr);

	spriteX2 = sizeTable[0] -  2;	// left   border
	spriteX1 = sizeTable[1] + 18;	// right  border
	spriteY2 = sizeTable[2] -  2;	// bottom border
	spriteY1 = sizeTable[3] +  2;	// top    border
	
/*	if (X == 28 && Y == 80 && scale == 1024)  {
		printf("0--> x1: %i, y1: %i, x2: %i, y2: %i\n", sizeTable[1], sizeTable[3], sizeTable[0], sizeTable[2]);
		printf("1--> x1: %i, y1: %i, x2: %i, y2: %i\n", spriteX1, spriteY1, spriteX2, spriteY2);
	} */
	if(spriteX2 >= 320)
		return;
	if(spriteX1 < 0)
		return;
	if(spriteY2 >= 200)
		return;
	if(spriteY1 < 0)
		return;

	if(spriteX2 < 0)
	{
		spriteX2 = 0;
	}
	if(spriteX1 > 320)
	{
		spriteX1 = 320;
	}
	if(spriteY2 < 0)
	{
		spriteY2 = 0;
	}
	if(spriteY1 > 200)
	{
		spriteY1 = 200;
	}

	if(spriteX1 == spriteX2)
		return;
	if(spriteY1 == spriteY2)
		return;

	ptr2 = NULL;
	var_8 = 0;

	if(pObject)
	{
		objectStruct* pCurrentObject = pObject;

		do
		{
			if(pCurrentObject->type == 2)
			{
				// ASSERT(0);
			}

			pCurrentObject = pCurrentObject->next;
		} while(pCurrentObject);
	}

	// this function builds the poly model and then calls the draw functions (OLD: mainDrawSub1Sub5)
	buildPolyModel(newX, newY, newScale, ptr2, destBuffer, newDataPtr);
}

void mainSprite(int globalX, int globalY, gfxEntryStruct* pGfxPtr, uint8* ouputPtr, int newColor, int idx)
{	
	// this is used for font only

	if(pGfxPtr)
	{
		uint8* initialOuput;
		uint8* output;
		int i;
		int j;
		int x;
		int y;
		uint8* ptr    = pGfxPtr->imagePtr;
		int height = pGfxPtr->height;
		int width  = pGfxPtr->width;

		if(globalY < 0)
		{
			globalY = 0;
		}

		if(globalY + pGfxPtr->height >= 198)
		{
			globalY = 198 - pGfxPtr->height;
		}

		initialOuput = ouputPtr + (globalY*320) + globalX;

		y = globalY;
		x = globalX;

		for(i = 0; i < height; i++)
		{
			output = initialOuput + 320 * i;
			
			for(j = 0; j < width; j++)
			{
				uint8 color = *(ptr++);

				if(color)
				{
					if((x >= 0) && (x < 320) && (y >= 0) && (y < 200))
					{
						if(color == 1)
						{
							*output = (uint8) 0;
						}
						else
						{
							*output = (uint8) newColor;
						}
					}
				}
				output++;
			}
		}
	}
}

void mainDrawSub4(int objX1, int var_6, objectStruct* currentObjPtr, char* data1, int objY2, int objX2, char* output, char* data2)
{
	int x = 0;
	int y = 0;

	for(y = 0; y < var_6; y++)
	{
		for(x = 0; x < (objX1 * 8); x++)
		{
			uint8 color = (data1[0]);
			data1++;

			if((x + objX2) >= 0 && (x + objX2) < 320 && (y + objY2) >= 0 && (y + objY2) < 200)
			{
				if(color != currentTransparent)
				{
					output[320 * (y + objY2) + x + objX2] = color;
				}
			}
		}
	}
}

void mainDraw5(int overlayIdx, int idx, int field_28, objectStruct* pObject, int newVal)
{
	drawVar1Struct* pNewEntry;

	pNewEntry = (drawVar1Struct*)malloc(sizeof(drawVar1Struct));

	pNewEntry->next = drawVar1.next;
	drawVar1.next   = pNewEntry;

	pNewEntry->field_2 = overlayIdx;
	pNewEntry->field_4 = idx;
	pNewEntry->field_6 = field_28;
	pNewEntry->field_8 = newVal;
	pNewEntry->field_A = pObject;
}

#ifdef _DEBUG
void drawCtp(void)
{
	int i;

	if(ctp_walkboxTable)
	{
		for(i = 0; i < 15; i++)
		{
			uint16* dataPtr = &ctp_walkboxTable[i * 40];
			int type = walkboxType[i];					// show different types in different colors

			if(*dataPtr)
			{
				int j;
				fillpoly((short*)dataPtr+1, *dataPtr, type);

				for(j = 0; j < (*dataPtr - 1); j++)
				{
					line(dataPtr[1 + j * 2], dataPtr[1 + j * 2 + 1], dataPtr[1 + (j + 1) * 2], dataPtr[1 + ( j + 1) * 2 + 1], 0);
				}

				line(dataPtr[1 + j * 2], dataPtr[1 + j * 2 + 1], dataPtr[1], dataPtr[2], 0);
			}
		}
	}
}
#endif

void drawMenu(menuStruct* pMenu)
{
	if(pMenu && pMenu->numElements)
	{
		int height;
		int x;
		int y;
		int var_10;
		int bx;
		int newX;
		int var_6;
		int currentY;
		int var_8;
		int di;
		menuElementStruct* si;

		height = pMenu->gfx->height;
		x      = pMenu->x;
		y      = pMenu->y;

		var_10 = pMenu->gfx->width / (199 - (pMenu->gfx->width * 2));

		bx = var_10 / (pMenu->numElements + 1); // rustine...

		if(!bx)
		{
			bx++;

			if((pMenu->numElements * height) + y > 199 - height)
			{
				y = ((-1 - pMenu->numElements) * height) + 200;
			}
		}
		else
		{
			if(var_10 % pMenu->numElements)
			{
				bx++;
			}

			y = height;
		}

		newX = 320 * (2 - bx);

		if(newX < x)
		{
			x = newX;
		}

		if(x < 0)
		{
			x = 0;
		}

		var_6 = (80 * (bx-1)) + x;

		if(var_6 <= 320)
		{
			mainSprite(var_6, y - height, pMenu->gfx, gfxModuleData.pPage10, video4, 320);
		}

		currentY = y;
		var_8 = 0;
		di = x;

		si = pMenu->ptrNextElement;

		if(si)
		{
			do
			{
				int color;

				gfxEntryStruct* var_2 = si->gfx;

				si->x    = di;
				si->y    = currentY;
				si->varA = 320;

				if(si->varC)
				{
					color = video3;
				}
				else
				{
					if(si->color != 255)
					{
						color = si->color;
					}
					else
					{
						color = video2;
					}
				}

				if(di < 320)
				{
					mainSprite(di, currentY, var_2, gfxModuleData.pPage10, color, 320);
				}

				currentY += height;
				var_8++;

				if(var_8 == var_10)
				{
					var_8    = 0;
					di      += 320;
					currentY = y;
				}

				si = si->next;
			} while(si);
		}
	}
}

int getValueFromObjectQuerry(objectParamsQuery* params, int idx)
{
	switch(idx)
	{
	case 0:
		return params->X;
	case 1:
		return params->Y;
	case 2:
		return params->baseFileIdx;
	case 3:
		return params->fileIdx;
	case 4:
		return params->scale;
	case 5:
		return params->var5;
	case 6:
		return params->var6;
	case 7:
		return params->var7;
	}

	assert(0);

	return 0;
}

void mainDraw(int16 param)
{
	uint8* bgPtr;
	objectStruct* currentObjPtr;
	int16 currentObjIdx;
	int16 objX1 = 0;
	int16 objY1 = 0;
	int16 objZ1 = 0;
	int16 objX2;
	int16 objY2;
	int16 spriteHeight;


	if(fadeVar)
	{
		return;
	}

	bgPtr = backgroundPtrtable[currentActiveBackgroundPlane];

	if(bgPtr)
	{
		gfxModuleData_gfxCopyScreen((char*)bgPtr,(char*)gfxModuleData.pPage10);
	}

	drawVar1.next = NULL;

	currentObjPtr = objectHead.next;

#ifdef _DEBUG
	polyOutputBuffer = (char*)bgPtr;
	drawCtp();
#endif

	//-------------------------------------------------- DRAW OBJECTS TYPE 4 -----------------------------------------//

	while(currentObjPtr)
	{
		if((currentActiveBackgroundPlane == currentObjPtr->backgroundPlane) && (currentObjPtr->hide == 0) && (currentObjPtr->type == 4))
		{
			int16 fileIdx;
			objectParamsQuery params;

			currentObjIdx = currentObjPtr->idx;

			if((currentObjPtr->followObjectOverlayIdx == currentObjPtr->overlay) && (currentObjPtr->followObjectIdx == currentObjPtr->idx))
			{
				objX1 = 0;
				objY1 = 0;
				objZ1 = 0;
			}
			else
			{	
				// Declaring this twice ?
				// objectParamsQuery params;

				getMultipleObjectParam(currentObjPtr->followObjectOverlayIdx, currentObjPtr->followObjectIdx, &params);

				objX1 = params.X;
				objY1 = params.Y;
				objZ1 = params.fileIdx;
			}

			getMultipleObjectParam(currentObjPtr->overlay, currentObjIdx, &params);

			objX2 = objX1 + params.X;
			objY2 = objY1 + params.Y;

			fileIdx = params.fileIdx;

			if(fileIdx >= 0)
			{
				fileIdx += objZ1;
			}

			if((params.var5 >= 0) && (fileIdx >= 0) && filesDatabase[fileIdx].subData.ptr)
			{
				if(filesDatabase[fileIdx].subData.resourceType == 8)
				{
					mainDrawPolygons(fileIdx, currentObjPtr, objX2, params.scale, objY2, (char*)gfxModuleData.pPage10, (char*)filesDatabase[fileIdx].subData.ptr); // poly
				}
				else if(filesDatabase[fileIdx].subData.resourceType == 6)
				{
				}
				else if(filesDatabase[fileIdx].resType == 1)
				{
				}
				else if(filesDatabase[fileIdx].subData.resourceType == 4)
				{
					objX1        = filesDatabase[fileIdx].width;	// width
					spriteHeight = filesDatabase[fileIdx].height;	// height

					if(filesDatabase[fileIdx].subData.ptr)
					{
						currentTransparent = filesDatabase[fileIdx].subData.transparency;

						mainDrawSub4(objX1, spriteHeight, currentObjPtr, (char*)filesDatabase[fileIdx].subData.ptr, objY2, objX2, (char*)gfxModuleData.pPage10, (char*)filesDatabase[fileIdx].subData.ptr);
					}
				}
			}

			if((currentObjPtr->field_26 != 0) && (param == 0))
			{
				if(currentObjPtr->currentAnimDelay <= 0) 
				{
					int newVal;
					objX1 = 1;

					newVal = getValueFromObjectQuerry(&params,currentObjPtr->field_28) + currentObjPtr->field_26;

					if(currentObjPtr->field_26 > 0)
					{
						if(currentObjPtr->field_22 < newVal)
						{
							if(currentObjPtr->field_30 >= 0)
							{
								int16 data2;

								newVal = currentObjPtr->field_20;

								if(currentObjPtr->field_30 > 0)
								{
									currentObjPtr->field_30--;
								}

								data2 = currentObjPtr->field_20;

								objX1 = 0;
								currentObjPtr->field_26 = 0;

								if(currentObjPtr->field_2A) // should we resume the script ?
								{
									if(currentObjPtr->field_1A == 20)
									{
										changeScriptParamInList(currentObjPtr->field_18, currentObjPtr->field_16, &scriptHandle2, 0, -1);
									}
									else
										if(currentObjPtr->field_1A == 30)
										{
											changeScriptParamInList(currentObjPtr->field_18, currentObjPtr->field_16, &scriptHandle1, 0, -1);
										}
								}
								newVal = data2;
							}
						}
					}
					else
					{
						ASSERT(0);
						/*     if(currentObjPtr->field_22>newVal)
						{
						} */
					}

					if(currentObjPtr->nextAnimDelay >= 0)
					{
						currentObjPtr->currentAnimDelay = currentObjPtr->nextAnimDelay;
					}

					if(currentObjPtr->field_2C >= 0 && (currentObjPtr->field_2C == newVal) && currentObjPtr->field_2A != 0)
					{
						if(currentObjPtr->field_1A == 20)
						{
							changeScriptParamInList(currentObjPtr->field_18, currentObjPtr->field_16, &scriptHandle2, 0, -1);
						}
						else
							if(currentObjPtr->field_1A == 30)
							{
								changeScriptParamInList(currentObjPtr->field_18, currentObjPtr->field_16, &scriptHandle1, 0, -1);
							}

							currentObjPtr->field_2A = 0;
					}

					if(objX1)
					{
						mainDraw5(currentObjPtr->overlay, currentObjPtr->idx, currentObjPtr->field_28, currentObjPtr, newVal);
					}
				}
				else
				{
					currentObjPtr->currentAnimDelay--;
				}
			}
		}

		currentObjPtr = currentObjPtr->next;
	}

	//----------------------------------------------------------------------------------------------------------------//

	mainDraw6();
	var20 = 0;

	//-------------------------------------------------- DRAW OBJECTS TYPE 5 -----------------------------------------//

	currentObjPtr = objectHead.next;

	while(currentObjPtr)
	{
		if(currentObjPtr->type == 5 && currentObjPtr->hide == 0)
		{
			mainSprite(currentObjPtr->field_A, currentObjPtr->field_C, currentObjPtr->gfxPtr, gfxModuleData.pPage10, currentObjPtr->field_10, currentObjPtr->spriteIdx);
			var20 = 1;
		}
		currentObjPtr = currentObjPtr->next;
	}

	//----------------------------------------------------------------------------------------------------------------//

	if(currentActiveMenu != -1)
	{
		if(menuTable[currentActiveMenu])
		{
			drawMenu(menuTable[currentActiveMenu]);
			return;
		}
	}

	if(mouseVar1)
	{
		ASSERT(0);
		// TODO: draw mouse here
	}
}

} // End of namespace Cruise
