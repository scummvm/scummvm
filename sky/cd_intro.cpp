/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sky/sky.h"
#include "sky/skydefs.h"

//CD intro file defines
#define cdv_00	59500
#define cd_pal	59501
#define cd_1_log	59502
#define cd_1	59503
#define cdv_01	59504
#define cdv_02	59505
#define cd_2	59506
#define cdv_03	59507
#define cdv_04	59508
#define cd_3	59509
#define cdv_05	59510
#define cdv_06	59511
#define cd_5	59512
#define cdv_07	59513
#define cdv_08	59514
#define cdv_09	59515
#define cd_7	59516
#define cdv_10	59518
#define cd_11	59519
#define cdv_11	59520
#define cd_11_pal	59521
#define cd_11_log	59522
#define cdv_12	59523
#define cd_13	59524
#define cdv_13	59525
#define cdv_14	59527
#define cdv_15	59528
#define cd_15_pal	59529
#define cd_15_log	59530
#define cdv_16	59531
#define cd_17_log	59532
#define cd_17	59533
#define cdv_17	59534
#define cdv_18	59535
#define cdv_19	59536
#define cd_19_pal	59537
#define cd_19_log	59538
#define cdv_20	59539
#define cd_20_log	59540
#define cdv_21	59541
#define cd_21_log	59542
#define cdv_22	59545
#define cdv_23	59546
#define cd_23_pal	59547
#define cd_24_log	59550
#define cdv_24	59551
#define cdv_25	59554
#define cdv_26	59556
#define cd_27	59557
#define cdv_27	59558
#define cd_27_pal	59559
#define cd_27_log	59560
#define cdv_28	59561
#define cdv_29	59562
#define cdv_30	59563
#define cdv_31	59565
#define cdv_32	59566
#define cdv_33	59567
#define cdv_34	59568
#define cd_35	59569
#define cdv_35	59570
#define cd_35_pal	59571
#define cd_35_log	59572
#define cdv_36	59574
#define cd_37	59575
#define cdv_37	59576
#define cd_37_pal	59577
#define cd_37_log	59578
#define cdv_38	59579
#define cdv_39	59581
#define cdv_40	59583
#define cd_40_pal	59584
#define cd_40_log	59585
#define cdv_41	59587
#define cdv_42	59588
#define cd_43	59589
#define cdv_43	59590
#define cd_43_pal	59591
#define cd_43_log	59592
#define cdv_44	59594
#define cd_45	59595
#define cdv_45	59596
#define cd_45_pal	59597
#define cd_45_log	59598
#define cdv_46	59600
#define cdv_47	59602
#define cd_47_pal	59603
#define cd_47_log	59604
#define cd_48	59605
#define cdv_48	59606
#define cd_48_pal	59607
#define cd_48_log	59608
#define cd_49	59609
#define cdv_49	59610
#define cd_50	59611
#define cdv_50	59612
#define cdv_51	59613
#define cdv_52	59614
#define cdv_53	59615
#define cdv_54	59616
#define cdv_55	59618
#define cd_55_pal	59619
#define cd_55_log	59620
#define cdv_56	59621
#define cdv_57	59622
#define cd_58	59623
#define cdv_58	59624
#define cd_58_pal	59625
#define cd_58_log	59626
#define cdv_59	59627
#define cdv_60	59628
#define cdv_61	59629
#define cdv_62	59630
#define cdv_63	59631
#define cdv_64	59632
#define cdv_65	59633
#define cdv_66	59635
#define cd_66_pal	59636
#define cd_66_log	59637
#define cdv_67	59639
#define cd_67_pal	59640
#define cd_67_log	59641
#define cdv_68	59642
#define cd_69	59643
#define cdv_69	59644
#define cd_69_pal	59645
#define cd_69_log	59646
#define cdv_70	59647
#define cdv_71	59648
#define cdv_72	59649
#define cd_72_pal	59650
#define cd_72_log	59651
#define cd_73_pal	59652
#define cd_73_log	59653
#define cdv_73	59654
#define cdv_74	59655
#define cdv_75	59656
#define cd_76_pal	59657
#define cd_76_log	59658
#define cdv_76	59659
#define cdv_77	59660
#define cd_78_pal	59661
#define cd_78_log	59662
#define cdv_78	59663
#define cdv_79	59664
#define cdv_80	59665
#define cdv_81	59666
#define cdv_82	59667
#define cdv_83	59668
#define cdv_84	59669
#define cdv_85	59670
#define cdv_86	59671
#define cdv_87	59672
#define cd_100	60087
#define cd_101_log	60088
#define cd_101	60099
#define cd_102_log	60090
#define cd_102	60091
#define cd_103_pal	60092
#define cd_103_log	60093
#define cd_103	60094
#define cd_104_pal	60095
#define cd_104_log	60096
#define cd_104	60097
#define cd_105	60098

#define START_VOICE	( delay(200), _sound->playVoice(vocBuffer, loadedVocSize) )
#define START_BG	( _sound->playBgSound(bgVocBuffer, bgVocSize) )
#define LOAD_NEW_VOICE(num)	( free (vocBuffer), vocBuffer = _skyDisk->loadFile(num, NULL), loadedVocSize = _skyDisk->_lastLoadedFileSize ) 
#define LOAD_NEW_BG(num)	( free (bgVocBuffer), bgVocBuffer = _skyDisk->loadFile(num, NULL), bgVocSize = _skyDisk->_lastLoadedFileSize )
#define WAIT_VOICE	while (_sound->_voiceHandle != 0) { delay(50); }
#define WAIT_SEQUENCE	while (_tseqFrames != 0) { delay(50); }
#define WAIT_RELATIVE(x)	( delay(20 * (x)) )
#define COPY_SCREEN	( memcpy(_workScreen, workScreen2, GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT) )

void SkyState::doCDIntro() {

	uint32 loadedVocSize, bgVocSize;
	byte *vocBuffer, *bgVocBuffer, *cd2_seq_data_1, *cd2_seq_data_2;

	assert(isCDVersion(_gameVersion));
	
	vocBuffer = _skyDisk->loadFile(cdv_00, NULL);
	loadedVocSize = _skyDisk->_lastLoadedFileSize;

	_tempPal = _skyDisk->loadFile(cd_pal, NULL);
	_workScreen = _skyDisk->loadFile(cd_1_log, NULL);
	
	cd2_seq_data_1 = _skyDisk->loadFile(cd_1, NULL);
	bgVocBuffer = _skyDisk->loadFile(59499, NULL);
	bgVocSize = _skyDisk->_lastLoadedFileSize;

	delay(2000); //keep gibbons screen up for 2 seconds
	fnFadeDown(0); //and fade out

	START_VOICE;
	START_BG; 
	free (vocBuffer);

	vocBuffer = _skyDisk->loadFile(cdv_01, NULL);
	loadedVocSize = _skyDisk->_lastLoadedFileSize;
	
	WAIT_VOICE; //wait for the voice to finish
	START_VOICE;
	START_BG; 
	showScreen();
	paletteFadeUp(_tempPal);
	startTimerSequence(cd2_seq_data_1);
	LOAD_NEW_VOICE(cdv_02);
	WAIT_VOICE;
	START_VOICE;
	START_BG; 
	cd2_seq_data_2 = _skyDisk->loadFile(cd_2, NULL); //load seq 2 while 1 is playing
	LOAD_NEW_VOICE(cdv_03);
	//WAIT_SEQUENCE;
	WAIT_VOICE;
	startTimerSequence(cd2_seq_data_2); //start second sequence
	START_VOICE; //03
	START_BG;
	
	LOAD_NEW_VOICE(cdv_04);
	
	WAIT_VOICE; //03
	START_VOICE; //04
	START_BG;

	free(cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_3, NULL);
	LOAD_NEW_VOICE(cdv_05);
	
	WAIT_SEQUENCE; //2
	WAIT_VOICE; //4

	START_VOICE; //5
	START_BG;
	
	WAIT_RELATIVE(100); 

	startTimerSequence(cd2_seq_data_1);
	LOAD_NEW_VOICE(cdv_06);
	WAIT_VOICE; //5
	START_VOICE; //6
	START_BG;

	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_5, NULL);
	LOAD_NEW_VOICE(cdv_07);

	WAIT_SEQUENCE; //3
	WAIT_VOICE; //6

	START_VOICE; //7
	startTimerSequence(cd2_seq_data_2); //5
	START_BG;

	LOAD_NEW_VOICE(cdv_08);
	WAIT_VOICE; //7
	START_VOICE; //8
	START_BG;
	LOAD_NEW_VOICE(cdv_09);
	WAIT_VOICE; //8
	START_VOICE; //9
	START_BG;

	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_7, NULL);
	LOAD_NEW_VOICE(cdv_10);

	WAIT_SEQUENCE; //5
	WAIT_VOICE; //9

	START_VOICE; //10
	startTimerSequence(cd2_seq_data_1); //7
	START_BG;
	
	_skyDisk->loadFile(cd_11_pal, _tempPal);
	byte *workScreen2 = _skyDisk->loadFile(cd_11_log, NULL); //need an extra screen or else the sequence will get messed up
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_11, NULL);
	LOAD_NEW_VOICE(cdv_11);

	WAIT_VOICE; //10
	START_VOICE; //11
	START_BG;

	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	LOAD_NEW_VOICE(cdv_12);
	WAIT_SEQUENCE; //7
	WAIT_VOICE; //11
	START_VOICE; //12
	START_BG;

	WAIT_RELATIVE(80);
	startTimerSequence(cd2_seq_data_2); //11

	LOAD_NEW_VOICE(cdv_13);
	WAIT_VOICE; //12
	START_VOICE; //13
	START_BG;

	free(cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_13, NULL);
	LOAD_NEW_VOICE(cdv_14);

	LOAD_NEW_BG(59498);

	WAIT_SEQUENCE; //11
	WAIT_VOICE; //13

	START_VOICE; //14
	startTimerSequence(cd2_seq_data_1); //13
	START_BG;

	LOAD_NEW_VOICE(cdv_15);
	_skyDisk->loadFile(cd_15_pal, _tempPal);
	_skyDisk->loadFile(cd_15_log, workScreen2);

	WAIT_SEQUENCE; //13
	WAIT_VOICE; //14

	START_VOICE; //15
	START_BG;
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	LOAD_NEW_VOICE(cdv_16);
	WAIT_VOICE; //15
	START_VOICE; //16
	START_BG;

	_skyDisk->loadFile(cd_17_log, workScreen2);
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_17, NULL);
	LOAD_NEW_VOICE(cdv_17);

	WAIT_VOICE; //16
	START_VOICE; //17

	WAIT_RELATIVE(40);
	COPY_SCREEN;
	showScreen();

	LOAD_NEW_VOICE(cdv_18);
	LOAD_NEW_BG(59497); //Loud heli
	
	WAIT_VOICE; //17
	startTimerSequence(cd2_seq_data_2); //17
	START_VOICE; //18
	START_BG;

	LOAD_NEW_VOICE(cdv_19);
	_skyDisk->loadFile(cd_19_pal, _tempPal);
	_skyDisk->loadFile(cd_19_log, workScreen2);
	START_BG;
	LOAD_NEW_BG(59496); //loud heli to quiet

	WAIT_SEQUENCE; //17
	WAIT_VOICE; //18

	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	START_VOICE; //19
	START_BG;
	LOAD_NEW_VOICE(cdv_20);
	_skyDisk->loadFile(cd_20_log, workScreen2);
	LOAD_NEW_BG(59496); //quiet heli

	WAIT_VOICE; //19
	START_VOICE; //20
	START_BG;

	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	LOAD_NEW_VOICE(cdv_21);
	_skyDisk->loadFile(cd_21_log, workScreen2);

	START_BG;
	WAIT_SEQUENCE; //19
	WAIT_VOICE; //20
	START_VOICE; //21
	START_BG;
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	LOAD_NEW_VOICE(cdv_22);
	LOAD_NEW_BG(59494); //heli whine

	WAIT_SEQUENCE; //20
	WAIT_VOICE; //21

	START_VOICE; //22
	START_BG;
	LOAD_NEW_VOICE(cdv_23);
	WAIT_VOICE; //22
	START_VOICE; //23
	fnFadeDown(0);

	_skyDisk->loadFile(cd_23_pal, _tempPal);
	_skyDisk->loadFile(cd_24_log, workScreen2);
	LOAD_NEW_VOICE(cdv_24);
	WAIT_VOICE; //23

	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	START_VOICE; //24
	showScreen();
	LOAD_NEW_VOICE(cdv_25);
	WAIT_VOICE; //24
	START_VOICE; //25
	LOAD_NEW_VOICE(cdv_26);
	WAIT_VOICE; //25
	START_VOICE; //26

	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_27, NULL);
	LOAD_NEW_VOICE(cdv_27);
	_skyDisk->loadFile(cd_27_pal, _tempPal);
	_skyDisk->loadFile(cd_27_log, workScreen2);
	WAIT_VOICE; //26
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();	
	paletteFadeUp(_tempPal);
	START_VOICE; //27
	LOAD_NEW_VOICE(cdv_29);
	WAIT_VOICE; //27
	START_VOICE; //29
	LOAD_NEW_VOICE(cdv_30);
	WAIT_VOICE; //29
	START_VOICE; //30
	LOAD_NEW_VOICE(cdv_31);
	WAIT_VOICE; //30
	startTimerSequence(cd2_seq_data_1);
	START_VOICE; //31
	LOAD_NEW_VOICE(cdv_32);
	WAIT_VOICE; //31
	START_VOICE; //32
	LOAD_NEW_VOICE(cdv_33);
	WAIT_VOICE; //32
	START_VOICE; //33
	LOAD_NEW_VOICE(cdv_34);
	WAIT_VOICE; //33
	START_VOICE; //34
	LOAD_NEW_VOICE(cdv_35);
	WAIT_SEQUENCE; //27
	WAIT_VOICE; //34
	START_VOICE; //35

	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_35, NULL);
	LOAD_NEW_VOICE(cdv_36);
	_skyDisk->loadFile(cd_35_pal, _tempPal);
	_skyDisk->loadFile(cd_35_log, workScreen2);
	WAIT_VOICE; //35
	START_VOICE; //36
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	
	LOAD_NEW_VOICE(cdv_37);
	WAIT_VOICE; //36
	startTimerSequence(cd2_seq_data_2);
	START_VOICE; //37

	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_37, NULL);
	LOAD_NEW_VOICE(cdv_38);

	WAIT_SEQUENCE; //35
	WAIT_VOICE; //37
	START_VOICE; //38
	startTimerSequence(cd2_seq_data_1);
	LOAD_NEW_VOICE(cdv_39);
	WAIT_SEQUENCE; //37
	WAIT_VOICE; //38
	START_VOICE; //39

	LOAD_NEW_VOICE(cdv_40);
	_skyDisk->loadFile(cd_40_pal, _tempPal);
	_skyDisk->loadFile(cd_40_log, workScreen2);
	WAIT_VOICE; //39
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	START_VOICE; //40
	LOAD_NEW_VOICE(cdv_41);
	WAIT_VOICE; //40
	START_VOICE; //41
	LOAD_NEW_VOICE(cdv_42);
	WAIT_VOICE; //41
	START_VOICE; //42
	LOAD_NEW_VOICE(cdv_43);

	_skyDisk->loadFile(cd_43_pal, _tempPal);
	_skyDisk->loadFile(cd_43_log, workScreen2);
	WAIT_VOICE; //42

	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	START_VOICE; //43
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_43, NULL);
	WAIT_VOICE; //43
	startTimerSequence(cd2_seq_data_2);
	LOAD_NEW_VOICE(cdv_45);
	_skyDisk->loadFile(cd_45_pal, _tempPal);
	_skyDisk->loadFile(cd_45_log, workScreen2);
	WAIT_SEQUENCE; //43
	START_VOICE; //45
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_45, NULL);
	LOAD_NEW_VOICE(cdv_46);
	WAIT_VOICE; //45
	startTimerSequence(cd2_seq_data_1);
	START_VOICE; //46
	LOAD_NEW_VOICE(cdv_47);

	_skyDisk->loadFile(cd_47_pal, _tempPal);
	_skyDisk->loadFile(cd_47_log, workScreen2);

	WAIT_SEQUENCE; //45
	WAIT_VOICE; //46

	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	START_VOICE; //47
	LOAD_NEW_VOICE(cdv_48);
	_skyDisk->loadFile(cd_48_pal, _tempPal);
	_skyDisk->loadFile(cd_48_log, workScreen2);
	WAIT_VOICE; //47
	START_VOICE; //48
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_48, NULL);
	LOAD_NEW_VOICE(cdv_49);
	WAIT_VOICE; //48
	startTimerSequence(cd2_seq_data_2);
	START_VOICE; //49
	LOAD_NEW_VOICE(cdv_50);
	WAIT_VOICE; //49
	START_VOICE; //50
	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_49, NULL);
	LOAD_NEW_VOICE(cdv_51);
	WAIT_SEQUENCE; //48
	WAIT_VOICE; //50
	START_VOICE; //51
	startTimerSequence(cd2_seq_data_1);
	LOAD_NEW_VOICE(cdv_52);
	WAIT_VOICE; //51
	START_VOICE; //52
	LOAD_NEW_VOICE(cdv_53);
	WAIT_VOICE; //52
	START_VOICE; //53
	LOAD_NEW_VOICE(cdv_54);
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_50, NULL);
	WAIT_VOICE; //53
	WAIT_SEQUENCE; //49

	START_VOICE; //54
	startTimerSequence(cd2_seq_data_2);
	LOAD_NEW_VOICE(cdv_55);
	WAIT_SEQUENCE; //50
	WAIT_VOICE; //54
	START_VOICE; //55

	_skyDisk->loadFile(cd_55_pal, _tempPal);
	_skyDisk->loadFile(cd_55_log, workScreen2);
	LOAD_NEW_VOICE(cdv_56);
	WAIT_VOICE; //55
	START_VOICE; //56

	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	LOAD_NEW_VOICE(cdv_57);
	WAIT_VOICE; //56
	START_VOICE; //57

	LOAD_NEW_VOICE(cdv_58);
	_skyDisk->loadFile(cd_58_pal, _tempPal);
	_skyDisk->loadFile(cd_58_log, workScreen2);

	WAIT_VOICE; //57
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	START_VOICE; //58
	LOAD_NEW_VOICE(cdv_59);
	WAIT_VOICE; //48
	START_VOICE; //59
	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_58, NULL);
	WAIT_VOICE; //59
	LOAD_NEW_VOICE(cdv_60);
	START_VOICE; //60
	LOAD_NEW_VOICE(cdv_61);
	WAIT_VOICE; //60
	START_VOICE; //61
	LOAD_NEW_VOICE(cdv_62);
	WAIT_VOICE; //61
	START_VOICE; //62
	startTimerSequence(cd2_seq_data_1); //58
	LOAD_NEW_VOICE(cdv_63);
	WAIT_VOICE; //62
	START_VOICE; //63
	LOAD_NEW_VOICE(cdv_64);
	WAIT_VOICE; //63
	START_VOICE; //64
	LOAD_NEW_VOICE(cdv_65);
	WAIT_SEQUENCE; //58
	WAIT_VOICE; //64
	START_VOICE; //65
	fnFadeDown(0);
	LOAD_NEW_VOICE(cdv_66);
	_skyDisk->loadFile(cd_66_pal, _tempPal);
	_skyDisk->loadFile(cd_66_log, _workScreen);
	WAIT_VOICE; //65
	showScreen();
	paletteFadeUp(_tempPal);
	START_VOICE; //66
	LOAD_NEW_VOICE(cdv_67);
	WAIT_VOICE; //66
	START_VOICE; //67
	_skyDisk->loadFile(cd_67_pal, _tempPal);
	_skyDisk->loadFile(cd_67_log, workScreen2);
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	LOAD_NEW_VOICE(cdv_68);
	WAIT_VOICE; //67
	START_VOICE; //68
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_69, NULL);
	LOAD_NEW_VOICE(cdv_69);
	_skyDisk->loadFile(cd_69_pal, _tempPal);
	_skyDisk->loadFile(cd_69_log, workScreen2);
	WAIT_VOICE; //68
	START_VOICE; //69
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	LOAD_NEW_VOICE(cdv_70);
	WAIT_VOICE; //69
	startTimerSequence(cd2_seq_data_2);
	START_VOICE; //70
	LOAD_NEW_VOICE(cdv_71);
	WAIT_VOICE; //70
	fnFadeDown(0);
	START_VOICE; //71
	_skyDisk->loadFile(cd_72_pal, _tempPal);
	_skyDisk->loadFile(cd_72_log, _workScreen);
	WAIT_VOICE; //71
	showScreen();
	paletteFadeUp(_tempPal);
	LOAD_NEW_VOICE(cdv_72);
	START_VOICE; //72

	_skyDisk->loadFile(cd_73_pal, _tempPal);
	_skyDisk->loadFile(cd_73_log, _workScreen);
	LOAD_NEW_VOICE(cdv_73);
	WAIT_VOICE; //72
	fnFadeDown(0);
	showScreen();
	paletteFadeUp(_tempPal);
	START_VOICE; //73
	LOAD_NEW_VOICE(cdv_74);
	WAIT_VOICE; //73
	START_VOICE; //74
	LOAD_NEW_VOICE(cdv_75);
	WAIT_VOICE; //74
	START_VOICE; //75
	_skyDisk->loadFile(cd_76_pal, _tempPal);
	free (workScreen2);
	workScreen2 = _skyDisk->loadFile(cd_76_log, NULL);
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);

	LOAD_NEW_VOICE(cdv_76);
	WAIT_VOICE; //75
	START_VOICE; //76
	LOAD_NEW_VOICE(cdv_77);
	WAIT_VOICE; //76
	START_VOICE; //77

	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_100, NULL);
	_skyDisk->loadFile(cd_78_pal, _tempPal);
	free (workScreen2);
	workScreen2 = _skyDisk->loadFile(cd_78_log, NULL);
	LOAD_NEW_VOICE(cdv_78);
	WAIT_VOICE; //77
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();	
	paletteFadeUp(_tempPal);
	START_VOICE; //78
	LOAD_NEW_VOICE(cdv_79);
	WAIT_VOICE; //78
	START_VOICE; //79
	LOAD_NEW_VOICE(cdv_80);
	WAIT_VOICE; //79
	START_VOICE; //80
	startTimerSequence(cd2_seq_data_1);
	LOAD_NEW_VOICE(cdv_81);
	WAIT_VOICE; //80
	START_VOICE; //81
	LOAD_NEW_VOICE(cdv_82);
	WAIT_VOICE; //81
	START_VOICE; //82
	LOAD_NEW_VOICE(cdv_83);
	WAIT_VOICE; //82
	_skyDisk->loadFile(cd_101_log, workScreen2);
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_101, NULL);
	WAIT_SEQUENCE; //100
	COPY_SCREEN;
	showScreen();
	startTimerSequence(cd2_seq_data_2);
	START_VOICE; //83
	LOAD_NEW_VOICE(cdv_84);
	WAIT_VOICE; //83
	START_VOICE; //84
	LOAD_NEW_VOICE(cdv_85);
	WAIT_VOICE; //84
	START_VOICE; //85
	LOAD_NEW_VOICE(cdv_86);
	WAIT_VOICE; //85

	free (workScreen2);
	workScreen2 = _skyDisk->loadFile(cd_102_log, NULL);
	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_102, NULL);
	WAIT_SEQUENCE; //101
	COPY_SCREEN;
	showScreen();
	startTimerSequence(cd2_seq_data_1);
	START_VOICE; //86
	LOAD_NEW_VOICE(cdv_87);
	_skyDisk->loadFile(cd_103_pal, _tempPal);
	free (workScreen2);
	workScreen2 = _skyDisk->loadFile(cd_103_log, NULL);
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_103, NULL);
	WAIT_SEQUENCE; //102
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	startTimerSequence(cd2_seq_data_2);
	WAIT_VOICE; //86
	START_VOICE; //87
	_skyDisk->loadFile(cd_104_pal, _tempPal);
	free (workScreen2);
	workScreen2 = _skyDisk->loadFile(cd_104_log, NULL);
	free (cd2_seq_data_1);
	cd2_seq_data_1 = _skyDisk->loadFile(cd_104, NULL);
	WAIT_SEQUENCE; //103

	//fn_start_music(2);
	fnFadeDown(0);
	COPY_SCREEN;
	showScreen();
	paletteFadeUp(_tempPal);
	startTimerSequence(cd2_seq_data_1);
	free (cd2_seq_data_2);
	cd2_seq_data_2 = _skyDisk->loadFile(cd_105, NULL);
	WAIT_SEQUENCE; //104
	startTimerSequence(cd2_seq_data_2);
	WAIT_SEQUENCE; //105
	
	free (cd2_seq_data_1);
	free (cd2_seq_data_2);
	free (workScreen2);
}

