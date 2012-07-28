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
 */

#ifndef TEENAGENT_RESOURCES_H
#define TEENAGENT_RESOURCES_H

#include "teenagent/pack.h"
#include "teenagent/segment.h"
#include "teenagent/font.h"
#include "graphics/surface.h"

struct ADGameDescription;

namespace TeenAgent {

// Code Segment Addresses (Read Only)
// Intro function : 0x024c
const uint16 csAddr_intro = 0x024c;
// Pole Climb Fail function : 0x4173
const uint16 csAddr_poleClimbFail = 0x4173;
// Move Ego (Mark) To Suspicious Position function : 0x505c
const uint16 csAddr_egoSuspiciousPosition = 0x505c;
// Guard Drinking function : 0x5189
const uint16 csAddr_guardDrinking = 0x5189;
// Move Ego (Mark) To Default Position function : 0x557e
const uint16 csAddr_egoDefaultPosition = 0x557e;
// Cave NOP function : 0x599b
const uint16 csAddr_caveNOP = 0x599b;
// Enter Cave function : 0x5a21
const uint16 csAddr_enterCave = 0x5a21;
// Ego (Mark) Scared By Spider function : 0x60b5
const uint16 csAddr_egoScaredBySpider = 0x60b5;
// Move to Ladder and Leave Cellar function : 0x60d9
const uint16 csAddr_moveToLadderAndLeaveCellar = 0x60d9;
// Leave Cellar function : 0x612b
const uint16 csAddr_leaveCellar = 0x612b;
// Too Dark function : 0x61fe
const uint16 csAddr_TooDark = 0x61fe;
// Move Ego (Mark) To Bottom-Right or Turn function : 0x6849
const uint16 csAddr_egoBottomRightTurn = 0x6849;
// Checking Drawers function : 0x68e6
const uint16 csAddr_checkingDrawers = 0x68e6;
// Drawer Open Message function : 0x6b86
const uint16 csAddr_DrawerOpenMessage = 0x6b86;
// Is Cook Gone function : 0x70e0
const uint16 csAddr_isCookGone = 0x70e0;
// Giving Flower to Old Lady function : 0x88de
const uint16 csAddr_givingFlowerToOldLady = 0x88de;
// Give Another Flower to Old Lady function : 0x890b
const uint16 csAddr_giveAnotherFlowerToOldLady = 0x890b;
// Giving Flower to Anne function : 0x8942
const uint16 csAddr_givingFlowerToAnne = 0x8942;
// Give Another Flower to Anne function : 0x89aa
const uint16 csAddr_giveAnotherFlowerToAnne = 0x89aa;
// Putting Rock in Hole function : 0x8d57
const uint16 csAddr_putRockInHole = 0x8d57;
// Robot Safe (Mike) Already Unlocked Check function : 0x9166
const uint16 csAddr_robotSafeAlreadyUnlockedCheck = 0x9166;
// Robot Safe (Mike) Unlock Check function : 0x9175
const uint16 csAddr_robotSafeUnlockCheck = 0x9175;
// Successfully Got Anchor function : 0x99e0
const uint16 csAddr_gotAnchor = 0x99e0;
// Get Out of Lake function : 0x9a7a
const uint16 csAddr_getOutOfLake = 0x9a7a;
// Mansion Intrusion Attempt function : 0x9d45
const uint16 csAddr_mansionIntrusionAttempt = 0x9d45;
// Second Mansion Intrusion function : 0x9d90
const uint16 csAddr_secondMansionIntrusion = 0x9d90;
// Third Mansion Intrusion function : 0x9de5
const uint16 csAddr_thirdMansionIntrusion = 0x9de5;
// Fourth Mansion Intrusion function : 0x9e54
const uint16 csAddr_fourthMansionIntrusion = 0x9e54;
// Fifth Mansion Intrusion function : 0x9ec3
const uint16 csAddr_fifthMansionIntrusion = 0x9ec3;
// Sixth Mansion Intrusion function : 0x9f3e
const uint16 csAddr_sixthMansionIntrusion = 0x9f3e;
// Display Message function : 0xa055
const uint16 csAddr_displayMsg = 0xa055;
// Reject Message function : 0xa4d6
const uint16 csAddr_rejectMsg = 0xa4d6;

// Data Segment Addresses
// Cursor Graphic 8*12px : 0x00da to 0x0139 (Read Only)
const uint16 dsAddr_cursor = 0x00da;

// Reject Message Address Pointers : (4 * 2-byte) = 0x339e to 0x33a5
const uint16 dsAddr_rejectMsgPtr = 0x339e;
// Reject Message #0 : 0x33a6 to 0x33c9
const uint16 dsAddr_rejectMsg0 = 0x33a6; // "I have no idea what to do with it"
// Reject Message #1 : 0x33ca to 0x33f5
const uint16 dsAddr_rejectMsg1 = 0x33ca; // "I can't imagine what I could do with this"
// Reject Message #2 : 0x33f6 to 0x3425
const uint16 dsAddr_rejectMsg2 = 0x33f6; // "I can't figure out what I should do with this"
// Reject Message #3 : 0x3426 to 0x344f
const uint16 dsAddr_rejectMsg3 = 0x3426; // "I can't find any reason to mess with it"

// Object Usage Error Message : 0x3457 to 0x3467
const uint16 dsAddr_objErrorMsg = 0x3457; // "That's no good"
// Car Jack Message : 0x3468 to 0x348f
const uint16 dsAddr_carJackMsg = 0x3468; // "Wow! There's a car jack inside! Great!"
// Spanner Message : 0x3490 to 0x34c6
const uint16 dsAddr_spannerMsg = 0x3490; // "There's something else inside the toolbox! A spanner!"
// Last Chance Message : 0x34c7 to 0x34d4
const uint16 dsAddr_lastChanceMsg = 0x34c7; // "Last chance?"
// Give Up Message : 0x34d5 to 0x34e0
const uint16 dsAddr_giveUpMsg = 0x34d5; // "I give up"
// Avoid Bees Message : 0x34e1 to 0x351e
const uint16 dsAddr_avoidBeesMsg = 0x34e1; // "I'm going to stay at least five meters away from these bees!"
// Boat Empty Message : 0x351f to 0x3541
const uint16 dsAddr_boatEmptyMsg = 0x351f; // "There's nothing else in the boat"
// Too Hard Wood Message : 0x3542 to 0x3562
const uint16 dsAddr_tooHardWoodMsg = 0x3542; // "This wood is too hard to break"
// Boo Message : 0x3563 to 0x3569
const uint16 dsAddr_BooMsg = 0x3563; // "Booo!"
// Dont Push Luck Message : 0x356a to 0x358f
const uint16 dsAddr_dontPushLuckMsg = 0x356a; // "I don't think I should push my luck"
// Ordinary Haystack Message : 0x3590 to 0x35b1
const uint16 dsAddr_ordinaryHaystackMsg = 0x3590; // "Just an ordinary hay stack. Now."
// Needle Haystack Message : 0x35b2 to 0x35e7
const uint16 dsAddr_needleHaystackMsg = 0x35b2; // "And they say you can't find a needle in a haystack"
// No Potatoes Message : 0x35e8 to 0x3604
const uint16 dsAddr_noPotatoMsg = 0x35e8; // "There are no more potatoes"
// Trousers Message : 0x3605 to 0x363e
const uint16 dsAddr_trousersMsg = 0x3605; // "Good I always asked mum for trousers with BIG pockets"
// Life Is Brutal Message : 0x363f to 0x364f
const uint16 dsAddr_lifeIsBrutalMsg = 0x363f; // "Life is brutal"
// Life Brutal Message : 0x3650 to 0x3667
const uint16 dsAddr_lifeBrutalMsg = 0x3650; // "Life is really brutal"
// Tickled Message : 0x3668 to 0x367e
const uint16 dsAddr_tickledMsg = 0x3668; // "Something tickled me!"
// Its Gone Message : 0x367f to 0x3693
const uint16 dsAddr_itsGoneMsg = 0x367f; // "At least it's gone"
// Monsters Message : 0x3694 to 0x36c1
const uint16 dsAddr_monstersMsg = 0x3694; // "Who knows what monsters live in there"
// No Hands Message : 0x36c2 to 0x370e
const uint16 dsAddr_noHandsMsg = 0x36c2; // "I'd better not put my hands in there..."
// Totally Empty Message : 0x370f to 0x372d
const uint16 dsAddr_totalEmptyMsg = 0x370f; // "I can see it's totally empty"
// One Small Step Message : 0x372e to 0x3765
const uint16 dsAddr_oneSmallStepMsg = 0x372e; // "One small step for man, one big pain in the head"
// No Chance Message : 0x3766 to 0x378f
const uint16 dsAddr_noChanceMsg = 0x3766; // "I won't take my chances a second time"
// Dinosaur Bone Message : 0x3790 to 0x37b7
const uint16 dsAddr_dinoBoneMsg = 0x3790; // "I really hope this is DINOSAUR bone"
// Wall Shaken Message : 0x37b8 to 0x37e9
const uint16 dsAddr_wallShakenMsg = 0x37b8; // "Wow! This must have shaken all the nearby walls!"
// Kinda Dark Message : 0x37ea to 0x3800
const uint16 dsAddr_kindaDarkMsg = 0x37ea; // "It's kinda dark here"
// Not in Dark Message : 0x3801 to 0x3831
const uint16 dsAddr_notInDarkMsg = 0x3801; // "I'm not going to wander here in the dark again"
// Shut Valve Message : 0x3832 to 0x387b
const uint16 dsAddr_shutValveMsg = 0x3832; // "Shutting the valve shook the dirt from the wall..."
// Need Sunglasses Message : 0x387c to 0x38a6
const uint16 dsAddr_needSunglassesMsg = 0x387c; // "Sorry buddy, but I need your sunglasses"
// Not Best Place Message : 0x38a7 to 0x38cd
const uint16 dsAddr_notBestPlaceMsg = 0x38a7; // "It's not the best place for diving"
// Not Here Message : 0x38ce to 0x38da
const uint16 dsAddr_notHereMsg = 0x38ce; // "Not here"

// Hooked Anchor Message : 0x3989 to 0x39ad
const uint16 dsAddr_hookedAnchorMsg = 0x3989; // "I was really hooked on this anchor!"
// Seaweed Message : 0x39ae to 0x39f5
const uint16 dsAddr_seaweedMsg = 0x39ae; // "This seaweed is just like the flowers I gave mum on her last birthday"
// Fish Boat Message : 0x39f6 to 0x3a27
const uint16 dsAddr_fishBoatMsg = 0x39f6; // "I wonder what fish do inside this boat at night"
// Fish Something Message : 0x3a28 to 0x3a59
const uint16 dsAddr_fishSomethingMsg = 0x3a28; // "I think I have to fish out something down there"

// Not Red Herring Message : 0x3a85 to 0x3ab6
const uint16 dsAddr_notRedHerringMsg = 0x3a85; // "I hope all this fish stuff is not a red herring"
// Nice Down Message : 0x3ab7 to 0x3acd
const uint16 dsAddr_niceDownMsg = 0x3ab7; // "It's nice down there"
// Hey Let Go Message : 0x3ace to 0x3ae5
const uint16 dsAddr_heyLetGoMsg = 0x3ace; // "Hey, let go, will ya?!"
// Aaahhh Message : 0x3ae6 to 0x3afc
const uint16 dsAddr_aaahhhMsg = 0x3ae6; // "Aaaaaaaaaaaaahhh!"
// Oops Message : 0x3afd to 0x3b03
const uint16 dsAddr_oopsMsg = 0x3afd; // "Oops"
// Found Food Message : 0x3b04 to 0x3b2e
const uint16 dsAddr_foundFoodMsg = 0x3b04; // "People leave food in unbelievable places"
// Come Here Message : 0x3b2f to 0x3b58
const uint16 dsAddr_comeHereMsg = 0x3b2f; // "Come here, I've got something for you"
// Cant Catch Message : 0x3b59 to 0x3b6b
const uint16 dsAddr_cantCatchMsg = 0x3b59; // "I can't catch it!"
// Trapped Mouse Message : 0x3b6c to 0x3b82
const uint16 dsAddr_trappedMouseMsg = 0x3b6c; // "The mouse is trapped!"
// Yikes Message : 0x3b83 to 0x3b8a
const uint16 dsAddr_yikesMsg = 0x3b83; // "Yikes!"
// Mouse Nerve Message : 0x3b8b to 0x3bab
const uint16 dsAddr_mouseNerveMsg = 0x3b8b; // "Boy, this mouse has some nerve!"
// Drawers Empty Message : 0x3bac to 0x3bd1
const uint16 dsAddr_drawersEmptyMsg = 0x3bac; // "There's nothing else in the drawers"
// Rid Bush Message 0x3bd2 to 0x3bf5
const uint16 dsAddr_ridBushMsg = 0x3bd2; // "I must get rid of this bush first"
// Mouse Gone Message : 0x3bf6 to 0x3c0a
const uint16 dsAddr_mouseGoneMsg = 0x3bf6; // "The mouse has gone!"
// Nonsense Message : 0x3c0b to 0x3c15
const uint16 dsAddr_nonsenseMsg = 0x3c0b; // "Nonsense"
// Good Doggy Message : 0x3c16 to 0x3c30
const uint16 dsAddr_goodDoggyMsg = 0x3c16; // "I understand. Good doggy"
// Here Boy Message : 0x3c31 to 0x3c3c
const uint16 dsAddr_hereBoyMsg = 0x3c31; // "Here, boy"
// Friends Now Message : 0x3c3d to 0x3c57
const uint16 dsAddr_friendsNowMsg = 0x3c3d; // "I hope we're friends now"
// Not Think Right Place Message : 0x3c58 to 0x3c7f
const uint16 dsAddr_notThinkRightPlaceMsg = 0x3c58; // "I don't think this is the right place"
// Cutscene Message A : 0x3c80 to 0x3c99
const uint16 dsAddr_cutsceneMsgA = 0x3c80; // "Hundred moments later"
// Cutscene Message B : 0x3c9a to 0x3cbb
const uint16 dsAddr_cutsceneMsgB = 0x3c9a; // "Another hundred moments later"
// Found Crude Oil Message : 0x3cbc to 0x3ce9
const uint16 dsAddr_foundCrudeOilMsg = 0x3cbc; // "At least I found crude oil and I'll be rich"
// My Life Message : 0x3cea to 0x3cfa
const uint16 dsAddr_myLifeMsg = 0x3cea; // "That's my life"
// Confusion Message : 0x3cfb to 0x3d00
const uint16 dsAddr_ConfusionMsg = 0x3cfb; // "!?&!"
// Grandpa Promise Message : 0x3d01 to 0x3d1f
const uint16 dsAddr_grandpaPromiseMsg = 0x3d01; // "But grandpa, you promised!"
// Oh Lets Go Message : 0x3d20 to 0x3d39
const uint16 dsAddr_ohLetsGoMsg = 0x3d20; // "Oh all right. Let's go"
// Bye Message : 0x3d3a to 0x3d3f
const uint16 dsAddr_byeMsg = 0x3d3a; // "Bye."
// No Need Message : 0x3d40 to 0x3d58
const uint16 dsAddr_noNeedMsg = 0x3d40; // "No need to do it again"
// Girl Talk Message : 0x3d59 to 0x3d85
const uint16 dsAddr_girlTalkMsg = 0x3d59; // "I really don't know how to talk to girls"
// Dont Work Purpose Message : 0x3d86 to 0x3dae
const uint16 dsAddr_dontWorkPurposeMsg = 0x3d86; // "I usually don't work without a purpose"
// Nut Real Message : 0x3daf to 0x3dc5
const uint16 dsAddr_nutRealMsg = 0x3daf; // "Only the nut is real"
// Hen Fly Message : 0x3dc6 to 0x3df3
const uint16 dsAddr_henFlyMsg = 0x3dc6; // "I wonder if hens can fly. Come here, baby"
// First Test Fail Message : 0x3df4 to 0x3e07
const uint16 dsAddr_firstTestFailMsg = 0x3df4; // "First test failed"
// Rid Frustations Message : 0x3e08 to 3e30
const uint16 dsAddr_ridFrustationsMsg = 0x3e08; // "I'd already got rid of my frustrations"
// Road Nowhere Message : 0x3e31 to 0x3e4e
const uint16 dsAddr_roadNowhereMsg = 0x3e31; // "Nah. It's a road to nowhere"
// Open Boot Message 0x3e4f to 0x3e62
const uint16 dsAddr_openBootMsg = 0x3e4f; // "It opens the boot"
// Shut Tight Message : 0x3e63 to 0x3e74
const uint16 dsAddr_shutTightMsg = 0x3e63; // "It's shut tight"
// Boot Empty Message : 0x3e75 to 0x3e97
const uint16 dsAddr_bootEmptyMsg = 0x3e75; // "There's nothing else in the boot"
// Clothes Dry Message : 0x3e98 to 0x3eb1
const uint16 dsAddr_clothesDryMsg = 0x3e98; // "The clothes are dry now."
// Crow Kill Message : 0x3eb2 to 0x3ed5
const uint16 dsAddr_crowKillMsg = 0x3eb2; // "I'm sure these crows will kill me"

// Not Hungry Message : 0x402e to 0x4046
const uint16 dsAddr_notHungryMsg = 0x402e; // "Thanks, I'm not hungry"

// Echo Message : 0x408a to 0x4090
const uint16 dsAddr_echoMsg = 0x408a; // "Echo!"
// Loud Echo Message : 0x4091 to 0x4097
const uint16 dsAddr_loudEchoMsg = 0x4091; // "ECHO!"
// Who There Message : 0x4098 to 0x40a6
const uint16 dsAddr_whoThereMsg = 0x4098; // "Who's there?!"
// Loud Who There Message : 0x40a7 to 0x40b5
const uint16 dsAddr_loudWhoThereMsg = 0x40a7; // "WHO'S THERE?!"
// Dont Copy Message : 0x40b6 to 0x40cd
const uint16 dsAddr_dontCopyMsg = 0x40b6; // "DON'T COPY ME!"
// Loud Dont Copy Message : 0x40ce to 0x40e7
const uint16 dsAddr_loudDontCopyMsg = 0x40ce; // "DON'T COPY ME!!!"
// Throw Rock Message : 0x40e8 to 0x410e
const uint16 dsAddr_throwRockMsg = 0x40e8; // "OR I WILL THROW A ROCK DOWN THERE!"
// Or I Will Message : 0x410f to 0x411c
const uint16 dsAddr_orIWillMsg = 0x410f; // "OR I WILL"
// Still There Message : 0x411d to 0x4132
const uint16 dsAddr_stillThereMsg = 0x411d; // "Are you still there?"

// Hmm Grass Message : 0x417e to 41b0
const uint16 dsAddr_hmmGrassMsg = 0x417e; // "Hmmm. Grass..."
// Find Nut Message : 0x41b1 to 0x41ee
const uint16 dsAddr_findNutMsg = 0x41b1; // "I won't find the nut just like that. The grass is too dense"

// No Fruit Message : 0x4380 to 0x43ab
const uint16 dsAddr_noFruitMsg = 0x4380; // "There are no more interesting fruits here"

// Mirror Mirror Message : 0x4412 to 0x444e
const uint16 dsAddr_mirrorMirrorMsg = 0x4412; // "Mirror, Mirror on the wall...."
// Think Too Long Message : 0x444f to 0x446a
const uint16 dsAddr_thinkTooLongMsg = 0x444f; // "Hey, don't think too long"
// Hint Male Message : 0x446b to 0x4491
const uint16 dsAddr_HintMaleMsg = 0x446b; // "A hint: Someone in this room, a male"
// OK Wait Message : 0x4492 to 0x44a6
const uint16 dsAddr_okWaitMsg = 0x4492; // "OK, take your time"
// Busy Thinking Message : 0x44a7 to 0x44d5
const uint16 dsAddr_busyThinkingMsg = 0x44a7; // "I'd better not interrupt it's thought process"
// No Dentists Message : 0x44d6 to 0x450d
const uint16 dsAddr_noDentistsMsg = 0x44d6; // "I don't want to have anything in common with dentists"
// Too Heavy Message : 0x450e to 0x4531
const uint16 dsAddr_tooHeavyMsg = 0x450e; // "It's too heavy. Not that I'm wimp"
// What Got Message : 0x4532 to 0x4554
const uint16 dsAddr_whatGotMsg = 0x4532; // "Let's look what we've got here"
// Strawberry Jam Message : 0x4555 to 0x4567
const uint16 dsAddr_strawberryJamMsg = 0x4555; // "Strawberry jam"
// Gooseberry Jam Message : 0x4568 to 0x457a
const uint16 dsAddr_gooseberryJamMsg = 0x4568; // "Gooseberry jam"
// Blackberry Jam Message : 0x457b to 0x458d
const uint16 dsAddr_blackberryJamMsg = 0x457b; // "Blackberry jam"
// Bilberry Jam Message : 0x458e to 0x459e
const uint16 dsAddr_bilberryJamMsg = 0x458e; // "Bilberry jam"
// Get Me Out Jam Message : 0x459f to 0x45b7
const uint16 dsAddr_getMeOutJamMsg = 0x459f; // "Get me out of this jam!"
// Rosemary Jam Message : 0x45b8 to 0x45d9
const uint16 dsAddr_rosemaryJamMsg = 0x45b8; // "Oh, and there is Rosemary jam"
// Know Rosemary Message : 0x45da to 0x4602
const uint16 dsAddr_knowRosemaryMsg = 0x45da; // "I used to know someone called Rosemary"
// Unwanted Jams Message : 0x4603 to 0x461c
const uint16 dsAddr_unwantedJamsMsg = 0x4603; // "I don't want those jams"
// Too Dark Message : 0x461d to 0x463b
const uint16 dsAddr_TooDarkMsg = 0x461d; // "It's too dark to see clearly"
// Yeow Message : 0x463c to 0x4649
const uint16 dsAddr_yeowMsg = 0x463c; // "YEEEOOOWWWW!"
// Yawn Message : 0x464a to 0x4651
const uint16 dsAddr_yawnMsg = 0x464a; // "(yawn)"
// Laughter Message : 0x4652 to 0x465d
const uint16 dsAddr_laughterMsg = 0x4652; // "(laughter)"

// Bird Attack Message : 0x47e7 to 0x4807
const uint16 dsAddr_birdAttackMsg = 0x47e7; // "Hey You! Wake up! Bird attack!"
// No Search Warrant Message : 0x4808 to 0x4827
const uint16 dsAddr_noSearchWarrantMsg = 0x4808; // "I don't have a search-warrant"

// Nut Rake Message : 0x4882 to 0x48be
const uint16 dsAddr_nutRakeMsg = 0x4882; // "It's pointless, the nut will slip between the rake's teeth"

// Bribe Message : 0x49ae to 0x49d0
const uint16 dsAddr_BribeMsg = 0x49ae; // "Here, let's make your pocket fat."
// Bank Note Message : 0x49d1 to 0x4a28
const uint16 dsAddr_bankNoteMsg = 0x49d1; // "It's a note from some bank..."
// Show Her Money Message : 0x4a29 to 0x4a5a
const uint16 dsAddr_showHerMoneyMsg = 0x4a29; // "If I just show her the money, she might take it"
// Hundred Bucks Message : 0x4a5b to 0x4a6e
const uint16 dsAddr_hundredBucksMsg = 0x4a5b; // "A hundred bucks!!!"
// Want Blood Message : 0x4a6f to 0x4a7d
const uint16 dsAddr_wantBloodMsg = 0x4a6f; // "I want Blood!"
// Dont Leave Mansion Message : 0x4a7e to 0x4aaf
const uint16 dsAddr_dontLeaveMansionMsg = 0x4a7e; // "I don't want to leave the mansion, I want blood!"
// Wimp Message : 0x4ab0 to 0x4acc
const uint16 dsAddr_WimpMsg = 0x4ab0; // "I'm a pathetic little wimp"
// Strange Drawer Message : 0x4acd to 0x4b0c
const uint16 dsAddr_strangeDrawerMsg = 0x4acd; // "Strange, but the drawer is stuck if the next drawer is open"
// Not Ordinary Drawers Message : 0x4b0d to 0x4b38
const uint16 dsAddr_notOrdinaryDrawersMsg = 0x4b0d; // "Maybe these are not just ordinary drawers!"
// Drawer Open Message : 0x4b39 to 0x4b6b
const uint16 dsAddr_drawerOpenMsg = 0x4b39; // "I cannot open the drawer if the next one is open!"
// Blue Interior Message 0x4b6c to 0x4b86
const uint16 dsAddr_blueInteriorMsg = 0x4b6c; // "It's got a blue interior"
// Red Interior Message : 0x4b87 to 0x4ba0
const uint16 dsAddr_redInteriorMsg = 0x4b87; // "It's got a red interior"
// Grey Interior Message : 0x4ba1 to 0x4bbb
const uint16 dsAddr_greyInteriorMsg = 0x4ba1; // "It's got a grey interior"
// Green Interior Message : 0x4bbc to 0x4bd7
const uint16 dsAddr_greenInteriorMsg = 0x4bbc; // "It's got a green interior"
// Brown Interior Message : 0x4bd8 to 0x4bf3
const uint16 dsAddr_brownInteriorMsg = 0x4bd8; // "It's got a brown interior"
// Pink Interior Message : 0x4bf4 to 0x4c0e
const uint16 dsAddr_pinkInteriorMsg = 0x4bf4; // "It's got a pink interior"
// Dictaphone Inside Message : 0x4c0f to 0x4c31
const uint16 dsAddr_dictaphoneInsideMsg = 0x4c0f; // "Wow! There's a dictaphone inside!"
// Found Polaroid Message : 0x4c32 to 0x4c60
const uint16 dsAddr_foundPolaroidMsg = 0x4c32; // "There's a polaroid inside! I might need that"
// Book Held Message : 0x4c61 to 0x4c83
const uint16 dsAddr_bookHeldMsg = 0x4c61; // "Something's got hold of the book!"
// Secret Compartment Message : 0x4c84 to 0x4c9f
const uint16 dsAddr_secretCompartmentMsg = 0x4c84; // "Wow! A secret compartment!"
// Dont Mess Message : 0x4ca0 to 0x4cc6
const uint16 dsAddr_dontMessMsg = 0x4ca0; // "I don't need to mess with it anymore"
// Full Automatic Message : 0x4cc7 to 0x4cd8
const uint16 dsAddr_fullAutomaticMsg = 0x4cc7; // "Fully Automatic"
// No More Sheets Message : 0x4cd9 to 0x4d01
const uint16 dsAddr_noMoreSheetsMsg = 0x4cd9; // "Right now I don't need any more sheets"
// No Deprave Message : 0x4d02 to 0x4d29
const uint16 dsAddr_noDepraveMsg = 0x4d02; // "Nah, I don't want to deprave the kids"
// No Read Again Message : 0x4d2a to 0x4d5a
const uint16 dsAddr_noReadAgainMsg = 0x4d2a; // "I don't want to read it again. I might like it."
// TV Off Message : 0x4d5b to 0x4d7f
const uint16 dsAddr_tvOffMsg = 0x4d5b; // "I just realised that the TV is off"
// Not Happen Message : 0x4d80 to 0x4d92
const uint16 dsAddr_NotHappenMsg = 0x4d80; // "Nothing happened"
// Tape Started Message : 0x4d93 to 0x4da5
const uint16 dsAddr_tapeStartedMsg = 0x4d93; // "The tape started!"
// Much Better Message : 0x4da6 to 0x4dba
const uint16 dsAddr_muchBetterMsg = 0x4da6; // "That's much better"
// No Sleep Message : 0x4dbb to 0x4dd2
const uint16 dsAddr_noSleepMsg = 0x4dbb; // "I don't want to sleep"
// Just Cork Message : 0x4dd3 to 0x4de5
const uint16 dsAddr_justCorkMsg = 0x4dd3; // "It's just a cork"
// Enough Photos Message : 0x4de6 to 0x4e04
const uint16 dsAddr_enoughPhotosMsg = 0x4de6; // "I don't need any more photos"
// Record Scare Message : 0x4e05 to 0x4e31
const uint16 dsAddr_recordScareMsg = 0x4e05; // "Yeah, I can record this and scare the cats"
// Already Recorded Message : 0x4e32 to 0x4e57
const uint16 dsAddr_alreadyRecordedMsg = 0x4e32; // "I already recorded what I wanted to"

// Not Right Moment Message : 0x4ea5 to 0x4ecd
const uint16 dsAddr_notRightMomentMsg = 0x4ea5; // "I don't think this is the right moment"
// Cook Around Message : 0x4ece to 0x4ef9
const uint16 dsAddr_cookAroundMsg = 0x4ece; // "I can't do anything with this cook around"

// Break Flatten Message : 0x4f3d to 0x4f68
const uint16 dsAddr_breakFlattenMsg = 0x4f3d; // "I wanted to break it, not to flatten it!"
// What Inside Message : 0x4f69 to 0x4f9a
const uint16 dsAddr_whatInsideMsg = 0x4f69; // "I was always curious what's inside these things"
// Rest Useless Message : 0x4f9b to 0x4fb0
const uint16 dsAddr_restUselessMsg = 0x4f9b; // "The rest is useless"
// Two Batteries Message : 0x4fb1 to 0x4fca
const uint16 dsAddr_twoBatteriesMsg = 0x4fb1; // "Wow! Two 1.5V batteries!"
// One Taken Message : 0x4fcb to 0x4fe1
const uint16 dsAddr_oneTakenMsg = 0x4fcb; // "This one's taken, OK?"
// Slight Mad Message : 0x4fe2 to 0x5009
const uint16 dsAddr_slightMadMsg = 0x4fe2; // "It finally happened. I'm slightly mad"
// Paper Burnt Message : 0x500a to 0x502a
const uint16 dsAddr_paperBurntMsg = 0x500a; // "The paper burnt out completely!"
// Burn Baby Message : 0x502b to 0x503d
const uint16 dsAddr_burnBabyMsg = 0x502b; // "Burn, baby, burn!"
// Voila Message : 0x503e to 0x5045
const uint16 dsAddr_voilaMsg = 0x503e; // "Voila"
// Too Hot Message : 0x5046 to 0x505d
const uint16 dsAddr_tooHotMsg = 0x5046; // "It's too hot to touch!"
// Frozen Shelf Message : 0x505e to 0x5081
const uint16 dsAddr_frozenShelfMsg = 0x505e; // "It has frozen hard onto the shelf!"
// Yummy Message : 0x5082 to 0x5089
const uint16 dsAddr_yummyMsg = 0x5082; // "Yummy"
// Dislike Veal Message : 0x508a to 0x50a5
const uint16 dsAddr_dislikeVealMsg = 0x508a; // "I never liked veal anyway"
// No Reason Message : 0x50a6 to 0x50c2
const uint16 dsAddr_noReasonMsg = 0x50a6; // "There's no reason to do it"
// Fooled Once Message : 0x50c3 to 0x50e0
const uint16 dsAddr_fooledOnceMsg = 0x50c3; // "I'd already fooled him once"
// Mike Voice Test Message : 0x50e1 to 0x5100
const uint16 dsAddr_mikeVoiceTestMsg = 0x50e1; // "Mike, activate the voice test"
// Not My Voice Message : 0x5101 to 0x5123
const uint16 dsAddr_notMyVoiceMsg = 0x5101; // "I won't cheat Mike with MY voice"
// Singing Message : 0x5124 to 0x5137
const uint16 dsAddr_singingMsg = 0x5124; // "siiiiinging!"
// Mike Scent Test Message : 0x5138 to 0x5160
const uint16 dsAddr_mikeScentTestMsg = 0x5138; // "Mike, let's get on with the scent test"
// Mike View Test Message : 0x5161 to 0x517a
const uint16 dsAddr_mikeViewTestMsg = 0x5161; // "Mike, run the view test"
// Cutscene Message #0 : 0x517b to 0x51a6
const uint16 dsAddr_cutsceneMsg0 = 0x517b; // "A secret diary of ..."
// Cant Hide Message : 0x51a7 to 0x51ba
const uint16 dsAddr_cantHideMsg = 0x51a7; // "I can't hide here!"
// John Outside Message : 0x51bb to 0x51e6
const uint16 dsAddr_johnOutsideMsg = 0x51bb; // "There's John Noty outside! I can't go out!"
// Was Close Message : 0x51e7 to 0x51f7
const uint16 dsAddr_wasCloseMsg = 0x51e7; // "That was close"
// Cork In Hole Message : 0x51f8 to 0x5217
const uint16 dsAddr_corkInHoleMsg = 0x51f8; // "The cork is stuck in the hole"
// Fits Perfect Message : 0x5218 to 0x522b
const uint16 dsAddr_fitsPerfectMsg = 0x5218; // "It fits perfectly!"
// Enough Water Message : 0x522c to 0x524e
const uint16 dsAddr_enoughWaterMsg = 0x522c; // "There's enough water in the sink"
// No Hot Water Message : 0x524f to 0x5271
const uint16 dsAddr_noHotWaterMsg = 0x524f; // "There's no hot water in the sink"
// Label Off Message : 0x5272 to 0x528a
const uint16 dsAddr_labelOffMsg = 0x5272; // "The label has come off!"
// Cork Too Small Message : 0x528b to 0x52a8
const uint16 dsAddr_corkTooSmallMsg = 0x528b; // "The cork is a bit too small"
// Not Try Now Message : 0x52a9 to 0x52ca
const uint16 dsAddr_notTryNowMsg = 0x52a9; // "There's no need to try them now"
// No Salad Message : 0x52cb to 0x52f5
const uint16 dsAddr_noSaladMsg = 0x52cb; // "I don't want to turn myself into a salad"
// Nah Message : 0x52f6 to 0x52fd
const uint16 dsAddr_nahMsg = 0x52f6; // "Nah"
// Vent First Message : 0x52fe to 0x5325
const uint16 dsAddr_ventFirstMsg = 0x52fe; // "I'd better stop this ventilator first"
// Catch John First Message : 0x5326 to 0x5348
const uint16 dsAddr_catchJohnFirstMsg = 0x5326; // "I'd better catch John Noty first"
// Only Chilli Message : 0x5349 to 0x5371
const uint16 dsAddr_onlyChilliMsg = 0x5349; // "Good this red stuff is only a chilli"
// Water Hot Message : 0x5372 to 0x538c
const uint16 dsAddr_waterHotMsg = 0x5372; // "The water looks very hot"
// Sink Full Message : 0x538d to 0x53ac
const uint16 dsAddr_sinkFullMsg = 0x538d; // "The sink is full of hot water"
// No Sock Store Message : 0x53ad to 0x53dc
const uint16 dsAddr_noSockStoreMsg = 0x53ad; // "I don't have anything to store these socks in"
// Show Papers Message : 0x53dd to 0x53f1
const uint16 dsAddr_showPapersMsg = 0x53dd; // "Here are my papers"
// Got Permission Message : 0x53f2 to 0x5410
const uint16 dsAddr_gotPermissionMsg = 0x53f2; // "I already got the permission"
// Saving Fine Message : 0x5411 to 0x5462
const uint16 dsAddr_SavingFineMsg = 0x5411; // "Saving is a very fine thing..."
// Love Captain Message : 0x5463 to 0x5474
const uint16 dsAddr_loveCaptainMsg = 0x5463; // "I love captain"
// Soccer Rulz Message : 0x5475 to 0x5483
const uint16 dsAddr_soccerRulzMsg = 0x5475; // "Soccer rulz"
// Tree Cut Message : 0x5484 to 0x54c3
const uint16 dsAddr_treeCutMsg = 0x5484; // "Don't cut the trees..."
// Visa Accepted Message : 0x54c4 to 0x54d4
const uint16 dsAddr_visaAcceptedMsg = 0x54c4; // "VISA Accepted"
// Other Graffiti Message : 0x54d5 to 0x54f6
const uint16 dsAddr_otherGraffitiMsg = 0x54d5; // "The rest of graffiti is obscene"
// First Trial Message : 0x54f7 to 0x5510
const uint16 dsAddr_firstTrialMsg = 0x54f7; // "Sir, I'm Mark. A rookie"
// Locked Message : 0x5511 to 0x551e
const uint16 dsAddr_lockedMsg = 0x5511; // "It's Locked!"
// Thanks Message : 0x551f to 0x5527
const uint16 dsAddr_ThanksMsg = 0x551f; // "Thanks."
// Unknown Usage Message : 0x5528 to 0x555c
const uint16 dsAddr_unkUsageMsg = 0x5528; // "I don't have any idea what to do with it right now"
// Idea Message : 0x555d to 0x5576
const uint16 dsAddr_ideaMsg = 0x555d; // "That gives me an idea"
// Check Works Message : 0x5577 to 0x5599
const uint16 dsAddr_checkWorksMsg = 0x5577; // "Now I got to check if it works"
// Time To Call Message : 0x559a to 0x55bf
const uint16 dsAddr_timeToCallMsg = 0x559a; // "I think it is time to call captain"
// Meal Finished Message : 0x55c0 to 0x55da
const uint16 dsAddr_mealFinishedMsg = 0x55c0; // "Hey! I finished my meal."
// Bowl Welded Message : 0x55db to 0x55fe
const uint16 dsAddr_bowlWeldedMsg = 0x55db; // "Wow. He got welded to the bowl"
// Gotcha Message : 0x55ff to 0x5607
const uint16 dsAddr_gotchaMsg = 0x55ff; // "Gotcha"
// No Pocket Message : 0x5608 to 0x5631
const uint16 dsAddr_noPocketMsg = 0x5608; // "I don't want to touch his pockets again."
// Does Not Work Message : 0x5632 to 0x5645
const uint16 dsAddr_doesNotWorkMsg = 0x5632; // "That doesn't work"
// Message : 0x5646 to 0x5655
const uint16 dsAddr_fnMsg1 = 0x5646; // "Piece of cake"
// Message : 0x5656 to 0x5679
const uint16 dsAddr_fnMsg2 = 0x5656; // "And how am I supposed to get back?"
// Message : 0x567a to 0x5681
const uint16 dsAddr_fnMsg3 = 0x567a; // "Great"
// Message : 0x5682 to 0x5695
const uint16 dsAddr_fnMsg4 = 0x5682; // "Oh, yeah, right"
// Pull Object Message #1 : 0x5696 to 0x56ab
const uint16 dsAddr_pullObjMsg1 = 0x5696; // "I can't pull it out"

// Fence Blocks Message : 0x56da to 0x56f6
const uint16 dsAddr_fenceBlocksMsg = 0x56da; // "The fence blocks the way"

// Pull Object Message #2 : 0x570f to 0x5721
const uint16 dsAddr_pullObjMsg2 = 0x570f; // "I can't reach it"
// Hello Question Message : 0x5722 to 0x5729
const uint16 dsAddr_helloQMsg = 0x5722; // "Hello?"
// Totally Addicted Message : 0x572a to 0x5741
const uint16 dsAddr_totallyAddictedMsg = 0x572a; // "He's totally addicted"
// What About Message : 0x5742 to 0x5756
const uint16 dsAddr_whatAboutMsg = 0x5742; // "What about a new"
// Hot Off Message : 0x5757 to 0x576f
const uint16 dsAddr_hotOffMsg = 0x5757; // "hot off the press"
// Full Color Message : 0x5770 to 0x5781
const uint16 dsAddr_fullColorMsg = 0x5770; // "full-color"
// Special Edition Message : 0x5782 to 0x5798
const uint16 dsAddr_specialEdMsg = 0x5782; // "special edition"
// Soldier News Message : 0x5799 to 0x57b1
const uint16 dsAddr_soldierNewsMsg = 0x5799; // "of Soldier News?!"
// Pole Climb Done Message : 0x57b2 to 0x57bf
const uint16 dsAddr_poleClimbDoneMsg = 0x57b2; // "Never Again!"
// Vac Message : 0x57c0 to 0x57de
const uint16 dsAddr_vacMsg = 0x57c0; // "What am I? A vacuum cleaner?!"
// Cutscene Message #1 : 0x57df to 0x5809
const uint16 dsAddr_cutsceneMsg1 = 0x57df; // "sixty seven rude words later"
// Cutscene Message #2 : 0x580a to 0x5826
const uint16 dsAddr_cutsceneMsg2 = 0x580a; // "Meanwhile in the mansion"
// Now Open Message : 0x5827 to 0x5836
const uint16 dsAddr_nowOpenMsg = 0x5827; // "Now it's open"
// Cmon Baby Message : 0x5837 to 0x5854
const uint16 dsAddr_cmonBabyMsg = 0x5837; // "C'mon baby, it's all yours!"
// Talk Not Now Message : 0x5855 to 0x5882
const uint16 dsAddr_talkNotNowMsg = 0x5855; // "I've got no reason to talk to him right now."
// Yeah Right Message : 0x5883 to 0x5893
const uint16 dsAddr_yeahRightMsg = 0x5883; // "Yeah right!"
// Barman Too Close Message : 0x5894 to 0x58af
const uint16 dsAddr_barmanTooCloseMsg = 0x5894; // "The barman is too close"
// Yuck Message : 0x58b0 to 0x58b6
const uint16 dsAddr_yuckMsg = 0x58b0; // "Yuck!"

// Food Alive Message : 0x5905 to 0x592e
const uint16 dsAddr_foodAliveMsg = 0x5905; // "No, thanks. This food seems still alive"
// Door Closed Message : 0x592f to 0x5954
const uint16 dsAddr_doorClosedMsg = 0x592f; // "The door is closed. What a surprise."
// Empty Message : 0x5955 to 0x5961
const uint16 dsAddr_emptyMsg = 0x5955; // "It's Empty"

// Never Learnt Message : 0x5c60 to 0x5c81
const uint16 dsAddr_neverLearntMsg = 0x5c60; // "I never learnt to how use one"

// Cognac Message : 0x5cac to 0x5cda
const uint16 dsAddr_cognacMsg = 0x5cac; // "Pfui! The cognac really didn't do any good"

// Hey What's The Matter Message : 0x5da8 to 0x5dc1
const uint16 dsAddr_HeyWtmQMsg = 0x5da8; // "Hey! What's the matter?!"

// Out Of Order Message : 0x5dce to 0x5de1
const uint16 dsAddr_outOfOrderMsg = 0x5dce; // "It's out of order"
// Captain Watching Message : 0x5de2 to 0x5e0a
const uint16 dsAddr_captainWatchingMsg = 0x5de2; // "with captain watching? Better not"

// First Business Message : 0x5e25 to 0x5e53
const uint16 dsAddr_firstBusinessMsg = 0x5e25; // "First I've got some business to take care of"

// Book Color Message Address Pointers : (6 * 2-byte) = 0x5f3c to 0x5f47
const uint16 dsAddr_bookColorMsgPtr = 0x5f3c;
// Book Color Message #0 : 0x5f48 to 0x5f60
const uint16 dsAddr_bookColorMsg0 = 0x5f48; // ""The history of blues""
// Book Color Message #1 : 0x5f61 to 0x5f8f
const uint16 dsAddr_bookColorMsg1 = 0x5f61; // ""Manchester United, or the Red Devils story""
// Book Color Message #2 : 0x5f90 to 0x5fb5
const uint16 dsAddr_bookColorMsg2 = 0x5f90; // ""Greyhounds and other hunting dogs""
// Book Color Message #3 : 0x5fb6 to 0x5fe6
const uint16 dsAddr_bookColorMsg3 = 0x5fb6; // ""Greenhorn, or my adventures in the Wild West""
// Book Color Message #4 : 0x5fe7 to 0x6008
const uint16 dsAddr_bookColorMsg4 = 0x5fe7; // ""Charlie Brown and his company""
// Book Color Message #5 : 0x6009 to 0x6034
const uint16 dsAddr_bookColorMsg5 = 0x6009; // ""Pink Panther: an unauthorised biography""

// Mansion Intrusion Function Pointers : (5 * 2-byte) = 0x6035 to 0x603e
const uint16 dsAddr_MansionIntrusionFnPtr = 0x6035;

// Save State Region : 0x6478 to 0xdbf1
const uint16 dsAddr_saveState = 0x6478;
const uint16 saveStateSize = 0x777a;

// Save Description String (24 bytes) : 0x6478 to 0x648f

// Ego (Mark) position in scene : 0x64af to 0x64b2
const uint16 dsAddr_egoX = 0x64af; // 2 bytes
const uint16 dsAddr_egoY = 0x64b1; // 2 bytes

// Idle Animation List Table : 0x6540 to 0x????
const uint16 dsAddr_idleAnimationListPtr = 0x6540;

// Palette Effect Data : 0x6609 to 0x????
const uint16 dsAddr_paletteEffectData = 0x6609;

// Scene Fade Table (2 byte address * 42): 0x663e to 0x6691
const uint16 dsAddr_sceneFadeTablePtr = 0x663e;

// Scene Walkbox Table (2 byte LE address * 42) : 0x6746 to 0x6799
const uint16 dsAddr_sceneWalkboxTablePtr = 0x6746;

// Scene Zoom Table (2 byte address * 42) : 0x70f4 to 0x7147
const uint16 dsAddr_sceneZoomTablePtr = 0x70f4;

// Scene Object Table (2 byte address * 42) : 0x7254 to 0x72a7
const uint16 dsAddr_sceneObjectTablePtr = 0x7254;

// Current Scene Id : 0xb4f3
const uint16 dsAddr_currentScene = 0xb4f3; // 1 byte

// Ons Animation Table (2 byte address * ??) : 0xb4f5 to 0x????
const uint16 dsAddr_onsAnimationTablePtr = 0xb4f5;

// Examine Object Callback Table (2 byte LE address * ??) : 0xb5ce to 0x????
const uint16 dsAddr_objExamineCallbackTablePtr = 0xb5ce;

// Use Object Callback Table (2 byte LE address * ??) : 0xb89c to 0x????
const uint16 dsAddr_objUseCallbackTablePtr = 0xb89c;

// Inventory Object Callback Table (3 byte (id, callbackAddr) * 7) : 0xbb72 to 0xbb86
const uint16 dsAddr_objCallbackTablePtr = 0xbb72;

// Scene Hotspots Table (2 byte LE address * ??) : 0xbb87 to 0x????
const uint16 dsAddr_sceneHotspotsPtr = 0xbb87;

// Inventory Object Combining Table (5 byte (id, id, new object id, msgAddr) * 34) : 0xc335 to 0xc3de
const uint16 dsAddr_objCombiningTablePtr = 0xc335;
// 3 byte null terminator for Combining table 0xc3df to 0xc3e1

// Object Combine Error Message : 0xc3e2 to 0xc41e
const uint16 dsAddr_objCombineErrorMsg = 0xc3e2; // "Using these two objects ..."

// Inventory (item ids held by Ego) (1 byte * 24) : 0xc48d to 0xc4a4
const uint16 dsAddr_inventory = 0xc48d;
// 0xc4a5 is null word alignment byte
// Inventory item data address table (2 bytes * 92) : 0xc4a6 to 0xc55d
const uint16 dsAddr_inventoryItemDataPtrTable = 0xc4a6;

// Lans Animation Table (4 byte * ??) : 0xd89e to 0x????
const uint16 dsAddr_lansAnimationTablePtr = 0xd89e;

// Current Music Id Playing : 0xdb90
const uint16 dsAddr_currentMusic = 0xdb90; // 1 byte

// Light On Flag : 0xdba4
const uint16 dsAddr_lightOnFlag = 0xdba4; // 1 byte

// Captain Drawer State : 0xdbe6
const uint16 dsAddr_captainDrawerState = 0xdbe6; // 1 byte

// Counter for Mansion Intrusion Attempts : 0xdbea
const uint16 dsAddr_mansionEntryCount = 0xdbea;// 1 byte

// Intro Credits #1 : 0xe3c2 to 0xe3e5 (Read Only)
const uint16 dsAddr_introCredits1 = 0xe3c2; // "backgrounds ..."
// Intro Credits #2 : 0xe3e6 to 0xe3fe (Read Only)
const uint16 dsAddr_introCredits2 = 0xe3e6; // "music ..."
// Intro Credits #3 : 0xe3ff to 0xe42e (Read Only)
const uint16 dsAddr_introCredits3 = 0xe3ff; // "animation..."
// Intro Credits #4 : 0xe42f to 0xe45b (Read Only)
const uint16 dsAddr_introCredits4 = 0xe42f; // "programming..."
// Credits #5 : 0xe45c to 0xe47b (Read Only)
const uint16 dsAddr_credits5 = 0xe45c; // "after the tiring journey..."
// Final Credits #6 : 0xe47c to 0xe487 (Read Only)
const uint16 dsAddr_finalCredits6 = 0xe47c; // "THE END..."
// Final Credits #7 : 0xe488 to 0xe782 (Read Only)
const uint16 dsAddr_finalCredits7 = 0xe488; // "programming..."
// 0xe783 to 0xe78f: 13 null bytes at end of dseg data - segment alignment padding?

class Resources {
public:
	Resources();
	~Resources();
	bool loadArchives(const ADGameDescription *gd);

	void loadOff(Graphics::Surface &surface, byte *palette, int id);
	Common::SeekableReadStream *loadLan(uint32 id) const;
	Common::SeekableReadStream *loadLan000(uint32 id) const;

	/*
	 * PSP (as the other sony playstation consoles - to be confirmed and 'ifdef'ed here too)
	 * is very sensitive to the number of simultaneously opened files.
	 * This is an attempt to reduce their number to zero.
	 * TransientFilePack does not keep opened file descriptors and reopens it on each request.
	 */
#ifdef __PSP__
	TransientFilePack off, on, ons, lan000, lan500, sam_mmm, sam_sam, mmm, voices;
#else
	FilePack off, on, ons, lan000, lan500, sam_mmm, sam_sam, mmm, voices;
#endif

	Segment cseg, dseg;
	Font font7, font8;

	//const byte *getDialog(uint16 dialogNum) { return eseg.ptr(dialogOffsets[dialogNum]); }
	uint16 getDialogAddr(uint16 dialogNum) { return dialogOffsets[dialogNum]; }

	Segment eseg;
private:
	void precomputeDialogOffsets();

	Common::Array<uint16> dialogOffsets;
};

} // End of namespace TeenAgent

#endif
