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

#ifndef TRECISION_SYSDEF_H
#define TRECISION_SYSDEF_H

/******************************************************************************
Game Structure
******************************************************************************/
#define MAXROOMS         		100            // Game rooms
#define MAXOBJ          		1400           // Game objects
#define MAXANIM         		750            // Animations
#define MAXICON          		82             // Inventory icons
#define MAXINVENTORY    		150            // Inventory Items
#define MAXSAMPLE 	 			145            // Maximum number of samples in the game
#define MAXATFDO				30

#define MAXOBJINROOM     		128            // Objects per room
#define MAXSOUNDSINROOM  		15             // Sounds per room
#define MAXACTIONINROOM  		32             // Number of actions per room
#define MAXACTION  				620            // Number of actions in the game
#define MAXACTIONFRAMESINROOM	1200       	   // Number of action frames per room

#define MAXSENTENCE    			4000           // Max Examine phrases
#define MAXOBJNAME  			1400

#define MAXSCRIPTFRAME 			50
#define MAXSCRIPT  				15
#define MAXTEXTSTACK			3

#define MAXDIALOG   			70
#define MAXCHOICE  				1000
#define MAXSUBTITLES  			1500
#define MAXDISPCHOICES			5
#define MAXNEWSMKPAL            40

// define smacker animations
#define MAXCHILD 				4
#define MAXATFRAME	 			16


/******************************************************************************
Null Objects
******************************************************************************/
#define NO_OBJECTS        		0


/******************************************************************************
Screen management
******************************************************************************/
#define MAXX            640
#define MAXY            480

#define AREA  			360
#define TOP         	((MAXY-AREA)/2)             // Coordinate y of game screen
#define VIDEOTOP		TOP
#define ZBUFFERSIZE		200000L
#define SPEECHSIZE		300000L

/******************************************************************************
Low level text and color management
******************************************************************************/
#define CARHEI   		10
#define MAXDTEXTLINES 	20
#define MAXDTEXTCHARS 	128
#define MAXCHARS  		128

#define MAXLENSUBSTRING 128
#define MAXSUBSTRING    16

/******************************************************************************
High level color management
******************************************************************************/
#define COLOR_CHARACTER   HYELLOW
#define COLOR_OBJECT      HWHITE
#define COLOR_INVENTORY   HBLUE

/******************************************************************************
Inventory
******************************************************************************/
#define INV_ON          1               // Active
#define INV_INACTION    2               // In use (the mouse is in the bottom)
#define INV_PAINT       4               // Not yet ON (Going down)
#define INV_DEPAINT     8               // Not yet OFF (Going up)
#define INV_OFF        	16              // Not on screen

#define INVENTORY_HIDE  40 		// ICONDY ...era a 48.
#define INVENTORY_SHOW  0

/******************************************************************************
Inventory icon management
******************************************************************************/
#define FIRSTLINE  	(TOP+AREA)          // First line of the inventory
#define ICONDY      40                  // Icon Height
#define ICONDX      48                  // Icon width
#define ICONSHOWN   12                  // Icons showed simultaneously
#define INVSCROLLSP  5                  // Scrolling speed
#define ICONMARGDX  32                  // Right margin
#define ICONMARGSX  32                  // Left margin

/******************************************************************************
Management of "Use with"
******************************************************************************/
#define USED 0
#define WITH 1

/******************************************************************************
Management of cards for Regen
******************************************************************************/
//#define TYPO_ANI 1
#define TYPO_BMP 2

//uint32 ReadTime();
#define MAXMESSAGE 128


#define DEFAULTACTIONS 19               // Number of default actions


#define DLG_SHOW   0
#define DLG_HIDE   1
#define DLG_SELECT 2

/* MACROS */
#define BETWEEN(a,x,b) ((x>=a) && (x<=b))
#define GAMEAREA(my) BETWEEN(VideoCent.y0,my,VideoCent.y1)
#define isInventoryArea(my)  (my>=VideoBott.y0)
#define ICONAREA(mx,my) ((my>=VideoBott.y0) && (my</*=VideoBott.y0+ICONDY*/MAXY) && (mx>=ICONMARGSX) && (mx<=MAXX-ICONMARGDX))
#define REEVENT doEvent(g_vm->_curMessage->_class, g_vm->_curMessage->_event, g_vm->_curMessage->_priority, g_vm->_curMessage->_u16Param1, g_vm->_curMessage->_u16Param2, g_vm->_curMessage->_u8Param, g_vm->_curMessage->_u32Param)


#define  COLOR_ARG     HWHITE
#define  COLOR_ARG_PT  HYELLOW

#define  dONETIME       1
#define  dPARTISUBITO   2
#define  dACTIVATE      4
#define  dDEACTIVATE    8
#define  dPREVARICA    16

#endif
