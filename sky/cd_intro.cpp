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


void SkyState::doCDIntro() {

	uint32 loadedVocSize, bgVocSize;
	byte *vocBuffer, *bgVocBuffer, *cd2_seq_data_1;

	vocBuffer = (byte *)loadFile(cdv_00, NULL);
	loadedVocSize = _lastLoadedFileSize;

	_tempPal = (byte *)loadFile(cd_pal, NULL);
	_workScreen = (byte *)loadFile(cd_1_log, NULL);
	
	cd2_seq_data_1 = (byte *)loadFile(cd_1, NULL);
	bgVocBuffer = (byte *)loadFile(59499, NULL);
	bgVocSize = _lastLoadedFileSize;

	delay(2000); //keep gibbons screen up for 2 seconds
	fn_fade_down(0); //and fade out

	_sound->playVoice(vocBuffer, loadedVocSize);
	_sound->playVoice(bgVocBuffer, bgVocSize);
	free (vocBuffer);

	vocBuffer = (byte *)loadFile(cdv_01, NULL);
	loadedVocSize = _lastLoadedFileSize;
	
	//waitForVoc(0);
	delay(5500); //hack!

	_sound->playVoice(vocBuffer, loadedVocSize);
	_sound->playVoice(bgVocBuffer, bgVocSize);
	free (vocBuffer);	
	
	showScreen();
	paletteFadeUp(_tempPal);
}


