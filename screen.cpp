// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "bits.h"
#include "screen.h"

#include <string.h>

unsigned short int dataTemp[640 * 480];
unsigned short int dataTemp2[640 * 480];

screenBlockDataStruct screenBlockData[NUM_SCREEN_BLOCK_WIDTH][NUM_SCREEN_BLOCK_HEIGHT];

void screenBlocksReset() {
	for (int i = 0; i < NUM_SCREEN_BLOCK_WIDTH; i++) {
		for (int j = 0; j < NUM_SCREEN_BLOCK_HEIGHT; j++) {
			screenBlockData[i][j].isDirty = false;
		}
	}
}

float getZbufferBlockDepth(char *zbuffer, int x, int y) {
	unsigned short int buffer[SCREEN_BLOCK_WIDTH * SCREEN_BLOCK_HEIGHT];
	char *writePtr = (char *)buffer;
	int i;

	for (i = 0; i < 16; i++) {
		char *readPtr = zbuffer + (y * 16 + i) * 640 + (x * 16);
		for (int j = 0; j < 16; j++) {
			*(writePtr++) = *(readPtr++);
			*(writePtr++) = *(readPtr++);
		}
	}

	unsigned short int bDepth = 0xFFFF;

	for (i = 0; i < SCREEN_BLOCK_SIZE; i++) {
		if (bDepth > buffer[i])
			bDepth = buffer[i];
	}

	return ((float)bDepth / 65535);
}

void screenBlocksInit(char* zbuffer) {
	memcpy(dataTemp, zbuffer, 640 * 480 * 2);
	memcpy(dataTemp2, zbuffer, 640 * 480 * 2);
	uint16 *zbufPtr = reinterpret_cast<uint16 *>(dataTemp);
	for (int y = 0; y < 480 / 2; y++) {
		uint16 *ptr1 = zbufPtr + y * 640;
		uint16 *ptr2 = zbufPtr + (479 - y) * 640;
		for (int x = 0; x < 640; x++, ptr1++, ptr2++) {
			uint16 tmp = *ptr1;
			*ptr1 = *ptr2;
			*ptr2 = tmp;
		}
	}

	for (int i = 0; i < NUM_SCREEN_BLOCK_WIDTH; i++) {
		for(int j = 0; j < NUM_SCREEN_BLOCK_HEIGHT; j++) {
			screenBlockData[i][j].isDirty = false;
			screenBlockData[i][j].depth = getZbufferBlockDepth((char *)dataTemp, i, j);
		}
	}
}

void screenBlocksInitEmpty() {
	for (int i = 0; i < NUM_SCREEN_BLOCK_WIDTH; i++ ) {
		for (int j = 0; j < NUM_SCREEN_BLOCK_HEIGHT; j++ ) {
			screenBlockData[i][j].isDirty = false;
			screenBlockData[i][j].depth = 1.f;
		}
	}
}

void screenBlocksAddRectangle( int top, int right, int left, int bottom, float depth) {
	// clip the rectange to the screen size

	int tempHeight = bottom - top;
	top = 480 - bottom;
	bottom = top + tempHeight;

	if (top < 0)
		top = 0;
	if (top >= SCREEN_HEIGHT)
		top = SCREEN_HEIGHT - 1;

	if (bottom < 0)
		bottom = 0;
	if (bottom >= SCREEN_HEIGHT)
		bottom = SCREEN_HEIGHT - 1;

	if (left < 0)
		left = 0;
	if (left >= SCREEN_WIDTH)
		left = SCREEN_WIDTH - 1;

	if (right < 0)
		right = 0;
	if (right >= SCREEN_WIDTH)
		right = SCREEN_WIDTH - 1;

	// exit in case of bad rectangle
	if ((left > right) || (top > bottom))
		return;

	int firstLeft = left / 16;
	int firstTop = top /16;

	int width = (right - left) / 16;
	if ((right - left) % 16)
		width++;

	int height = (bottom - top) / 16;
	if ((bottom - top) % 16)
		height++;

	// temp hack
	width++;
	height++;

	for (int i = firstLeft; i < firstLeft + width; i++) {
		for (int j = firstTop; j < firstTop + height; j++) {
			if (screenBlockData[i][j].depth < depth)
				screenBlockData[i][j].isDirty = true;
		}
	}
}

void screenBlocksDrawDebug() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glColor4f(1.f, 0.3f, 1.f, 0.4f);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < 40; i++) {
		for (int j = 0;j < 30; j++) {
			if(screenBlockData[i][j].isDirty) {
				glBegin(GL_QUADS);
				glVertex2i(i * 16, j * 16);
				glVertex2i((i + 1) * 16, j * 16);
				glVertex2i((i + 1) * 16, (j + 1) * 16);
				glVertex2i(i * 16, (j + 1) * 16);
				glEnd();
			}
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void screenBlocksBlitDirtyBlocks() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);

	for (int j = 0; j < 30; j++) {
		for (int i = 0; i < 40; i++) {
			if (screenBlockData[i][j].isDirty) {
				int width = 1;
				int start = i++;
				// find the largest possible line
				while ((screenBlockData[i][j].isDirty) && (i < 40)) {
					i++;
					width++;
				}
				glRasterPos2i(start * 16, 479 - (j * 16));
				glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
				glDrawPixels(16 * width, 16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, dataTemp2 + (j * 16 * 640) + (start * 16));
				glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			}
		}
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
	glDepthFunc(GL_LESS);
}
