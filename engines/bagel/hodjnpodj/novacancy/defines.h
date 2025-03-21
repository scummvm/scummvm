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
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_NOVACANCY_DEFINES_H
#define HODJNPODJ_NOVACANCY_DEFINES_H

namespace Bagel {
namespace HodjNPodj {
namespace NoVacancy {

#define BACKGROUND_BMP "novac/art/novacsy3.bmp"
#define RULES_TEXT "novac/novac.txt"
#define RULES_WAV "novac/sound/rlnv.wav"

#define LEFT 0
#define RIGHT 1                             

#define FIXED			2     
#define CLOSED	 	1
#define OPEN			 0

#define  toggle(V)  (V=!(V))
#define is_locked(door)		(m_iDoorStatus[door]==FIXED)

#define NUM_DOOR_CELS 5                   //originally 9.
#define NUM_LDIE_CELS 	12					//originally 22
#define NUM_RDIE_CELS	11//9					//originally 17
#define NUM_SINGLE_DIE_CELS			 NUM_LDIE_CELS	
#define BOTTLE_CELS			28
#define HAT4_CELS			10
#define SLEEP_OPENING_TIME			(810/NUM_DOOR_CELS)                            
#define SLEEP_CLOSING_TIME			(740/NUM_DOOR_CELS)                

#define _STRINGTABLE	1                 // use stringtable for retrieving string constants.

#define RULES					"novac/novac.txt"						//	rules text file.                                                                                                      
#define DOOR1_TOP		      116										//	dimensions of doors.
#define DOOR1_LEFT		 		61
#define DOOR1_BOTTOM	 	(DOOR1_TOP+ 143)           //156                                              
#define DOOR1_RIGHT 		(DOOR1_LEFT+ 52) 
#define DOOR1_MIDPOINT_X ((DOOR1_LEFT+DOOR1_RIGHT)>>1)		//	midpoint of door#1.                                                    
#define DOOR1_MIDPOINT_Y ((DOOR1_TOP+DOOR1_BOTTOM)>>1)                                                    

#define DOOR2_TOP		      116
#define DOOR2_LEFT		 		119                                                     
#define DOOR2_BOTTOM	 	(DOOR2_TOP+ 142)                 //155                                        
#define DOOR2_RIGHT 		(DOOR2_LEFT+ 50)      

#define DOOR3_TOP		      114
#define DOOR3_LEFT		 	174                                                          
#define DOOR3_BOTTOM	 (DOOR3_TOP+145)                     //157                                     
#define DOOR3_RIGHT 		(DOOR3_LEFT+ 52)      

#define DOOR4_TOP		      116
#define DOOR4_LEFT		 	232                                                          
#define DOOR4_BOTTOM	 (DOOR4_TOP+137)                     //156                                   
#define DOOR4_RIGHT 		(DOOR4_LEFT+ 53)      

#define DOOR5_TOP		      115
#define DOOR5_LEFT		 		292                                                          
#define DOOR5_BOTTOM	 	(DOOR5_TOP+156)                                                      
#define DOOR5_RIGHT 		(DOOR5_LEFT+ 49)      

#define DOOR6_TOP		      115
#define DOOR6_LEFT		 		347                                                          
#define DOOR6_BOTTOM	 	(DOOR6_TOP+136)                  //157                                    
#define DOOR6_RIGHT 		(DOOR6_LEFT+ 51)     

#define DOOR7_TOP		      114
#define DOOR7_LEFT		 		404                                                          
#define DOOR7_BOTTOM	 (DOOR7_TOP+157  )                                         
#define DOOR7_RIGHT 		(DOOR7_LEFT+ 52)      

#define DOOR8_TOP		      114
#define DOOR8_LEFT		 		464                                                          
#define DOOR8_BOTTOM	 (DOOR8_TOP+ 156)                                        
#define DOOR8_RIGHT 		(DOOR8_LEFT+ 52)      

#define DOOR9_TOP		      114
#define DOOR9_LEFT		 		522                                                          
#define DOOR9_BOTTOM	 (DOOR9_TOP+156 )                                             
#define DOOR9_RIGHT 		(DOOR9_LEFT+ 51)      

#define LDIE_LEFT    233                                                                                  //left hand die
#define LDIE_TOP     342
#define LDIE_RIGHT    LDIE_LEFT+59
#define LDIE_BOTTOM    LDIE_TOP+61
#define LDIE_MIDPOINT_X ((LDIE_LEFT+LDIE_RIGHT)>>1)                  //	midpoint of L-die.
#define LDIE_MIDPOINT_Y ((LDIE_TOP+LDIE_BOTTOM)>>1)

#define SINGLE_DIE_LEFT               LDIE_LEFT                                              //the die in the single die case
#define SINGLE_DIE_TOP                LDIE_TOP                                                
#define SINGLE_DIE_MIDPOINT_X (SINGLE_DIE_LEFT+10)       //	WE ARENT NECESSARILY  LOOKIN FOR midpoint OF die.
#define SINGLE_DIE_MIDPOINT_Y (SINGLE_DIE_TOP+10)        //Any point in the rectangle spanned by the die will suffice.

#define RDIE_LEFT    336                                                                                  //the right hand die
#define RDIE_TOP     345
#define RDIE_RIGHT    RDIE_LEFT+57
#define RDIE_BOTTOM    RDIE_TOP+60                                      

#define DOOR_SPACING  58																//	average horiz. spacing between... 	
#define SPACING 			DOOR_SPACING                                          //	midpoints of doors.                        

#define	PAPER_L				40
#define	PAPER_T             260
#define	PAPER_R             79
#define	PAPER_B             275               

#define	aBRSHOES_L      108
#define	aBRSHOES_T       259
#define	aBRSHOES_R       133
#define	aBRSHOES_B       275

#define	bBRSHOES_L        122
#define	bBRSHOES_T        265
#define	bBRSHOES_R        152
#define	bBRSHOES_B         280

#define	BLSHOES_L           575
#define	BLSHOES_T           260
#define	BLSHOES_R           609
#define	BLSHOES_B            270

#define	BOTTLE_L             169
#define	BOTTLE_T              268
#define	BOTTLE_R              231
#define	BOTTLE_B              279                
#define BOTTLE_SPRITE_L	168
#define BOTTLE_SPRITE_T	200

#define	HAT4_L                   253
#define HAT4_T                   259
#define	HAT4_R                   308
#define	HAT4_B                   304          
#define HAT4_SPRITE_L	 224
#define HAT4_SPRITE_T	  215

#define	HAT6_L                    363
#define HAT6_T                    254
#define	HAT6_R                    389
#define	HAT6_B                    342

#define	STAND_L                 457
#define	STAND_T                 206
#define	STAND_R                 463
#define	STAND_B                 272

#define	GLASS_L                 517
#define	GLASS_T                 259
#define	GLASS_R                 522
#define	GLASS_B                 270

} // namespace NoVacancy
} // namespace HodjNPodj
} // namespace Bagel

#endif
