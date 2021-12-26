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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRECISION_DEFINES_H
#define TRECISION_DEFINES_H

// OBJECT MODES
#define OBJMODE_FULL    		1
#define OBJMODE_MASK    		2
#define OBJMODE_LIM     		4
#define OBJMODE_OBJSTATUS   	8
#define OBJMODE_HIDDEN  		16

// DEF PER _obj._nbox
#define BOX_FOREGROUND   			255
#define BOX_BACKGROUND   			254
#define BOX_NORMAL					0

enum ObjectFlags {
	kObjFlagUse = 0,
	kObjFlagExamine = 1,
	kObjFlagRoomOut = 2,
	kObjFlagRoomIn = 4,
	kObjFlagPerson = 8,
	kObjFlagTake = 16,
	kObjFlagUseWith = 32,
	kObjFlagExtra = 64,
	kObjFlagDone = 128
};

enum SoundFlags {
	kSoundFlagSoundOn = 1,
	kSoundFlagSoundLoop = 2,
	kSoundFlagBgMusic = 4,
	kSoundFlagStepLeft = 8,
	kSoundFlagStepRight = 16
};

enum AnimationTypes {
	kAnimTypeCharacter = 0,
	kAnimTypeBackground = 1,
	kAnimTypeIcon = 2
};

#define ATFNONE				0
#define ATFTEXT				1
#define ATFSND				2
#define ATFTEXTACT			3
#define ATFDO				4
#define ATFCLR				5
#define ATFSET				6
#define ATFEND				7
#define ATFCEX				8
#define ATFCACT 			9
#define ATFCLRI 			10
#define ATFSETI 			11
#define ATFROOM 			12
#define ATFONESPEAK 		13
#define ATFTEXTEX			14
#define ATFSETPOS			15
#define ATFCOBJANIM			16
#define ATFCOBJBOX			18
#define ATFCOBJPOS			19
#define ATFREADBOX			20
#define ATFSETFORE			21
#define ATFSETBACK			22
#define ATFSWITCH			23
#define ATFDIALOG			24
#define ATFSETROOMT			25
#define ATFSETROOMF			26

// Dialogs
#define DLGCHOICE_EXITNOW   	2
#define DLGCHOICE_ONETIME       4
#define DLGCHOICE_FRAUD    		16
#define DLGCHOICE_EXITDLG		32
#define DLGCHOICE_HIDE		  	64

// REDEFINE
#define MASKCOL  	 		0xEEEE
#define MOUSECOL    		0x7FFF
#define HGREEN     	   		0x03E0
#define HYELLOW     		0x7FE0
#define HWHITE      		0x7777
#define HBLUE				0x001F

// Functions for ATFDO
#define fCLROBJSTATUS 		1
#define fSETOBJSTATUS 		2
#define fANIMOFF1			3
#define fANIMOFF2			4
#define fANIMOFF3			5
#define fANIMOFF4			6
#define fANIMON1			7
#define fANIMON2			8
#define fANIMON3			9
#define fANIMON4			10
#define fSTOP2TXT			11
#define fONETIME 			12
#define fENDDEMO			13
#define fCREPACCIO			14
#define fCHARACTEROFF		15
#define fCHARACTERON		16
#define fSETEXTRA			17
#define fCLREXTRA			18
#define fSERPVIA			19
#define fPIRANHA			20
#define fMOREAU				21
#define fDOOR58			22
#define fHELLEN				23
#define fCHARACTERFOREGROUND	24
#define fCHARACTERBACKGROUND	25
#define fCHARACTERNORM		26
#define fVALVEON34		27
#define fVALVEOFF34		28

/*-------------------------------------------------------------------------*/
/*                                 ROOMS             					   */
/*-------------------------------------------------------------------------*/

enum RoomId {
	// Act 1
	kRoom11 = 1,	// Building (roof)
	kRoom12 = 2,	// Building (elevator)
	kRoom13 = 3,	// Ruby's apartment (outside)
	kRoom14 = 4,	// Ruby's apartment (kitchen / living room)
	kRoom15 = 5,	// Ruby's apartment (bedroom)
	kRoom16 = 6,	// Subway (building)
	kRoom17 = 7,	// Subway (Rotmall 17)
	kRoom18 = 8,	// Free Climax club (square, outside)
	kRoom19 = 9,	// Free Climax club (inside)
	kRoom1A = 10,	// Liquor store
	kRoom1B = 11,	// Alley (between the Liquor store and Free Climax)
	kRoom1C = 12,	// Underground tunnel (below the alley)
	kRoom1D = 13,	// Store room (below the liquor store)
	// Act 2
	kRoom21 = 14,	// Platforms (subway platform)
	kRoom22 = 15,	// Platforms (maintenance room, subway platform right door)
	kRoom23A = 16,	// Platforms (passageway with magnetic field)
	kRoom23B = 82,	// Platforms (passageway without magnetic field) - alt room
	kRoom24 = 17,	// Deserted platform (subway platform outside the zoo)
	kRoom25 = 18,	// Deserted platform (info booth, shed with trapdoor)
	kRoom26 = 19,	// Zoo (entrance)
	kRoom27 = 20,	// Zoo (bar)
	kRoom28 = 21,	// Zoo (goddess)
	kRoom29 = 22,	// Platforms (storeroom inside the maintenance room, lights off)
	kRoom29L = 83, 	// Platforms (storeroom inside the maintenance room, lights on) - alt room
	kRoom2A = 23,	// Platforms (underground passageway below the storeroom)
	kRoom2B = 24,	// Platforms (bookstore)
	kRoom2C = 25,	// Zoo (sphinx)
	kRoom2D = 26,	// Zoo (control room below the sphinx)
	kRoom2E = 27,	// Zoo (fissures with bridge, outside cave)
	kRoom2F = 28,	// Zoo (cave, after fissures)
	kRoom2G = 29,	// Zoo (pond)
	kRoom2GV = 85,	// Zoo (pond, after water is drained through the sluice gate) - alt room
	kRoom2H = 30,	// Deserted platform (on the bridge)
	// Act 3
	kRoom31 = 31,	// Warehouse
	kRoom32 = 32,	// Warehouse walkway (above)
	kRoom33 = 33,	// Warehouse (roof)
	kRoom34 = 34,	// Warehouse (machinery room)
	kRoom35 = 35,	// Warehouse (hideout)
	kRoom36 = 36,	// Warehouse (hideout upstairs)
	kRoom37 = 37,	// Warehouse (medical room)
	// Act 4
	kRoom41 = 38,	// Amusement park (entrance)
	kRoom42 = 39,	// Amusement park (shooting range)
	kRoom43 = 40,	// Amusement park (cable car and hut)
	kRoom44 = 41,	// Amusement park (inside hut)
	kRoom45 = 42,	// Amusement park (boiler room below hut)
	kRoom45S = 90,	// Amusement park (boiler room below hut, after shutting down the boiler) - alt room
	kRoom46 = 43,	// Castle (courtyard)
	kRoom47 = 44,	// Castle (church)
	kRoom48 = 45,	// Castle (alchemist's study)
	kRoom49 = 46,	// Castle (tower)
	kRoom4A = 47,	// Hugh's office (entrance/lobby with secretary)
	kRoom4B = 48,	// Hugh's office (meeting room)
	kRoom4C = 49,	// Hugh's office (desk)
	kRoom4D = 50,	// Labyrinth (entrance)
	kRoom4E = 51,	// Labyrinth
	kRoom4F = 52,	// Labyrinth
	kRoom4G = 53,	// Labyrinth (first intersection)
	kRoom4H = 54,	// Labyrinth
	kRoom4I = 55,	// Labyrinth
	kRoom4J = 56,	// Labyrinth
	kRoom4K = 57,	// Labyrinth (dead end)
	kRoom4L = 58,	// Labyrinth (water font)
	kRoom4M = 59,	// Labyrinth (corridor with painting)
	kRoom4N = 60,	// Labyrinth (painting with chandelier)
	kRoom4O = 61,	// Labyrinth
	kRoom4P = 62,	// Labyrinth (werewolf)
	kRoom4Q = 63,	// Labyrinth (chandeliers)
	kRoom4R = 64,	// Labyrinth (skeleton)
	kRoom4S = 65,	// Labyrinth
	kRoom4T = 66,	// Labyrinth (outside lava room)
	kRoom4U = 67,	// Labyrinth (lava room)
	kRoom4V = 68,	// Labyrinth
	kRoom4W = 69,	// Labyrinth
	kRoom4X = 70,	// Labyrinth (dead end)
	// Act 5
	kRoom51 = 71,	// Rocas Perdida (beach)
	kRoom52 = 72,	// Rocas Perdida (fence)
	kRoom53 = 73,	// Rocas Perdida (conduit with grating)
	kRoom54 = 74,	// Prison (laundry room)
	kRoom55 = 75,	// Prison (corridor outside detention area)
	kRoom56 = 76,	// Prison (sentry post)
	kRoom57 = 77,	// Prison (infirmary)
	kRoom58 = 78,	// Prison (detention area)
	kRoom59 = 79,	// Prison (laboratory)
	kRoom5A = 80,	// Prison (looking outside the window)
	// Item closeups
	kRoom2BL = 84,	// Inventory (Egyptian gods book closeup)
	kRoom36F = 88,  // Inventory (warehouse hideout upstairs, paper behind the painting closeup)
	kRoom31P = 86,	// Warehouse (alarm panel closeup)
	kRoom35P = 87,	// Warehouse (hideout security system panel closeup)
	kRoom41D = 89,	// Positioner interface closeup
	kRoom49M = 91,	// Castle (tower, Meridian circle closeup)
	kRoom4CT = 92,	// Hugh's office (terminal lock screen closeup)
	kRoom58M = 93,	// Prison (detention area, Eva's map closeup)
	kRoom58T = 94,	// Prison (detention area, Governor's office lock closeup)
	kRoom59L = 95,	// Prison (laboratory, blackboard closeup)
	kRoom12CU = 97, // Building (elevator button closeup)
	kRoom13CU = 98, // Magnetic reader closeup outside Ruby's apartment
	// Special rooms
	kRoomIntro = 81,
	kRoomControlPanel = 96
};

enum SentenceId {
	kSentenceMapZoo = 334,	// According to the map, the zoo should be along the passageway, ...
	kSentenceGoodDeterrent = 621,	// Damn! Mechanical or not, that's a pretty good deterrent
	kSentenceHopeDidntWasteTheKid = 659,	// Damn! I hope I didn't waste the kid
	kSentenceItWorked = 1502,	// Jeez, it worked
	kSentenceWastedCritter = 1561,	// OK I wasted the critter but I used all the charges in the gun
	kSentenceOnlyGotOne = 1565,	// I'd better not. I've only got one, and I don't want to waste it here
	kSentenceItDidntWork = 1707,	// Damn! It didn't work
	kSentenceTakeThatWolfman = 1708,	// Take that wolfman! I kicked his virtual ass
	kSentenceTheSpiderHasEscaped = 1712,	// The spider has escaped
	kSentenceSecretPassage = 1997,	// Interesting, there's a secret passage in the warehouse
	kSentencePutHimOutOfAction = 2003	// I've got to put him out of action first
};

/*-------------------------------------------------------------------------*/
/*                                 OBJECTS           					   */
/*-------------------------------------------------------------------------*/
#define oBUILDING11				1
#define oRAILING11				2
#define oBAR11			   		3
#define oBARC11			   		4
#define oDOOR11			 		5
#define oCARASHADOW11			6
#define oCARA11					7
#define oCARB11					8
#define oTINFOIL11				9
#define oRINGMASKSX	  			10
#define oRINGMASKCX				11
#define oRINGMASKDX				12
#define oLIGHTS11				13
#define oTAKE12					14
#define oASCENSOREC12			15
#define oPANNELLOC12			16
#define oPANELA12				17
#define oFUSIBILE12				18
#define oLIFTA12				19
#define oSLOT12					20
#define oBOXES12				21
#define oBOX12					22
#define oPORTAL12				23
#define oMAPPA12				24
#define oPIANTA12				25
#define oCARTELLO12				26
#define oPANNELLO13				27
#define oASCENSOREA13			28
#define oSLOT13					29
#define oDOORC13				30
#define oDOORD13				31
#define oDOORB13				32
#define oPENPADA13				33
#define oPENPADB13				34
#define oSCAVO13				35
#define oDOORA13				36
#define oLETTERA13				37
#define oDISTRIBUTORE13			38
#define oPORTASIMONMASK			39
#define oFESSURA13				40
#define oSCOMPARTO13			41
#define oLATTINA13				42
#define oMONETA13				43
#define oASCENSOREC13			44
#define oMASKLEFT13				45
#define oMASKSCAVO13			46
#define oMASKRIGHT13			47
#define oCUCININO14				48
#define oSCAFFALE14				49
#define oCESTINO14				50
#define oTELEVISORES14			51
#define oTELEVISOREA14			52
#define oLAMPADA14				53
#define od14TO15				54
#define oACQUARIO14				55
#define oDOORL14				56
#define oDOORR14				57
#define oMASKTAVOLOFRIGO		58
#define oFRIGORIFEROC14			59
#define oTELEFAX14				60
#define oDIVANO14				61
#define oTAVOLINOD14			62
#define oFRIGORIFEROA14			63
#define oFRIGORIFEROV14			64
#define oFOGLIETTO14			65
#define oPORTAFOTOF14			66
#define oPORTAFOTOV14			67
#define oMASKPIANTA14			68
#define oMASKLEFT14				69
#define oMASKP1415				70
#define oTAVOLINOPP14			71
#define oMASKSEDIA14			72
#define oCASSAFORTEA15			73
#define oSTRONGBOXC15			74
#define oVESTITI15				75
#define oARMADIO15				76
#define oTASTOA15				77
#define oTASTOB15				78
#define od15TO14				79
#define oBAGNO15				80
#define oBOCCETTE15				81
#define oSPECCHIO15				82
#define oMENSOLA15				83
#define oPANNELLO15				84
#define oWINDOWA15			85
#define oTAPPARELLAA15			86
#define oNASTRO15				87
#define oMASKWINDOW15			88
#define oWINDOWC15			89
#define oCASSETTI15				90
#define oLETTO15				91
#define oQUADRO15				92
#define oPIANTA15				93
#define oMASKDOOR15			94
#define oMASKSEDIA15			95
#define oMASKTAVOLO15			96
#define oBICCHIERE15			97
#define oASCENSOREA16			98
#define oSLOT16					99
#define oPANNELLO16				100
#define oMONITORSA16			101
#define oSEGGIOLE16				102
#define oSCALA16				103
#define ocPOLIZIOTTO16			104
#define oTICKETOFFICE16			105
#define oFINGERPADB16			106
#define oMAPPA16				107
#define oDOORA16				108
#define oDOORU16				109
#define oASCENSOREC16			110
#define oMASKDOORA16			111
#define oFINGERPADP16			112
#define oSBARRAP16				113
#define oMONITORSP16			114
#define oCARTACCE16				115
#define oCESTINOG16				116
#define oCESTINOS16				117
#define oMASKSEDIADX16			118
#define oRETE17					119
#define oDOORA17				120
#define oDOORUA17				121
#define oDOORUB17				122
#define oFINGERPAD17			123
#define oMURALES17				124
#define oTELEFAXF17				125
#define oTELEFONO17				126
#define oSCATOLONE17			127
#define oTELEFAXR17				128
#define oTRAMPD17				129
#define ocTRAMP17				130
#define oSCALA17				131
#define oFAX17					132
#define oNUMERO17				133
#define oPLASTICA17				134
#define oMASKTRANS17			135
#define oMASKCOLONNA17			136
#define oSCHERMO18				137
#define oCANCELLATA18			138
#define od18TO17				139
#define oDOOR18					140
#define oMONITORS18				141
#define ocGUARD18				142
#define oDOORC18				143
#define oDOORN18				144
#define oMASKPALO18				145
#define oMASKMETRO18			146
#define od19TO18				147
#define oMASK19TO18			148
#define oPOSTER19				149
#define oTAVOLINO19				150
#define oBAR19					151
#define ocEVA19					152
#define oMASKTAVOLOEVA19		153
#define oMASKTAVOLODX19			154
#define oMASKSCALA19			155
#define oVETRINA1A				156
#define od1ATO18				157
#define oDOOR1A				158
#define oTROFEI1A				159
#define oBOTTIGLIE1A			160
#define ocNEGOZIANTE1A			161
#define oBANCONE1A				162
#define oBOTTE1A				163
#define oDISTILLATRICE1A		164
#define oTESSERA1A				165
#define oMASK11A				166
#define oMASK21A				167
#define oMASK31A				168
#define oPALIZZATA1B			169
#define oBIDONE1B				170
#define oBUCO1B					171
#define oASSE1B					172
#define oASSEROTTO1B			173
#define oTOMBINOA1B				174
#define oMANHOLEC1B				175
#define oCOPERCHIO1B			176
#define oMASKBIDONE1B			177
#define oMASKTUBO1B				178
#define oBOTOLAA1B				179
#define oCARTACCE1B				180
#define oBOTOLAC1B				181
#define oPADLOCK1B				182
#define oLUCCHETTOFUSO1B		183
#define oMASKCASSE1B			184
#define oCANCELLATA1B			185
#define oMASKCANC11B			186
#define oMASKCANC21B			187
#define oSCALETTA1C				188
#define oLAMPADA1C				189
#define oTUBOS1C				190
#define oTUBOP1C				191
#define oGRATA1C				192
#define oBOTOLAGIU1C			193
#define oASSEOK1C				194
#define oLUCCHETTOFUSO1C		195
#define oLUCCHETTOOK1C			196
#define oASSEROTTO1C			197
#define oTOPO1C					198
#define oMASKMURO1D				199
#define oSCAFFALE1D				200
#define oSCALA1D				201
#define oDOOR1D				202
#define oSGABELLO1D				203
#define oDONNA1D				204
#define oMASKCOLONNA21D			205
#define oCASSA1D				206
#define oTANICHE1D				207
#define oMASKCOLONNA11D			208
#define oCASSETTAA1D			209
#define oBOTTIGLIA1D			210
#define oCASSETTAC1D			211
#define oBOTTI1D				212
#define oBARILOTTO1D			213
#define oMASKBOTTIGLIE1D		214
#define oLASTLEV1				215
#define od21TO23				216
#define oEXIT21				217
#define oBINARIO21				218
#define oTUBO21					219
#define oCHAIN21				220
#define oCUNICLE21				221
#define od21TO22				222
#define om21TO22				223
#define oDOORA21				224
#define oDOORC21				225
#define oCATENA21				226
#define oRAMPINO21				227
#define oBILLBOARD21			228
#define oESSE21					229
#define od22TO21				230
#define omDOOR22				231
#define oLAVANDINOS22			232
#define oLAVANDINOA22			233
#define oARMADIETTOLA22			234
#define omARMADIETTOCA22		235
#define oARMADIETTOCA22			236
#define oBORSA22				237
#define omARMADIETTOCC22		238
#define oARMADIETTOCC22			239
#define oPOSTERC22				240
#define oPORTELLOC22			241
#define oMANIGLIONE22			242
#define oMANIGLIONEO22			243
#define od22TO29				244
#define om22TO29				245
#define oBOTOLAA22				246
#define od22TO29I				247
#define omARMADIETTORA22		248
#define oARMADIETTORA22			249
#define oKEY22					250
#define oARMADIETTORC22			251
#define oPOSTERR22				252
#define oPANCHINE22				253
#define om2PANCHINE22			254
#define om3PANCHINE22			255
#define omBARILI22				256
#define oCAMPO23				257
#define od23TO21				258
#define oSLOT23					259
#define oSCATOLETTA23			260
#define oCONTATTI23				261
#define oLEVAS23				262
#define oLEVAG23				263
#define oCAVIE23				264
#define oCAVI23					265
#define oPANNELLOA23			266
#define oPANELC23				267
#define od23TO24				268
#define oPANNELLOB23			269
#define oCAVIB23				270
#define od23BTO21				271
#define od24TO23				272
#define om24TO23				273
#define od24TO26				274
#define om24TO26				275
#define od24TO25				276
#define oPASSERELLA24			277
#define oBINARIO24				278
#define oRUINS24				279
#define oBIDONE24				280
#define oVIDEOTELEFONO24		281
#define oINSEGNA24				282
#define oCARTELLONE24			283
#define oPASSAGE24				284
#define omPASSAGGIO24			285
#define omSCALASX24				286
#define omSCALADX24				287
#define oDUMMY24A				288
#define oDUMMY24				289
#define oBOTOLAA25				290
#define oSCAFFALE25				291
#define oTRONCHESE25			292
#define omSTANZINO25			293
#define omPORTELLO25			294
#define oPORTELLO25				295
#define oBOTOLAC25				296
#define oSEDILIL25				297
#define oINFOPOINT25			298
#define oTELEFONO25				299
#define oCOCCI25				300
#define oMAGNETE25				301
#define oBIDONE25				302
#define oCOCCI25B				303
#define oALTOPARLANTE25			304
#define oSEDILIR25				305
#define od25TO24				306
#define od26TO24				307
#define oDOOR26				308
#define oWINDOW26				309
#define oINSEGNA26				310
#define oCARTELLOR26			311
#define oPALMA26				312
#define od26TO2G				313
#define od26TO27				314
#define od26TO2C				315
#define omPIANTER26				316
#define oRIPOSTIGLIO27			317
#define oBOMBOLA27				318
#define oSCOPA27				319
#define oTENDA27				320
#define oBANCONE27				321
#define oTANICHETTA27			322
#define od27TO26				323
#define om27TO26				324
#define omBOTTIGLIA27			325
#define omSEDIAG27				326
#define oTELEVISORE27			327
#define oCACTUS27				328
#define od27TO28				329
#define om27TO28				330
#define oFOGLIO27				331
#define oLATTINA27				332
#define oCOMPUTER27				333
#define oTAVOLINO27				334
#define omTAVOLINOR27			335
#define omSEDIAR27				336
#define omSEDIAL27				337
#define oSTATUA28				338
#define oCESPUGLIOS28			339
#define oSERPENTEC28			340
#define oRUBINETTO28			341
#define oSERPENTECM28			342
#define oRUBINETTOC28			343
#define oCESPUGLIO28			344
#define oBRACIEREA28			345
#define oBRACIERES28			346
#define oTEMPIO28				347
#define oSERPENTET28			348
#define oSERPENTETM28			349
#define oSERPENTEA28			350
#define oSERPENTEAM28			351
#define omROCCIA28				352
#define oCAMPO28				353
#define oCARTELLOS28			354
#define oCARTELLOA28			355
#define od28TO27				356
#define omSASSOR28				357
#define od28TO2B				358
#define omPIANTAL28				359
#define od29TO22				360
#define oSWITCH29			361
#define oBOTOLAC29				362
#define oBOTOLAA29				363
#define oPORTALAMPADE29			364
#define oLAMPADINAF29			365
#define oLAMPADINAS29			366
#define omA29					367
#define omB29					368
#define omC29					369
#define od29LTO22				370
#define oTUBI29L				371
#define oMANOPOLA29L			372
#define oMANOPOLAG29L			373
#define oBOTOLAA29L				374
#define oSCAFFALE29L			375
#define oBARILE29L				376
#define oTRIPLA29L				377
#define oLAVANDINO29L			378
#define oLAMPADINAS29L			379
#define oBARATTOLI29L			380
#define omA29L					381
#define omB29L					382
#define omC29L					383
#define oDOOR2A					384
#define od2ATO2B				385
#define oSCATOLA2A				386
#define oTUBI2A					387
#define od2ATO29				388
#define oSBARRE2A				389
#define od2ATO25				390
#define omL2A					391
#define omR2A					392
#define oDUMMY2A2				393
#define oDUMMY2A				394
#define oPOZZO2B				395
#define oLIBRILE2B				396
#define oLIBRISE2B				397
#define oLIBRIPI2B				398
#define oLIBRIKA2B				399
#define oLIBRIEG2B				400
#define oLIBRO2B				401
#define oLIBRIRE2B				402
#define oLIBRIDI2B				403
#define oLIBRIUO2B				404
#define oLIBRIUC2B				405
#define oDINOSAURO2B			406
#define oLEOPARDO2B				407
#define omMAPPAMONDO2B			408
#define oMAPPAMONDO2B			409
#define oBACHECA2B				410
#define oSTATUA2B				411
#define od2BTO28				412
#define omDOOR2B				413
#define oDOOR2B				414
#define oCARTELLO2B				415
#define oPELLICANO2B			416
#define oSERPENTE2B				417
#define omORSO2B				418
#define oSCRIVANIA2B			419
#define oPORTALAMPADE2B			420
#define omLAMPADINA2B			421
#define oEXIT2BL				422	// Exit from Egyptology book
#define oUEPOIS					423
#define oHATOR					424
#define oNUT					425
#define oHORUS					426
#define oBASTET					427
#define oBES					428
#define oDEADELSICOMORO			429
#define oASTARTE				430
#define oKHEPHERI				431
#define oKADESH					432
#define oRA						433
#define oISIDE					434
#define oCAMPO2C				435
#define oTEMPIO2C				436
#define oLEONE2C				437
#define od2CTO2D				438
#define oDOORC2C				439
#define oSFINGE2C				440
#define omZAMPA2C				441
#define oSTATUA2C				442
#define oWHEELS2C				443
#define od2CTO2E				444
#define oCARTELLOS2C			445
#define oCARTELLOA2C			446
#define od2CTO26				447
#define oBASEWHEELS2C			448
#define oWHEEL1A2C				449
#define oWHEEL1B2C				450
#define oWHEEL1C2C				451
#define oWHEEL2A2C				452
#define oWHEEL2B2C				453
#define oWHEEL2C2C				454
#define oWHEEL3A2C				455
#define oWHEEL3B2C				456
#define oWHEEL3C2C				457
#define oWHEEL4A2C				458
#define oWHEEL4B2C				459
#define oWHEEL4C2C				460
#define oWHEEL5A2C				461
#define oWHEEL5B2C				462
#define oWHEEL5C2C				463
#define oWHEEL6A2C				464
#define oWHEEL6B2C				465
#define oWHEEL6C2C				466
#define oWHEEL7A2C				467
#define oWHEEL7B2C				468
#define oWHEEL7C2C				469
#define oWHEEL8A2C				470
#define oWHEEL8B2C				471
#define oWHEEL8C2C				472
#define oWHEEL9A2C				473
#define oWHEEL9B2C				474
#define oWHEEL9C2C				475
#define oWHEEL10A2C				476
#define oWHEEL10B2C				477
#define oWHEEL10C2C				478
#define oWHEEL11A2C				479
#define oWHEEL11B2C				480
#define oWHEEL11C2C				481
#define oWHEEL12A2C				482
#define oWHEEL12B2C				483
#define oWHEEL12C2C				484
#define oWHEELA2C				485
#define oWHEELB2C				486
#define oWHEELC2C				487
#define omWHEELS2C				488
#define oPULSANTE2C				489
#define od2DTO2C				490
#define oPANNELLOA2D			491
#define oPANNELLOB2D			492
#define oFOGLIO2D				493
#define oPANNELLOC2D			494
#define oPANNELLOD2D			495
#define omDOOR2D				496
#define omTAVOLO2D				497
#define oFUCILE2D				498
#define oGIACCA2D				499
#define oROBOT2D				500
#define omTAV2D					501
#define oPULSANTEAA2D			502
#define oPULSANTEBA2D			503
#define oPULSANTEABA2D			504
#define oPULSANTEABS2D			505
#define oPULSANTEBBS2D			506
#define oPULSANTEBBA2D			507
#define oPULSANTEACA2D			508
#define oPULSANTEACS2D			509
#define oPULSANTEBC2D			510
#define oPULSANTEADA2D			511
#define oPULSANTEADS2D			512
#define oPULSANTEBD2D			513
#define omFILIA2D				514
#define omFILIB2D				515
#define omFILIC2D				516
#define oENTRANCE2E				517
#define oCATWALKB2E			518
#define omPASSERELLAB2E			519
#define oCRACK2E			520
#define oCRATER2E				521
#define oSHRUBS2E				522
#define oDINOSAURO2E			523
#define oCARTELLOS2E			524
#define oCARTELLOA2E			525
#define od2ETO2C				526
#define oDUMMY2E				527
#define oCATWALKA2E				528
#define oCAMPO2E				529
#define od2FTO2E				530
#define omGABBIAA2F				531
#define oGABBIAA2F				532
#define oGABBIAC2F				533
#define oTASTO2F				534
#define oASCENSOREC2F			535
#define omASCENSORE2F			536
#define oASCENSORE2F			537
#define oBIDONE2F				538
#define oTAVOLO2F				539
#define oCANDELOTTO2F			540
#define oPLASTICA2F				541
#define oROBOT2F				542
#define oCHIUSAC2G				543
#define oMACCHINARIO2G			544
#define oPANELM2G			545
#define oSERBATOIOC2G			546
#define oCOPERCHIO2G			547
#define oSERBATOIOA2G			548
#define oCARTELLO2G				549
#define oSTAGNO2G				550
#define oPANNELLOE2G			551
#define oRAGAZZOP2G				552
#define oRAGAZZOS2G				553
#define oCOCCODRILLO2G			554
#define oTUBO2G					555
#define od2GTO26				556
#define omPIANTEA2G				557
#define omPIANTEB2G				558
#define oPANNELLOE2GV			559
#define oGIRLS2GV			560
#define oCHIUSAA2GV				561
#define oSERBATOIOA2GV			562
#define oCARTELLO2GV			563
#define oVIADUCT2GV			564
#define oCROCODILE2GV			565
#define od2GVTO26 566
#define omPIANTEA2GV			567
#define omPIANTEB2GV			568
#define oDUMMY2GV				569
#define oMONITORS2H				570
#define oSCALAS2H				571
#define oCARTELLONE2H			572
#define oCAVO2H					573
#define oSCALAR2H				574
#define omRING2H				575
#define oPORTONE31				576
#define oPINZAB31				577
#define oMONTACARICHI31			578
#define oKEYBOARDB31			579
#define od31TO35				580
#define omd31TO35				581
#define oPANNELLO31				582
#define oPANNELLON31			583
#define oPINZAA31				584
#define oASCENSORE31			585
#define oAUTO31					586
#define oKEYBOARDA31			587
#define oCASSA31				588
#define oCASSEL31				589
#define oPANNELLOMA31			590
#define oPANNELLOM31			591
#define oSIGARO31				592
#define oCORPO31				593
#define omCASSED31				594
#define omCASSEC31				595
#define omCOLONNAB31			596
#define omCASSEB31				597
#define omCASSEA31				598
#define omCOLONNAA31			599
#define oCASSER31				600
#define oFILOTC31				601
#define oFILOS31				602
#define oFILOT31				603
#define oCONTATTOP31			604
#define oTERRA31				605
#define oFILON31				606
#define oFILOP31				607
#define oCOPERCHIOA31			608
#define oCOPERCHIOC31			609
#define oINTERRUTTORE31			610
#define oMONTACARICHI32			611
#define omMONTACARICHI32		612
#define oBOTOLAA32				613
#define oBOTOLAB32				614
#define oBOTOLAC32				615
#define oSENSOREV32				616
#define oPULSANTI32				617
#define oSENSOREL32				618
#define omRINGHIERA32			619
#define oSCALA32				620
#define oCOSTRUZIONE33			621
#define oWINDOW33				622
#define oTUBOF33				623
#define oRUBINETTO33			624
#define oTUBOR33				625
#define omTUBOR33				626
#define oTUBOP33				627
#define omTUBOP33				628
#define oTUBET33				629
#define omTUBOT33				630
#define oTUBOS33				631
#define omTUBOS33				632
#define od33TO34				633
#define oDOORC33				634
#define omDOOR33				635
#define oBOTOLAA33				636
#define oBRUCIATURA33			637
#define oBOTOLAC33				638
#define oSERRATURA33			639
#define oSAMA33					640
#define oSERRATURAF33			641
#define oSAMD33					642
#define od34TO33				643
#define omDOOR34				644
#define oFOGLI34				645
#define oBOMBOLAI34				646
#define oBOMBOLAO34				647
#define oWINDOW34				648
#define oMISCELATORE34			649
#define oCONDOTTO34				650
#define oVALVOLAC34				651
#define oVALVOLA34				652
#define omVALVOLA34				653
#define oMANOMETROC34			654
#define oMANOMETRO34			655
#define oTUBOA34				656
#define oTUBOT34				657
#define oTUBOF34				658
#define oTUBOFT34				659
#define oGENERATORE34			660
#define od35TO31				661
#define oSCAFFALE35				662
#define oFRONTOFFICEC35			663
#define oFRONTOFFICEA35			664
#define omASCENSORE35			665
#define oASCENSORE35			666
#define oMONITOR35				667
#define oSEDIA35				668
#define oOSEDIA35				669
#define oRIBELLEA35				670
#define oCOMPUTER35				671
#define omPIANO35				672
#define oGIORNALE35				673
#define omUSCITA35				674
#define oPRESA35				675
#define oTRIPLA35				676
#define oPROIETTORE35			677
#define oRIBELLES35				678
#define oDOORC35				679
#define omDOORC35				680
#define oDOORA35				681
#define omDOORA35				682
#define oCASSE35				683
#define oCASSE35B				684
#define omCASSE35				685
#define oPULSANTIV35			686
#define oLEDS35					687
#define oPULSANTEA35			688
#define oPULSANTEB35			689
#define oPULSANTEC35			690
#define oPULSANTED35			691
#define oPULSANTEE35			692
#define oPULSANTEF35			693
#define oPULSANTEG35			694
#define oPULSANTEAA35			695
#define oPULSANTEBA35			696
#define oPULSANTECA35			697
#define oPULSANTEDA35			698
#define oPULSANTEEA35			699
#define oPULSANTEFA35			700
#define oPULSANTEGA35			701
#define oCARD35					702
#define oROOM36					703
#define oSCANNERLA36			704
#define oSCANNERLS36			705
#define oPORTALC36				706
#define oSPIONCINOL36			707
#define oQUADRO36				708
#define oSCANNERMA36			709
#define oSCANNERMS36			710
#define od36TO37				711
#define oDOORMC36				712
#define oSPIONCINOM36			713
#define omDOOR36				714
#define oSCAFFALE36				715
#define od36TO35				716
#define om36TO35				717
#define omSCRIVANIA36           718
#define oCOMPUTER36				719
#define oCASSETTOA36			720
#define oCASSETTOC36			721
#define omSEDIA36				722
#define oSCATOLONE36			723
#define omCARTACCE36			724
#define omSCATOLONEA36			725
#define oEXIT36F				726	// Exit from security system sequence
#define oSIMON37				727
#define oSPORTELLOA37			728
#define oSPORTELLOI37			729
#define oSPORTELLOC37			730
#define oPANNELLOA37			731
#define oPARATIA37				732
#define oPANNELLO37				733
#define oCAMICE37				734
#define od37TO36				735
#define omDOOR37				736
#define oARMADIETTO37			737
#define oLETTINO37				738
#define omTAVOLINO37			739
#define omPANCA37				740
#define od41TO4D				741
#define oRAGNO41				742
#define oSLOT41					743
#define oC1A41					744
#define oC2A41					745
#define oC3A41					746
#define oC1B41					747
#define oC2B41					748
#define oC1C41					749
#define oC2C41					750
#define oFESSURA41				751
#define oSCODELLA41				752
#define oMONETA41				753
#define oMONETE41				754
#define oZAMPAS41				755
#define oZAMPA41				756
#define oLEVASLOT41				757
#define od41TO42				758
#define od41TO43				759
#define oCARRETTO42				760
#define oFESSURA42				761
#define oCARTELLOF42			762
#define oPUPAZZO42				763
#define oVETRINETTA42			764
#define oCARTELLOV42			765
#define oFUCILE42				766
#define oGUIDE42				767
#define oPOLTIGLIA42			768
#define od42TO41				769
#define od42TO43				770
#define oCASTELLO43				771
#define oGABBIA43				772
#define oTOTEM43				773
#define oPESO43					774
#define oCAMPANAT43				775
#define oTAMBURO43				776
#define oMARTELLOT43			777
#define oMARTELLOR43			778
#define oCAMPANAC43				779
#define oMANICO43				780
#define omDOOR43				781
#define od43TO44				782
#define oDOORC43				783
#define od43TO41				784
#define od43TO42				785
#define om44TO43				786
#define od44TO43				787
#define oROTORE44				788
#define oBIGLIETTERIA44			789
#define oPULSANTE44				790
#define oCASSETTOAC44			791
#define oCASSETTOAA44			792
#define oPUPAZZO44				793
#define oCASSETTOBC44			794
#define oCASSETTOBA44			795
#define omSCALA44				796
#define od44TO45				797
#define oTROFEO44				798
#define oQUADRO44				799
#define omFIGURE44				800
#define od45TO44				801
#define oCALDAIAA45				802
#define oTUBOR45				803
#define oRAGNATELA45			804
#define oRAGNO45				805
#define oMANOPOLAR45			806
#define oSFIATO45				807
#define oMANOMETROA45			808
#define oSALNITRO45				809
#define oBOILERS45				810
#define oTUBOF45				811
#define oMANOMETROS45			812
#define oMANOPOLAF45			813
#define oPOLTIGLIA45			814
#define od45BTO44 815
#define oBARILOTTI46			816
#define od46TO47 817
#define omDOORB46				818
#define od46TO49				819
#define omDOORC46				820
#define omSASSO46				821
#define oCARRO46				822
#define oPOZZO46				823
#define oENTRATA46				824
#define oSPIDER46				825
#define od46TO48				826
#define omDOORA46				827
#define omALBERO46				828
#define omPIETRA46				829
#define od46TO43				830
#define omBASTIONE46			831
#define oMACERIE46				832
#define omMACERIE46				833
#define omCAPANNA46				834
#define od47TO46				835
#define oTORCIA47				836
#define oSTATUAA47				837
#define oSTATUAB47				838
#define oBRACIEREA47			839
#define oCARBONE47				840
#define oBRACIEREC47			841
#define omBRACIERE47			842
#define oPENTACOLO47			843
#define oTESCHIO47				844
#define omTESCHIO47				845
#define oALTARE47				846
#define oLIBRO48				847
#define oASTA48					848
#define oTILE48			849
#define oTESCHIOROTTO48			850
#define oSKULL48				851
#define oTORCHS48				852
#define oTORCIAA48				853
#define oCONCAROVENTE48			854
#define oSPECCHIOA48			855
#define oSPECCHIO48				856
#define oACQUA48				857
#define oACQUAS48				858
#define oCONCA48				859
#define oWINDOW48				860
#define oSFERA48				861
#define oPLANETARIO48			862
#define oPERGAMENAS48			863
#define oPERGAMENAP48			864
#define od48TO46				865
#define oSCAFFALE48				866
#define oSACCHETTO48			867
#define oBARATTOLO48			868
#define omFRANTOIO48			869
#define omCAFFETTIERA48			870
#define od49TO46				871
#define oCANNOCCHIALE49			872
#define oOMBRAS49				873
#define oSCOMPARTO49			874
#define oAGENDA49				875
#define oSUNDIAL49			876
#define omPIETRA49				877
#define omMUROA49				878
#define oBARILI49				879
#define omMUROB49				880
#define oFOROC49				881
#define oFORO849				882
#define oFORO949				883
#define oFORO1049				884
#define oFORO1149				885
#define oFORO1249				886
#define oFORO149				887
#define oFORO249				888
#define oFORO349				889
#define oFORO449				890
#define oFORO549				891
#define oFORO649				892
#define oFORO749				893
#define oASTAC49				894
#define oASTA849				895
#define oASTA949				896
#define oASTA1049				897
#define oASTA1149				898
#define oASTA1249				899
#define oASTA149				900
#define oASTA249				901
#define oASTA349				902
#define oASTA449				903
#define oASTA549				904
#define oASTA649				905
#define oASTA749				906
#define oQUADROS4A		  		907
#define oQUADROP4A		   		908
#define od4ATO4B		   		909
#define oDOORC4A		   		910
#define oBUSTO4A		   		911
#define omDOORC4A		   		912
#define oDISPOSITIVO4A			913
#define oSTATUA4A		   		914
#define oCHOCOLATES4A			915
#define oHELLENA4A		   		916
#define ocHELLEN4A		   		917
#define oPULSANTE4A		   		918
#define oDIVANOR4A		   		919
#define oDIVANOL4A		   		920
#define oCOMPUTER4A				921
#define oOLOGRAMMAA4B			922
#define oOLOGRAMMAB4B			923
#define oOLOGRAMMAS4B			924
#define oBACHECAC4B				925
#define oBACHECAA4B				926
#define od4BTO4A				927
#define omPORTAL4B				928
#define oARMATURA4B				929
#define oMAZZA4B				930
#define oPROIETTORE4B			931
#define oDIAPOSITIVE4B			932
#define oDIAPRO4B				933
#define oDOORC4B				934
#define oSERRATURA4B			935
#define oSERRATURAC4B			936
#define od4BTO4C				937
#define omANTAR4B				938
#define omANTAL4B				939
#define omPIANTA4B				940
#define od4CTO4B				941
#define oQUADROA4C				942
#define oQUADROB4C				943
#define oWINDOW4C				944
#define omSEDIAL4C				945
#define omSEDIAR4C				946
#define oSCRIVANIA4C			947
#define oCOMPUTER4C				948
#define oFOGLIO4C				949
#define oCASSETTOC4C			950
#define omBUSTO4C				951
#define omSEDIA4C				952
#define omLAMPADA4C				953
#define od4DTO41				954
#define od4DTO4E				955
#define od4DTO4F				956
#define od4DTO4G				957
#define omPIETREC4D				958
#define omDOOR4D				959
#define omPIETREa4D				960
#define omPIETREB4D				961
#define od4ETO4D				962
#define omPIETREL4E				963
#define omPIETRER4E				964
#define od4FTO4D				965
#define omMUROA4F				966
#define omMUROB4F				967
#define omMUROC4F				968
#define omMUROD4F				969
#define omMUROE4F				970
#define od4GTO4W				971
#define od4GTO4H				972
#define od4GTO4D				973
#define omDOORL4G				974
#define omDOORR4G				975
#define od4HTO4G				976
#define od4HTO4I				977
#define omDOOR4H				978
#define omCANCELLO4H			979
#define omCOLONNA4H				980
#define od4ITO4J				981
#define od4ITO4H				982
#define od4ITO4M				983
#define omPASSAGGIOC4I			984
#define omPIETREC4I				985
#define omMUROL4I				986
#define omMUROA4I				987
#define omMUROB4I				988
#define od4JTO4I				989
#define od4JTO4K				990
#define od4JTO4L				991
#define omDOOR4J				992
#define omPIETRE4J				993
#define od4KTO4J				994
#define oPIETRE4K				995
#define oACQUASANTIERA4L		996
#define oMONETA4L				997
#define od4LTO4J				998
#define omPIETRAL4L				999
#define omPIETRAR4L				1000
#define od4MTO4I				1001
#define od4MTO4N				1002
#define od4MTO4O				1003
#define omMUROA4M				1004
#define omMUROB4M				1005
#define omMUROC4M				1006
#define od4NTO4M				1007
#define od4OTO4Q				1008
#define od4OTO4M				1009
#define od4OTO4P				1010
#define od4OTO4S				1011
#define omPIETREC4O				1012
#define omMUROF4O				1013
#define omPIETREL4O				1014
#define omMUROE4O				1015
#define omMUROD4O				1016
#define omMUROC4O				1017
#define omMUROB4O				1018
#define omMUROA4O				1019
#define od4PTO4O				1020
#define oLICANTROPO4P			1021
#define oLICANTROPOM4P			1022
#define oSANGUE4P				1023
#define omMURO4P				1024
#define omCOLONNA4P				1025
#define od4QTO4R				1026
#define omPIETRE4Q				1027
#define od4QTO4O				1028
#define od4RTO4Q				1029
#define oSCHELETRO4R			1030
#define oSTRACCIO4R				1031
#define od4STO4O				1032
#define omMURO4S				1033
#define oDEMONIO4U				1039
#define oPOZZA4U				1040
#define oCAMPANA4U				1041
#define oBIGLIA4U				1042
#define od4VTO4S				1043
#define od4VTO4W				1044
#define od4WTO4G				1045
#define od4WTO4V				1046
#define od4WTO4X				1047
#define od4XTO4W				1048
#define oLASTLEV4				1049
#define oNUMERO14C		 		1050
#define oNUMERO24C		   		1051
#define oNUMERO34C		   		1052
#define oNUMERO44C		   		1053
#define oNUMERO54C		   		1054
#define oNUMERO64C		   		1055
#define oNUMERO74C		   		1056
#define oNUMERO84C		   		1057
#define oNUMERO94C		   		1058
#define oNUMERO04C		   		1059
#define oAST14C		 	  		1060
#define oFIUME51		        1066
#define od51TO53		        1067
#define oAIRCAR51		        1068
#define oTRACCE51		        1069
#define oPUNTO51		        1070
#define oSCAVO51		        1071
#define od51TO52		        1072
#define omALBERO51		        1077
#define omFOGLIE51		        1078
#define oCARCERE52              1079
#define oRETE52                 1080
#define oTELECAMERA52           1081
#define oCARTELLO52             1082
#define oSERPBKG52              1083
#define oSERPENTET52            1084
#define oSNAKEU52               1085
#define oSERPENTEA52            1086
#define oSERPENTEB52            1087
#define oSERPENTEBF52           1088
#define oMASSO52                1089
#define od52TO51              1090
#define omPIANTAL52             1091
#define oGRATAA53		        1092
#define oGRATAC53		        1093
#define omGRATAA53		        1094
#define omGRATAC53		        1095
#define oLUCCHETTO53	        1096
#define oLAGO53			        1097
#define od53TO51		        1098
#define omFOGLIE53		        1099
#define oSCAFFALES54	        1100
#define oSCAFFALEP54	        1101
#define oLENZUOLAS54	        1102
#define oSAPONE54		        1103
#define oLENZUOLA54		        1104
#define od54TO55 1105
#define oCHIAVI54		        1106
#define oDOORC54		        1107
#define oSECCHIOA54		        1108
#define oSECCHIOS54		        1109
#define oSECCHIOV54		        1110
#define oCONDOTTO54		        1111
#define oGRATAA54		        1112
#define omGRATAA54		        1113
#define oGRATAC54		        1114
#define oLAVATRICEF54	        1115
#define oLAVATRICEL54	        1116
#define omLAVATRICEL54	        1117
#define oSPORTELLO54	        1118
#define oLAVATRICE54	        1119
#define oSCAFFALEV54	        1120
#define oPANCA54		        1121
#define oCESTA54		        1122
#define od55TO58 1123
#define oDOOR58C55		        1124
#define oWINDOW58P55	        1125
#define od55TO57 1126
#define odm55TO57 1127
#define oDOOR57C55		        1128
#define oWINDOW57P55	        1129
#define oTELECAMERA55	        1130
#define oCARTELLO55		        1131
#define od55TO54 1132
#define odm55TO54 1133
#define oPANNELLO55		        1134
#define od55TO56 1135
#define oWINDOW56		        1136
#define oPOSTER56		        1137
#define oCAVOTAGLIATO56	        1138
#define oCOPERCHIOP56	        1139
#define oPANNELLOA		        1140
#define oPANNELLOC56	        1141
#define oCESTINO56		        1142
#define oARMADIETTO56	        1143
#define od56TO59 1144
#define omDIREZIONE56	        1145
#define oDOORC56		        1146
#define oKEYBOARD56		        1147
#define oCARTELLO56		        1148
#define oDOORS56		        1149
#define oTAVOLO56		        1150
#define oSEDIA56		        1151
#define oBOTTIGLIA56	        1152
#define oRIVISTA56		        1153
#define oMDVD56			        1154
#define od56TO55 1155
#define oCOMPUTER59		        1156
#define oLAVAGNA59		        1157
#define oSIMBOLI59		        1158
#define oMICROSCOPIO59	        1159
#define oGUANTI59		        1160
#define oFIALE59		        1161
#define oTESTER59		        1162
#define omSCALA59		        1163
#define od59TO56 1164
#define oLETTINO59		        1165
#define omBIDONI59		        1166
#define oEXIT59L		        1167
#define oARMADIETTO57	        1168
#define oSCRIVANIA57	        1169
#define oBOMBOLA57		        1170
#define oCAMICE57		        1171
#define oDOORS57		        1172
#define od57TO55 1173
#define omDOOR57		        1174
#define oBRANDE57		        1175
#define oBRANDAF57		        1176
#define oWINDOWA57	        1177
#define oWINDOW57		        1178
#define od58TO55 1179
#define oKEYBOARD58		        1180
#define oDOOR58		        1181
#define oBIDONE58		        1182
#define oCARTELLO58		        1183
#define oGUARDIA58		        1184
#define oPORTACA58		        1185
#define oSLOTA58		        1186
#define oPORTACB58		        1187
#define oSLOTB58		        1188
#define oPORTACC58		        1189
#define oWINDOWA58	        1190
#define oWINDOWB58	        1191
#define oWINDOWC58	        1192
#define oSPORTELLOA58	        1193
#define oSPORTELLOB58	        1194
#define oSPORTELLOC58	        1195
#define oEXIT58M		        1196
#define od5ATO57		        1197
#define oWINDOWA5A	        1198
#define oWINDOWB5A	        1199
#define omWINDOW5A	        1200
#define oPIANTE5A		        1201
#define oEXIT58T		        1202
#define oEXIT41D				1203	// Exit from positioner
#define oPULSANTECD				1205
#define oPULSANTE1AD			1247
#define oPULSANTE33AD			1279
#define oROOM41					1280
#define oROOM45					1284
#define oROOM4I					1297
#define oROOM4X					1312
#define oROOM45B				1313
#define oTASTO158				1314
#define oTASTO258				1315
#define oTASTO358				1316
#define oTASTO458				1317
#define oTASTO558				1318
#define oTASTO658				1319
#define oTASTO758				1320
#define oTASTO858				1321
#define oTASTO958				1322
#define oTASTO058				1323
#define oLED158			        1324
#define oLED258			        1325
#define oLED358			        1326
#define oLED458			        1327
#define oLED558			        1328
#define oLED658			        1329
#define o00SAVE					1330
#define o00LOAD					1331
#define o00QUIT					1332
#define o00EXIT					1333
#define o00SPEECHON				1334
#define o00SPEECHOFF			1335
#define o00TEXTON				1336
#define o00TEXTOFF				1337
#define o00SPEECH				1338
#define o00MUSIC				1339
#define o00SOUND				1340
#define o00SPEECH1D				1341
#define o00SPEECH1U				1342
#define o00SPEECH2D				1343
#define o00SPEECH2U				1344
#define o00SPEECH3D				1345
#define o00SPEECH3U				1346
#define o00SPEECH4D				1347
#define o00SPEECH4U				1348
#define o00SPEECH5D				1349
#define o00SPEECH5U				1350
#define o00SPEECH6D				1351
#define o00MUSIC1D				1352
#define o00MUSIC1U				1353
#define o00MUSIC2D				1354
#define o00MUSIC2U				1355
#define o00MUSIC3D				1356
#define o00MUSIC3U				1357
#define o00MUSIC4D				1358
#define o00MUSIC4U				1359
#define o00MUSIC5D				1360
#define o00MUSIC5U				1361
#define o00MUSIC6D				1362
#define o00SOUND1D				1363
#define o00SOUND1U				1364
#define o00SOUND2D				1365
#define o00SOUND2U				1366
#define o00SOUND3D				1367
#define o00SOUND3U				1368
#define o00SOUND4D				1369
#define o00SOUND4U				1370
#define o00SOUND5D				1371
#define o00SOUND5U				1372
#define o00SOUND6D				1373

// Item IDs 1374 - 1384 have been removed

// Added Items
#define oFUSE12CU				1385
#define oEXIT12CU				1386
#define oLETTER13CU				1387
#define oEXIT13CU				1388
#define oTURTLE51				1389
#define oEXIT4CT				1390

#define oLASTLEV5				1391

/*-------------------------------------------------------------------------*/
/*                               ANIMS	             					   */
/*-------------------------------------------------------------------------*/

#define aBKG11									1
#define aBKG12									2
#define aBKG13									3
#define aBKG14									4
#define aBKG16									5
#define aBKG17A 								6
#define aBKG17B 								7
#define aBKG18									8
#define aBKG19									9
#define aBKG1A									10
#define aBKG1B									11
#define aBKG1C									12
#define aBKG1D									13
#define aBKG21									14
#define aBKG22									15
#define aBKG23A									16
#define aBKG23B									17
#define aBKG24									18
#define aBKG25  								19
#define aBKG26									20
#define aBKG27  								21
#define aBKG28									22
#define aBKG2A									23
#define aBKG2B									24
#define aBKG2C									25
#define aBKG2D									26
#define aBKG2E									27
#define aBKG2F									28
#define aBKG2G									29
#define aBKG2GV									30
#define aBKG2H                                  31
#define aBKG33									32
#define aBKG34									33
#define aBKG35  								34
#define aBKG36									35
#define aBKG37  								36
#define aBKG41									37
#define aBKG42									38
#define aBKG43									39
#define aBKG44									40
#define aBKG45									41
#define aBKG46									42
#define aBKG47									43
#define aBKG48									44
#define aBKG49									45
#define aBKG4A									46
#define aBKG4B									47
#define aBKG4C									48
#define aBKG4CT                                 49
#define aBKG4D									50
#define aBKG4E									51
#define aBKG4G									52
#define aBKG4K									53
#define aBKG4L									54
#define aBKG4N									55
#define aBKG4O									56
#define aBKG4P									57
#define aBKG4Q									58
#define aBKG4R									59
#define aBKG4S									60
#define aBKG4T									61
#define aBKG4U									62
#define aBKG4V									63
#define aBKG4X									64
#define aBKG51									65
#define aBKG52									66
#define aBKG53									67
#define aBKG54									68
#define aBKG56									69
#define aBKG57									70
#define aBKG58									71
#define aBKG59									72
#define aBKG5A									73

// redefine di h
#define aWALKIN 								14
#define aWALKOUT								18

enum AnimationId {
	kAnim113UseVial = 77,
	kAnim121TryOpenLetterboxes = 82,
	kAnim122OpenLetterbox17 = 83
};


#define a127USEWRAPPEDFUSED	  					87
#define a128RIUSABOTTONE		              	88
#define a129PARLACOMPUTERESCENDE              	89
#define a1211OSSERVAMAPPAPALAZZO              	90
#define a1213ARRIVACONASCENSORE	              	91
#define a131USABANCONOTA				        92
#define a132PRENDELATTINA				        93
#define a133CPRENDEMONETA				        95
#define a134USAMAGNETICPEN				        96
#define a135METTELETTERA				        97
#define a137RICALCAFIRMA				        99
#define a139CHIUDONOPORTESU				        101
#define a1311DABOTTADISTRIBUTORE		        102
#define a1312METTELETTERARICALCA		        103
#define a1314ARRIVAASCENSOREDABASSO		        105
#define a1315ARRIVAASCENSOREALTOENTRA           106
#define a1316CHIUDONOPORTEGIU			        107
#define a141APREFRIGORIFERO			            108
#define a142PRENDEBOTTIGLIA			            109
#define a143PRENDECREDITCARD		            110
#define a143CPRENDECREDITCARD		            111
#define a144PRENDEFOGLIO			            112
#define a145ESAMINASCAFFALE			            113
#define a145CESAMINASCAFFALE			        114
#define a146PRENDEPORTAFOTO			            115
#define a147PORTAAPPARTAMENTOESCE              	116
#define a148PORTACAMERAENTRA		            117
#define a149ACCENDETELEVISORE		            118
#define a1410SPEGNETELEVISORE		            119
#define a1411SIEDEDIVANO			            120
#define a1412SPOSTASEDIA			            121
#define a1413OSSERVAPIANOCOTTURA	            122
#define a1414ENTERROOM							123
#define a1510TOCCABOCCETTE		                133
#define a1511SISPECCHIA			                134
#define a165USABANCONOTA	                    142
#define a166USECREDITCARD			            143
#define a167USEMONEY			                144
#define a1613GUARDACARTACCE			            149
#define a1614GUARDAMONITORS14			        150
#define a1615GUARDAMONITORS15			        151
#define a1616SALECONASCENSORE		            152
#define a1617SCENDECONASCENSORE		            153
#define a172USAPLASTICA							155
#define a177SICHINA								159
#define a179MUOVETESTA							160
#define a181ESAMINACARTELLONE					162
#define a184ENTRACLUB							165
#define a186GUARDAPIAZZA						166
#define a189ENTRADALCLUB						168
#define a1810ENTRADALNEGOZIO					169
#define a1A1ESAMINAVETRINA						170
#define a1A5ENTRA								172
#define a1B1USASBARRA							173
#define a1B2VERSAACIDO							174
#define a1B3APREBOTOLA							175
#define a1B4ESCEBOTOLA							176
#define a1B8FRUGACARTACCIE						180
#define a1B10GUARDABIDONE						182
#define a1B11ESCETOMBINO						183
#define a1B12SCAPPATOPO							184
#define a1B2AVERSAACIDO							185
#define a1B3AAPREBOTOLA							186
#define a1B6ASCENDEBOTOLA						187
#define a1C1ENTRACUNICOLO						188
#define a1C2GUARDAGRATA							189
#define a1C3RACCOGLIETOPO						190
#define a1D1SCENDESCALE							192
#define a1D7SPINGEBARILOTTO						197
#define a1D8SALESGABELLO						198
#define a1D10ESAMINABOTTIGLIE					200
#define a1D12SCENDESCALA						202
#define a211									203
#define a212									204
#define a213									205
#define a214									206
#define a216									208
#define a219									210
#define a221									211
#define a222   									213
#define a227									218
#define a228									219
#define a229									220
#define a2214									224
#define a2215									225
#define a227A                                   227
#define a228A                                   228
#define a229A                                   229
#define a2214A                                  230
#define a2215A                                  231
#define a231									232
#define a233									234
#define a234									235
#define a235									236
#define a236									237
#define a238									239
#define a239									240
#define a2310									241
#define a2311									242
#define a242									244
#define a243									247
#define a244									248
#define a251									249
#define a254									252
#define a254B									253
#define a257									256
#define a262									258
#define a263									259
#define a281									269
#define a282									270
#define a283									271
#define a286									274
#define a2810									278
#define a2811									279
#define a291USAINTERRUTTORELUCE					280
#define a293AVVITALAMPADINA						282
#define a298ESCEBOTOLA							287
#define a2910ESCEBOTOLA							289
#define a2A2USEKEY								293
#define a2B1PROVAAPRIREPORTA					297
#define a2B2ESCEPOZZO							298
#define a2B4TAKEBOOK							300
#define a2B6METTELAMPADINA						302
#define a2B9ESAMINALEOPARDO						305
#define a2B10ESAMINAPELLICANO					306
#define a2B11ESAMINASERPENTE					307
#define a2B12MAPPAMONDO							308
#define a2B13OSSERVAREFARFALLE					309
#define a2C1PROVACAMPO							310
#define a2C2OPERATREWHEELS						311
#define a2C3OPERATEWHEEL1						312
#define a2C4OPERATEWHEEL2						313
#define a2C5OPERATEWHEEL3						314
#define a2C6PREMEPULSANTE						315
#define a2C6PREMEPULSANTEAPERTURA				316
#define a2D1PRENDEFUCILE						317
#define a2D2SCHIACCIATASTO1						318
#define a2D3SCHIACCIATASTO2						319
#define a2D4SCHIACCIATASTO3						320
#define a2D5SCHIACCIATASTO4						321
#define a2D6SCHIACCIATASTO5R					322
#define a2D6SCHIACCIATASTO5V					323
#define a2D7SCHIACCIATASTO6R					324
#define a2D7SCHIACCIATASTO6V					325
#define a2D8SCHIACCIATASTO7R					326
#define a2D8SCHIACCIATASTO7V					327
#define a2D9SCHIACCIATASTO8						328
#define a2D10FRUGAGIACCA						329
#define a2D11TOCCALEONE							330
#define a2E1PROVACAMPOMAGNETICO					331
#define a2E2PRIMAPALLONTANANDO					332
#define a2E3FIRSTAPPROACH					333
#define a2E4SECONDAPENTRAGROTTA					334
#define a2E5SECONDAPARRIVANDO					335
#define a2E6GUARDACREPACCIODIQUA				336
#define a2E7GUARDACREPACCIODILA					337
#define a2F1PRENDECANDELOTTO					338
#define a2F3USAASCENSORE						339
#define a2F4ESCEASCENSORE						340
#define a2F5FRUGABIDONE							341
#define a2F5CFRUGABIDONE						342
#define a2F6TOCCADINOSAURO						343
#define a2F7TASTOCHIUDERE						344
#define a2F8TASTOAPRIRE							345
#define a2F9PANNELLOSIAPRE						346
#define a2F9PPANNELLOSIAPRE						347
#define a2F10PANNELLOSICHIUDE					348
#define a2G1CHIAMARAGAZZO						349
#define a2G2PRENDETELECOMANDO					350
#define a2G3TENTACOPERCHIO						351
#define a2G4VERSALATTINA						352
#define a2G5METTEBOMBOLA						353
#define a2G7CROSSBRIDGE				354
#define a2G8PONTICELLOTORNA						355
#define a2H1ARRIVA								356
#define a2H2SCENDE								357
#define a311USACOMANDI							358
#define a312PROVAPORTONE						359
#define a313PROVPANNELLO						360
#define a314APREPANNELLO						361
#define a315USACACCIAVITE						362
#define a317FRUGAPROIETTORE						363
#define a316FRUGAPINZA							364
#define a318SALEMONTACARICHI					365
#define a319SCENDEMONTACARICHI					366
#define a3111TRASCINA							367
#define a3113ENTRAPASSAGGIO						368
#define a3114ESCEPASSAGGIO						369
#define a3115USACOMANDISECONDA					370
#define a3119FRUGANULLA							371
#define a3110PRENDESIGARO						372
#define a3118CHIUDEPANNELLO                     373
#define a321SALEMONTACARICHI					374
#define a322SCENDEMONTACARICHI					375
#define a327USAMONTACARICHI						376
#define a323BOTOLAAPRE							377
#define a324SALESCALE							378
#define a325SCENDESCALE							379
#define a326BOTOLACHIUDE						380
#define a331PROVABOTOLA							381
#define a332PROVAPORTA							382
#define a333LOSEBAR								383
#define a334SALERUBINETTO						384
#define a335RUBINETTONULLA						385
#define a336STACCATUBO							386
#define a337PRENDETUBO							387
#define a338POSASAM								388
#define a3311SALESCALE							389
#define a3310SCENDESCALE						390
#define a3312USARETUBOSAM						391
#define a3313CHIUDEBOTOLA						392
#define a341USAPINZE							393
#define a342PIEGATUBO							394
#define a343RIPIEGATUBO							395
#define a344APREVALVOLA							396
#define a345CHIUDEVALVOLA						397
#define a347ESAMINAGENERATORE					398
#define a348ESAMINAFOGLI						399
#define a349COLPETTIBOMBOLE						400
#define a351PROVAPROIETTOREPRESA				401
#define a352ATTPROIETTOREETRIPLAPRESA			402
#define a353ATTACCATRIPLAPRESA					403
#define a354STACCATRIPLAPRESA					404
#define a355ATTPROIETTORETRIPLAEPRESA	  		405
#define a356PROVASPORTELLO						406
#define a357ACCENDEPROIETTORE					407
#define a358INSERISCECARD						408
#define a359RITIRACARD							409
#define a3510PREMEPULSANTE						410
#define a3511APRESPORTELLO						411
#define a3512PROVASECURITYSYSTEM				412
#define a3513PROVAPORTACHIUSA					413
#define a3514ENTRAASCENSORE						414
#define a3515ESCEASCENSORE						415
#define a3517ESAMINACIANFRUSAGLIE				416
#define a3517AESAMINACIANFRUSAGLIE				417
#define a3521LEGGEGIORNALE						418
#define a3522ESAMINACASSE						419
#define a3523GUARDIA							420
#define a361ESCEASCENSORE						421
#define a362ENTRAASCENSORE						422
#define a363PROVACASSETTO						423
#define a364APRECASSETTO						424
#define a365TROVACARD							425
#define a366ALZAQUADRO							426
#define a367PROVASCANNER						427
#define a368USASTETOSCOPIO						428
#define a369PROVAPORTA							429
#define a3610APREPORTA							430
#define a3612GUARDA37							431
#define a3613GUARDA38							432
#define a3615CPRENDESCAFFALE					433
#define a3615APRENDESCAFFALE					434
#define a3615AAPRENDESCAFFALE					435
#define a3616APRESCATOLONE						436
#define a371APREPARATIA							437
#define a372APRESPORTELLO						438
#define a3721APRESPORTELLO						439
#define a373CHIUDESPORTELLO						440
#define a374CHIUDEPARATIA						441
#define a376FRUGACAMICE							442
#define a377APREARMADIETTO						443
#define a378GUARDASOTTO							444
#define a411									445
#define a412									446
#define a413									447
#define a414									448
#define a415									449
#define a416									450
#define a417									451
#define a418									452
#define a419									453
#define a4110   								454
#define a421									455
#define a422									456
#define a423									457
#define a424									458
#define a425									459
#define a426									460
#define a427									461
#define a428									462
#define a429									463
#define a4210									464
#define a4213									465
#define a431									466
#define a431R									467
#define a432									468
#define a432R									469
#define a433									470
#define a434									471
#define a435									472
#define a436									473
#define a437									474
#define a4311									475
#define a4312									476
#define a441									477
#define a442									478
#define a443									479
#define a444									480
#define a445									481
#define a446									482
#define a447									483
#define a448									484
#define a451									485
#define a452									486
#define a453									487
#define a454									488
#define a455									489
#define a456									490
#define a457									491
#define a458									492
#define a459									493
#define a461									494
#define a468									495
#define a471									496
#define a472									497
#define a473									498
#define a474									499
#define a475									500
#define a476									501
#define a477									502
#define a481									503
#define a482									504
#define a483									505
#define a484									506
#define a485									507
#define a486									508
#define a487									509
#define a488									510
#define a489									511
#define a4810									512
#define a4811									513
#define a4812									514
#define a4813									515
#define a4814									516
#define a491									517
#define a493									518
#define a494									519
#define a495									520
#define a496									521
#define a4A1									522
#define a4A2									523
#define a4A3									524
#define a4A5									525
#define a4B1									526
#define a4B2									527
#define a4B3									528
#define a4B4									529
#define a4B5									530
#define a4B6A   								531
#define a4B6B   								532
#define a4B6C   								533
#define a4B9A   								534
#define a4B9B   								535
#define a4B9C   								536
#define a4B12									537
#define a4C1									538
#define a4C3									539
#define a4C4									540
#define a4C15									541
#define a4L1									542
#define a4P7									543
#define a4R1									544
#define a4U2									545
#define a4U3									546
#define a4U4									547
#define a4U5									548
#define a4U6									549
#define a511									550
#define a512									551
#define a513									552
#define a516                                    553
#define a521									554
#define a522									555
#define a523									556
#define a526									557
#define a528									558
#define a527									559
#define a531									560
#define a532									561
#define a533									562
#define a533C                                   563
#define a534									564
#define a541									565
#define a542									566
#define a543									567
#define a544									568
#define a544G									569
#define a545									570
#define a546									571
#define a547									572
#define a548									573
#define a5411									574
#define a551									575
#define a552									576
#define a553									577
#define a554									578
#define a555									579
#define a556									580
#define a5511									581
#define a5512									582
#define a561									583
#define a562									584
#define a563									585
#define a564									586
#define a565									587
#define a566									588
#define a567									589
#define a568									590
#define a569									591
#define a5614									592
#define a5614P									593
#define a571									594
#define a572									595
#define a573									596
#define a574									597
#define a575									598
#define a576									599
#define a587									600
#define a582									601
#define a586									602
#define a588									603
#define a589									604
#define a583									605
#define a591									606
#define a592									607
#define a593									608
#define a596									609
#define a597									610

#define FIRST_INV_ITEM 621

/*-------------------------------------------------------------------------*/
/*                           INVENTORY OBJECTS          				   */
/*-------------------------------------------------------------------------*/
#define READICON   		121 //64 //56 //28
#define EMPTYSLOT   	121	// for saving/loading

enum InventoryItem {
	kItemNull = 0,
	kItemBankNote = 1,
	kItemMinicom = 2,
	kItemLiftCard = 3,
	kItemPen = 4,
	kItemLetterboxKey = 5,
	kItemFoilPaper = 6,
	kItemBar = 7,
	kItemBurnedFuse = 8,
	kItemRepairedFuse = 9,
	kItemLetter = 10,
	kItemFiveCreditCoin = 11,
	kItemBeerCan = 12,
	kItemWineBottle = 13,
	kItemNoteRotmall17 = 14,
	kItemSubwayCard = 15,
	kItemRubysPhoto = 16,
	kItemRubysSafeboxKey = 17,
	kItemPistol = 18,
	kItemTubeOfAcidFull = 19,
	kItemRubysReport = 20,
	kItemTubeOfAcidHalfFull = 21,
	kItemPlasticPlate = 22,
	kItemSubwayMap = 23,
	kItemSkate = 24,
	kItemMembershipCard = 25,
	kItemDeadRat = 26,
	kItemBottleOfChateau = 27,
	kItemRatOnSkate = 28,
	kItemMetallicS = 29,
	kItemMagneticBar = 30,
	kItemBookstoreKey = 31,
	kItemPoster = 32,
	kItemCopperLeads = 33,
	kItemScrewdriver = 34,
	kItemSocketSpanner = 35,
	kItemEmptyTinCan = 36,
	kItemPliers = 37,
	kItemMagnet = 38,
	kItemEmptyBeerCan = 39,
	kItemGasCylinder = 40,
	kItemTinCanWithOil = 41,
	kItemBroom = 42,
	kItemCanWithFuel = 43,
	kItemFaultyBulb = 44,
	kItemThreeWayAdapter = 45,
	kItemEgyptologyBook = 46,
	kItemWorkingBulb = 47,
	kItemMicrowaveGun = 48,
	kItemFlare = 49,
	kItemElevatorRemoteControl = 50,
	kItemLargeCans = 51,
	kItemDamagedMinicom = 52,
	kItemMatch = 53,
	kItemProjector = 54,
	kItemPincers = 55,
	kItemMakeshiftTorch = 56,
	kItemProjector2 = 57,
	kItemDeskKey = 58,
	kItemLaserGun = 59,
	kItemSecurityCard = 60,
	kItemSecuritySystemSequence = 61,
	kItemStethoscope = 62,
	kItemEmptySyringe = 63,
	kItemRubysMedicalReport = 64,
	kItemPositioner = 65,
	kItemHalfCreditCoins = 66,
	kItemPuppet = 67,
	kItemHammer = 68,
	kItemBell = 69,
	kItemMatchInPacket = 70,
	kItemPuppetRemoteControl = 71,
	kItemMud = 72,
	kItemSaltpetre = 73,
	kItemSkull = 74,
	kItemTorch = 75,
	kItemCharcoalSlack = 76,
	kItemBagWithPapaverine = 77,
	kItemJarWithSulfurPowder = 78,
	kItemShaft = 79,
	kItemAncientParchment = 80,
	kItemCharcoalSaltpetrePowder = 81,
	kItemCharcoalSulfurPowder = 82,
	kItemSaltpetreSulfurPowder = 83,
	kItemGunpowder = 84,
	kItemEmptyGlassJar = 85,
	kItemMeetingRoomSlide1 = 86,
	kItemMeetingRoomSlide2 = 87,
	kItemPiratePistol = 88,
	kItemClub = 89,
	kItemIronBullet1 = 90,
	kItemIronBullet2 = 91,
	kItemHugsOfficeSlide = 92,
	kItemHalfCreditCoin = 93,
	kItemJarWithWerewolfBlood = 94,
	kItemLeftoverGunpowder = 95,
	kItemPistolWithGunpowder = 96,
	kItemLoadedPistol = 97,
	kItemAsbestosCloth = 98,
	kItemSilverBullet = 99,
	kItemParallelCutter = 100,
	kItemWristComm = 101,
	kItemPrisonMap = 102,
	kItemTurtleEgg = 103,
	kItemSnakeHead = 104,
	kItemSnakeTail = 105,
	kItemGuardKeys = 106,
	kItemSoapPowder = 107,
	kItemVideoRecorder = 108,
	kItemDentalFloss = 109,
	kItemMedicalThermometer = 110,
	kItemInflatedGloves = 111,
	kItemGovernorsCode = 112,
	kItemLowPowerLaser = 113,
	kItemTester = 114,
	kItemTubes = 115,
	kItemSurgicalGloves = 116,
	kItemPoisonSyringe = 117,
	kItemInflatedGlovesWithFloss = 118,
	kItemPoisonSyringeWithFloss = 119,
	kItemFloatingPoisonSyringe = 120
};

/*-------------------------------------------------------------------------*/
/*                                 CHARACTERS         					   */
/*-------------------------------------------------------------------------*/

#define hSTAND			0
#define hSTART			1
#define hWALK 			2
#define hEND  			3
#define hSTOP0			4
#define hSTOP1			5
#define hSTOP2			6
#define hSTOP3			7
#define hSTOP4			8
#define hSTOP5			9
#define hSTOP6			10
#define hSTOP7			11
#define hSTOP8			12
#define hSTOP9			13
#define hWALKIN 		14
#define hBOH  			15
#define hUSEGG			16
#define hUSETT			17
#define hWALKOUT		18

#define hLAST									19  // Last Default Action

/*------------------------------------------------------------------------*/
/*                                SCRIPT        						  */
/*------------------------------------------------------------------------*/
#define s16CARD				1
#define S16MONEY			2
#define s19EVA				3
#define s34PANEL			4
#define s4AHELLEN			5
#define s49SUNDIAL			6
#define s21TO22				7
#define s21TO23				8
#define s24TO23				9
#define s24TO26				10
#define s2ETO2C				11
#define s2ETO2F				12

/*------------------------------------------------------------------------*/
/*                                SOUND         						  */
/*------------------------------------------------------------------------*/

#define wPAD1               129
#define wPAD5               133
#define wASTA49             139
#define wWHEELS2C           141
#define wCOVER31			142
#define wWIN35				143

/*------------------------------------------------------------------------*/
/*                                DIALOGS                                 */
/*------------------------------------------------------------------------*/

#define dASCENSORE12         1    // Elevator floor selection (roof level)
#define dASCENSORE13         2    // Elevator floor selection (apartment level)
#define dASCENSORE16         3    // Elevator floor selection (subway level)
#define dBIGLIETTERIA16      4
#define dPOLIZIOTTO16        5
#define dTRAMP171            6    // Bum asks for booze
#define dTRAMP1713           7
#define dTRAMP1714           8
#define dTRAMP1716           9
#define dTRAMP1717          10    // The bum asks 10 credits for his skate
#define dGUARDIAN18         11    // The Doorman outside the Free Climax club asks for a membership card
#define dEVA19              12    // Talking with Eva at the Free Climax club
#define dSHOPKEEPER1A       13    // Talking with the shopkeeper in the Liquor store
#define dSAM17              14    // Talking with Hugh via the Minicom
#define dSFINGE2C           15    // Sphinx pre-recorded message
#define dC381               16    // Talking with Al and Eva before entering the Amusement Park
#define dC4A1               17    // Talking with Hugh's secretary
#define dC581               18    // Talking with Moreau from his cell's door
#define dC582               19    // Joshua gets the governor's code from Eva from her cell's door
#define dC5A1               20    // Joshua talks to Eve through her prison cell's window
#define dINTRO              21    // Intro
#define dF151               22
#define dF181               23    // News report about closing the subway
#define dF161               24    // Joshua boarding train
#define dF171               25    // Train arriving at Rotmall 17
#define dF1A1               26    // The shopkeeper and his wife argue about her fear of rats
#define dF1C1               27    // Opening the grating and going to the platforms
#define dF1D1               28    // Rolling the rat on the skate towards the shopkeeper's wife
#define dF211               29    // Crossing the train tracks with the makeshift grapple
#define dF212               30    // Using the magnet to attract the grapple and cross the tracks
#define dF212B              31    // Using the pincers to grab the grapple and cross the tracks
#define dF213               32    // Using the bar to grab the grapple and cross the tracks
#define dF213B              33    // Using the pincers to grab the grapple and cross the tracks
#define dF231               34    // Using the leads to short-circuit the magnetic field
#define dF291               35    // Turning the storeroom light on
#define dF2E1               36    // Trying to cross the footbridge, a mechanical dinosaur appears
#define dF2E2               37    // Shooting the mechanical dinosaur with the gun
#define dF2G1               38    // Opening the machinery panel at the pond
#define dF2G2               39    // Throwing the flare in the gas tank, opening the sluice gate
#define dF2H1               40    // Cutting the cable, the sign drops down and forms a passage to the Zoo
#define dF321               41    // Setting off the warehouse alarm with the torch, electrocuting the guard
#define dF331               42    // Blowing the trapdoor open at the warehouse, alarm goes off, guard deactivates it
#define dF351               43    // Using the projector with the hologram, knocking off the terrorist
#define dF361               44    // Checking out the computer at the hideout and the report on Moreau
#define dF362               45    // Using the stethoscope at the hideout, listening to Al and Eva, leads to dialog 16
#define dF371               46    // Discovering Ruby at the Medical room of the hideout
#define dF372               47    // Opening the panel next to Ruby, finding a report on him
#define dF381               48    // Talking with Al and Eva
#define dF431               49    // Using the puppet's remote control to use the cablecar and go to the castle
#define dF451               50    // Shutting down the boiler with the cloth, driving the spider away
#define dF491               51    // Joshua finds Moreau's diary and goes to Hugh's office, leads to dialog 17
#define dF4A3               52    // Hugh's secretary eats a spiked chocolate and falls asleep
#define dF4C1               53    // Joshua reads evidence from Hugh's computer, leaves cyberspace, goes to Rocas Perdida
#define dF4P1               54    // Shooting at the werewolf with the wrong type of bullet
#define dF4P2               55    // Shooting and killing the werewolf
#define dF541               56    // Peeking through the door, checking the guard
#define dF542               57    // The guard comes to investigate and falls through the grating
#define dF561               58    // Joshua talks with Al about the fence cables
#define dF562               59    // Joshua cuts the fence cables, Al and Eva enter the prison and Eva gets captured
#define dF581               60    // Talking with Al after talking with Moreau
#define dF5A1               61    // Joshua sends the poisonous syringe to Eva, she uses it to kill the guard
#define dF582               62    // Ending sequence, Joshua confronts Hugh
#define dF4PI               63    // Joshua meets the werewolf for the first time
#define dFLOG               64    // Company logos
#define dFCRED              65    // Credits

#define PATCHOBJ_ROOM41D 89
#define PATCHOBJ_ROOM2C  20

// 3D Rendering
#define MAXVERTEX 1000
#define MAXFACE 1000

#define CALCPOINTS 1
#define DRAWFACES 2
#define DOALL 3

#define SHADOWAMBIENT 27      // 0 (black) ... 127 (blank) shadow colors - opposite
#define LIGHTRANGE 2048.0     // after 127 * val the light doesn't have an effect (deleted)
#define CHARACTERMIDSIZE 91.0 // character half height

/******************************************************************************
Game Structure
******************************************************************************/
#define MAXICON 82 // Inventory icons

#define MAXOBJINROOM 128           // Objects per room
#define MAXSOUNDSINROOM 15         // Sounds per room
#define MAXACTIONINROOM 32         // Number of actions per room

#define MAXSENTENCE 4000 // Max Examine phrases
#define MAXOBJNAME 1400

#define MAXSCRIPTFRAME 50
#define MAXSCRIPT 15
#define MAXTEXTSTACK 3

// define smacker animations
#define MAXAREA 4
#define MAXATFRAME 16

/******************************************************************************
Null Objects
******************************************************************************/
#define NO_OBJECTS 0

/******************************************************************************
Screen management
******************************************************************************/
#define MAXX 640
#define MAXY 480

#define AREA 360
#define TOP ((MAXY - AREA) / 2) // Coordinate y of game screen (60)
#define VIDEOTOP TOP
#define ZBUFFERSIZE 200000L

/******************************************************************************
Low level text and color management
******************************************************************************/
#define CARHEI 10
#define MAXDTEXTLINES 20
#define MAXDTEXTCHARS 128
#define MAXCHARS 128

/******************************************************************************
High level color management
******************************************************************************/
#define COLOR_CHARACTER HYELLOW
#define COLOR_OBJECT HWHITE
#define COLOR_INVENTORY HBLUE

/******************************************************************************
Inventory
******************************************************************************/
#define INV_ON 1       // Active
#define INV_INACTION 2 // In use (the mouse is in the bottom)
#define INV_PAINT 4    // Not yet ON (Going down)
#define INV_DEPAINT 8  // Not yet OFF (Going up)
#define INV_OFF 16     // Not on screen

#define INVENTORY_HIDE 40 // ICONDY ...was 48.
#define INVENTORY_SHOW 0

/******************************************************************************
Inventory icon management
******************************************************************************/
#define FIRSTLINE (TOP + AREA) // First line of the inventory
#define ICONDY 40              // Icon Height
#define ICONDX 48              // Icon width
#define ICONSHOWN 12           // Icons showed simultaneously
#define INVSCROLLSP 5          // Scrolling speed
#define ICONMARGDX 32          // Right margin
#define ICONMARGSX 32          // Left margin

/******************************************************************************
Management of "Use with"
******************************************************************************/
#define USED 0
#define WITH 1


#define PI 3.1415927f
#define PI2 6.2831853f
#define EPSILON 0.007f
#define MAXSTEP 1000
#define MAXPATHNODES 50
#define MAXPANELSINROOM 400

#define TEXT_OFF 0
#define TEXT_DRAW 2
#define TEXT_DEL 4

#define MAXTEXTAREA 200000L
#define MAXSYSTEXT 30
#define MAXMAT 20

/*************************************************************************
* PRIORITIES                                                             *
*************************************************************************/
#define MP_SYSTEM 255 /* ABSOLUTE PRIORITY */

#define MP_MED 64
#define MP_HIGH 128
#define MP_DEFAULT MP_MED

/*************************************************************************
* EVENT CLASSES                                                         *
*                                                                       *
* MC Message Classes Notify                                             *
* ME Message Events  Notify                                             *
*************************************************************************/

#define CLASS_GAME 63 //   0- 63 --> code GAME
// 64-127 --> code ANIM (removed)
#define CLASS_CHAR 255 // 128-255 --> code CHAR

/*************************************************************************
* IDLE                                                                  *
*************************************************************************/
#define MC_IDLE 0

/*************************************************************************
* MOUSE                                                                 *
*************************************************************************/
/* Mouse events */
#define MC_MOUSE 1

#define ME_MLEFT 0  // (mx,my,...) - used by game scripts
#define ME_MRIGHT 1 // (mx,my,...) - used by game scripts

/*************************************************************************
* STRINGS                                                                *
*************************************************************************/
/* String names */
#define MC_STRING 4

#define ME_CHARACTERSPEAKING 3 // (someone,)
#define ME_SOMEONESPEAKING 5   // (...)


/*************************************************************************
* Operations with the mouse                                              *
*************************************************************************/
/* Notification of mouse action */
#define MC_ACTION 5

#define ME_MOUSEOPERATE 0 // used by game scripts
#define ME_MOUSEEXAMINE 1


/*************************************************************************
* DIALOGS                                                                *
*************************************************************************/
/* Notification of dialog event */
#define MC_DIALOG 6

#define ME_STARTDIALOG 1	// used by game scripts

/*************************************************************************
* SCRIPT                                                                *
*************************************************************************/
#define MC_SCRIPT 9      // (b,w1,w2,l)
#define ME_CHANGER 18  // (0,room,door,0) - used by game scripts

/*************************************************************************
* CHARACTER                                                             *
*************************************************************************/
/* Notification of dialog event */
#define MC_CHARACTER 128

#define ME_CHARACTERGOTO 2
#define ME_CHARACTERGOTOACTION 3
#define ME_CHARACTERGOTOEXAMINE 4

#define ME_CHARACTERACTION 9	// used by game scripts
#define ME_CHARACTERCONTINUEACTION 10
#define ME_CHARACTERGOTOEXIT 11
#define ME_CHARACTERDOACTION 12


#endif
