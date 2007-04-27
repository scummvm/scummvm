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

#include "common/file.h"

#include "cruise/cruise_main.h"

namespace Cruise {

// (old: fontProc1(int16 param, uint8* ptr1, uint8* ptr2))
int32 getLineHeight(int16 charCount, uint8* fontPtr, uint8* fontPrt_Desc)
{
	uint8* dest;
	int32 highestChar = 0;
	int32 i;

	if(!charCount)
	{
		return(0);
	}
	dest = fontPrt_Desc + 6;		// fontPtr + 20		// char height

	for(i = 0; i < charCount; i++)
	{
		if((*(int16*) dest) > highestChar)
		{
			highestChar = *(int16*) dest;
		}
		dest += 12;
	}
	return highestChar;
}

// this function determins how many lines the text will have (old: fontProc2(int32 param1, int32 param2, uint8* ptr, uint8* string))
int32 getTextLineCount(int32 rightBorder_X, int32 wordSpacingWidth, uint8* ptr, uint8* textString)
{
	uint8* localString = textString;
	uint8* currentStringPtr;
	uint8  character;

	int32 var_6      = 0;
	int32 lineLength = 0;

	uint8* tempPtr = 0;

	if(!*localString)
	{
		return(0);
	}
	currentStringPtr = localString;
	character        = *localString;

	do
	{
		int32 charData = fontCharacterTable[character];

		if(character == '|')
		{
			lineLength  = rightBorder_X;
			localString = tempPtr;
		}
		else
		{
			if(charData >= 0)
			{						// + 0xA jump to last 2 bytes of the 12 bytes slice = letter width
				lineLength += wordSpacingWidth + *(int16*) (ptr + 0xA + charData * 12);
			}
			else
			{
				if(character == ' ')
				{
					lineLength += wordSpacingWidth + 5;
					localString = currentStringPtr;
				}
			}
		}

		tempPtr = currentStringPtr;

		if(rightBorder_X <= lineLength)
		{
			var_6           += rightBorder_X;
			currentStringPtr = localString;
			tempPtr          = localString;
			lineLength       = 0;
		} 

		character        = *(tempPtr++);
		currentStringPtr = tempPtr;

	} while(character);

	if(lineLength == 0)
	{
		return (var_6 / rightBorder_X);
	}
	else
	{
		return ((var_6 + rightBorder_X) / rightBorder_X);
	}
}

void loadFNT(const void* fileNameChar)
{
	uint8    header[6];
	int32   fontSize;
	int32   data2;
	uint8    data3[6];
	uint8*   fileName       = (uint8*) fileNameChar;
	_systemFNT     = NULL;

	Common::File fontFileHandle;
	
	if(!fontFileHandle.exists((char*)fileName))
	{
		return;
	}

	fontFileHandle.open((char*)fileName);

	fontFileHandle.read(header, 4);

	if(strcmpuint8(header, "FNT") == 0)
	{
		fontFileHandle.read(&fontSize, 4);
		flipLong(&fontSize);

		fontFileHandle.read(&data2, 4);
		flipLong(&data2);

		fontFileHandle.read(data3, 6); // may need an endian flip ?
		flipGen(&data3, 6);

		_systemFNT = (uint8*) mallocAndZero(fontSize);

		if(_systemFNT != NULL)
		{
			int32 i;
			uint8* currentPtr;

			fontFileHandle.seek(0);
			fontFileHandle.read(header, 4);	// not really require, we could fseek to 4

			fontFileHandle.read(_systemFNT, fontSize);

			flipLong((int32*)  _systemFNT);
			flipLong((int32*) (_systemFNT + 4));
			flipGen(_systemFNT + 8, 6);

			currentPtr = _systemFNT + 14;

			for(i = 0; i < *(int16*) (_systemFNT + 8); i++)
			{
				flipLong((int32*) currentPtr);
				currentPtr += 4;

				flipGen(currentPtr, 8);
				currentPtr += 8;
			}
		}
	}

	fontFileHandle.close();
}

void loadSystemFont(void)
{
	int32 i;

	video2 = 15;
	video4 = 9;
	video3 = 13;
	colorOfSelectedSaveDrive = 10;

	for(i = 0; i < 64; i++)
	{
		mediumVar[i].ptr      = 0;
		mediumVar[i].field_1C = 0;
	}

	initVar1 = 0;
	main5    = 0;
	var22    = 0;
	initVar2 = 0;
	initVar3 = 0;
	currentActiveBackgroundPlane = 0;

	//changeCursor();

	initVar4[0] = 0;

	loadFNT("system.fnt");
}

void flipShort(int16* var)
{
	uint8* varPtr = (uint8*) var;
	uint8 temp   = varPtr[0];
	varPtr[0] = varPtr[1];
	varPtr[1] = temp;
}

void flipShort(uint16* var)
{
	uint8* varPtr = (uint8*) var;
	uint8 temp   = varPtr[0];
	varPtr[0] = varPtr[1];
	varPtr[1] = temp;
}

void flipLong(int32* var)
{
	char swap1;
	char swap2;
	char* varPtr = (char*) var;

	swap1     = varPtr[0];
	varPtr[0] = varPtr[3];
	varPtr[3] = swap1;

	swap2     = varPtr[1];
	varPtr[1] = varPtr[2];
	varPtr[2] = swap2;
}

void flipLong(uint32* var)
{
	char swap1;
	char swap2;
	char* varPtr = (char*) var;

	swap1     = varPtr[0];
	varPtr[0] = varPtr[3];
	varPtr[3] = swap1;

	swap2     = varPtr[1];
	varPtr[1] = varPtr[2];
	varPtr[2] = swap2;
}

void flipGen(void* var, int32 length)
{
	int i;
	short int* varPtr = (int16*) var;

	for(i = 0; i < (length / 2); i++)
	{
		flipShort(&varPtr[i]);
	}
}


void renderWord(uint8* fontPtr_Data, uint8* outBufferPtr, int32 drawPosPixel_X, int32 heightOff, int32 height, int32 param4, int32 stringRenderBufferSize, int32 width, int32 charWidth)
{
	int i;
	int j;
	uint8* fontPtr_Data2  = fontPtr_Data + height * 2;

	outBufferPtr += heightOff * width * 2;		// param2 = height , param6 = width
	outBufferPtr += drawPosPixel_X;				// param1 = drawPosPixel_X

	for(i = 0; i < height; i++)				// y++
	{
		uint16 currentColor1 = (*(fontPtr_Data)  << 8) | *(fontPtr_Data  + 1);
		uint16 currentColor2 = (*(fontPtr_Data2) << 8) | *(fontPtr_Data2 + 1);

		fontPtr_Data  += 2;
		fontPtr_Data2 += 2;

		for (j = 0; j < charWidth; j++)
		{
			*outBufferPtr = ((currentColor1 >> 15) & 1) | ((currentColor2 >> 14) & 2);
			outBufferPtr++;
			
			currentColor1  <<= 1;
			currentColor2  <<= 1;
		}
		outBufferPtr += (width * 2) - charWidth;
	}
}

// returns character count and pixel size (via pointer) per line of the string (old: prepareWordRender(int32 param, int32 var1, int16* out2, uint8* ptr3, uint8* string))
int32 prepareWordRender(int32 inRightBorder_X, int32 wordSpacingWidth, int16* strPixelLength, uint8* ptr3, uint8* textString)
{
	uint8* localString = textString;

	int32 counter    = 0;
	int32 finish     = 0;
	int32 temp_pc    = 0;		// var_A		// temporary pixel count save
	int32 temp_cc    = 0;		// var_C		// temporary char  count save
	int32 pixelCount = 0;		// si

	do
	{
		uint8 character = *(localString++);
		int16 charData = fontCharacterTable[character];

		if(character == ' ')
		{
			temp_cc = counter;
			temp_pc = pixelCount;

			if(pixelCount + wordSpacingWidth + 5 >= inRightBorder_X)
			{
				finish = 1;
			}
			else
			{
				pixelCount += wordSpacingWidth + 5 ;
			}
		}
		else
		{
			if(character == '|' || !character)
			{
				finish = 1;
			}
			else
			{
				if(charData)
				{
					if(pixelCount + wordSpacingWidth + *(int16*) ((ptr3 + charData * 12) + 0xA) >= inRightBorder_X)
					{
						finish = 1;
						if(temp_pc)
						{
							pixelCount = temp_pc;
							counter    = temp_cc;
						}
					}
					else
					{
						pixelCount += wordSpacingWidth + *(int16*) ((ptr3 + charData * 12) + 0xA);
					}
				}
			}
		}
		counter++;
	} while(!finish);

	*strPixelLength = (int16) pixelCount;
	return counter;
}


void drawString(int32 x, int32 y, uint8* string, uint8* buffer, uint8 color, int32 inRightBorder_X)
{
	uint8* fontPtr;
	uint8* fontPtr_Data;							// ptr2
	uint8* fontPtr_Desc;							// ptr3
	int32 wordSpacingWidth;						// var1
	int32 wordSpacingHeight;						// var2
	int32 rightBorder_X;							// param2
	int32 lineHeight;								// fontProc1result
	int32 numLines;
	int32 stringHeight;
	int32 stringFinished;
	int32 stringWidth;							// var_1C
	int32 stringRenderBufferSize;
	int32 useDynamicBuffer;
	uint8* currentStrRenderBuffer;
	// int32 var_8;								// don't need that on
	int32 heightOffset;							// var_12
	int32 renderBufferSize;						// var_1E
	int needFlip;

	if(!buffer || !string)
	{
		return;
	}

	if(fontFileIndex != -1)
	{
		fontPtr = filesDatabase[fontFileIndex].subData.ptr;

		if(!fontPtr)
		{
			fontPtr = _systemFNT;
		}
	}
	else
	{
		fontPtr = _systemFNT;
	}

	if(!fontPtr)
	{
		return;
	}

	fontPtr_Data = fontPtr + *(int16*) (fontPtr + 4);
	fontPtr_Desc = fontPtr + 14;

	lineHeight = getLineHeight(*(int16*) (fontPtr + 8), fontPtr, fontPtr_Desc); // ok

	wordSpacingWidth  = *(int16*) (fontPtr + 10);
	wordSpacingHeight = *(int16*) (fontPtr + 12);

	if(inRightBorder_X > 310)
	{
		rightBorder_X = 310;
	}
	else
	{
		rightBorder_X = inRightBorder_X;
	}
	if(x + rightBorder_X > 319)
	{
		x = 319 - rightBorder_X;
	}
	if(y < 0)
	{
		y = 0;
	}
	if(x < 0)
	{
		x = 0;
	}
	numLines = getTextLineCount(rightBorder_X, wordSpacingWidth, fontPtr_Desc, string); // ok

	if(!numLines) 
	{
		return;
	}
	stringHeight = ((wordSpacingHeight + lineHeight + 2) * numLines) + 1;

	if (y + stringHeight > 199)
	{
		y = 200 - stringHeight;
	}
	stringFinished         = 0;
	stringWidth            = (rightBorder_X / 16) + 2;
	stringRenderBufferSize = stringWidth * stringHeight * 4;
	inRightBorder_X        = rightBorder_X;

	if(stringRenderBufferSize > 0x2000)
	{
		currentStrRenderBuffer = (uint8*) mallocAndZero(stringRenderBufferSize);

		if(!currentStrRenderBuffer)
		{
			return;
		}
		useDynamicBuffer = 1;
	}
	else
	{
		currentStrRenderBuffer = (uint8*) ptr_something;
		useDynamicBuffer       = 0;
	}

	resetRaster(currentStrRenderBuffer, stringRenderBufferSize);

	// var_8        = 0;
	heightOffset = 0;
	renderBufferSize  = stringRenderBufferSize;

	do
	{
		int       spacesCount = 0;			// si
		char      character   = *(string);
		short int strPixelLength;			// var_16;
		uint8*       ptrStringEnd;				// var_4	//ok
		int       drawPosPixel_X;			// di

		while(character == ' ')
		{
			spacesCount++;
			character = *(string + spacesCount);
		}

		string      += spacesCount;
		ptrStringEnd = string + prepareWordRender(inRightBorder_X, wordSpacingWidth, &strPixelLength, fontPtr_Desc, string); //ok

		if(inRightBorder_X > strPixelLength)
		{
			drawPosPixel_X = (inRightBorder_X - strPixelLength) / 2;
		}
		else
		{
			drawPosPixel_X = 0;
		}
		// drawPosPixel_X = var_8;

		do
		{
			character = *(string++);

			short int data = fontCharacterTable[character];

			if(character)
			{
				if(character == ' ' || character == 0x7D)
				{
					drawPosPixel_X += var1 + 5;
				}
				else
				{
					if(data)
					{
						short int* si = (int16*) (fontPtr_Desc + data * 12);
						//int var_2     = si[5];

						renderWord(fontPtr_Data + si[0], currentStrRenderBuffer, drawPosPixel_X, si[4] - si[3] + lineHeight + heightOffset, si[3], si[2], renderBufferSize / 2, stringWidth * 2, si[5]);

						drawPosPixel_X += wordSpacingWidth + si[5];
					}
				}
			}
			else
			{
				stringFinished = 1;
			}

			if(ptrStringEnd <= string) 
			{
				break;
			}

		} while(!stringFinished);

		// var_8  = 0;
		heightOffset = wordSpacingHeight + lineHeight;

	} while(!stringFinished);

	needFlip = 0;

	if(buffer == gfxModuleData.pPage00)
	{
		if(gfxModuleData.field_1 != 0)
		{
			needFlip = 1;
			gfxModuleData_field_90();
		}

		gfxModuleData_gfxWaitVSync();
	}

	gfxModuleData_field_64((char*)currentStrRenderBuffer, stringWidth, stringHeight, (char*)buffer, x, y, 0);
	gfxModuleData_field_64((char*)currentStrRenderBuffer, stringWidth, stringHeight, (char*)buffer, x, y, color);

	if(needFlip)
	{
		gfxModuleData_flip();
	}

	if(useDynamicBuffer)
	{
		free(currentStrRenderBuffer);
	}
}


// calculates all necessary datas and renders text
gfxEntryStruct* renderText(int inRightBorder_X, uint8* string)
{
	uint8* fontPtr;
	uint8* fontPtr_Data;						// pt2
	uint8* fontPtr_Desc;						// ptr3
	int32 wordSpacingWidth;                   // var1		//0 or -1
	int32 wordSpacingHeight;                  // var2		//0 or -1
	int32 rightBorder_X;
	int32 lineHeight;							// fontProc1result
	int32 numLines;
	int32 stringHeight;
	int32 stringFinished;
	int32 stringWidth;                        // var_1C
	int32 stringRenderBufferSize;
	//  int32 useDynamicBuffer;
	uint8* currentStrRenderBuffer;
    //	int32 var_8;                          // don't need that one
	int32 heightOffset;                       // var_12	// how much pixel-lines have already been drawn
	//  int32 var_1E;
	gfxEntryStruct* generatedGfxEntry;

    // check if string is empty
	if(!string)
	{
		return NULL;
	}
    
    // check if font has been loaded, else get system font
	if(fontFileIndex != -1)
	{
		fontPtr = filesDatabase[fontFileIndex].subData.ptr;

		if(!fontPtr) 
		{
			fontPtr = _systemFNT;
		}
	}
	else
	{
		fontPtr = _systemFNT;
	}

	if(!fontPtr) 
	{
		return NULL;
    }
	fontPtr_Data = fontPtr + *(int16*) (fontPtr + 4);		// offset to char data
	fontPtr_Desc = fontPtr + 14;						// offset to char description

	lineHeight = getLineHeight(*(int16*) (fontPtr + 8), fontPtr, fontPtr_Desc); // ok

	wordSpacingWidth  = *(int16*) (fontPtr + 10);
	wordSpacingHeight = *(int16*) (fontPtr + 12);

	// if right border is higher then screenwidth (+ spacing), adjust border
	if(inRightBorder_X > 310)
	{
		rightBorder_X = 310;
	}
	else
	{
		rightBorder_X = inRightBorder_X;
    }
	numLines = getTextLineCount(rightBorder_X, wordSpacingWidth, fontPtr_Desc, string); // ok

	if(!numLines)
	{
		return NULL;
    }
    
	stringHeight           = ((wordSpacingHeight + lineHeight + 2) * numLines) + 1;
	stringFinished         = 0;
	stringWidth            = rightBorder_X + 2;                 // max render width to the right
	stringRenderBufferSize = stringWidth * stringHeight * 4;
	inRightBorder_X        = rightBorder_X;

	currentStrRenderBuffer = (uint8*) mallocAndZero(stringRenderBufferSize);
	resetRaster(currentStrRenderBuffer, stringRenderBufferSize);

	generatedGfxEntry = (gfxEntryStruct*) malloc(sizeof(gfxEntryStruct));
	generatedGfxEntry->imagePtr  = currentStrRenderBuffer;
	generatedGfxEntry->imageSize = stringRenderBufferSize / 2;
	generatedGfxEntry->fontIndex = fontFileIndex;
	generatedGfxEntry->height    = stringHeight;
	generatedGfxEntry->width     = stringWidth;         // maximum render width to the right

	// var_8 = 0;
	heightOffset = 0;

	do
	{
		int       spacesCount      = 0;         // si
		char      character        = *string;
		short int strPixelLength;               // var_16
		uint8*       ptrStringEnd;                 // var_4	//ok
		int       drawPosPixel_X;               // di

        // find first letter in string, skip all spaces
		while(character == ' ')
		{
			spacesCount++;
			character = *(string + spacesCount);
		}

		string += spacesCount;
        
        // returns character count and pixel length (via pointer) per line of the text string
		ptrStringEnd = string + prepareWordRender(inRightBorder_X, wordSpacingWidth, &strPixelLength, fontPtr_Desc, string); //ok

        // determine how much space is left to the right and left (center text)
		if(inRightBorder_X > strPixelLength)
		{
			//var_8 = (inRightBorder_X - strPixelLength) / 2;
			drawPosPixel_X = (inRightBorder_X - strPixelLength) / 2;
		}
		else
		{
			drawPosPixel_X = 0;
		}
		//drawPosPixel_X = var_8;

		// draw textline, character wise
		do
		{
			character = *(string++);

			short int charData = fontCharacterTable[character];     // get character position

			if(character)
			{
				if(character == ' ' || character == 0x7C)
				{
					drawPosPixel_X += wordSpacingWidth + 5;     // if char = "space" adjust word starting postion (don't render space though);
				}
				else
				{
					if(charData >= 0)
					{
						short int* si = (int16*) (fontPtr_Desc + charData * 12);  // offset font data
						// int var_2 = si[5];                                   // don't need this
						
						// should ist be stringRenderBufferSize/2 for the second last param?
						renderWord(fontPtr_Data + si[0], currentStrRenderBuffer, drawPosPixel_X, si[4] - si[3] + lineHeight + heightOffset, si[3], si[2], stringRenderBufferSize, stringWidth / 2, si[5]);

						drawPosPixel_X += wordSpacingWidth + si[5];
					}
				}
			}
			else
			{
				stringFinished = 1;         // character = 0x00
			}
            
            // check if string already reached the end
			if(ptrStringEnd <= string)
			{
				break;
            }
		} while(!stringFinished);

		// var_8 = 0;
		heightOffset += wordSpacingHeight + lineHeight;
	}while(!stringFinished);

	return generatedGfxEntry;
}


} // End of namespace Cruise
