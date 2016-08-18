//#include <stdint.h>

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "gui/EventRecorder.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/fs.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "common/timer.h"

//#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "cryo/cryo.h"
#include "cryo/eden.h"

namespace Cryo {

#include "cryo/platdefs.h"
#include "cryo/defs.h"
#include "cryo/CryoLib.h"
#include "cryo/CryoLibStub.c"

short       word_2C300 = 0;
short       word_2C302 = 0;
short       word_2C304 = 0;

unsigned char   allow_doubled = 1;
int     curs_center = 11;

struct {
	short   x, y;
} saved_repadam = { -1, -1 };

class EdenGameImpl : EdenGame {
private:

	short           old_scroll_pos, scroll_pos;
	short           word_2F514;
	unsigned char   fresqTalk;
	unsigned char   keep01, keep02, keep10, keep11, keep12, keep13, keep21, keep22;
	unsigned char   curs_keepbuf[2500];
	short           curs_keepy, curs_keepx;
	short           torchCursor;
	short           cur_bank_num;
	short           glow_h;
	short           glow_w;
	short           glow_y;
	short           glow_x;
	unsigned char   needPaletteUpdate;
	unsigned char   curs_saved;
	unsigned char   showBlackBars;
	unsigned char   fond_saved;
	unsigned char   *bank_data_ptr;
	color3_t        pal_entry;
	color_t         global_palette[256];    //TODO palette_t
	perso_t         *tyranPtr;
	int             last_anim_frame_num;
	int             cur_anim_frame_num;
	int             last_anim_ticks;
	prect_t         *cur_perso_rect;
	short           num_anim_frames;
	short           max_perso_desc;
	short           num_img_desc;
	unsigned char   restartAnimation;
	unsigned char   animationActive;
	unsigned char   animationDelay;
	unsigned char   animationIndex;
	unsigned char   lastAnimationIndex;

	unsigned char   *dword_30724;
	unsigned char   *dword_30728;   //TODO: rename - something amim-related
	unsigned char   *dword_3072C;   //TODO ditto
	unsigned char   *animationTable;
	unsigned char   imagedesc[512];
	unsigned char   *perso_img_bank_data_ptr;
	unsigned char   savedUnderSubtitles;
	short           num_text_lines;
	unsigned char   phraseBuffer[400];
	unsigned char   *text_ptr;
	unsigned char   phraseIconsBuffer[10];
	unsigned char   phraseCoordsBuffer[22];
	unsigned char   *textoutptr;
	unsigned char   *textout;
	object_t        *currentSpecialObject;
	short           word_30AFC;
	unsigned char   byte_30AFE;

	unsigned char   byte_30B00;
	int             dword_30B04;

	char            lastPhrasesFile;
	char            dialogSkipFlags;

	color3_t        newColor;
	color_t         oldPalette[256];    // TODO palette_t ?
	color_t         newPalette[256];
	rect_t          rect_dst, rect_src;
	void            *voiceSamplesBuffer;    //TODO: sound sample buffer
	file_t          h_bigfile;
	unsigned char   info_list[16];
	unsigned char   needToFade;
	unsigned char   lastMusicNum;
	unsigned char   *main_bank_buf;
	unsigned char   *music_buf;
	unsigned char   *gameLipsync;
	unsigned char   *gamePhrases;
	unsigned char   *gameDialogs;   //TODO: rename to dialogs?
	unsigned char   *gameConditions;
	void            *sal_buf;   //TODO: fixme
	unsigned char   *bank_data_buf;
	icon_t          *gameIcons;
	room_t          *gameRooms;
	pak_t           *bigfile_header;
	unsigned char   *glow_buffer;
	unsigned char   *p_mainview_buf;
	unsigned char   *p_view2_buf;
	unsigned char   *gameFont;  //TODO: rename to font?
	unsigned char   *p_subtitlesview_buf;
	unsigned char   *p_underSubtitlesView_buf;
	global_t        *p_global;
	unsigned short  mouse_y_center, mouse_x_center;
	int             quit_flag3;     //TODO: some obsolete error flag?
	unsigned short  machine_speed;
	unsigned char   quit_flag;

	unsigned char   gameStarted;

	unsigned char   quit_flag2;
	unsigned char   soundAllocated;
	soundchannel_t  *music_channel;
	soundchannel_t  *hnmsound_ch;
	sound_t         *voiceSound;

	view_t          *p_view2;
	view_t          *p_underSubtitlesView;
	view_t          *p_subtitlesview;
	view_t          *p_underBarsView;
	view_t          *p_mainview;
	view_t          *p_hnmview;
	hnm_t           *p_hnmcontext;
	filespec_t      bigfilespec;
	rect_t          underSubtitlesBackupRect, underSubtitlesScreenRect, underBottomBarBackupRect, underBottomBarScreenRect,
	                underTopBarBackupRect, underTopBarScreenRect, rect_31C7A;
	int             demoCurrentTicks;
	int             demoStartTicks;
	int             currentTime;
	short           mouse_y;
	short           mouse_x;
	short           doubled;
	short           curs_x_pan;
	short           inventoryScrollDelay;
	short           curs_y, curs_x;
	short           current_cursor;
	icon_t          *current_spot;
	icon_t          *current_spot2;
	unsigned char   pomme_q;
	unsigned char   keybd_held;
	unsigned char   mouse_held;
	unsigned char   normalCursor;
	unsigned char   *p_hnmview_buf;
	unsigned char   showVideoSubtitle;
	unsigned char   videoCanceled;  //TODO: hnm_canceled
	unsigned char   specialTextMode;
	int             hnm_position;
	int             voiceSamplesSize;   //TODO: perso vox sample data len
	short           mus_vol_right;
	short           mus_vol_left;


	unsigned char   animateTalking;
	unsigned char   personTalking;
	unsigned char   mus_fade_flags;

	char            musicSequencePos;
	unsigned char   musicPlaying;

	unsigned char   *mus_samples_ptr;
	unsigned char   *mus_patterns_ptr;  //TODO: sndblock_t ?
	unsigned char   *mus_sequence_ptr;
	soundgroup_t    *mus_queue_grp;
	short           *pCurrentObjectLocation;
	unsigned char   own_objects[128];
	unsigned char   byte_31D64;

	unsigned char   no_palette;
	unsigned char   gameLoaded;
#define MAX_TAPES 16
	tape_t          tapes[MAX_TAPES];
	unsigned char   confirmMode;
	unsigned char   *cur_slider_value_ptr;
	unsigned char   lastMenuItemIdLo;
	short           lastTapeRoomNum;
	short           cur_slider_x;
	short           cur_slider_y;
	short           destinationRoom;
	short           word_31E7A;

	short           word_378CC; //TODO: set by CLComputer_Init to 0
	short           word_378CE;

	void RemoveConsole() {
	}
	void scroll() {
		restaurefrises();
		p_mainview->norm.src_left = scroll_pos;
		p_mainview->zoom.src_left = scroll_pos;

	}
	void resetscroll() {
		old_scroll_pos = scroll_pos;
		scroll_pos = 0;
		restaurefrises();   //TODO: inlined scroll() ?
		p_mainview->norm.src_left = 0;
		p_mainview->zoom.src_left = 0;
	}
	void scrollfresques() {
		if (curs_y > 16 && curs_y < 176) {
			if (curs_x >= 0 && curs_x < 32 && scroll_pos > 3) {
				scroll_pos -= 4;
			} else if (curs_x > 288 && curs_x < 320 && scroll_pos < p_global->fresqWidth) {
				scroll_pos += 4;
			}
		}
		scroll();

	}
	void afffresques() {
		use_bank(p_global->fresqImgBank);
		noclipax(0, 0, 16);
		use_bank(p_global->fresqImgBank + 1);
		noclipax(0, 320, 16);
		needPaletteUpdate = 1;
	}
	void gametofresques() {
		fresqTalk = 0;
		rundcurs();
		sauvefrises();
		afffresques();
		p_global->displayFlags = DisplayFlags::dfFresques;
	}
	void dofresques() {
		curs_saved = 0;
		torchCursor = 1;
		glow_x = -1;
		glow_y = -1;
		p_global->gameFlags |= GameFlags::gfFlag20;
		p_global->ff_D4 = 0;
		p_global->curObjectId = 0;
		p_global->iconsIndex = 13;
		p_global->autoDialog = 0;
		gametofresques();
		p_global->fresqNumber = 3;
	}
	void finfresques() {
		torchCursor = 0;
		curs_saved = 1;
		p_global->displayFlags = DisplayFlags::dfFlag1;
		resetscroll();
		p_global->ff_100 = -1;
		maj_salle(p_global->roomNum);
		if (p_global->phaseNum == 114)
			p_global->narratorSequence = 1;
		p_global->eventType = EventType::etEvent8;
		showevents();
	}
	void scrollmiroir() {
		if (curs_y > 16 && curs_y < 165) {
			if (curs_x >= 0 && curs_x < 16) {
				if (scroll_pos > 3) {
					if (doubled)
						scroll_pos -= 2;
					else
						scroll_pos -= 1;
					scroll();
				}
			} else if (curs_x > 290 && curs_x < 320) {
				if (scroll_pos < 320) {
					if (doubled)
						scroll_pos += 2;
					else
						scroll_pos += 1;
					scroll();
				}
			}
		}
	}
	void scrollpano() {
		if (curs_y > 16 && curs_y < 165) {
			if (curs_x >= 0 && curs_x < 16) {
				if (scroll_pos > 3) {
					if (doubled)
						scroll_pos -= 2;
					else
						scroll_pos -= 1;
				}
			} else if (curs_x > 290 && curs_x < 320) {
				if (scroll_pos < 320) {
					if (doubled)
						scroll_pos += 2;
					else
						scroll_pos += 1;
				}
			}
		}
		scroll();
	}
	void affsuiveur(suiveur_t *suiveur, short x, short y) {
		use_bank(suiveur->bank);
		noclipax(suiveur->image, x, y + 16);
	}
	void persoinmiroir() {
		icon_t  *icon1 = &gameIcons[3];
		icon_t  *icon = &gameIcons[28];
		suiveur_t *suiveur = suiveurs_list;
		short num = 1;
		int i;
		for (i = 0; i < 16; i++) {
			if (p_global->party & (1 << i))
				num++;
		}
		icon += num;
		icon->sx = -1;
		icon--;
		icon->sx = icon1->sx;
		icon->sy = icon1->sy;
		icon->ex = icon1->ex;
		icon->ey = 170;
		icon->cursor_id = icon1->cursor_id;
		icon->action_id = icon1->action_id;
		icon->object_id = icon1->object_id;
		icon--;
		affsuiveur(suiveur, suiveur->sx, suiveur->sy);
		for (; suiveur->id != -1; suiveur++) {
			perso_t *perso;
			for (perso = kPersons; perso != &kPersons[PER_UNKN_156]; perso++) {
				if (perso->id != suiveur->id)                       continue;
				if (perso->flags & PersonFlags::pf80)               continue;
				if ((perso->flags & PersonFlags::pfInParty) == 0)   continue;
				if (perso->roomNum != p_global->roomNum)            continue;
				icon->sx = suiveur->sx;
				icon->sy = suiveur->sy;
				icon->ex = suiveur->ex;
				icon->ey = suiveur->ey;
				icon->cursor_id = 8;
				icon->action_id = perso->actionId;
				icon--;
				affsuiveur(suiveur, suiveur->sx, suiveur->sy);
				break;
			}
		}
	}
	void gametomiroir(unsigned char arg1) {
		short bank;
		if (p_global->displayFlags != DisplayFlags::dfFlag2) {
			rundcurs();
			restaurefrises();
			afftopscr();
			showobjects();
			sauvefrises();
		}
		bank = p_global->roomBgBankNum;
		if (bank == 76 || bank == 128)
			bank = 2161;
		use_bank(bank + 326);
		noclipax(0, 0, 16);
		use_bank(bank + 327);
		noclipax(0, 320, 16);
		persoinmiroir();
		needPaletteUpdate = 1;
		p_global->iconsIndex = 16;
		p_global->autoDialog = 0;
		p_global->displayFlags = DisplayFlags::dfMirror;
		p_global->ff_102 = arg1;
	}
	void flipmode() {
		if (personTalking) {
			endpersovox();
			if (p_global->displayFlags == DisplayFlags::dfPerson) {
				if (p_global->perso_ptr == &kPersons[PER_THOO] && p_global->phaseNum >= 80)
					af_subtitle();
				else {
					getdatasync();
					load_perso_cour();
					addanim();
					restartAnimation = 1;
					anim_perso();
				}
			} else
				af_subtitle();
			persovox();
		} else {
			if (p_global->displayFlags != DisplayFlags::dfFresques && p_global->displayFlags != DisplayFlags::dfFlag2) {
				closesalle();
				if (p_global->displayFlags & DisplayFlags::dfFlag1)
					gametomiroir(1);
				else {
					quitmiroir();
					maj_salle(p_global->roomNum);
					if (byte_31D64) {
						dialautoon();
						parle_moi();
					}
					byte_31D64 = 0;
				}
			}
		}
	}
	void quitmiroir() {
		rundcurs();
		afficher();
		resetscroll();
		sauvefrises();
		p_global->displayFlags = DisplayFlags::dfFlag1;
		p_global->ff_100 = -1;
		p_global->eventType = EventType::etEventC;
		p_global->ff_102 = 1;
	}
	void clictimbre() {
		flipmode();
	}
	void clicplanval() {
		if ((p_global->partyOutside & PersonMask::pmDina) && p_global->phaseNum == 371) {
			quitmiroir();
			maj_salle(p_global->roomNum);
			return;
		}
		if (p_global->roomNum == 8 || p_global->roomNum < 16)
			return;
		rundcurs();
		afficher();
		if (p_global->displayFlags == DisplayFlags::dfMirror)
			quitmiroir();
		deplaval((p_global->roomNum & 0xFF00) | 1); //TODO: check me
	}
	void gotolieu(goto_t *go) {
		p_global->valleyVidNum = go->arriveVid;
		p_global->travelTime = go->travelTime * 256;
		p_global->stepsToFindAppleFast = 0;
		p_global->eventType = EventType::etEvent2;
		init_oui();
		showevents();
		if (!verif_oui())
			return;
		if (p_global->ff_113) {
			waitendspeak();
			if (!pomme_q)
				close_perso();
		}
		if (go->departVid) {
			bars_out();
			playhnm(go->departVid);
			needToFade = 1;
		}
		initlieu(p_global->newRoomNum);
		specialoutside();
		faire_suivre(p_global->newRoomNum);
		closesalle();
		saved_repadam.x = -1;
		saved_repadam.y = -1;
		temps_passe(p_global->travelTime);
		p_global->ff_100 = p_global->room_ptr->ff_0;
		p_global->roomNum = p_global->newRoomNum;
		p_global->areaNum = p_global->roomNum >> 8;
		p_global->eventType = EventType::etEvent5;
		p_global->newMusicType = MusicType::mt2;
		setpersohere();
		musique();
		majsalle1(p_global->roomNum);
		afftopscr();
		saved_repadam.x = -1;
		saved_repadam.y = -1;
	}
	void deplaval(unsigned short roomNum) {
		unsigned char c1, newAreaNum, curAreaNum;
		short newRoomNum;
		p_global->newLocation = roomNum & 0xFF;
		p_global->valleyVidNum = 0;
		p_global->phaseActionsCount++;
		closesalle();
		endpersovox();
		c1 = roomNum & 0xFF;
		if (c1 == 0)
			return;
		if (c1 < 0x80) {
			p_global->displayFlags = DisplayFlags::dfFlag1;
			init_oui();
			p_global->eventType = EventType::etEvent1;
			showevents();
			if (!verif_oui())
				return;
			if (p_global->ff_113) {
				waitendspeak();
				if (!pomme_q)
					close_perso();
			}
			specialout();
			if (p_global->area_ptr->type == AreaType::atValley) {
				temps_passe(32);
				p_global->stepsToFindAppleFast++;
				p_global->stepsToFindAppleNormal++;
			}
			faire_suivre((roomNum & 0xFF00) | p_global->newLocation);
			p_global->ff_100 = p_global->room_ptr->ff_0;
			p_global->roomNum = roomNum;
			p_global->areaNum = roomNum >> 8;
			p_global->eventType = EventType::etEvent5;
			setpersohere();
			p_global->newMusicType = MusicType::mtNormal;
			musique();
			majsalle1(roomNum);
			p_global->chrono_on = 0;
			p_global->chrono = 0;
			p_global->ff_54 = 0;
			if (p_global->roomPersoType == PersonFlags::pftTyrann)
				chronoon(3000);
			return;
		}
		if (c1 == 0xFF) {
			p_global->eventType = EventType::etEventE;
			showevents();
			if (!kPersons[PER_MESSAGER].roomNum) {
				if (eloirevientq())
					chronoon(800);
			}
			return;
		}
		p_global->stepsToFindAppleFast = 0;
		newAreaNum = c1 & 0x7F;
		curAreaNum = p_global->roomNum >> 8;
		newRoomNum = newAreaNum << 8;
		if (curAreaNum == Areas::arTausCave && newAreaNum == Areas::arMo)
			newRoomNum |= 0x16;
		else if (curAreaNum == Areas::arMoorkusLair)
			newRoomNum |= 4;
		else
			newRoomNum |= 1;
		p_global->newRoomNum = newRoomNum;
		if (newAreaNum == Areas::arTausCave)
			gotolieu(&gotos[0]);
		else {
			goto_t *go;
			for (go = gotos + 1; go->curAreaNum != 0xFF; go++)
				if (go->curAreaNum == curAreaNum) {
					gotolieu(go);
					break;
				}
		}
	}
	void deplacement(short dir) {
		room_t *room = p_global->room_ptr;
		short roomNum = p_global->roomNum;
		debug("deplacement: from room %4X", roomNum);
		char newLoc;
		rundcurs();
		afficher();
		p_global->prevLocation = roomNum & 0xFF;
		switch (dir) {
		case 0:
			newLoc = room->exits[0];
			break;
		case 1:
			newLoc = room->exits[1];
			break;
		case 2:
			newLoc = room->exits[2];
			break;
		case 3:
			newLoc = room->exits[3];
			break;
		}
		deplaval((roomNum & 0xFF00) | newLoc);
	}
	void deplacement2(short dir) {
		room_t *room = p_global->room_ptr;
		short roomNum = p_global->roomNum;
		char newLoc;
		p_global->prevLocation = roomNum & 0xFF;
		switch (dir) {
		case 0:
			newLoc = room->exits[0];
			break;
		case 1:
			newLoc = room->exits[1];
			break;
		case 2:
			newLoc = room->exits[2];
			break;
		case 3:
			newLoc = room->exits[3];
			break;
		}
		deplaval((roomNum & 0xFF00) | newLoc);

	}
	void dinosoufle() {
		if (p_global->curObjectId == 0) {
			bars_out();
			playhnm(148);
			maj2();
		}
	}
	void plaquemonk() {
		if (p_global->curObjectId != 0) {
			if (p_global->curObjectId == Objects::obPrism) {
				loseobject(Objects::obPrism);
				bars_out();
				specialTextMode = 1;
				playhnm(89);
				word_2F514 |= 0x8000;
				maj2();
				p_global->eventType = EventType::etEventB;
				showevents();
			}
		} else {
			bars_out();
			playhnm(7);
			maj2();
			p_global->eventType = EventType::etEvent4;
			showevents();
		}
	}
	void fresquesgraa() {
		if (p_global->curObjectId == 0) {
			p_global->fresqWidth = 320;
			p_global->fresqImgBank = 113;
			dofresques();
			dinaparle();
		}
	}
	void fresqueslasc() {
		if (p_global->curObjectId == 0) {
			p_global->fresqWidth = 112;
			p_global->fresqImgBank = 315;
			dofresques();
		}
	}
	void pushpierre() {
		if (p_global->curObjectId == 0) {
			gameRooms[22].exits[0] = 17;
			gameRooms[26].exits[2] = 9;
			deplacement(0);
		}
	}
	void tetemomie() {
		if (p_global->curObjectId == Objects::obTooth) {
			p_global->gameFlags |= GameFlags::gfMummyOpened;
			deplacement(0);
		} else if (p_global->curObjectId == 0) {
			if (p_global->gameFlags & GameFlags::gfMummyOpened)
				deplacement(0);
			else {
				p_global->eventType = EventType::etEvent6;
				persoparle(PersonId::pidMonk);
				p_global->eventType = 0;
			}
		}
	}
	void tetesquel() {
		if (p_global->curObjectId == Objects::obTooth) {
			gameRooms[22].exits[0] = 16;
			gameRooms[26].exits[2] = 13;
			gameIcons[16].cursor_id |= 0x8000;
			loseobject(Objects::obTooth);
			deplacement(0);
		}
	}
	void squelmoorkong() {
		p_global->eventType = EventType::etEvent9;
		showevents();
	}
	void choisir() {
		unsigned char obj, objid = current_spot2->object_id;
		switch (objid) {
		case 0:
			obj = p_global->giveobj1;
			break;
		case 1:
			obj = p_global->giveobj2;
			break;
		case 2:
			obj = p_global->giveobj3;
			break;
		}
		objectmain(obj);
		winobject(obj);
		p_global->iconsIndex = 16;
		p_global->autoDialog = 0;
		p_global->ff_60 = 0;
		parle_moi();
	}
	void dinaparle() {
		short num;
		char res;
		perso_t *perso = &kPersons[PER_DINA];
		if (perso->party & (p_global->party | p_global->partyOutside)) {
			if (p_global->fresqNumber < 3)
				p_global->fresqNumber = 3;
			p_global->fresqNumber++;
			if (p_global->fresqNumber < 15) {
				endpersovox();
				if (p_global->fresqNumber == 7 && p_global->phaseNum == 113)
					incphase1();
				p_global->perso_ptr = perso;
				p_global->dialogType = DialogType::dtInspect;
				num = (perso->id << 3) | DialogType::dtInspect; //TODO: combine
				res = dialoscansvmas((dial_t *)GetElem(gameDialogs, num));
				fresqTalk = 0;
				if (res) {
					restaurefondbulle();
					fresqTalk = 1;
					persovox();
				}
				p_global->ff_CA = 0;
				p_global->dialogType = 0;
			} else
				finfresques();
		}
	}
	void roiparle() {
		if (p_global->phaseNum <= 400)
			persoparle(0);
	}
	void roiparle1() {
		if (p_global->curObjectId == Objects::obSword) {
			p_global->gameFlags |= GameFlags::gfFlag80;
			bars_out();
			playhnm(76);
			deplacement2(0);
		} else {
			p_global->fresqNumber = 1;
			roiparle();
		}
	}
	void roiparle2() {
		p_global->fresqNumber = 2;
		roiparle();
	}
	void roiparle3() {
		p_global->fresqNumber = 3;
		roiparle();
	}
	void getcouteau() {
		if (p_global->phaseNum >= 80) {
			gameRooms[113].video = 0;
			getobject(Objects::obKnife);
		}
		p_global->eventType = EventType::etEvent7;
		showevents();
	}
	void getprisme() {
		getobject(Objects::obPrism);
		p_global->eventType = EventType::etEvent7;
		showevents();
	}
	void getchampb() {
		getobject(Objects::obShroom);
	}
	void getchampm() {
		getobject(Objects::obBadShroom);
	}
	void getor() {
		getobject(Objects::obGold);
	}
	void getnido() {
		if (p_global->curObjectId != 0)
			return;
		p_global->room_ptr->bank = 282; //TODO: fix me
		p_global->room_ptr--;
		p_global->room_ptr->bank = 281; //TODO: fix me
		p_global->room_ptr->ff_0 = 3;
		getobject(Objects::obFullNest);
	}
	void getnidv() {
		if (p_global->curObjectId != 0)
			return;
		p_global->room_ptr->bank = 282; //TODO: fix me
		p_global->room_ptr--;
		p_global->room_ptr->bank = 281; //TODO: fix me
		p_global->room_ptr->ff_0 = 3;
		getobject(Objects::obNest);
	}
	void getcorne() {
		if (p_global->curObjectId != 0)
			return;
		getobject(Objects::obHorn);
		p_global->eventType = EventType::etEvent7;
		showevents();
		bigphase1();
		setpersohere();
		p_global->room_ptr = getsalle(p_global->roomNum);
	}
	void getsoleil() {
		if (p_global->curObjectId != 0)
			return;
		gameRooms[238].video = 0;
		gameRooms[238].flags = RoomFlags::rf80;
		getobject(Objects::obSunStone);
	}
	void getoeuf() {
		if (p_global->curObjectId != 0)
			return;
		p_global->room_ptr->flags = 0;
		p_global->room_ptr->video = 0;
		getobject(Objects::obEgg);
	}
	void getplaque() {
		int i;
		if (p_global->curObjectId != 0 && p_global->curObjectId < Objects::obTablet1)
			return;
		p_global->curObjectId = 0;
		getobject(Objects::obTablet2);
		putobject();
		for (i = 0; i < 6; i++)
			objects[Objects::obTablet1 - 1 + i].count = 0;
		p_global->curObjectFlags = 0;
		p_global->inventoryScrollPos = 0;
		p_global->curObjectCursor = 9;
		gameIcons[16].cursor_id |= 0x8000;
		showobjects();
		gameRooms[131].video = 0;
		bars_out();
		playhnm(149);
		p_global->ff_F1 = RoomFlags::rf04;
		p_global->drawFlags = DrawFlags::drDrawFlag20;
		normalCursor = 1;
		maj2();
	}
	void voirlac() {
		perso_t *perso = &kPersons[PER_MORKUS];
		room_t *room = p_global->room_ptr;
		area_t *area = p_global->area_ptr;
		short vid = p_global->curObjectId == Objects::obApple ? 81 : 54;
		for (++perso; perso->roomNum != 0xFFFF; perso++) {
			if (perso->roomNum != p_global->roomNum)
				continue;
			vid++;
			if (p_global->curObjectId != Objects::obApple)
				continue;                   //TODO: pc breaks here
			if ((perso->flags & PersonFlags::pfTypeMask) != PersonFlags::pftMosasaurus)
				continue;
			if (!(perso->flags & PersonFlags::pf80))
				return;
			perso->flags &= ~PersonFlags::pf80; //TODO: useless? see above
			area->flags |= AreaFlags::afFlag8;
			p_global->curAreaFlags |= AreaFlags::afFlag8;
			room->ff_0 = 3;
		}
		debug("sea monster: room = %X, d0 = %X\n", p_global->roomNum, p_global->roomImgBank);
		bars_out();
		playhnm(vid);
		maj_salle(p_global->roomNum);           //TODO: getting memory trashed here?
		if (p_global->curObjectId == Objects::obApple)
			loseobject(Objects::obApple);
		p_global->eventType = EventType::etEventF;
		showevents();
	}
	void gotohall() {
		p_global->prevLocation = p_global->roomNum & 0xFF;
		deplaval((p_global->roomNum & 0xFF00) | 6);
	}
	void demitourlabi() {
		unsigned short target;
		p_global->prevLocation = p_global->roomNum & 0xFF;
		p_global->ff_100 = -1;
		target = (p_global->roomNum & 0xFF00) | p_global->room_ptr->exits[2];
		faire_suivre(target);
		p_global->roomNum = target;
		p_global->eventType = EventType::etEvent5;
		maj_salle(p_global->roomNum);
	}
	void gotonido() {
		p_global->room_ptr++;
		p_global->eventType = 0;
		p_global->roomImgBank = p_global->room_ptr->bank;
		p_global->roomVidNum = p_global->room_ptr->video;
		p_global->curRoomFlags = p_global->room_ptr->flags;
		p_global->ff_F1 = p_global->room_ptr->flags;
		animpiece();
		p_global->ff_100 = 0;
		maj2();
	}
	void gotoval() {
		unsigned short target = p_global->roomNum;
		char obj;
		rundcurs();
		afficher();
		scroll_pos = 0;
		obj = current_spot2->object_id - 14;    //TODO
		p_global->prevLocation = target & 0xFF;
		deplaval((target & 0xFF00) | obj);  //TODO careful!
	}
	void visiter() {
		bars_out();
		playhnm(144);
		p_global->ff_F1 = RoomFlags::rf04;
		maj2();
	}
	void final() {
		if (p_global->curObjectId != 0)
			return;
		bars_out();
		*(short *)(gameRooms + 0x6DC) = 319; //TODO
		p_global->roomImgBank = 319;
		playhnm(97);
		maj2();
		p_global->eventType = EventType::etEvent12;
		showevents();
		p_global->narratorSequence = 54;
	}
	void goto_nord() {
		if (p_global->curObjectId == 0)
			deplacement(0);
	}
	void goto_est() {
		if (p_global->curObjectId == 0)
			deplacement(1);
	}
	void goto_sud() {
		if (p_global->curObjectId == 0)
			deplacement(2);
	}
	void goto_ouest() {
		if (p_global->curObjectId == 0)
			deplacement(3);
	}
	void afficher() {
		if (!p_global->ff_102 && !p_global->ff_103) {
			if (needPaletteUpdate) {
				needPaletteUpdate = 0;
				CLPalette_Send2Screen(global_palette, 0, 256);
			}
			CLBlitter_CopyView2Screen(p_mainview);
		} else {
			if (p_global->ff_102)
				effet3();
			else
				effet2();
			p_global->ff_103 = 0;
			p_global->ff_102 = 0;
		}
	}
	void afficher128() {
		if (p_global->updatePaletteFlag == 16) {
			CLPalette_Send2Screen(global_palette, 0, 129);
			CLBlitter_CopyView2Screen(p_mainview);
			p_global->updatePaletteFlag = 0;
		} else {
			ClearScreen();
			fadetoblack128(1);
			if (showBlackBars)
				blackbars();
			CLBlitter_CopyView2Screen(p_mainview);
			fadefromblack128(1);
		}
	}
	void sauvefrises() {
		sauvefriseshaut(0);
		sauvefrisesbas();
	}
	void sauvefriseshaut(short x) { // Save top bar
		underTopBarScreenRect.sy = 0;       //TODO: wrong fields order?
		underTopBarScreenRect.sx = x;
		underTopBarScreenRect.ex = x + 320 - 1;
		underTopBarScreenRect.ey = 15;
		underTopBarBackupRect.sy = 0;
		underTopBarBackupRect.sx = 0;
		underTopBarBackupRect.ex = 320 - 1;
		underTopBarBackupRect.ey = 15;
		CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &underTopBarScreenRect, &underTopBarBackupRect);
	}
	void sauvefrisesbas() {         // Save bottom bar
		underBottomBarScreenRect.sx = 0;
		underBottomBarScreenRect.ex = 320 - 1;
		CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &underBottomBarScreenRect, &underBottomBarBackupRect);
	}
	void restaurefrises() {
		restaurefriseshaut();
		restaurefrisesbas();
	}
	void restaurefriseshaut() {
		underTopBarScreenRect.sx = scroll_pos;
		underTopBarScreenRect.ex = scroll_pos + 320 - 1;
		CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &underTopBarBackupRect, &underTopBarScreenRect);
	}
	void restaurefrisesbas() {
		underBottomBarScreenRect.sx = scroll_pos;
		underBottomBarScreenRect.ex = scroll_pos + 320 - 1;
		CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &underBottomBarBackupRect, &underBottomBarScreenRect);
	}
	void use_main_bank() {
		bank_data_ptr = main_bank_buf;
	}
	void use_bank(short bank) {
		if (bank > 2500)
			debug("attempt to load bad bank %d", bank);
		bank_data_ptr = bank_data_buf;
		if (cur_bank_num != bank) {
			loadfile(bank, bank_data_buf);
			verifh(bank_data_buf);
			cur_bank_num = bank;
		}
	}
	void sundcurs(short x, short y) {
		unsigned char *scr, *keep = curs_keepbuf;
		short w, h;
		curs_keepx = x - 4;
		curs_keepy = y - 4;
		scr = p_mainview_buf + curs_keepx + curs_keepy * 640;
		for (h = 48; h--;) {
			for (w = 48; w--;)
				*keep++ = *scr++;
			scr += 640 - 48;
		}
		curs_saved = 1;
	}
	void rundcurs() {
		unsigned char *scr, *keep = curs_keepbuf;
		short w, h;
		scr = p_mainview_buf + curs_keepx + curs_keepy * 640;
		if (!curs_saved || (curs_keepx == -1 && curs_keepy == -1))  //TODO ...
			return;
		for (h = 48; h--;) {
			for (w = 48; w--;)
				*scr++ = *keep++;
			scr += 640 - 48;
		}

	}
	void noclipax(short index, short x, short y) {
		unsigned char *pix = bank_data_ptr;
		unsigned char *scr = p_mainview_buf + x + y * 640;
		unsigned char h0, h1, mode;
		short w, h;
		if (cur_bank_num != 117 && !no_palette) {
			if (PLE16(pix) > 2)
				readpalette(pix + 2);
		}
		pix += PLE16(pix);
		pix += PLE16(pix + index * 2);
		//  short   height:9
		//  short   pad:6;
		//  short   flag:1;
		h0 = *pix++;
		h1 = *pix++;
		w = ((h1 & 1) << 8) | h0;
		h = *pix++;
		mode = *pix++;
		debug("- draw sprite %d at %d:%d, %dx%d", index, x, y, w, h);
		if (mode != 0xFF && mode != 0xFE)
			return;
		if (y + h > 200)
			h -= (y + h - 200);
		if (h1 & 0x80) {
			// compressed
			for (; h-- > 0;) {
				short ww;
				for (ww = w; ww > 0;) {
					unsigned char c = *pix++;
					if (c >= 0x80) {
						if (c == 0x80) {
							unsigned char fill = *pix++;
							if (fill == 0) {
								scr += 128 + 1;
								ww -= 128 + 1;
							} else {
								unsigned char run;
								*scr++ = fill;  //TODO: wha?
								*scr++ = fill;
								ww -= 128 + 1;
								for (run = 127; run--;)
									*scr++ = fill;
							}
						} else {
							unsigned char fill = *pix++;
							unsigned char run = 255 - c + 2;
							ww -= run;
							if (fill == 0)
								scr += run;
							else
								for (; run--;)
									*scr++ = fill;
						}
					} else {
						unsigned char run = c + 1;
						ww -= run;
						for (; run--;) {
							unsigned char p = *pix++;
							if (p == 0)
								scr++;
							else
								*scr++ = p;
						}
					}
				}
				scr += 640 - w;
			}
		} else {
			// uncompressed
			for (; h--;) {
				short ww;
				for (ww = w; ww--;) {
					unsigned char p = *pix++;
					if (p == 0)
						scr++;
					else
						*scr++ = p;
				}
				scr += 640 - w;
			}
		}
	}
	void noclipax_avecnoir(short index, short x, short y) {
		unsigned char *pix = bank_data_ptr;
		unsigned char *scr = p_mainview_buf + x + y * 640;
		unsigned char h0, h1, mode;
		short w, h;
		if (cur_bank_num != 117) {
			if (PLE16(pix) > 2)
				readpalette(pix + 2);
		}
		pix += PLE16(pix);
		pix += PLE16(pix + index * 2);
		//  short   height:9
		//  short   pad:6;
		//  short   flag:1;
		h0 = *pix++;
		h1 = *pix++;
		w = ((h1 & 1) << 8) | h0;
		h = *pix++;
		mode = *pix++;
		if (mode != 0xFF && mode != 0xFE)
			return;
		if (y + h > 200)
			h -= (y + h - 200);
		if (h1 & 0x80) {
			// compressed
			for (; h-- > 0;) {
				short ww;
				for (ww = w; ww > 0;) {
					unsigned char c = *pix++;
					if (c >= 0x80) {
						if (c == 0x80) {
							unsigned char fill = *pix++;
							unsigned char run;
							*scr++ = fill;  //TODO: wha?
							*scr++ = fill;
							ww -= 128 + 1;
							for (run = 127; run--;)
								*scr++ = fill;
						} else {
							unsigned char fill = *pix++;
							unsigned char run = 255 - c + 2;
							ww -= run;
							for (; run--;)
								*scr++ = fill;
						}
					} else {
						unsigned char run = c + 1;
						ww -= run;
						for (; run--;) {
							unsigned char p = *pix++;
							*scr++ = p;
						}
					}
				}
				scr += 640 - w;
			}
		} else {
			// uncompressed
			for (; h--;) {
				short ww;
				for (ww = w; ww--;) {
					unsigned char p = *pix++;
					*scr++ = p;
				}
				scr += 640 - w;
			}
		}
	}
	void getglow(short x, short y, short w, short h) {
		unsigned char *scr = p_mainview_buf + x + y * 640;
		unsigned char *gl = glow_buffer;
		glow_x = x;
		glow_y = y;
		glow_w = w;
		glow_h = h;
		for (; h--;) {
			short ww;
			for (ww = w; ww--;)
				*gl++ = *scr++;
			scr += 640 - w;
		}
	}
	void unglow() {
		unsigned char *gl = glow_buffer;
		unsigned char *scr = p_mainview_buf + glow_x + glow_y * 640;
		if (glow_x < 0 || glow_y < 0)   //TODO: move it up
			return;
		for (; glow_h--;) {
			short ww;
			for (ww = glow_w; ww--;)
				*scr++ = *gl++;
			scr += 640 - glow_w;
		}
	}
	void glow(short index) {
		unsigned char pixbase;
		unsigned char *pix = bank_data_ptr;
		unsigned char *scr;
		unsigned char h0, h1, mode;
		short w, h, x, y, ex, dx, dy, pstride, sstride;
		index += 9;
		pix += PLE16(pix);
		pix += PLE16(pix + index * 2);
		//  short   height:9
		//  short   pad:6;
		//  short   flag:1;
		h0 = *pix++;
		h1 = *pix++;
		w = ((h1 & 1) << 8) | h0;
		h = *pix++;
		mode = *pix++;
		if (mode != 0xFF && mode != 0xFE)
			return;

		x = curs_x + scroll_pos - 38;
		y = curs_y - 28;
		ex = p_global->fresqWidth + 320;

		if (x + w <= 0 || x >= ex || y + h <= 0 || y >= 176)
			return;

		if (x < 0) {
			dx = -x;
			x = 0;
		} else if (x + w > ex)
			dx = x + w - ex;
		else
			dx = 0;

		if (y < 16) {
			dy = 16 - y;
			y = 16;
		} else if (y + h > 175)
			dy = y + h - 175;
		else
			dy = 0;
		pstride = dx;
		sstride = 640 - (w - dx);
		if (y == 16)
			pix += w * dy;
		if (x == 0)
			pix += dx;

		scr = p_mainview_buf + x + y * 640;

		w -= dx;
		h -= dy;

		getglow(x, y, w, h);

		for (; h--;) {
			short ww;
			for (ww = w; ww--;) {
				unsigned char p = *pix++;
				if (p == 0)
					scr++;
				else
					*scr++ += p << 4;
			}
			pix += pstride;
			scr += sstride;
		}
	}
	void readpalette(unsigned char *ptr) {
		int doit = 1;
		while (doit) {
			unsigned short idx = *ptr++;
			if (idx != 0xFF) {
				unsigned short cnt = *ptr++;
				while (cnt--) {
					if (idx == 0) {
						pal_entry.r = 0;
						pal_entry.g = 0;
						pal_entry.b = 0;
						ptr += 3;
					} else {
						pal_entry.r = *ptr++ << 10;
						pal_entry.g = *ptr++ << 10;
						pal_entry.b = *ptr++ << 10;
					}
					CLPalette_SetRGBColor(global_palette, idx, &pal_entry);
					idx++;
				}
			} else
				doit = 0;
		}
	}
	void spritesurbulle(short index, short x, short y) {
		unsigned char *pix = bank_data_ptr;
		unsigned char *scr = p_subtitlesview_buf + x + y * subtitles_x_width;
		unsigned char h0, h1, mode;
		short w, h;
		if (cur_bank_num != 117) {
			if (PLE16(pix) > 2)
				readpalette(pix + 2);
		}
		pix += PLE16(pix);
		pix += PLE16(pix + index * 2);
		//  short   height:9
		//  short   pad:6;
		//  short   flag:1;
		h0 = *pix++;
		h1 = *pix++;
		w = ((h1 & 1) << 8) | h0;
		h = *pix++;
		mode = *pix++;
		if (mode != 0xFF && mode != 0xFE)
			return;
		if (h1 & 0x80) {
			// compressed
			for (; h-- > 0;) {
				short ww;
				for (ww = w; ww > 0;) {
					unsigned char c = *pix++;
					if (c >= 0x80) {
						if (c == 0x80) {
							unsigned char fill = *pix++;
							if (fill == 0) {
								scr += 128 + 1;
								ww -= 128 + 1;
							} else {
								unsigned char run;
								*scr++ = fill;  //TODO: wha?
								*scr++ = fill;
								ww -= 128 + 1;
								for (run = 127; run--;)
									*scr++ = fill;
							}
						} else {
							unsigned char fill = *pix++;
							unsigned char run = 255 - c + 2;
							ww -= run;
							if (fill == 0)
								scr += run;
							else
								for (; run--;)
									*scr++ = fill;
						}
					} else {
						unsigned char run = c + 1;
						ww -= run;
						for (; run--;) {
							unsigned char p = *pix++;
							if (p == 0)
								scr++;
							else
								*scr++ = p;
						}
					}
				}
				scr += subtitles_x_width - w;
			}
		} else {
			// uncompressed
			for (; h--;) {
				short ww;
				for (ww = w; ww--;) {
					unsigned char p = *pix++;
					if (p == 0)
						scr++;
					else
						*scr++ = p;
				}
				scr += subtitles_x_width - w;
			}
		}
	}
	void bars_out() {
		short i, r19, r20, r25, r24;
		unsigned int *scr40, *scr41, *scr42;
		if (showBlackBars)
			return;
		afficher();
		underTopBarScreenRect.sx = scroll_pos;
		underTopBarScreenRect.ex = scroll_pos + 320 - 1;
		CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &underTopBarScreenRect, &underTopBarBackupRect);
		underBottomBarScreenRect.sx = underTopBarScreenRect.sx;
		underBottomBarScreenRect.ex = underTopBarScreenRect.ex;
		CLBlitter_CopyViewRect(p_mainview, p_underBarsView, &underBottomBarScreenRect, &underBottomBarBackupRect);
		r19 = 14;   // TODO - init in decl?
		r20 = 176;
		r25 = 14;
		r24 = 21;
		underTopBarScreenRect.sx = 0;
		underTopBarScreenRect.ex = 320 - 1;
		underTopBarBackupRect.sx = scroll_pos;
		underTopBarBackupRect.ex = scroll_pos + 320 - 1;
		while (r24 > 0) {
			if (r25 > 0) {
				underTopBarScreenRect.sy = 16 - r25;
				underTopBarScreenRect.ey = 16 - 1;
				underTopBarBackupRect.sy = 0;
				underTopBarBackupRect.ey = r25 - 1;
				CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &underTopBarScreenRect, &underTopBarBackupRect);
				scr40 = ((unsigned int *)p_mainview_buf) + r19 * 640 / 4;
				scr41 = scr40 + 640 / 4;
				for (i = 0; i < 320; i += 4) {
					*scr40++ = 0;
					*scr41++ = 0;
				}
			}
			underTopBarScreenRect.sy = 16;
			underTopBarScreenRect.ey = r24 + 16 - 1;
			underTopBarBackupRect.sy = 200 - r24;
			underTopBarBackupRect.ey = 200 - 1;
			CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &underTopBarScreenRect, &underTopBarBackupRect);
			scr40 = ((unsigned int *)p_mainview_buf) + r20 * 640 / 4;
			scr41 = scr40 + 640 / 4;
			scr42 = scr41 + 640 / 4;
			for (i = 0; i < 320; i += 4) {
				*scr40++ = 0;
				*scr41++ = 0;
				*scr42++ = 0;
			}
			r19 -= 2;
			r20 += 3;
			r25 -= 2;
			r24 -= 3;
			afficher();
		}
		scr40 = (unsigned int *)p_mainview_buf;
		scr41 = scr40 + 640 / 4;
		for (i = 0; i < 320; i += 4) {
			*scr40++ = 0;
			*scr41++ = 0;
		}
		scr40 = ((unsigned int *)p_mainview_buf) + r20 * 640 / 4;
		scr41 = scr40 + 640 / 4;
		scr42 = scr41 + 640 / 4;
		for (i = 0; i < 320; i += 4) {
			*scr40++ = 0;
			*scr41++ = 0;
			*scr42++ = 0;
		}
		afficher();
		initrect();
		showBlackBars = 1;
	}
	void bars_in() {
		short r29, r28;
		if (!showBlackBars)
			return;
		blackbars();
		r29 = 2;
		r28 = 2;
		underTopBarScreenRect.sx = 0;
		underTopBarScreenRect.ex = 320 - 1;
		underTopBarBackupRect.sx = scroll_pos;
		underTopBarBackupRect.ex = scroll_pos + 320 - 1;
		while (r28 < 24) {
			if (r29 <= 16) {
				underTopBarScreenRect.sy = 16 - r29;
				underTopBarScreenRect.ey = 16 - 1;
				underTopBarBackupRect.sy = 0;
				underTopBarBackupRect.ey = r29 - 1;
				CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &underTopBarScreenRect, &underTopBarBackupRect);
			}
			underTopBarScreenRect.sy = 16;
			underTopBarScreenRect.ey = 16 + r28;
			underTopBarBackupRect.sy = 200 - 1 - r28;
			underTopBarBackupRect.ey = 200 - 1;
			CLBlitter_CopyViewRect(p_underBarsView, p_mainview, &underTopBarScreenRect, &underTopBarBackupRect);
			r29 += 2;
			r28 += 3;
			afficher();
		}
		initrect();
		showBlackBars = 0;
	}
	void sauvefondbouche() {
		rect_src.sx = cur_perso_rect->sx;
		rect_src.sy = cur_perso_rect->sy;
		rect_src.ex = cur_perso_rect->ex;
		rect_src.ey = cur_perso_rect->ey;
		rect_dst.sx = cur_perso_rect->sx + 320;
		rect_dst.sy = cur_perso_rect->sy;
		rect_dst.ex = cur_perso_rect->ex + 320;
		rect_dst.ey = cur_perso_rect->ey;
		CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
		fond_saved = 1;
	}
	void restaurefondbouche() {
		rect_src.sx = cur_perso_rect->sx;
		rect_src.sy = cur_perso_rect->sy;
		rect_src.ex = cur_perso_rect->ex;
		rect_src.ey = cur_perso_rect->ey;
		rect_dst.sx = cur_perso_rect->sx + 320;
		rect_dst.sy = cur_perso_rect->sy;
		rect_dst.ex = cur_perso_rect->ex + 320;
		rect_dst.ey = cur_perso_rect->ey;
		CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_dst, &rect_src);
	}
	void blackbars() {
		unsigned char *scr = p_mainview_buf;
		short x, y;
		for (y = 0; y < 16; y++)
			for (x = 0; x < 640; x++)
				*scr++ = 0;
		scr += 640 * (200 - 16 - 24);
		for (y = 0; y < 24; y++)
			for (x = 0; x < 640; x++)
				*scr++ = 0;
	}
	void afftopscr() {  // Draw  top bar (location / party / map)
		perso_t *perso;
		p_global->drawFlags &= ~DrawFlags::drDrawTopScreen;
		use_bank(314);
		noclipax(36, 83, 0);
		noclipax(p_global->area_ptr->num - 1, 0, 0);
		noclipax(23, 145, 0);
		for (perso = &kPersons[PER_DINA]; perso != &kPersons[PER_UNKN_156]; perso++) {
			if ((perso->flags & PersonFlags::pfInParty) && !(perso->flags & PersonFlags::pf80))
				noclipax(perso->targetLoc + 18, perso->lastLoc + 120, 0);
		}
		saved_repadam.x = -1;
		saved_repadam.y = -1;
		affplanval();
		needPaletteUpdate = 1;
	}
	void affplanval() { // Draw mini-map
		short loc;
		perso_t *perso;
		if (p_global->area_ptr->type == AreaType::atValley) {
			noclipax(p_global->area_ptr->num + 9, 266, 1);
			for (perso = &kPersons[PER_UNKN_18C]; perso->roomNum != 0xFFFF; perso++) {
				if (((perso->roomNum >> 8) == p_global->areaNum)
				        && !(perso->flags & PersonFlags::pf80) && (perso->flags & PersonFlags::pf20))
					affrepere(33, perso->roomNum & 0xFF);
			}
			if (p_global->area_ptr->citadelLevel)
				affrepere(34, p_global->area_ptr->citadelRoom->location);
			sauvefriseshaut(0);
			loc = p_global->roomNum & 0xFF;
			if (loc >= 16)
				affrepereadam(loc);
			restaurefriseshaut();
		} else {
			sauvefriseshaut(0);
			restaurefriseshaut();
		}
	}
	void affrepere(short index, short location) {
		noclipax(index, 269 + location % 16 * 4, 2 + (location - 16) / 16 * 3);
	}
	void affrepereadam(short location) {
		short x = 269;
		short y = 2;
		short w;
		unsigned char *pix;
		rest_repadam();
		if (location > 15 && location < 76) {
			x += (location & 15) * 4;
			y += ((location - 16) >> 4) * 3;
			save_repadam(x, y);
			pix = p_underBarsView->p_buffer;
			w = p_underBarsView->width;
			pix += x + w * y;
			pix[1] = 0xC3;
			pix[2] = 0xC3;
			pix += w;
			pix[0] = 0xC3;
			pix[1] = 0xC3;
			pix[2] = 0xC3;
			pix[3] = 0xC3;
			pix += w;
			pix[1] = 0xC3;
			pix[2] = 0xC3;
		}
	}
	void rest_repadam() {
		short x, y, w;
		unsigned char *pix;
		if (saved_repadam.x == -1 && saved_repadam.y == -1)
			return;
		x = saved_repadam.x;
		y = saved_repadam.y;
		pix = p_underBarsView->p_buffer;
		w = p_underBarsView->width;
		pix += x + w * y;
		pix[1] = keep01;    //TODO keep is array?
		pix[2] = keep02;
		pix += w;
		pix[0] = keep10;
		pix[1] = keep11;
		pix[2] = keep12;
		pix[3] = keep13;
		pix += w;
		pix[1] = keep21;
		pix[2] = keep22;
	}
	void save_repadam(short x, short y) {
		short w;
		unsigned char *pix;
		saved_repadam.x = x;
		saved_repadam.y = y;
		pix = p_underBarsView->p_buffer;
		w = p_underBarsView->width;
		pix += x + w * y;
		keep01 = pix[1];
		keep02 = pix[2];
		pix += w;
		keep10 = pix[0];
		keep11 = pix[1];
		keep12 = pix[2];
		keep13 = pix[3];
		pix += w;
		keep21 = pix[1];
		keep22 = pix[2];
	}
	char istrice(short roomNum) {
		char loc = roomNum & 0xFF;
		short area = roomNum & 0xFF00;
		perso_t *perso;
		for (perso = &kPersons[PER_UNKN_18C]; perso != &kPersons[PER_UNKN_372]; perso++) {
			if ((perso->flags & PersonFlags::pf80) || (perso->flags & PersonFlags::pfTypeMask) != PersonFlags::pftTriceraptor)
				continue;
			if (perso->roomNum == (area | (loc - 16)))
				return 1;
			if (perso->roomNum == (area | (loc + 16)))
				return 1;
			if (perso->roomNum == (area | (loc - 1)))
				return 1;
			if (perso->roomNum == (area | (loc + 1)))
				return 1;
		}
		return 0;
	}
	char istyran(short roomNum) {
		char loc = roomNum & 0xFF;
		short area = roomNum & 0xFF00;
		// TODO: orig bug: this ptr is not initialized when first called from getsalle
		// PC version scans kPersons[] directly and is not affected
		if (!tyranPtr)
			return 0;

		for (; tyranPtr->roomNum != 0xFFFF; tyranPtr++) {
			if (tyranPtr->flags & PersonFlags::pf80)
				continue;
			if (tyranPtr->roomNum == (area | (loc - 16)))
				return 1;
			if (tyranPtr->roomNum == (area | (loc + 16)))
				return 1;
			if (tyranPtr->roomNum == (area | (loc - 1)))
				return 1;
			if (tyranPtr->roomNum == (area | (loc + 1)))
				return 1;
		}
		return 0;
	}
	void istyranval(area_t *area) {
		perso_t *perso;
		unsigned char areaNum = area->num;
		area->flags &= ~AreaFlags::HasTyrann;
		for (perso = &kPersons[PER_UNKN_372]; perso->roomNum != 0xFFFF; perso++) {
			if (perso->flags & PersonFlags::pf80)
				continue;
			if ((perso->roomNum >> 8) == areaNum) {
				area->flags |= AreaFlags::HasTyrann;
				break;
			}
		}
	}
	char getdirection(perso_t *perso) {
		char dir = -1;
		unsigned char trgLoc = perso->targetLoc;
		unsigned char curLoc = perso->roomNum & 0xFF;   //TODO name
		if (curLoc != trgLoc) {
			curLoc &= 0xF;
			trgLoc &= 0xF;
			if (curLoc != trgLoc) {
				dir = 2;
				if (curLoc > trgLoc)
					dir = 5;
			}
			trgLoc = perso->targetLoc;
			curLoc = perso->roomNum & 0xFF;
			curLoc &= 0xF0;
			trgLoc &= 0xF0;
			if (curLoc != trgLoc) {
				if (curLoc > trgLoc)
					dir++;
				dir++;
			}
		}
		return dir;
	};
	char caselibre(char loc, perso_t *perso) {
		short roomNum;
		room_t *room = p_global->cita_area_firstRoom;
		if (loc <= 0x10 || loc > 76 || (loc & 0xF) >= 12 || loc == perso->lastLoc)
			return 0;
		roomNum = (perso->roomNum & ~0xFF) | loc;   //TODO: danger! signed
		if (roomNum == p_global->roomNum)
			return 0;
		for (; room->ff_0 != 0xFF; room++) {
			if (room->location != loc)
				continue;
			if (!(room->flags & RoomFlags::rf01))
				return 0;
			for (perso = &kPersons[PER_UNKN_18C]; perso->roomNum != 0xFFFF; perso++) {
				if (perso->flags & PersonFlags::pf80)
					continue;
				if (perso->roomNum == roomNum)
					break;
			}
			if (perso->roomNum != 0xFFFF)
				return 0;
			return 1;
		}
		return 0;
	}
	void melange1(char elem[4]) {
		if (g_ed->_rnd->getRandomNumber(1) & 1) {
			char e1 = elem[1];
			char e2 = elem[2];
			elem[1] = e2;
			elem[2] = e1;
		}
	}
	void melange2(char elem[4]) {
		if (g_ed->_rnd->getRandomNumber(1) & 1) {
			char e0 = elem[0];
			char e1 = elem[1];
			elem[0] = e1;
			elem[1] = e0;
		}
		if (g_ed->_rnd->getRandomNumber(1) & 1) {
			char e2 = elem[2];
			char e3 = elem[3];
			elem[2] = e3;
			elem[3] = e2;
		}
	}
	void melangedir() {
		melange1(tab_2CB1E[0]);
		melange1(tab_2CB1E[1]);
		melange1(tab_2CB1E[2]);
		melange2(tab_2CB1E[3]);
		melange2(tab_2CB1E[4]);
		melange1(tab_2CB1E[5]);
		melange2(tab_2CB1E[6]);
		melange2(tab_2CB1E[7]);
	}
	char naitredino(char persoType) {
		perso_t *perso;
		for (perso = &kPersons[PER_MORKUS]; (++perso)->roomNum != 0xFFFF;) {
			char areaNum = perso->roomNum >> 8;
			if (areaNum != p_global->cita_area_num)
				continue;
			if ((perso->flags & PersonFlags::pf80) && (perso->flags & PersonFlags::pfTypeMask) == persoType) {
				perso->flags &= ~PersonFlags::pf80;
				return 1;
			}
		}
		return 0;
	}
	void newcita(char arg1, short arg2, room_t *room) {
		unsigned short index;
		short *ptr;
		cita_t *cita = cita_list;
		while (cita->ff_0 < arg2)
			cita++;
		index = ((room->flags & 0xC0) >> 6);    //TODO: this is very wrong
		ptr = cita->ff_2 + index * 2;
		if (arg1 == 4 || arg1 == 6)
			ptr++;
		room->bank = ptr[0];
		room->video = ptr[8];
		room->flags |= RoomFlags::rf02;
	}
	void citaevol(short level) {
		room_t *room = p_global->cur_area_ptr->citadelRoom;
		perso_t *perso = &kPersons[PER_UNKN_372];
		unsigned char speed, loc = room->location;
		if (level >= 80 && !istrice((p_global->cita_area_num << 8) | room->location)) { //TODO: loc ?
			room->level = 79;
			return;
		}
		if (level > 160)
			level = 160;
		if (room->level < 64 && level >= 64 && naitredino(PersonFlags::pftTriceraptor)) {
			p_global->cur_area_ptr->flags |= AreaFlags::HasTriceraptors;
			ajouinfo(p_global->cita_area_num + ValleyNews::vnTriceraptorsIn);
		}
		if (room->level < 40 && level >= 40 && naitredino(PersonFlags::pftVelociraptor)) {
			p_global->cur_area_ptr->flags |= AreaFlags::HasVelociraptors;
			ajouinfo(p_global->cita_area_num + ValleyNews::vnVelociraptorsIn);
		}
		room->level = level;
		newcita(p_global->cita_area_num, level, room);
		speed = kDinoSpeedForCitaLevel[room->level >> 4];
		for (; perso->roomNum != 0xFFFF; perso++) {
			if (perso->flags & PersonFlags::pf80)
				continue;
			if ((perso->roomNum >> 8) == p_global->cita_area_num && perso->targetLoc == loc)
				perso->speed = speed;
		}
	}
	void citacapoute(short roomNum) {
		perso_t *perso = &kPersons[PER_UNKN_18C];
		room_t *room = p_global->cur_area_ptr->citadelRoom;
		room->flags |= RoomFlags::rf01;
		room->flags &= ~RoomFlags::rfHasCitadel;
		room->bank = 193;
		room->video = 0;
		room->level = 0;
		p_global->cur_area_ptr->citadelLevel = 0;
		p_global->cur_area_ptr->citadelRoom = 0;
		roomNum = (roomNum & ~0xFF) | room->location;
		for (; perso->roomNum != 0xFFFF; perso++) {
			if (perso->roomNum == roomNum) {
				perso->flags &= ~PersonFlags::pf80;
				delinfo((roomNum >> 8) + ValleyNews::vnTyrannIn);
				break;
			}
		}
	}
	void buildcita() {
		area_t *area = p_global->area_ptr;
		p_global->cur_area_ptr = p_global->area_ptr;
		if (area->citadelRoom)
			citacapoute(p_global->roomNum);
		p_global->ff_6A = p_global->ff_69;
		p_global->narratorSequence = p_global->ff_69 | 0x80;
		area->citadelRoom = p_global->room_ptr;
		p_global->room_ptr->flags &= ~RoomFlags::rf01;
		p_global->room_ptr->flags |= RoomFlags::rfHasCitadel;
		p_global->room_ptr->level = 32;
		newcita(p_global->areaNum, 32, p_global->room_ptr);
		area->flags &= ~AreaFlags::TyrannSighted;
		if (!(area->flags & AreaFlags::afFlag8000)) {
			if (p_global->phaseNum == 304 || p_global->phaseNum != 384) //TODO: wha
				eloirevient();
			area->flags |= AreaFlags::afFlag8000;
		}
		p_global->room_perso->flags |= PersonFlags::pf80;
		p_global->cita_area_num = p_global->areaNum;
		naitredino(1);
		delinfo(p_global->areaNum + ValleyNews::vnCitadelLost);
		delinfo(p_global->areaNum + ValleyNews::vnTyrannLost);
		if (p_global->phaseNum == 193 && p_global->areaNum == Areas::arUluru)
			bigphase1();
	}
	void citatombe(char level) {
		if (level)
			newcita(p_global->cita_area_num, level, p_global->cur_area_ptr->citadelRoom);
		else {
			citacapoute(p_global->cita_area_num << 8);
			ajouinfo(p_global->cita_area_num + ValleyNews::vnCitadelLost);
		}
	}
	void constcita() {
		unsigned char level;
		room_t *room;
		unsigned char loc;
//	room_t *room = p_global->cur_area_ptr->room_ptr; //TODO: wrong? chk below
//	unsigned char id = room->ff_C;
		if (!p_global->cur_area_ptr->citadelLevel || !p_global->cur_area_ptr->citadelRoom)
			return;
		room = p_global->cur_area_ptr->citadelRoom; //TODO: copied here by me
		loc = room->location;
		tyranPtr = &kPersons[PER_UNKN_372];
		if (istyran((p_global->cita_area_num << 8) | loc)) {
			if (!(p_global->cur_area_ptr->flags & AreaFlags::TyrannSighted)) {
				ajouinfo(p_global->cita_area_num + ValleyNews::vnTyrannIn);
				p_global->cur_area_ptr->flags |= AreaFlags::TyrannSighted;
			}
			level = room->level - 1;
			if (level < 32)
				level = 32;
			room->level = level;
			citatombe(level);
		} else {
			p_global->cur_area_ptr->flags &= ~AreaFlags::TyrannSighted;
			citaevol(room->level + 1);
		}
	}
	void depladino(perso_t *perso) {
		char *dirs, dir, dir2;
		unsigned char loc;
		dir = getdirection(perso);
		if (dir != -1) {
			melangedir();
			dirs = tab_2CB1E[dir];
			loc = perso->roomNum & 0xFF;
			dir2 = *dirs++;
			if (dir2 & 0x80)
				dir2 = -(dir2 & ~0x80);
			dir2 += loc;
			if (caselibre(dir2, perso))
				goto ok;
			dir2 = *dirs++;
			if (dir2 & 0x80)
				dir2 = -(dir2 & ~0x80);
			dir2 += loc;
			if (caselibre(dir2, perso))
				goto ok;
			dir2 = *dirs++;
			if (dir2 & 0x80)
				dir2 = -(dir2 & ~0x80);
			dir2 += loc;
			if (caselibre(dir2, perso))
				goto ok;
			dir2 = *dirs++;
			if (dir2 & 0x80)
				dir2 = -(dir2 & ~0x80);
			dir2 += loc;
			if (caselibre(dir2, perso))
				goto ok;
			dir2 = perso->lastLoc;
			perso->lastLoc = 0;
			if (!caselibre(dir2, perso))
				return;
ok:
			;
			perso->lastLoc = perso->roomNum & 0xFF;
			perso->roomNum &= ~0xFF;
			perso->roomNum |= dir2 & 0xFF;
			if (perso->targetLoc - 16 == (perso->roomNum & 0xFF))
				perso->targetLoc = 0;
			if (perso->targetLoc + 16 == (perso->roomNum & 0xFF))
				perso->targetLoc = 0;
			if (perso->targetLoc - 1 == (perso->roomNum & 0xFF))
				perso->targetLoc = 0;
			if (perso->targetLoc + 1 == (perso->roomNum & 0xFF))
				perso->targetLoc = 0;
		} else
			perso->targetLoc = 0;
	}
	void deplaalldino() {
		perso_t *perso = &kPersons[PER_UNKN_18C - 1];   //TODO fix this
		while ((++perso)->roomNum != 0xFFFF) {
			if (((perso->roomNum >> 8) & 0xFF) != p_global->cita_area_num)
				continue;
			if (perso->flags & PersonFlags::pf80)
				continue;
			if (!perso->targetLoc)
				continue;
			if (--perso->steps)
				continue;
			perso->steps = 1;
			if (perso->roomNum == p_global->roomNum)
				continue;
			perso->steps = perso->speed;
			depladino(perso);
		}
	}
	void newvallee() {
		perso_t *perso = &kPersons[PER_UNKN_372];
		short *ptr = tab_2CB16;
		short roomNum;
		while ((roomNum = *ptr++) != -1) {
			perso->roomNum = roomNum;
			perso->flags &= ~PersonFlags::pf80;
			perso->flags &= ~PersonFlags::pf20; //TODO: combine?
			perso++;
		}
		perso->roomNum = -1;
		kAreasTable[7].flags |= AreaFlags::HasTyrann;
		p_global->worldHasTyrann = 32;
	}
	char whereiscita() {
		room_t *room = p_global->cita_area_firstRoom;
		char res = -1;
		for (; room->ff_0 != 0xFF; room++) {
			if (!(room->flags & RoomFlags::rfHasCitadel))
				continue;
			res = room->location;
			break;
		}
		return res;
	}
	char iscita(short loc) {
		room_t *room = p_global->cita_area_firstRoom;
		loc &= 0xFF;
		for (; room->ff_0 != 0xFF; room++) {
			if (!(room->flags & RoomFlags::rfHasCitadel))
				continue;
			if (room->location == loc + 16)
				return 1;
			if (room->location == loc - 16)
				return 1;
			if (room->location == loc - 1)
				return 1;
			if (room->location == loc + 1)
				return 1;
		}
		return 0;
	}
	void lieuvava(area_t *area) {
		unsigned char mask;
		if (area->type == AreaType::atValley) {
			istyranval(area);
			area->citadelLevel = 0;
			if (area->citadelRoom)
				area->citadelLevel = p_global->cita_area_firstRoom->level;  //TODO: no search?
			mask = ~(1 << (area->num - Areas::arChamaar));
			p_global->worldTyrannSighted &= mask;
			p_global->ff_4E &= mask;
			p_global->worldGaveGold &= mask;
			p_global->worldHasVelociraptors &= mask;
			p_global->worldHasTriceraptors &= mask;
			p_global->worldHasTyrann &= mask;
			p_global->ff_53 &= mask;
			mask = ~mask;
			if (area->flags & AreaFlags::TyrannSighted)
				p_global->worldTyrannSighted |= mask;
			if (area->flags & AreaFlags::afFlag4)
				p_global->ff_4E |= mask;
			if (area->flags & AreaFlags::HasTriceraptors)
				p_global->worldHasTriceraptors |= mask;
			if (area->flags & AreaFlags::afGaveGold)
				p_global->worldGaveGold |= mask;
			if (area->flags & AreaFlags::HasVelociraptors)
				p_global->worldHasVelociraptors |= mask;
			if (area->flags & AreaFlags::HasTyrann)
				p_global->worldHasTyrann |= mask;
			if (area->flags & AreaFlags::afFlag20)
				p_global->ff_53 |= mask;
			if (area == p_global->area_ptr) {
				p_global->curAreaFlags = area->flags;
				p_global->curCitadelLevel = area->citadelLevel;
			}
		}
		p_global->ff_4D &= p_global->worldTyrannSighted;
	}
	void vivredino() {
		char cita;
		perso_t *perso = &kPersons[PER_UNKN_18C];
		for (; perso->roomNum != 0xFFFF; perso++) {
			if (((perso->roomNum >> 8) & 0xFF) != p_global->cita_area_num)
				continue;
			if (perso->flags & PersonFlags::pf80)
				continue;
			switch (perso->flags & PersonFlags::pfTypeMask) {
			case PersonFlags::pftTyrann:
				if (iscita(perso->roomNum))
					perso->targetLoc = 0;
				else if (!perso->targetLoc) {
					cita = whereiscita();
					if (cita != -1) {
						perso->targetLoc = cita;
						perso->speed = 2;
						perso->steps = 1;
					}
				}
				break;
			case PersonFlags::pftTriceraptor:
				if (perso->flags & PersonFlags::pfInParty) {
					if (iscita(perso->roomNum))
						perso->targetLoc = 0;
					else if (!perso->targetLoc) {
						cita = whereiscita();
						if (cita != -1) {
							perso->targetLoc = cita;
							perso->speed = 3;
							perso->steps = 1;
						}
					}
				}
				break;
			case PersonFlags::pftVelociraptor:
				if (perso->flags & PersonFlags::pf10) {
					if (perso->roomNum == p_global->roomNum) {
						perso_t *perso2 = &kPersons[PER_UNKN_372];
						char found = 0;
						for (; perso2->roomNum != 0xFFFF; perso2++) {
							if ((perso->roomNum & ~0xFF) == (perso2->roomNum & ~0xFF)) {
								if (perso2->flags & PersonFlags::pf80)
									continue;
								perso->targetLoc = perso2->roomNum & 0xFF;
								perso->steps = 1;
								found = 1;
								break;
							}
						}
						if (found)
							continue;
					} else {
						tyranPtr = &kPersons[PER_UNKN_372];
						if (istyran(perso->roomNum)) {
							if (p_global->phaseNum < 481 && (perso->powers & (1 << (p_global->cita_area_num - 3)))) {
								tyranPtr->flags |= PersonFlags::pf80;
								tyranPtr->roomNum = 0;
								perso->flags &= ~PersonFlags::pf10;
								perso->flags |= PersonFlags::pfInParty;
								ajouinfo(p_global->cita_area_num + ValleyNews::vnTyrannLost);
								delinfo(p_global->cita_area_num + ValleyNews::vnTyrannIn);
								if (naitredino(PersonFlags::pftTriceraptor))
									ajouinfo(p_global->cita_area_num + ValleyNews::vnTriceraptorsIn);
								constcita();
								p_global->cur_area_ptr->flags &= ~AreaFlags::TyrannSighted;
							} else {
								perso->flags &= ~PersonFlags::pf10;
								perso->flags &= ~PersonFlags::pfInParty;
								ajouinfo(p_global->cita_area_num + ValleyNews::vnVelociraptorsLost);
							}
							continue;
						}
					}
				}
				if (!perso->targetLoc) {
					short loc;
					perso->lastLoc = 0;
					do {
						loc = (g_ed->_rnd->getRandomNumber(63) & 63) + 16;
						if ((loc & 0xF) >= 12)
							loc &= ~4;  //TODO: ??? same as -= 4
					} while (!caselibre(loc, perso));
					perso->targetLoc = loc;
					perso->steps = 1;
				}
				break;
			}
		}
	}
	void vivreval(short areaNum) {
		p_global->cita_area_num = areaNum;
		p_global->cur_area_ptr = &kAreasTable[areaNum - 1];
		p_global->cita_area_firstRoom = &gameRooms[p_global->cur_area_ptr->firstRoomIndex];
		deplaalldino();
		constcita();
		vivredino();
		newchampi();
		newnido();
		newnidv();
		if (p_global->phaseNum >= 226)
			newor();
		lieuvava(p_global->cur_area_ptr);
	}
	void chaquejour() {
		vivreval(3);
		vivreval(4);
		vivreval(5);
		vivreval(6);
		vivreval(7);
		vivreval(8);
		p_global->drawFlags |= DrawFlags::drDrawTopScreen;
	}
	void temps_passe(short t) {
		short days = p_global->gameDays;
		short lo = p_global->ff_56;
		lo += t;
		if (lo > 255) {
			days++;
			lo &= 0xFF;
		}
		p_global->ff_56 = lo;
		t = ((t >> 8) & 0xFF) + days;
		t -= p_global->gameDays;
		if (t) {
			p_global->gameDays += t;
			while (t--)
				chaquejour();
		}
	}
	void heurepasse() {
		temps_passe(5);
	}
	void anim_perso() {
		if (cur_bank_num != p_global->perso_img_bank)
			load_perso(p_global->perso_ptr);
		restaurefondbulle();
		if (restartAnimation) {
			last_anim_ticks = TimerTicks;
			restartAnimation = 0;
		}
		cur_anim_frame_num = (TimerTicks - last_anim_ticks) >> 2;   // TODO: check me!!!
		if (cur_anim_frame_num > num_anim_frames)               // TODO: bug?
			animateTalking = 0;
		if (p_global->curPersoAnimPtr && !p_global->animationFlags && cur_anim_frame_num != last_anim_frame_num) {
			last_anim_frame_num = cur_anim_frame_num;
			if (*p_global->curPersoAnimPtr == 0xFF)
				getanimrnd();
			bank_data_ptr = perso_img_bank_data_ptr;
			num_img_desc = 0;
			perso_spr(p_global->curPersoAnimPtr);
			p_global->curPersoAnimPtr += num_img_desc + 1;
			dword_3072C = imagedesc + 200;
			virespritebouche();
			if (*dword_3072C)
				af_image();
			animationDelay--;
			if (!animationDelay) { //TODO: combine
				p_global->animationFlags = 1;
				animationDelay = 8;
			}
		}

		animationDelay--;
		if (!animationDelay) { //TODO: combine
			getanimrnd();
			//TODO: no reload?
		}
		if (animateTalking) {
			if (!animationTable) {
				animationTable = gameLipsync + 7262;    //TODO: fix me
				if (!fond_saved)
					sauvefondbouche();
			}
			if (!personTalking)
				cur_anim_frame_num = num_anim_frames - 1;
			animationIndex = animationTable[cur_anim_frame_num];
			if (animationIndex == 0xFF)
				animateTalking = 0;
			else if (animationIndex != lastAnimationIndex) {
				bank_data_ptr = perso_img_bank_data_ptr;
				restaurefondbouche();
//			debug("perso spr %d", animationIndex);
				perso_spr(p_global->persoSpritePtr2 + animationIndex * 2);  //TODO: shorts?
				dword_3072C = imagedesc + 200;
				if (*dword_3072C)
					af_image();
				lastAnimationIndex = animationIndex;
			}
		}
		af_subtitle();
	}
	void getanimrnd() {
		short rnd;
		animationDelay = 8;
		rnd = g_ed->_rnd->getRandomNumber(65535) & (unsigned char)~0x18;    //TODO
		dword_30724 = p_global->persoSpritePtr + 16;    //TODO
		p_global->curPersoAnimPtr = p_global->persoSpritePtr + ((dword_30724[1] << 8) + dword_30724[0]);
		p_global->animationFlags = 1;
		if (rnd >= 8)
			return;
		p_global->animationFlags = 0;
		if (rnd <= 0)
			return;
		for (rnd *= 8; rnd > 0; rnd--) {
			while (*p_global->curPersoAnimPtr)
				p_global->curPersoAnimPtr++;
			p_global->curPersoAnimPtr++;
		}
	}
	void addanim() {
		lastAnimationIndex = -1;
		last_anim_ticks = 0;
		p_global->animationFlags = 0xC0;
		p_global->curPersoAnimPtr = p_global->persoSpritePtr;
		getanimrnd();
		animationActive = 1;
		if (p_global->perso_ptr == &kPersons[PER_ROI])
			return;
		perso_spr(p_global->persoSpritePtr + PLE16(p_global->persoSpritePtr));  //TODO: GetElem(0)
		dword_3072C = imagedesc + 200;
		if (p_global->perso_ptr->id != PersonId::pidCabukaOfCantura && p_global->perso_ptr->targetLoc != 7) //TODO: targetLoc is minisprite idx
			virespritebouche();
		if (*dword_3072C)
			af_image();
	}
	void virespritebouche() {
		unsigned char *src = dword_3072C + 2;
		unsigned char *dst = src;
		char cnt = dword_3072C[0];
		while (cnt--) {
			unsigned char a = *src++;
			unsigned char b = *src++;
			unsigned char c = *src++;
			dst[0] = a;
			dst[1] = b;
			dst[2] = c;
			if (dword_30728[0] != 0xFF) {
				if ((a < dword_30728[0] || a > dword_30728[1])
				        && (a < dword_30728[2] || a > dword_30728[3]))
					dst += 3;
				else
					dword_3072C[0]--;
			} else
				dst += 3;
		}
	}
	void anim_perfin() {
		p_global->animationFlags &= ~0x80;
		animationDelay = 0;
		animationActive = 0;
	}
	void perso_spr(unsigned char *spr) {
		unsigned char *img = imagedesc + 200 + 2;
		short count = 0;
		unsigned char c, cc;
		while ((c = *spr++)) {
			unsigned char *src;
			short index;
			cc = 0;
			if (c == 1) {
				cc = index;
				c = *spr++;
			}
			num_img_desc++;
			index = (cc << 8) | c;
			index -= 2;
//	debug("anim sprite %d", index);

			if (index > max_perso_desc)
				index = max_perso_desc;
			index *= 2;         //TODO: src = GetElem(ff_C2, index)
			src = p_global->ff_C2;
			src = src + PLE16(src + index);
			while ((c = *src++)) {
				*img++ = c;
				*img++ = *src++;
				*img++ = *src++;
				count++;
			}
		}
		imagedesc[200] = count & 0xFF;
		imagedesc[201] = count >> 8;
	}
	void af_image() {
		unsigned char *img = imagedesc + 200, *img_start, *curimg = imagedesc;
		short count;

		count = PLE16(img);
		if (!count)
			return;
		img_start = img;
		img += 2;
		count *= 3;
		while (count--)
			*curimg++ = *img++;
		img = img_start;
		count = PLE16(img);
		img += 2;
/////// draw it
		while (count--) {
			unsigned short index = *img++;
			unsigned short x = *img++ + gameIcons[0].sx;
			unsigned short y = *img++ + gameIcons[0].sy;
			unsigned char *pix = bank_data_ptr;
			unsigned char *scr = p_mainview_buf + x + y * 640;
			unsigned char h0, h1, mode;
			short w, h;
			index--;
			if (PLE16(pix) > 2)
				readpalette(pix + 2);
			pix += PLE16(pix);
			pix += PLE16(pix + index * 2);
			//  short   height:9
			//  short   pad:6;
			//  short   flag:1;
			h0 = *pix++;
			h1 = *pix++;
			w = ((h1 & 1) << 8) | h0;
			h = *pix++;
			mode = *pix++;
			if (mode != 0xFF && mode != 0xFE)
				continue;   //TODO: enclosing block?
			if (h1 & 0x80) {
				// compressed
				for (; h-- > 0;) {
					short ww;
					for (ww = w; ww > 0;) {
						unsigned char c = *pix++;
						if (c >= 0x80) {
							if (c == 0x80) {
								unsigned char fill = *pix++;
								if (fill == 0) {
									scr += 128 + 1;
									ww -= 128 + 1;
								} else {
									unsigned char run;
									*scr++ = fill;  //TODO: wha?
									*scr++ = fill;
									ww -= 128 + 1;
									for (run = 127; run--;)
										*scr++ = fill;
								}
							} else {
								unsigned char fill = *pix++;
								unsigned char run = 255 - c + 2;
								ww -= run;
								if (fill == 0)
									scr += run;
								else
									for (; run--;)
										*scr++ = fill;
							}
						} else {
							unsigned char run = c + 1;
							ww -= run;
							for (; run--;) {
								unsigned char p = *pix++;
								if (p == 0)
									scr++;
								else
									*scr++ = p;
							}
						}
					}
					scr += 640 - w;
				}
			} else {
				// uncompressed
				for (; h--;) {
					short ww;
					for (ww = w; ww--;) {
						unsigned char p = *pix++;
						if (p == 0)
							scr++;
						else
							*scr++ = p;
					}
					scr += 640 - w;
				}
			}
		}
	}
	void af_perso1() {
		perso_spr(p_global->persoSpritePtr + PLE16(p_global->persoSpritePtr));
		af_image();
	}
	void af_perso() {
		load_perso_cour();
		af_perso1();
	}
	void ef_perso() {
		p_global->animationFlags &= 0x3F;
	}
	void load_perso(perso_t *perso) {
		unsigned char *ptr, *baseptr;
		perso_img_bank_data_ptr = 0;
		if (!perso->bank)
			return;
		if (perso->bank != p_global->perso_img_bank) {
			cur_perso_rect = &perso_rects[perso->id];   //TODO: array of short?
			dword_30728 = tab_persxx[perso->id];
			ef_perso();
			p_global->perso_img_bank = perso->bank;
			use_bank(p_global->perso_img_bank);
			perso_img_bank_data_ptr = bank_data_ptr;
			ptr = bank_data_ptr;
			ptr += PLE16(ptr);
			baseptr = ptr;
			ptr += PLE16(ptr) - 2;
			ptr = baseptr + PLE16(ptr) + 4;
			gameIcons[0].sx = PLE16(ptr);
			gameIcons[0].sy = PLE16(ptr + 2);
			gameIcons[0].ex = PLE16(ptr + 4);
			gameIcons[0].ey = PLE16(ptr + 6);
			ptr += 8;
			p_global->ff_C2 = ptr + 2;
			max_perso_desc = PLE16(ptr) / 2;
			ptr += PLE16(ptr);
			baseptr = ptr;
			ptr += PLE16(ptr) - 2;
			p_global->persoSpritePtr = baseptr;
			p_global->persoSpritePtr2 = baseptr + PLE16(ptr);
			debug("load perso: b6 len is %d", p_global->persoSpritePtr2 - p_global->persoSpritePtr);
		} else {
			use_bank(p_global->perso_img_bank);
			perso_img_bank_data_ptr = bank_data_ptr;
		}
	}
	void load_perso_cour() {
		load_perso(p_global->perso_ptr);
	}
	void fin_perso() {
		p_global->animationFlags &= 0x3F;
		p_global->curPersoAnimPtr = 0;
		p_global->ff_CA = 0;
		p_global->perso_img_bank = -1;
		anim_perfin();
	}
	void no_perso() {
		if (p_global->displayFlags == DisplayFlags::dfPerson) {
			p_global->displayFlags = p_global->oldDisplayFlags;
			fin_perso();
		}
		endpersovox();
	}
	void close_perso() {
		char old;
		endpersovox();
		if (p_global->displayFlags == DisplayFlags::dfPerson && p_global->perso_ptr->id != PersonId::pidNarrator && p_global->eventType != EventType::etEventE) {
			rundcurs();
			savedUnderSubtitles = 1;
			restaurefondbulle();
			afficher();
			p_global->ff_103 = 16;
		}
		if (p_global->perso_ptr->id == PersonId::pidNarrator)
			p_global->ff_103 = 69;
		p_global->eloiHaveNews &= 1;
		p_global->ff_CA = 0;
		p_global->ff_F6 = 0;
		if (p_global->displayFlags == DisplayFlags::dfPerson) {
			p_global->displayFlags = p_global->oldDisplayFlags;
			p_global->animationFlags &= 0x3F;
			p_global->curPersoAnimPtr = 0;
			anim_perfin();
			if (p_global->displayFlags & DisplayFlags::dfMirror) {
				gametomiroir(1);
				scroll_pos = old_scroll_pos;
				scroll();
				return;
			}
			if (p_global->numGiveObjs) {
				if (!(p_global->displayFlags & DisplayFlags::dfFlag2))
					showobjects();
				p_global->numGiveObjs = 0;
			}
			if (p_global->ff_F2 & 1) {
				p_global->ff_102 = 6;
				p_global->ff_F2 &= ~1;
			}
			old = p_global->newLocation;
			p_global->newLocation = 0;
			if (!(p_global->narratorSequence & 0x80))
				p_global->ff_100 = -1;
			maj_salle(p_global->roomNum);
			p_global->newLocation = old;
		}
		if (p_global->chrono)
			p_global->chrono_on = 1;
	}
	void af_fondsuiveur() {
		char id = p_global->perso_ptr->id;
		suiveur_t *suiveur = suiveurs_list;
		for (; suiveur->id != -1; suiveur++) {
			if (suiveur->id == id) {
				int bank = 326;
				if (suiveur->sx >= 320)
					bank = 327;
				use_bank(bank + p_global->roomBgBankNum);
				noclipax_avecnoir(0, 0, 16);
				break;
			}
		}
	}
	void af_fondperso1() {
		unsigned char bank;
		char *ptab;
		if (p_global->perso_ptr == &kPersons[PER_MESSAGER]) {
			gameIcons[0].sx = 0;
			perso_rects[PER_MESSAGER].sx = 2;
			bank = p_global->persoBackgroundBankIdx;
			if (p_global->eventType == EventType::etEventE) {
				p_global->ff_103 = 1;
				goto no_suiveur;
			}
			gameIcons[0].sx = 60;
			perso_rects[PER_MESSAGER].sx = 62;
		}
		if (p_global->perso_ptr == &kPersons[PER_THOO]) {
			bank = 37;
			if (p_global->curObjectId == Objects::obShell)
				goto no_suiveur;
		}
		ptab = kPersoRoomBankTable + p_global->perso_ptr->roomBankIdx;
		bank = *ptab++;
		if (!(p_global->perso_ptr->party & p_global->party)) {
			while ((bank = *ptab++) != 0xFF) {
				if (bank == (p_global->roomNum & 0xFF)) { //TODO: signed vs unsigned - chg r31 to uns?
					bank = *ptab;
					break;
				}
				ptab++;
			}
			if (bank != 0xFF)
				goto no_suiveur;
			ptab = kPersoRoomBankTable + p_global->perso_ptr->roomBankIdx;
			bank = *ptab++;
		}
		af_fondsuiveur();
no_suiveur:
		;
		if (!bank)
			return;
		use_bank(bank);
		if (p_global->perso_ptr == &kPersons[PER_UNKN_156])
			noclipax_avecnoir(0, 0, 16);
		else
			noclipax(0, 0, 16);
	}
	void af_fondperso() {
		if (p_global->perso_ptr->bank) {
			fond_saved = 0;
			af_fondperso1();
		}
	}
	void setpersoicon() {
		icon_t *icon = gameIcons, *icon2 = &gameIcons[28];
		if (p_global->iconsIndex == 4)
			return;
		if (p_global->perso_ptr == &kPersons[PER_MESSAGER] && p_global->eventType == EventType::etEventE) {
			p_global->iconsIndex = 123;
			return;
		}
		*icon2++ = *icon++; //TODO: is this ok?
		*icon2++ = *icon++;
		icon2->sx = -1;
	}
	void show_perso() {
		perso_t *perso = p_global->perso_ptr;
		if (perso->bank) {
			closesalle();
			if (p_global->displayFlags != DisplayFlags::dfPerson) {
				if (p_global->displayFlags & DisplayFlags::dfMirror)
					resetscroll();
				p_global->oldDisplayFlags = p_global->displayFlags;
				p_global->displayFlags = DisplayFlags::dfPerson;
				load_perso(perso);
				setpersoicon();
				af_fondperso();
				if (perso == &kPersons[PER_THOO] && p_global->curObjectId == Objects::obShell) {
					af_subtitle();
					update_cursor();
					needPaletteUpdate = 1;
					afficher();
					rundcurs();
					return;
				}
			}
			load_perso_cour();
			addanim();
			if (!p_global->curPersoAnimPtr) {
				af_perso();
				af_subtitle();
			}
			restartAnimation = 1;
			anim_perso();
			if (perso != &kPersons[PER_UNKN_156])
				update_cursor();
			needPaletteUpdate = 1;
			if (perso != &kPersons[PER_UNKN_156])
				rundcurs();
			afficher();
		} else {
			aflieu();
			af_subtitle();
		}
	}
	void showpersopanel() {
		perso_t *perso = p_global->perso_ptr;
		load_perso_cour();
		addanim();
		if (!p_global->curPersoAnimPtr) {
			af_perso();
			af_subtitle();
		}
		restartAnimation = 1;
		needPaletteUpdate = 1;
		if (p_global->drawFlags & DrawFlags::drDrawFlag8)
			return;
		anim_perso();
		if (perso != &kPersons[PER_UNKN_156])
			update_cursor();
		afficher();
		if (perso != &kPersons[PER_UNKN_156])
			rundcurs();
		p_global->drawFlags |= DrawFlags::drDrawFlag8;
		p_global->iconsIndex = 112;
	}
	void getdatasync() {
		short num = p_global->textNum;
		if (p_global->textBankIndex != 1)
			num += 565;
		if (p_global->textBankIndex == 3)
			num += 707;
		if (num == 144)
			num = 142;
		animateTalking = ReadDataSync(num - 1);
		if (animateTalking)
			num_anim_frames = ReadNombreFrames();
		else
			num_anim_frames = 0;
		if (p_global->textNum == 144)
			num_anim_frames = 48;
		animationTable = 0;
	}
	short ReadNombreFrames() {
		short num = 0;
		animationTable = gameLipsync + 7262;    //TODO: fix me
		while (*animationTable++ != 0xFF)
			num++;
		return num;
	}
	void waitendspeak() {
		for (;;) {
			if (animationActive)
				anim_perso();
			musicspy();
			afficher();
			CLKeyboard_Read();
			testPommeQ();
			if (pomme_q) {
				close_perso();
				PommeQ();
				break;
			}
			if (!mouse_held)
				if (CLMouse_IsDown())
					break;
			if (mouse_held)
				if (!CLMouse_IsDown())
					mouse_held = 0;
		}
		mouse_held = 1;
	}
	void my_bulle() {
		unsigned char c;
		unsigned char i;
		short words_on_line, word_width, line_width;
		unsigned char *icons = phraseIconsBuffer;
		unsigned char *lines = phraseCoordsBuffer;
		unsigned char *phrasePtr = phraseBuffer;
		if (!p_global->textNum)
			return;
		p_global->numGiveObjs = 0;
		p_global->giveobj1 = 0;
		p_global->giveobj2 = 0;
		p_global->giveobj3 = 0;
		p_global->textWidthLimit = subtitles_x_width;
		text_ptr = gettxtad(p_global->textNum);
		num_text_lines = 0;
		words_on_line = 0;
		word_width = 0;
		line_width = 0;
		while ((c = *text_ptr++) != 0xFF) {
			if (c == 0x11 || c == 0x13) {
				if (p_global->phaseNum <= 272 || p_global->phaseNum == 386) {
					p_global->eloiHaveNews = c & 0xF;
					p_global->ff_4D = p_global->worldTyrannSighted;
				}
			} else if (c >= 0x80 && c < 0x90)
				SysBeep(1);
			else if (c >= 0x90 && c < 0xA0) {
				while (*text_ptr++ != 0xFF) ;
				text_ptr--;
			} else if (c >= 0xA0 && c < 0xC0)
				p_global->textToken1 = c & 0xF;
			else if (c >= 0xC0 && c < 0xD0)
				p_global->textToken2 = c & 0xF;
			else if (c >= 0xD0 && c < 0xE0) {
				unsigned char c1 = *text_ptr++;
				if (c == 0xD2)
#ifdef FAKE_DOS_VERSION
					p_global->textWidthLimit = c1 + 160;
#else
					p_global->textWidthLimit = c1 + subtitles_x_center; //TODO: signed? 160 in pc ver
#endif
				else {
					unsigned char c2 = *text_ptr++;
					switch (p_global->numGiveObjs) {
					case 0:
						p_global->giveobj1 = c2;
						break;
					case 1:
						p_global->giveobj2 = c2;
						break;
					case 2:
						p_global->giveobj3 = c2;
						break;
					}
					p_global->numGiveObjs++;
					*icons++ = *text_ptr++;
					*icons++ = *text_ptr++;
					*icons++ = c2;
				}
			} else if (c >= 0xE0 && c < 0xFF)
				SysBeep(1);
			else if (c != '\r') {
				unsigned char width;
				short overrun;
				*phrasePtr++ = c;
				width = gameFont[c];
#ifdef FAKE_DOS_VERSION
				if (c == ' ')
					width = space_width;
#endif
				word_width += width;
				line_width += width;
				overrun = line_width - p_global->textWidthLimit;
				if (overrun > 0) {
					num_text_lines++;
					if (c != ' ') {
						*lines++ = words_on_line;
						*lines++ = word_width + space_width - overrun;
						line_width = word_width;
					} else {
						*lines++ = words_on_line + 1;
						*lines++ = space_width - overrun;   //TODO: checkme
						line_width = 0;
					}
					word_width = 0;
					words_on_line = 0;
				} else {
					if (c == ' ') {
						words_on_line++;
						word_width = 0;
					}
				}
			}
		}
		num_text_lines++;
		*lines++ = words_on_line + 1;
		*lines++ = word_width;
		*phrasePtr = c;
		if (p_global->textBankIndex == 2 && p_global->textNum == 101 && p_global->pref_language == 1)
			patchphrase();
		my_pr_bulle();
		if (!p_global->numGiveObjs)
			return;
		use_main_bank();
		if (num_text_lines < 3)
			num_text_lines = 3;
		icons = phraseIconsBuffer;
		for (i = 0; i < p_global->numGiveObjs; i++) {
			unsigned char x = *icons++;
			unsigned char y = *icons++;
			unsigned char s = *icons++;
			spritesurbulle(52, x + subtitles_x_center, y - 1);
			spritesurbulle(s + 9, x + subtitles_x_center + 1, y);
		}
	}
	void my_pr_bulle() {
		unsigned char *cur_out;
		unsigned char *coo = phraseCoordsBuffer;
		short extra_spacing, lines;
		char done = 0;
		CLBlitter_FillView(p_subtitlesview, 0);
		if (p_global->pref_language == 0)
			return;
		textout = p_subtitlesview_buf;
		text_ptr = phraseBuffer;
		lines = 1;
		while (!done) {
			unsigned char c;
			short num_words = *coo++;       // num words on line
			short pad_size = *coo++;        // amount of extra spacing
			cur_out = textout;
			extra_spacing = num_words > 1 ? pad_size / (num_words - 1) + 1 : 0;
			if (lines == num_text_lines)
				extra_spacing = 0;
			c = *text_ptr++;
			while (!done & (num_words > 0)) { //TODO: bug - missed & ?
				if (c < 0x80 && c != '\r') {
					if (c == ' ') {
						num_words--;
						if (pad_size >= extra_spacing) {
							textout += extra_spacing + space_width;
							pad_size -= extra_spacing;
						} else {
							textout += pad_size + space_width;
							pad_size = 0;
						}
					} else {
						short char_width = gameFont[c];
						if (!(p_global->drawFlags & DrawFlags::drDrawMenu)) {
							textout += subtitles_x_width;
							if (!specialTextMode)
								charsurbulle(c, 195, char_width);
							textout++;
							if (!specialTextMode)
								charsurbulle(c, 195, char_width);
							textout -= subtitles_x_width + 1;
						}
						if (specialTextMode)
							charsurbulle(c, 250, char_width);
						else
							charsurbulle(c, 230, char_width);
						textout += char_width;
					}
				} else
					monbreak();
				c = *text_ptr++;
				if (c == 0xFF)
					done = 1;
			}
			textout = cur_out + subtitles_x_width * FONT_HEIGHT;
			lines++;
			text_ptr--;
		}
	}
	void charsurbulle(unsigned char c, unsigned char color, short width) {
		short w, h;
		unsigned char *glyph = gameFont + 256 + c * FONT_HEIGHT;
		textoutptr = textout;
		for (h = 0; h < FONT_HEIGHT; h++) {
			unsigned char bits = *glyph++;
			short mask = 0x80;
			for (w = 0; w < width; w++) {
				if (bits & mask)
					*textoutptr = color;
				textoutptr++;
				mask >>= 1;
			}
			textoutptr += subtitles_x_width - width;
		}
	}
	void af_subtitle() {
		short w, h, y;
		unsigned char *src = p_subtitlesview_buf, *dst = p_mainview_buf;
		if (p_global->displayFlags & DisplayFlags::dfFlag2) {
			y = 174;
			if ((p_global->drawFlags & DrawFlags::drDrawMenu) && num_text_lines == 1)
				y = 167;
			dst += 640 * (y - num_text_lines * FONT_HEIGHT) + subtitles_x_scr_margin;
		} else {
			y = 174;
			dst += 640 * (y - num_text_lines * FONT_HEIGHT) + scroll_pos + subtitles_x_scr_margin;
		}
		if (animationActive && !personTalking)
			return;
		sauvefondbulle(y);
		for (h = 0; h < num_text_lines * FONT_HEIGHT + 1; h++) {
			for (w = 0; w < subtitles_x_width; w++) {
				unsigned char c = *src++;
				if (c)
					*dst = c;
				dst++;
			}
			dst += 640 - subtitles_x_width;
		}
	}
	void sauvefondbulle(short y) {
		underSubtitlesScreenRect.sy = y - num_text_lines * FONT_HEIGHT;
		underSubtitlesScreenRect.sx = scroll_pos + subtitles_x_scr_margin;
		underSubtitlesScreenRect.ex = scroll_pos + subtitles_x_scr_margin + subtitles_x_width - 1;
		underSubtitlesScreenRect.ey = y;
		underSubtitlesBackupRect.sy = 0;
		underSubtitlesBackupRect.ey = num_text_lines * FONT_HEIGHT;
		CLBlitter_CopyViewRect(p_mainview, p_underSubtitlesView, &underSubtitlesScreenRect, &underSubtitlesBackupRect);
		savedUnderSubtitles = 1;
	}
	void restaurefondbulle() {
		if (!savedUnderSubtitles)
			return;
		CLBlitter_CopyViewRect(p_underSubtitlesView, p_mainview, &underSubtitlesBackupRect, &underSubtitlesScreenRect);
		savedUnderSubtitles = 0;
	}
	void af_subtitlehnm() {
		short x, y;
		unsigned char *src = p_subtitlesview_buf;
		unsigned char *dst = p_hnmview_buf + subtitles_x_scr_margin + (158 - num_text_lines * FONT_HEIGHT) * 320;
		for (y = 0; y < num_text_lines * FONT_HEIGHT; y++) {
			for (x = 0; x < subtitles_x_width; x++) {
				char c = *src++;
				if (c)
					*dst = c;
				dst++;
			}
			dst += 320 - subtitles_x_width;
		}
	}
	void patchphrase() {
		phraseBuffer[36] = 'c';
	}
	void vavapers() {
		perso_t *perso = p_global->perso_ptr;
		p_global->curPersoFlags = perso->flags;
		p_global->curPersoItems = perso->items;
		p_global->curPersoPowers = perso->powers;
	}
	void citadelle() {
		p_global->ff_69++;
		p_global->ff_F6++;
		byte_30AFE = 1;
		byte_30B00 = 1;
	}
	void choixzone() {
		if (p_global->giveobj3)
			p_global->iconsIndex = 6;
		else
			p_global->iconsIndex = 10;
		p_global->autoDialog = 0;
		putobject();
	}
	void showevents1() {
		p_global->ff_113 = 0;
		perso_ici(3);
	}
	void showevents() {
		if (p_global->eventType && p_global->displayFlags != DisplayFlags::dfPerson)
			showevents1();
	}
	void parle_mfin() {
		char curobj;
		object_t *obj;
		perso_t *perso = p_global->perso_ptr;
		if (p_global->curObjectId) {
			curobj = p_global->curObjectId;
			if (p_global->dialogType == DialogType::dtHint)
				return;
			obj = getobjaddr(p_global->curObjectId);
			if (p_global->dialogType == DialogType::dtDinoItem)
				perso = p_global->room_perso;
			if (verif_oui()) {
				loseobject(p_global->curObjectId);
				perso->powers |= obj->powerMask;
			}
			perso->items |= obj->itemMask;
			SpecialObjets(perso, curobj);
			return;
		}
		if (!verif_oui())
			return;
		nextinfo();
		if (!p_global->last_info)
			byte_30B00 = 1;
		else {
			p_global->next_dialog_ptr = 0;
			byte_30B00 = 0;
		}
	}
	void parlemoi_normal() {
		char ok;
		dial_t *dial;
		if (!p_global->next_dialog_ptr) {
			perso_t *perso = p_global->perso_ptr;
			if (perso) {
				short num = (perso->id << 3) | p_global->dialogType;
				dial = (dial_t *)GetElem(gameDialogs, num);
			} else {
				close_perso();
				return;
			}
		} else {
			if (byte_30B00) {
				close_perso();
				return;
			}
			dial = p_global->next_dialog_ptr;
		}
		ok = dial_scan(dial);
		p_global->next_dialog_ptr = p_global->dialog_ptr;
		byte_30B00 = 0;
		if (!ok)
			close_perso();
		else
			parle_mfin();
	}
	void parle_moi() {
		unsigned char r28;
		char ok;
		dial_t *dial;
		endpersovox();
		r28 = p_global->ff_F6;
		p_global->ff_F6 = 0;
		if (!r28) {
			init_non();
			if (p_global->drawFlags & DrawFlags::drDrawInventory)
				showobjects();
			if (p_global->drawFlags & DrawFlags::drDrawTopScreen)
				afftopscr();
			if (p_global->curObjectId) {
				if (p_global->dialogType == DialogType::dtTalk) {
					p_global->dialogType = DialogType::dtItem;
					p_global->next_dialog_ptr = 0;
					byte_30B00 = 0;
				}
				parlemoi_normal();
				return;
			}
			if (p_global->dialogType == DialogType::dtItem) {
				p_global->dialogType = DialogType::dtTalk;
				if (!byte_30B00)
					p_global->next_dialog_ptr = 0;
			}
			if (byte_30AFE) {
				parlemoi_normal();
				return;
			}
			if (!p_global->last_dialog_ptr) {
				short num = 160;
				if (p_global->phaseNum >= 400)
					num++;
				dial = (dial_t *)GetElem(gameDialogs, num);
			} else
				dial = p_global->last_dialog_ptr;
			ok = dial_scan(dial);
			p_global->last_dialog_ptr = p_global->dialog_ptr;
			byte_30AFE = 0;
			if (!ok) {
				byte_30AFE = 1;
				if (p_global->ff_60) {
					if (p_global->perso_ptr == &kPersons[PER_MESSAGER]) {
						p_global->dialogType = 0;
						if (p_global->eloiHaveNews)
							parlemoi_normal();
						else
							close_perso();
					} else
						close_perso();
				} else
					parlemoi_normal();
			} else
				parle_mfin();
		} else
			close_perso();
	}
	void init_perso_ptr(perso_t *perso) {
		p_global->metPersonsMask1 |= perso->party;
		p_global->metPersonsMask2 |= perso->party;
		p_global->next_dialog_ptr = 0;
		byte_30B00 = 0;
		dialogSkipFlags = DialogFlags::dfSpoken;
		p_global->ff_60 = 0;
		p_global->textToken1 = 0;
	}
	void perso1(perso_t *perso) {
		p_global->phaseActionsCount++;
		if (perso == &kPersons[PER_THOO])
			p_global->phaseActionsCount--;
		p_global->perso_ptr = perso;
		init_perso_ptr(perso);
		parle_moi();
	}
	void perso_normal(perso_t *perso) {
		p_global->last_dialog_ptr = 0;
		p_global->dialogType = 0;
		byte_30AFE = 0;
		perso1(perso);
	}
	void persoparle(short pers) {
		char res;
		unsigned short idx;
		unsigned char *ptr;
		perso_t *perso = &kPersons[pers];
		p_global->perso_ptr = perso;
		p_global->dialogType = DialogType::dtInspect;
		idx = perso->id * 8 | p_global->dialogType;
		res = dialoscansvmas((dial_t *)GetElem(gameDialogs, idx));
		aflieu();
		af_subtitle();
		persovox();
		p_global->ff_CA = 0;
		p_global->dialogType = 0;
	}
	void roi()  {
		perso_normal(&kPersons[PER_ROI]);
	}
	void dina() {
		perso_normal(&kPersons[PER_DINA]);
	}
	void thoo() {
		perso_normal(&kPersons[PER_THOO]);
	}
	void monk() {
		perso_normal(&kPersons[PER_MONK]);
	}
	void bourreau() {
		perso_normal(&kPersons[PER_BOURREAU]);
	}
	void messager() {
		perso_normal(&kPersons[PER_MESSAGER]);
	}
	void mango()    {
		perso_normal(&kPersons[PER_MANGO]);
	}
	void eve()  {
		perso_normal(&kPersons[PER_EVE]);
	}
	void azia() {
		perso_normal(&kPersons[PER_AZIA]);
	}
	void mammi() {
		perso_t *perso;
		for (perso = &kPersons[PER_MAMMI]; perso->party == PersonMask::pmLeader; perso++) {
			if (perso->roomNum == p_global->roomNum) {
				perso_normal(perso);
				break;
			}
		}
	}
	void gardes()   {
		perso_normal(&kPersons[PER_GARDES]);
	}
	void bambou()   {
		perso_normal(&kPersons[PER_BAMBOO]);
	}
	void kabuka()   {
		if (p_global->roomNum == 0x711) perso_normal(&kPersons[PER_KABUKA]);
		else bambou();
	}
	void fisher()   {
		if (p_global->roomNum == 0x902) perso_normal(&kPersons[PER_FISHER]);
		else kabuka();
	}
	void dino() {
		perso_t *perso = p_global->room_perso;
		if (!perso)
			return;
		byte_30AFE = 1;
		p_global->dialogType = 0;
		p_global->roomPersoFlags = perso->flags;
		p_global->roomPersoItems = perso->items;
		p_global->roomPersoPowers = perso->powers;
		p_global->perso_ptr = perso;
		init_perso_ptr(perso);
		debug("beg dino talk");
		parle_moi();
		debug("end dino talk");
		if (p_global->areaNum == Areas::arWhiteArch)
			return;
		if (p_global->ff_60)
			waitendspeak();
		if (pomme_q)
			return;
		perso = &kPersons[PER_MANGO];
		if (!(p_global->party & PersonMask::pmMungo)) {
			perso = &kPersons[PER_DINA];
			if (!(p_global->party & PersonMask::pmDina)) {
				perso = &kPersons[PER_EVE];
				if (!(p_global->party & PersonMask::pmEve)) {
					perso = &kPersons[PER_GARDES];
				}
			}
		}
		p_global->dialogType = DialogType::dtDinoAction;
		if (p_global->curObjectId)
			p_global->dialogType = DialogType::dtDinoItem;
		perso1(perso);
		if (p_global->roomPersoType == PersonFlags::pftMosasaurus && !p_global->curObjectId) {
			p_global->area_ptr->flags |= AreaFlags::afFlag20;
			lieuvava(p_global->area_ptr);
		}
	}
	void tyran() {
		perso_t *perso = p_global->room_perso;
		if (!perso)
			return;
		byte_30AFE = 1;
		p_global->dialogType = 0;
		p_global->roomPersoFlags = perso->flags;
		p_global->perso_ptr = perso;
		init_perso_ptr(perso);
		perso = &kPersons[PER_MANGO];
		if (!(p_global->party & PersonMask::pmMungo)) {
			perso = &kPersons[PER_DINA];
			if (!(p_global->party & PersonMask::pmDina)) {
				perso = &kPersons[PER_EVE];
				if (!(p_global->party & PersonMask::pmEve)) {
					perso = &kPersons[PER_GARDES];
				}
			}
		}
		p_global->dialogType = DialogType::dtDinoAction;
		if (p_global->curObjectId)
			p_global->dialogType = DialogType::dtDinoItem;
		perso1(perso);
	}
	void morkus()   {
		perso_normal(&kPersons[PER_MORKUS]);
	}
	void comment() {
		perso_t *perso;
		perso = &kPersons[PER_DINA];
		if (!(p_global->party & PersonMask::pmDina)) {
			perso = &kPersons[PER_EVE];
			if (!(p_global->party & PersonMask::pmEve)) {
				perso = &kPersons[PER_GARDES];
				if (!(p_global->party & PersonMask::pmThugg)) {
					return;
				}
			}
		}
		p_global->dialogType = DialogType::dtHint;
		perso1(perso);
	}
	void adam() {
		char *objvid;
		object_t *object;
		short vid;
		resetscroll();
		switch (p_global->curObjectId) {
		case Objects::obNone:
			gotopanel();
			break;
		case Objects::obRoot:
			if (p_global->roomNum == 2817
			        && p_global->phaseNum > 496 && p_global->phaseNum < 512) {
				bigphase1();
				loseobject(Objects::obRoot);
				p_global->ff_100 = -1;
				quitmiroir();
				maj_salle(p_global->roomNum);
				reste_ici(5);
				p_global->eventType = EventType::etEvent3;
				showevents();
				waitendspeak();
				if (pomme_q)
					return;
				close_perso();
				reste_ici(5);
				p_global->roomNum = 2818;
				p_global->areaNum = Areas::arWhiteArch;
				p_global->eventType = EventType::etEvent5;
				p_global->valleyVidNum = 0;
				p_global->ff_102 = 6;
				p_global->newMusicType = MusicType::mtNormal;
				maj_salle(p_global->roomNum);
			} else {
				p_global->dialogType = DialogType::dtHint;
				perso1(&kPersons[PER_EVE]);
			}
			break;
		case Objects::obShell:
			p_global->dialogType = DialogType::dtHint;
			perso1(&kPersons[PER_THOO]);
			break;
		case Objects::obFlute:
		case Objects::obTrumpet:
			if (p_global->roomPersoType) {
				quitmiroir();
				maj_salle(p_global->roomNum);
				dino();
			} else
				comment();
			break;
		case Objects::obTablet1:
		case Objects::obTablet2:
		case Objects::obTablet3:
		case Objects::obTablet4:
		case Objects::obTablet5:
		case Objects::obTablet6:
			if ((p_global->partyOutside & PersonMask::pmDina)
			        && p_global->curObjectId == Objects::obTablet1 && p_global->phaseNum == 370)
				incphase1();
			objvid = &kTabletView[(p_global->curObjectId - Objects::obTablet1) * 2];
			object = getobjaddr(*objvid++);
			vid = 84;
			if (!object->count)
				vid = *objvid;
			bars_out();
			specialTextMode = 1;
			playhnm(vid);
			needPaletteUpdate = 1;
			p_global->ff_102 = 16;
			bars_in();
			gametomiroir(0);
			break;
		case Objects::obApple:
		case Objects::obShroom:
		case Objects::obBadShroom:
		case Objects::obNest:
		case Objects::obFullNest:
		case Objects::obDrum:
			if (p_global->party & PersonMask::pmThugg) {
				p_global->dialogType = DialogType::dtHint;
				perso1(&kPersons[PER_GARDES]);
			}
			break;
		default:
			comment();
		}
	}
	void init_oui() {
		word_30AFC = -1;
	}
	void init_non() {
		word_30AFC =  0;
	}
	void oui()  {
		word_30AFC = -1;
	}
	void non()  {
		word_30AFC =  0;
	}
	char verif_oui() {
		return word_30AFC == -1;
	}
	void SpcChampi(perso_t *perso) {
		perso->flags |= PersonFlags::pf10;
		p_global->area_ptr->flags |= AreaFlags::afFlag2;
		p_global->curAreaFlags |= AreaFlags::afFlag2;
	}
	void SpcNidv(perso_t *perso) {
		if (!verif_oui())
			return;
		perso->flags |= PersonFlags::pf10;
		p_global->roomPersoFlags |= PersonFlags::pf10;
		p_global->gameFlags |= GameFlags::gfFlag400;
		if (p_global->perso_ptr == &kPersons[PER_EVE]) {
			p_global->area_ptr->flags |= AreaFlags::afFlag4;
			p_global->curAreaFlags |= AreaFlags::afFlag4;
			perso->flags |= PersonFlags::pfInParty;
			p_global->roomPersoFlags |= PersonFlags::pfInParty;
			lieuvava(p_global->area_ptr);
		} else {
			perso->flags &= ~PersonFlags::pf10;
			p_global->roomPersoFlags &= ~PersonFlags::pf10;
		}
	}
	void SpcNido(perso_t *perso) {
		if (perso == &kPersons[PER_GARDES])
			giveobject();
	}
	void SpcPomme(perso_t *perso) {
		perso->flags |= PersonFlags::pf10;
		p_global->area_ptr->flags |= AreaFlags::afFlag8;
		p_global->curAreaFlags |= AreaFlags::afFlag8;
		p_global->gameFlags |= GameFlags::gfFlag200;
	}
	void SpcOr(perso_t *perso) {
		if (!verif_oui())
			return;
		perso->items = currentSpecialObject->itemMask;
		p_global->roomPersoItems = currentSpecialObject->itemMask;
		perso->flags |= PersonFlags::pf10;
		perso->flags &= ~PersonFlags::pfInParty;
		perso->targetLoc = 0;
		p_global->area_ptr->flags |= AreaFlags::afGaveGold;
		p_global->curAreaFlags |= AreaFlags::afGaveGold;
		if (p_global->phaseNum == 226)
			incphase1();
	}
	void SpcPrisme(perso_t *perso) {
		if (perso == &kPersons[PER_DINA]) {
			if (p_global->partyOutside & PersonMask::pmMonk)
				p_global->gameFlags |= GameFlags::gfPrismAndMonk;
		}
	}
	void SpcTalisman(perso_t *perso) {
		if (perso == &kPersons[PER_DINA])
			suis_moi(1);
	}
	void SpcMasque(perso_t *perso) {
		if (perso == &kPersons[PER_BAMBOO]) {
			dialautoon();
			byte_30AFE = 1;
		}
	}
	void SpcSac(perso_t *perso) {
		if (p_global->textToken1 != 3)
			return;
		if (perso == &kPersons[PER_KABUKA] || perso == &kPersons[PER_MAMMI_3])
			loseobject(currentSpecialObject->id);
	}
	void SpcTrompet(perso_t *perso) {
		if (!verif_oui())
			return;
		p_global->ff_54 = 4;
		winobject(Objects::obTrumpet);
		p_global->drawFlags |= DrawFlags::drDrawInventory;
		byte_30B00 = 1;
		TyranMeurt(p_global->room_perso);
	}
	void SpcArmes(perso_t *perso) {
		if (!verif_oui())
			return;
		perso->powers = currentSpecialObject->powerMask;
		p_global->roomPersoPowers = currentSpecialObject->powerMask;
		giveobject();
	}
	void SpcInstru(perso_t *perso) {
		if (!verif_oui())
			return;
		if (perso == &kPersons[PER_MONK]) {
			p_global->partyInstruments &= ~1;   //TODO: check me
			if (currentSpecialObject->id == Objects::obRing) {
				p_global->partyInstruments |= 1;
				p_global->monkGotRing++;                //TODO: |= 1 ?
			}
		}
		if (perso == &kPersons[PER_GARDES]) {
			p_global->partyInstruments &= ~2;
			if (currentSpecialObject->id == Objects::obDrum)
				p_global->partyInstruments |= 2;
		}
		perso->powers = currentSpecialObject->powerMask;
		p_global->curPersoPowers = currentSpecialObject->powerMask;
		giveobject();
	}
	void SpcOeuf(perso_t *perso) {
		if (!verif_oui())
			return;
		gameIcons[131].cursor_id &= ~0x8000;
		p_global->persoBackgroundBankIdx = 62;
		dialautoon();
	}
	void TyranMeurt(perso_t *perso) {
		perso->flags |= PersonFlags::pf80;
		perso->roomNum = 0;
		delinfo(p_global->areaNum + ValleyNews::vnTyrannIn);
		p_global->roomPersoType = 0;
		p_global->roomPersoFlags = 0;
		p_global->chrono_on = 0;
	}
	void SpecialObjets(perso_t *perso, char objid) {
		typedef void (EdenGameImpl::*disp_t)(perso_t *perso);
		struct spcobj_t {
			char    persoType;
			char    objectId;
//		void    (EdenGameImpl::*disp)(perso_t *perso);
			disp_t  disp;
		};

		static spcobj_t kSpecialObjectActions[] = {
			//    perso, obj, disp
			{ PersonFlags::pfType8, Objects::obShroom, &EdenGameImpl::SpcChampi },
			{ PersonFlags::pftTriceraptor, Objects::obNest, &EdenGameImpl::SpcNidv },
			{ PersonFlags::pfType0, Objects::obFullNest, &EdenGameImpl::SpcNido },
			{ PersonFlags::pftMosasaurus, Objects::obApple, &EdenGameImpl::SpcPomme },
			{ PersonFlags::pftVelociraptor, Objects::obGold, &EdenGameImpl::SpcOr },
			{ PersonFlags::pfType0, Objects::obPrism, &EdenGameImpl::SpcPrisme },
			{ PersonFlags::pfType0, Objects::obTalisman, &EdenGameImpl::SpcTalisman },
			{ PersonFlags::pfType2, Objects::obMaskOfDeath, &EdenGameImpl::SpcMasque },
			{ PersonFlags::pfType2, Objects::obMaskOfBonding, &EdenGameImpl::SpcMasque },
			{ PersonFlags::pfType2, Objects::obMaskOfBirth, &EdenGameImpl::SpcMasque },
			{ PersonFlags::pfType0, Objects::obBag, &EdenGameImpl::SpcSac },
			{ PersonFlags::pfType2, Objects::obBag, &EdenGameImpl::SpcSac },
			{ PersonFlags::pftTyrann, Objects::obTrumpet, &EdenGameImpl::SpcTrompet },
			{ PersonFlags::pftVelociraptor, Objects::obEyeInTheStorm, &EdenGameImpl::SpcArmes },
			{ PersonFlags::pftVelociraptor, Objects::obSkyHammer, &EdenGameImpl::SpcArmes },
			{ PersonFlags::pftVelociraptor, Objects::obFireInTheClouds, &EdenGameImpl::SpcArmes },
			{ PersonFlags::pftVelociraptor, Objects::obWithinAndWithout, &EdenGameImpl::SpcArmes },
			{ PersonFlags::pftVelociraptor, Objects::obEyeInTheCyclone, &EdenGameImpl::SpcArmes },
			{ PersonFlags::pftVelociraptor, Objects::obRiverThatWinds, &EdenGameImpl::SpcArmes },
			{ PersonFlags::pfType0, Objects::obTrumpet, &EdenGameImpl::SpcInstru },
			{ PersonFlags::pfType0, Objects::obDrum, &EdenGameImpl::SpcInstru },
			{ PersonFlags::pfType0, Objects::obRing, &EdenGameImpl::SpcInstru },
			{ PersonFlags::pfType0, Objects::obEgg, &EdenGameImpl::SpcOeuf },
			{ -1, -1, nullptr }
		};
		spcobj_t *spcobj = kSpecialObjectActions;
		char persoType = perso->flags & PersonFlags::pfTypeMask;
		currentSpecialObject = &objects[objid - 1];
		for (; spcobj->persoType != -1; spcobj++) {
			if (spcobj->objectId == objid && spcobj->persoType == persoType) {
				(this->*spcobj->disp)(perso);
				break;
			}
		}
	}

	void dialautoon() {
		p_global->iconsIndex = 4;
		p_global->autoDialog = -1;
		putobject();
	}
	void dialautooff() {
		p_global->iconsIndex = 0x10;
		p_global->autoDialog = 0;
	}
	void follow() {
		if (p_global->roomPersoType == PersonFlags::pfType12) {
			debug("follow: hiding person %d", p_global->room_perso - kPersons);
			p_global->room_perso->flags |= PersonFlags::pf80;
			p_global->room_perso->roomNum = 0;
			p_global->gameFlags |= GameFlags::gfFlag8;
			gameIcons[123].object_id = 18;
			gameIcons[124].object_id = 35;
			gameIcons[125].cursor_id &= ~0x8000;
			p_global->persoBackgroundBankIdx = 56;
		} else
			suis_moi5();
	}
	void dialonfollow() {
		p_global->iconsIndex = 4;
		p_global->autoDialog = -1;
		follow();
	}
	void abortdial() {
		p_global->ff_F6++;
		if (p_global->roomPersoType != PersonFlags::pftTriceraptor || p_global->perso_ptr != &kPersons[PER_EVE])
			return;
		p_global->area_ptr->flags |= AreaFlags::afFlag4;
		p_global->curAreaFlags |= AreaFlags::afFlag4;
		p_global->room_perso->flags |= PersonFlags::pfInParty;
		p_global->roomPersoFlags |= PersonFlags::pfInParty;
		lieuvava(p_global->area_ptr);
	}
	void narrateur() {
		if (!(p_global->displayFlags & DisplayFlags::dfFlag1))
			return;
		if (!p_global->narratorSequence) {
			if (p_global->ff_6A == p_global->ff_69)
				goto skip;
			buildcita();
		}
		p_global->ff_F5 |= 0x80;
		p_global->ff_F2 &= ~1;  //TODO: check me
		p_global->perso_ptr = &kPersons[PER_UNKN_156];
		p_global->ff_60 = 0;
		p_global->eventType = 0;
		p_global->ff_103 = 69;
		if (dialo_even(&kPersons[PER_UNKN_156])) {
			p_global->narrator_dialog_ptr = p_global->dialog_ptr;
			dialautoon();
			p_global->ff_F2 |= 1;
			waitendspeak();
			if (pomme_q)
				return;
			endpersovox();
			while (dialoscansvmas(p_global->narrator_dialog_ptr)) {
				p_global->narrator_dialog_ptr = p_global->dialog_ptr;
				waitendspeak();
				if (pomme_q)
					return;
				endpersovox();
			}
			p_global->narrator_dialog_ptr = p_global->dialog_ptr;
			p_global->ff_102 = 0;
			p_global->ff_103 = 0;
			close_perso();
			lieuvava(p_global->area_ptr);
			if (p_global->narratorSequence == 8)
				deplaval(134);
		}
		p_global->ff_103 = 0;
		if (p_global->narratorSequence == 10) {
			suis_moi(5);
			suis_moi(7);
			suis_moi(3);
			suis_moi(18);
			reste_ici(6);
			p_global->eloiHaveNews = 0;
			deplaval(139);
		}
		p_global->eventType = EventType::etEventD;
		showevents();
		p_global->ff_F5 &= ~0x80;
skip:
		;
		p_global->ff_F2 &= ~1;  //TODO: check me
		if (p_global->narratorSequence > 50 && p_global->narratorSequence <= 80)
			p_global->endGameFlag = 50;
		if (p_global->narratorSequence == 3)
			chronoon(1200);
		p_global->narratorSequence = 0;

	}
	void vrf_phrases_file() {
		short num = 3;
		if (p_global->dialog_ptr < (dial_t *)GetElem(gameDialogs, 48))
			num = 1;
		else if (p_global->dialog_ptr < (dial_t *)GetElem(gameDialogs, 128))
			num = 2;
		p_global->textBankIndex = num;
		if (p_global->pref_language)
			num += (p_global->pref_language - 1) * 3;
		if (num == lastPhrasesFile)
			return;
		lastPhrasesFile = num;
		num += 404;
		loadfile(num, gamePhrases);
		verifh(gamePhrases);
	}
	unsigned char *gettxtad(short id) {
		vrf_phrases_file();
		return (unsigned char *)GetElem(gamePhrases, id - 1);
	}
	void gotocarte() {
		char newArea, curArea;
		goto_t *go = &gotos[current_spot2->object_id];
		endpersovox();
		newArea = go->areaNum;
		p_global->newRoomNum = (go->areaNum << 8) | 1;
		p_global->newLocation = 1;
		p_global->prevLocation = p_global->roomNum & 0xFF;
		curArea = p_global->roomNum >> 8;
		if (curArea == go->areaNum)
			newArea = 0;
		else {
			for (; go->curAreaNum != 0xFF; go++)
				if (go->curAreaNum == curArea)
					break;
			if (go->areaNum == 0xFF)
				return;
		}
		p_global->eventType = EventType::etGotoArea | newArea;
		init_oui();
		showevents1();
		waitendspeak();
		if (pomme_q)
			return;
		close_perso();
		if (verif_oui())
			gotolieu(go);
	}
	void record() {
		short i;
		tape_t *tape;
		perso_t *perso;
		if (p_global->curObjectId)
			return;
		if (p_global->perso_ptr >= &kPersons[PER_UNKN_18C])
			return;
		if (p_global->eventType == EventType::etEventE || p_global->eventType >= EventType::etGotoArea)
			return;
		for (tape = tapes; tape != tapes + MAX_TAPES; tape++)
			if (tape->textNum == p_global->textNum)
				return;
		for (tape = tapes, i = 0; i < MAX_TAPES - 1; i++) {
			tape->textNum = tape[+1].textNum;
			tape->perso = tape[+1].perso;
			tape->party = tape[+1].party;
			tape->roomNum = tape[+1].roomNum;
			tape->bgBankNum = tape[+1].bgBankNum;
			tape->dialog = tape[+1].dialog;
			tape++;
		}
		perso = p_global->perso_ptr;
		if (perso == &kPersons[PER_EVE])
			perso = p_global->phaseNum >= 352 ? &kPersons[PER_UNKN_372]
			        : &kPersons[PER_UNKN_402];
		tape->textNum = p_global->textNum;
		tape->perso = perso;
		tape->party = p_global->party;
		tape->roomNum = p_global->roomNum;
		tape->bgBankNum = p_global->roomBgBankNum;
		tape->dialog = p_global->dialog_ptr;
	}
	char dial_scan(dial_t *dial) {
		unsigned char flags;
		unsigned char hidx, lidx;
		char bidx, pnum;
		short i;
		unsigned short mask;
		perso_t *perso;
		if (p_global->numGiveObjs) {
			if (!(p_global->displayFlags & DisplayFlags::dfFlag2))
				showobjects();
			p_global->numGiveObjs = 0;
		}
		p_global->dialog_ptr = dial;
		vavapers();
		p_global->phraseBufferPtr = phraseBuffer;
		for (;; p_global->dialog_ptr++) {
			for (;; p_global->dialog_ptr++) {
				if (p_global->dialog_ptr->flags == -1 && p_global->dialog_ptr->condNumLow == -1)
					return 0;
				flags = p_global->dialog_ptr->flags;
				p_global->dialogFlags = flags;
				if (!(flags & DialogFlags::dfSpoken) || (flags & DialogFlags::dfRepeatable)) {
					hidx = (p_global->dialog_ptr->textCondHiMask >> 6) & 3;
					lidx = p_global->dialog_ptr->condNumLow;
					if (flags & 0x10)
						hidx |= 4;
					if (testcondition(((hidx << 8) | lidx) & 0x7FF))
						break;
				} else {
					if (flags & dialogSkipFlags)
						continue;
					hidx = (p_global->dialog_ptr->textCondHiMask >> 6) & 3;
					lidx = p_global->dialog_ptr->condNumLow;
					if (flags & 0x10)
						hidx |= 4;
					if (testcondition(((hidx << 8) | lidx) & 0x7FF))
						break;
				}
			}
			bidx = (p_global->dialog_ptr->textCondHiMask >> 2) & 0xF;
			if (!bidx)
				goto no_perso;  //TODO: rearrange
			mask = (p_global->party | p_global->partyOutside) & (1 << (bidx - 1));
			if (mask)
				break;
		}
		for (perso = kPersons; !(perso->party == mask && perso->roomNum == p_global->roomNum); perso++) ; //Find matching
		p_global->perso_ptr = perso;
		init_perso_ptr(perso);
		no_perso();
no_perso:
		;
		hidx = p_global->dialog_ptr->textCondHiMask;
		lidx = p_global->dialog_ptr->textNumLow;
		p_global->textNum = ((hidx << 8) | lidx) & 0x3FF;
		if (p_global->phraseBufferPtr != phraseBuffer) {
			for (i = 0; i < 32; i++)
				SysBeep(1);
		} else
			my_bulle();
		if (!dword_30B04) {
			static void (EdenGameImpl::*talk_subject[])() = {
				&EdenGameImpl::oui,
				&EdenGameImpl::non,
				&EdenGameImpl::eloipart,
				&EdenGameImpl::dialautoon,
				&EdenGameImpl::dialautooff,
				&EdenGameImpl::stay_here,
				&EdenGameImpl::follow,
				&EdenGameImpl::citadelle,
				&EdenGameImpl::dialonfollow,
				&EdenGameImpl::abortdial,
				&EdenGameImpl::incphase,
				&EdenGameImpl::bigphase,
				&EdenGameImpl::giveobject,
				&EdenGameImpl::choixzone,
				&EdenGameImpl::lostobject
			};
			pnum = p_global->dialog_ptr->flags & 0xF;
			if (pnum)
				(this->*talk_subject[pnum - 1])();
			p_global->ff_60 = -1;
			p_global->dialog_ptr->flags |= DialogFlags::dfSpoken;
			p_global->dialog_ptr++;
		}
		if (p_global->dialogType != DialogType::dtInspect) {
			record();
			getdatasync();
			show_perso();
			persovox();
		}
		return 1;
	}
	char dialoscansvmas(dial_t *dial) {
		char res;
		char old = dialogSkipFlags;
		dialogSkipFlags = DialogFlags::df20;
		res = dial_scan(dial);
		dialogSkipFlags = old;
		return res;
	}
	char dialo_even(perso_t *perso) {
		char res;
		int num;
		dial_t *dial;
		p_global->perso_ptr = perso;
		num = (perso->id << 3) | DialogType::dtEvent;
		dial = (dial_t *)GetElem(gameDialogs, num);
		res = dialoscansvmas(dial);
		p_global->last_dialog_ptr = 0;
		byte_30AFE = 0;
		return res;
	}
	void stay_here() {
		if (p_global->perso_ptr == &kPersons[PER_DINA] && p_global->roomNum == 260)
			p_global->gameFlags |= GameFlags::gfFlag1000;
		reste_ici5();
	}
	void mort(short vid) {
		bars_out();
		playhnm(vid);
		fadetoblack(2);
		CLBlitter_FillScreenView(0);
		CLBlitter_FillView(p_mainview, 0);
		bars_in();
		p_global->narratorSequence = 51;
		p_global->newMusicType = MusicType::mtNormal;
		musique();
		musicspy();
	}
	void evenchrono() {
		unsigned short old;
		if (!(p_global->displayFlags & DisplayFlags::dfFlag1))
			return;
		old = p_global->gameTime;
		currentTime = TimerTicks / 100;
		p_global->gameTime = currentTime;
		if (p_global->gameTime <= old)
			return;
		heurepasse();
		if (!(p_global->chrono_on & 1))
			return;
		p_global->chrono -= 200;
		if (p_global->chrono == 0)
			p_global->chrono_on |= 2;
		if (!(p_global->chrono_on & 2))
			return;
		p_global->chrono_on = 0;
		p_global->chrono = 0;
		if (p_global->roomPersoType == PersonFlags::pftTyrann) {
			short vid = 272;
			if (p_global->curRoomFlags & 0xC0) {
				vid += 2;
				if ((p_global->curRoomFlags & 0xC0) != 0x80) {
					vid += 2;
					mort(vid);
					return;
				}
			}
			if (p_global->areaNum == Areas::arUluru || p_global->areaNum == Areas::arTamara) {
				mort(vid);
				return;
			}
			vid++;
			mort(vid);
			return;
		}
		if (p_global->roomNum == 2817) {
			suis_moi(5);
			p_global->gameFlags |= GameFlags::gfFlag40;
			dialautoon();
		} else
			eloirevient();
		p_global->eventType = EventType::etEvent10;
		showevents();
	}
	void chronoon(short t) {
		p_global->chrono = t;
		p_global->chrono_on = 1;
	}
	void prechargephrases(short vid) {
		int num;
		dial_t *dial;
		perso_t *perso = &kPersons[PER_MORKUS];
		if (vid == 170)
			perso = &kPersons[PER_UNKN_156];
		p_global->perso_ptr = perso;
		p_global->dialogType = DialogType::dtInspect;
		num = (perso->id << 3) | p_global->dialogType;
		dial = (dial_t *)GetElem(gameDialogs, num);
		dialoscansvmas(dial);
	}
	void effet1() {
		debug(__FUNCTION__);
		int x, y;
		short i;
		short dy, ny;
		rectanglenoir32();
		if (!doubled) {
			setRS1(0, 0, 16 - 1, 4 - 1);
			y = p_mainview->norm.dst_top;
			for (i = 16; i <= 96; i += 4) {
				for (x = p_mainview->norm.dst_left; x < p_mainview->norm.dst_left + 320; x += 16) {
					setRD1(x, y + i, x + 16 - 1, y + i + 4 - 1);
					CLBlitter_CopyViewRect(p_view2, &ScreenView, &rect_src, &rect_dst);
					setRD1(x, y + 192 - i, x + 16 - 1, y + 192 - i + 4 - 1);
					CLBlitter_CopyViewRect(p_view2, &ScreenView, &rect_src, &rect_dst);
				}
				CLBlitter_UpdateScreen();
				wait(1);
			}
		} else {
			setRS1(0, 0, 16 * 2 - 1, 4 * 2 - 1);
			y = p_mainview->zoom.dst_top;
			for (i = 16 * 2; i <= 96 * 2; i += 4 * 2) {
				for (x = p_mainview->zoom.dst_left; x < p_mainview->zoom.dst_left + 320 * 2; x += 16 * 2) {
					setRD1(x, y + i, x + 16 * 2 - 1, y + i + 4 * 2 - 1);
					CLBlitter_CopyViewRect(p_view2, &ScreenView, &rect_src, &rect_dst);
					setRD1(x, y + 192 * 2 - i, x + 16 * 2 - 1, y + 192 * 2 - i + 4 * 2 - 1);
					CLBlitter_CopyViewRect(p_view2, &ScreenView, &rect_src, &rect_dst);
				}
				wait(1);
			}
		}
		CLPalette_Send2Screen(global_palette, 0, 256);
		p_mainview->norm.height = 2;
		p_mainview->zoom.height = 4;
		ny = p_mainview->norm.dst_top;
		dy = p_mainview->zoom.dst_top;
		for (i = 0; i < 100; i += 2) {
			p_mainview->norm.src_top = 99 - i;
			p_mainview->zoom.src_top = 99 - i;
			p_mainview->norm.dst_top = 99 - i + ny;
			p_mainview->zoom.dst_top = (99 - i) * 2 + dy;
			CLBlitter_CopyView2Screen(p_mainview);
			p_mainview->norm.src_top = 100 + i;
			p_mainview->zoom.src_top = 100 + i;
			p_mainview->norm.dst_top = 100 + i + ny;
			p_mainview->zoom.dst_top = (100 + i) * 2 + dy;
			CLBlitter_CopyView2Screen(p_mainview);
			CLBlitter_UpdateScreen();
			wait(1);
		}
		p_mainview->norm.height = 200;
		p_mainview->zoom.height = 400;
		p_mainview->norm.src_top = 0;
		p_mainview->zoom.src_top = 0;
		p_mainview->norm.dst_top = ny;
		p_mainview->zoom.dst_top = dy;
		p_global->ff_F1 = 0;
	}
	void effet2() {
		debug(__FUNCTION__);
		static int eff2pat = 0;
		if (p_global->ff_103 == 69) {
			effet4();
			return;
		}
		switch (++eff2pat) {
		case 1: {
			static short pattern[] = {0, 1, 2, 3, 7, 11, 15, 14, 13, 12, 8, 4, 5, 6, 10, 9};
			colimacon(pattern);
			break;
		}
		case 2: {
			static short pattern[] = {0, 15, 1, 14, 2, 13, 3, 12, 7, 8, 11, 4, 5, 10, 6, 9};
			colimacon(pattern);
			break;
		}
		case 3: {
			static short pattern[] = {0, 2, 5, 7, 8, 10, 13, 15, 1, 3, 4, 6, 9, 11, 12, 14};
			colimacon(pattern);
			break;
		}
		case 4: {
			static short pattern[] = {0, 3, 15, 12, 1, 7, 14, 8, 2, 11, 13, 4, 5, 6, 10, 9};
			colimacon(pattern);
			eff2pat = 0;
			break;
		}
		}
	}
	void effet3() {
		debug(__FUNCTION__);
		unsigned short i, c;
		CLPalette_GetLastPalette(oldPalette);
		for (i = 0; i < 6; i++) {
			for (c = 0; c < 256; c++) {
				newColor.r = oldPalette[c].r >> i;
				newColor.g = oldPalette[c].g >> i;
				newColor.b = oldPalette[c].b >> i;
				CLPalette_SetRGBColor(newPalette, c, &newColor);
			}
			CLPalette_Send2Screen(newPalette, 0, 256);
			wait(1);
		}
		CLBlitter_CopyView2Screen(p_mainview);
		for (i = 0; i < 6; i++) {
			for (c = 0; c < 256; c++) {
				newColor.r = global_palette[c].r >> (5 - i);
				newColor.g = global_palette[c].g >> (5 - i);
				newColor.b = global_palette[c].b >> (5 - i);
				CLPalette_SetRGBColor(newPalette, c, &newColor);
			}
			CLPalette_Send2Screen(newPalette, 0, 256);
			wait(1);
		}
	}
	void effet4() {
		debug(__FUNCTION__);
		unsigned char *scr, *pix, *r24, *r25, *r30, c;
		short i;
		short x, y;
		short w, h, ww;
		short r17, r23, r16, r18, r19, r22, r27, r31;
		CLPalette_Send2Screen(global_palette, 0, 256);
		w = ScreenView.width;
		h = ScreenView.height;
		ww = ScreenView.pitch;
		if (!doubled) {
			x = p_mainview->norm.dst_left;
			y = p_mainview->norm.dst_top;
			for (i = 32; i > 0; i -= 2) {
				scr = ScreenView.p_buffer;
				scr += (y + 16) * ww + x;
				pix = p_mainview->p_buffer + 16 * 640;
				r17 = 320 / i;
				r23 = 320 - 320 / i * i;  //TODO: 320 % i ?
				r16 = 160 / i;
				r18 = 160 - 160 / i * i;  //TODO: 160 % i ?
				for (r19 = r16; r19 > 0; r19--) {
					r24 = scr;
					r25 = pix;
					for (r22 = r17; r22 > 0; r22--) {
						c = *r25;
						r25 += i;
						r30 = r24;
						for (r27 = i; r27 > 0; r27--) {
							for (r31 = i; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - i;
						}
						r24 += i;
					}
					if (r23) {
						c = *r25;
						r30 = r24;
						for (r27 = i; r27 > 0; r27--) {
							for (r31 = r23; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - r23;
						}
					}
					scr += i * ww;
					pix += i * 640;
				}
				if (r18) {
					r24 = scr;
					r25 = pix;
					for (r22 = r17; r22 > 0; r22--) {
						c = *r25;
						r25 += i;
						r30 = r24;
						for (r27 = r18; r27 > 0; r27--) {
							for (r31 = i; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - i;
						}
						r24 += i;
					}
					if (r23) {
						c = *r25;
						r30 = r24;
						for (r27 = r18; r27 > 0; r27--) {
							for (r31 = r23; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - r23;
						}
					}
				}
				CLBlitter_UpdateScreen();
				wait(3);
			}
		} else {
			x = p_mainview->zoom.dst_left;
			y = p_mainview->zoom.dst_top;
			for (i = 32; i > 0; i -= 2) {
				scr = ScreenView.p_buffer;
				scr += (y + 16 * 2) * ww + x;
				pix = p_mainview->p_buffer + 16 * 640;
				r17 = 320 / i;
				r23 = 320 % i;
				r16 = 160 / i;
				r18 = 160 % i;
				for (r19 = r16; r19 > 0; r19--) {
					r24 = scr;
					r25 = pix;
					for (r22 = r17; r22 > 0; r22--) {
						c = *r25;
						r25 += i;
						r30 = r24;
						for (r27 = i * 2; r27 > 0; r27--) {
							for (r31 = i * 2; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - i * 2;
						}
						r24 += i * 2;
					}
					if (r23) {
						c = *r25;
						r30 = r24;
						for (r27 = i * 2; r27 > 0; r27--) {
							for (r31 = r23 * 2; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - r23 * 2;
						}
					}
					scr += i * ww * 2;
					pix += i * 640;
				}
				if (r18) {
					r24 = scr;
					r25 = pix;
					for (r22 = r17; r22 > 0; r22--) {
						c = *r25;
						r25 += i;
						r30 = r24;
						for (r27 = r18 * 2; r27 > 0; r27--) {
							for (r31 = i * 2; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - i * 2;
						}
						r24 += i * 2;
					}
					if (r23) {
						c = *r25;
						r30 = r24;
						for (r27 = i * 2; r27 > 0; r27--) {
							for (r31 = r23 * 2; r31 > 0; r31--)
								*r30++ = c;
							r30 += ww - r23 * 2;
						}
					}
				}
				wait(3);
			}
		}
		CLBlitter_CopyView2Screen(p_mainview);
	}
	void ClearScreen() {
		unsigned char *scr;
		short x, y, xx, yy;
		short w, h, ww;
		w = ScreenView.width;
		h = ScreenView.height;
		ww = ScreenView.pitch;
		if (!doubled) {
			x = p_mainview->norm.dst_left;
			y = p_mainview->norm.dst_top;
			scr = ScreenView.p_buffer;
			scr += (y + 16) * ww + x;
			for (yy = 0; yy < 160; yy++) {
				for (xx = 0; xx < 320; xx++)
					*scr++ = 0;
				scr += ww - 320;
			}
		} else {
			x = p_mainview->zoom.dst_left;
			y = p_mainview->zoom.dst_top;
			scr = ScreenView.p_buffer;
			scr += (y + 32) * ww + x;
			for (yy = 0; yy < 160; yy++) {
				for (xx = 0; xx < 320; xx++) {
					scr[0] = 0;
					scr[1] = 0;
					scr[ww] = 0;
					scr[ww + 1] = 0;
					scr += 2;
				}
				scr += (ww - 320) * 2;
			}
		}
		CLBlitter_UpdateScreen();
	}
	void colimacon(short pattern[16]) {
		unsigned char *scr, *pix;
		short x, y, xx, yy;
		short w, h, ww;
		short i, j, p, r27, r25;
		w = ScreenView.width;
		h = ScreenView.height;
		ww = ScreenView.pitch;
		if (!doubled) {
			x = p_mainview->norm.dst_left;
			y = p_mainview->norm.dst_top;
			scr = ScreenView.p_buffer;
			scr += (y + 16) * ww + x;
			for (i = 0; i < 16; i++) {
				p = pattern[i];
				r27 = p % 4 + p / 4 * ww;
				for (j = 0; j < 320 * 160 / 16; j++)
					scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] = 0;
				CLBlitter_UpdateScreen();
				wait(1);
			}
		} else {
			x = p_mainview->zoom.dst_left;
			y = p_mainview->zoom.dst_top;
			scr = ScreenView.p_buffer;
			scr += (y + 16 * 2) * ww + x;
			for (i = 0; i < 16; i++) {
				p = pattern[i];
				r27 = p % 4 * 2 + p / 4 * ww * 2;
				for (j = 0; j < 320 * 160 / 16; j++) {
					unsigned char *sc = &scr[j / (320 / 4) * ww * 4 * 2 + j % (320 / 4) * 4 * 2 + r27];
					sc[0] = 0;
					sc[1] = 0;
					sc[ww] = 0;
					sc[ww + 1] = 0;
				}
				wait(1);
			}
		}
		CLPalette_Send2Screen(global_palette, 0, 256);
		if (!doubled) {
			pix = p_mainview->p_buffer;
			x = p_mainview->norm.dst_left;
			y = p_mainview->norm.dst_top;
			pix += 640 * 16;
			scr = ScreenView.p_buffer;
			scr += (y + 16) * ww + x;
			for (i = 0; i < 16; i++) {
				p = pattern[i];
				r25 = p % 4 + p / 4 * 640;
				r27 = p % 4 + p / 4 * ww;
				for (j = 0; j < 320 * 160 / 16; j++)
					scr[j / (320 / 4) * ww * 4 + j % (320 / 4) * 4 + r27] =
					    pix[j / (320 / 4) * 640 * 4 + j % (320 / 4) * 4 + r25];
				CLBlitter_UpdateScreen();
				wait(1);
			}
		} else {
			pix = p_mainview->p_buffer;
			x = p_mainview->zoom.dst_left;
			y = p_mainview->zoom.dst_top;
			pix += 640 * 16;
			scr = ScreenView.p_buffer;
			scr += (y + 16 * 2) * ww + x;
			for (i = 0; i < 16; i++) {
				p = pattern[i];
				r25 = p % 4 + p / 4 * 640;
				r27 = p % 4 * 2 + p / 4 * ww * 2;
				for (j = 0; j < 320 * 160 / 16; j++) {
					unsigned char c = pix[j / (320 / 4) * 640 * 4 + j % (320 / 4) * 4 + r25];
					unsigned char *sc = &scr[j / (320 / 4) * ww * 4 * 2 + j % (320 / 4) * 4 * 2 + r27];
					sc[0] = c;
					sc[1] = c;
					sc[ww] = c;
					sc[ww + 1] = c;
				}
				wait(1);
			}
		}
	}
	void fadetoblack(int delay) {
		short i, j;
		CLPalette_GetLastPalette(oldPalette);
		for (i = 0; i < 6; i++) {
			for (j = 0; j < 256; j++) {
				newColor.r = oldPalette[j].r >> i;
				newColor.g = oldPalette[j].g >> i;
				newColor.b = oldPalette[j].b >> i;
				CLPalette_SetRGBColor(newPalette, j, &newColor);
			}
			CLPalette_Send2Screen(newPalette, 0, 256);
			wait(delay);
		}
	}
	void fadetoblack128(int delay) {
		short i, j;
		CLPalette_GetLastPalette(oldPalette);
		for (i = 0; i < 6; i++) {
			for (j = 0; j < 129; j++) { //TODO: wha?
				newColor.r = oldPalette[j].r >> i;
				newColor.g = oldPalette[j].g >> i;
				newColor.b = oldPalette[j].b >> i;
				CLPalette_SetRGBColor(newPalette, j, &newColor);
			}
			CLPalette_Send2Screen(newPalette, 0, 128);
			wait(delay);
		}
	}
	void fadefromblack128(int delay) {
		short i, j;
		for (i = 0; i < 6; i++) {
			for (j = 0; j < 129; j++) { //TODO: wha?
				newColor.r = global_palette[j].r >> (5 - i);
				newColor.g = global_palette[j].g >> (5 - i);
				newColor.b = global_palette[j].b >> (5 - i);
				CLPalette_SetRGBColor(newPalette, j, &newColor);
			}
			CLPalette_Send2Screen(newPalette, 0, 128);
			wait(delay);
		}
	}
	void rectanglenoir32() {
		// blacken 32x32 rectangle
		int i;
		int *pix = (int *)p_view2_buf;
		for (i = 0; i < 32; i++) {
			pix[0] = 0;
			pix[1] = 0;
			pix[2] = 0;
			pix[3] = 0;
			pix[4] = 0;
			pix[5] = 0;
			pix[6] = 0;
			pix[7] = 0;
			pix += 32 / 4;
		}
	}
	void setRS1(short sx, short sy, short ex, short ey) {
		rect_src.sx = sx;
		rect_src.sy = sy;
		rect_src.ex = ex;
		rect_src.ey = ey;
	}
	void setRD1(short sx, short sy, short ex, short ey) {
		rect_dst.sx = sx;
		rect_dst.sy = sy;
		rect_dst.ex = ex;
		rect_dst.ey = ey;
	}
	void wait(int howlong) {
		int t2, t = TickCount();
#ifdef EDEN_DEBUG
		howlong *= 10;
#endif
		for (t2 = t; t2 - t < howlong; t2 = TickCount()) g_system->delayMillis(10); // waste time
	}
	void effetpix() {
		debug(__FUNCTION__);
		unsigned char *scr, *pix;
		short x, y, xx, yy;
		short w, h, ww;
		short i, j, p, r25, r18, r31, r30;  //TODO: change to xx/yy
		unsigned char r24, r23;         //TODO: change to p0/p1
		short r26, r27, r20;
		w = ScreenView.width;
		h = ScreenView.height;
		ww = ScreenView.pitch;
		r25 = ww * 80;
		r18 = 640 * 80;
		pix = p_mainview->p_buffer + 16 * 640;
		if (!doubled) {
			x = p_mainview->norm.dst_left;
			y = p_mainview->norm.dst_top;
			scr = ScreenView.p_buffer;
			scr += (y + 16) * ww + x;
		} else {
			x = p_mainview->zoom.dst_left;
			y = p_mainview->zoom.dst_top;
			scr = ScreenView.p_buffer;
			scr += (y + 16 * 2) * ww + x;
			r25 *= 2;
		}
		r20 = 0x4400;   //TODO
		r27 = 1;
		r26 = 0;
		do {
			char r8 = r27 & 1;
			r27 >>= 1;
			if (r8)
				r27 ^= r20;
			if (r27 < 320 * 80) {
				r31 = r27 / 320;
				r30 = r27 % 320;
				if (doubled) {
					r31 *= 2;
					r30 *= 2;
					scr[r31 * ww + r30] = 0;
					scr[r31 * ww + r30 + 1] = 0;
					scr[r31 * ww + r25 + r30] = 0;
					scr[r31 * ww + r25 + r30 + 1] = 0;
					r31++;
					scr[r31 * ww + r30] = 0;
					scr[r31 * ww + r30 + 1] = 0;
					scr[r31 * ww + r25 + r30] = 0;
					scr[r31 * ww + r25 + r30 + 1] = 0;
					if (++r26 == 960) {
						wait(1);
						r26 = 0;
					}
				} else {
					scr[r31 * ww + r30] = 0;
					scr[r31 * ww + r25 + r30] = 0;
					if (++r26 == 960) {
						CLBlitter_UpdateScreen();
						wait(1);
						r26 = 0;
					}
				}
			}
		} while (r27 != 1);
		CLPalette_Send2Screen(global_palette, 0, 256);
		r20 = 0x4400;
		r27 = 1;
		r26 = 0;
		do {
			char r8 = r27 & 1;
			r27 >>= 1;
			if (r8)
				r27 ^= r20;
			if (r27 < 320 * 80) {
				r31 = r27 / 320;
				r30 = r27 % 320;
				r24 = pix[r31 * 640 + r30];
				r23 = pix[r31 * 640 + r18 + r30];
				if (doubled) {
					r31 *= 2;
					r30 *= 2;
					scr[r31 * ww + r30] = r24;
					scr[r31 * ww + r30 + 1] = r24;
					scr[r31 * ww + r25 + r30] = r23;
					scr[r31 * ww + r25 + r30 + 1] = r23;
					r31++;
					scr[r31 * ww + r30] = r24;
					scr[r31 * ww + r30 + 1] = r24;
					scr[r31 * ww + r25 + r30] = r23;
					scr[r31 * ww + r25 + r30 + 1] = r23;
					if (++r26 == 960) {
						wait(1);
						r26 = 0;
					}
				} else {
					scr[r31 * ww + r30] = r24;
					scr[r31 * ww + r25 + r30] = r23;
					if (++r26 == 960) {
						CLBlitter_UpdateScreen();
						wait(1);
						r26 = 0;
					}
				}
			}
		} while (r27 != 1);
		assert(ScreenView.pitch == 320);
	}
////// datfile.c
	void verifh(void *ptr) {
		unsigned char sum = 0;
		unsigned char *data;
		unsigned char *head = (unsigned char *)ptr;
		unsigned short h0, h3;
		short i;
		char h2;
		for (i = 0; i < 6; i++)
			sum += *head++;
		if (sum != 0xAB)
			return;
		debug("* Begin unpacking resource");
		head -= 6;
		h0 = PLE16(head);
		head += 2;
		h2 = *head++;
		h3 = PLE16(head);
		head += 2;
		data = h0 + head + 26;
		h3 -= 6;
		head += h3;
		for (; h3; h3--)
			*data-- = *head--;
		head = data + 1;
		data = (unsigned char *)ptr;
		Expand_hsq(head, data);
	}
	void openbigfile() {
		assert(sizeof(pakfile_t) == 25);
		long size = 0x10000;
		CLFile_MakeStruct(0, 0, "EDEN.DAT", &bigfilespec);
		CLFile_Open(&bigfilespec, 1, h_bigfile);
		CLFile_Read(h_bigfile, bigfile_header, &size);
		p_hnmcontext = CLHNM_New(128);
		CLHNM_SetFile(p_hnmcontext, &h_bigfile);
	}
	void closebigfile() {
		CLFile_Close(h_bigfile);
	}
	void loadfile(unsigned short num, void *buffer) {
		short retry, res = 1;
		assert(num < bigfile_header->count);
		for (retry = 0; res && retry < 5; retry++) {
			pakfile_t *file = &bigfile_header->files[num];
			long size = PLE32(&file->size);
			long offs = PLE32(&file->offs);
			debug("* Loading resource %d (%s) at 0x%X, %d bytes", num, file->name, offs, size);
			CLFile_SetPosition(h_bigfile, fsFromStart, offs);
			CLFile_Read(h_bigfile, buffer, &size);
			res = 0;
		}
		if (res)
			quit_flag = 1;
	}
	void shnmfl(unsigned short num) {
		int res;
		assert(num + 484 < bigfile_header->count);
		pakfile_t *file = &bigfile_header->files[num + 484];
		int size = PLE32(&file->size);
		int offs = PLE32(&file->offs);
		debug("* Loading movie %d (%s) at 0x%X, %d bytes", num, file->name, offs, size);
		CLHNM_SetPosIntoFile(p_hnmcontext, offs);
	}
	int ssndfl(unsigned short num) {
		int res;
		assert(num + 660 < bigfile_header->count);
		pakfile_t *file = &bigfile_header->files[num + 660];
		long size = PLE32(&file->size);
		long offs = PLE32(&file->offs);
		if (soundAllocated) {
			CLMemory_Free(voiceSamplesBuffer);
			soundAllocated = 0; //TODO: bug??? no alloc
		} else {
			voiceSamplesBuffer = CLMemory_Alloc(size);
			soundAllocated = 1;
		}
		CLFile_SetPosition(h_bigfile, 1, offs);
		CLFile_Read(h_bigfile, voiceSamplesBuffer, &size);
		return size;

	}
#if 1
	void ConvertIcons(icon_t *icon, int count) {
		int i;
		for (i = 0; i < count; i++, icon++) {
			icon->sx = BE16(icon->sx);
			icon->sy = BE16(icon->sy);
			icon->ex = BE16(icon->ex);
			icon->ey = BE16(icon->ey);
			icon->cursor_id = BE16(icon->cursor_id);
			icon->object_id = BE32(icon->object_id);
			icon->action_id = BE32(icon->action_id);
		}
	}

	void ConvertLinks(room_t *room, int count) {
		int i;
		for (i = 0; i < count; i++, room++) {
			room->bank = BE16(room->bank);
			room->party = BE16(room->party);
		}
	}

	void ConvertMacToPC() {
		// Conert all mac (big-endian) resources to native format
		ConvertIcons(gameIcons, 136);
		ConvertLinks(gameRooms, 424);
		// Array of longs
		int *p = (int *)gameLipsync;
		for (int i = 0; i < 7240 / 4; i++)
			p[i] = BE32(p[i]);
	}
#endif

	void loadpermfiles() {
		loadfile(2498, gameIcons);
		loadfile(2497, gameRooms);
		loadfile(2486, gameLipsync);
		loadfile(0, main_bank_buf);
		loadfile(402, gameFont);
		loadfile(404, gameDialogs);
		loadfile(403, gameConditions);
#if 1
		ConvertMacToPC();
#endif
	}
	char ReadDataSync(unsigned short num) {
		long pos, len;
		pos = PLE32(gameLipsync + num * 4);
		len = 1024;
		if (pos != -1) {
			loadpartoffile(1936, gameLipsync + 7260, pos, len);
			return 1;
		}
		return 0;
	}
	void loadpartoffile(unsigned short num, void *buffer, long pos, long len) {
		short res;
		assert(num < bigfile_header->count);
		pakfile_t *file = &bigfile_header->files[num];
		long offs = PLE32(&file->offs);
		debug("* Loading partial resource %d (%s) at 0x%X(+0x%X), %d bytes", num, file->name, offs, pos, len);
		CLFile_SetPosition(h_bigfile, 1, offs + pos);
		CLFile_Read(h_bigfile, buffer, &len);
	}
	void Expand_hsq(void *input, void *output) {
		unsigned char   *src = (unsigned char *)input;
		unsigned char   *dst = (unsigned char *)output;
		unsigned char   *ptr;
		unsigned short  bit;        // bit
		unsigned short  queue = 0;  // queue
		unsigned short  len = 0;
		short       ofs;
#define GetBit                          \
	bit = queue & 1;                    \
	queue >>= 1;                        \
	if(!queue)                          \
	{                                   \
		queue = (src[1] << 8) | src[0]; src += 2;   \
		bit = queue & 1;                            \
		queue = (queue >> 1) | 0x8000;              \
	}

		for (;;) {
			GetBit;
			if (bit)
				*dst++ = *src++;
			else {
				len = 0;
				GetBit;
				if (!bit) {
					GetBit;
					len = (len << 1) | bit;
					GetBit;
					len = (len << 1) | bit;
					ofs = 0xFF00 | *src++;      //TODO: -256
				} else {
					ofs = (src[1] << 8) | src[0];
					src += 2;
					len = ofs & 7;
					ofs = (ofs >> 3) | 0xE000;
					if (!len) {
						len = *src++;
						if (!len)
							break;
					}
				}
				ptr = dst + ofs;
				len += 2;
				while (len--)
					*dst++ = *ptr++;
			}
		}
	}
//////
	void ajouinfo(unsigned char info) {
		unsigned char idx = p_global->next_info_idx;
		if (kPersons[PER_MESSAGER].roomNum)
			info |= 0x80;
		info_list[idx] = info;
		if (idx == p_global->last_info_idx)
			p_global->last_info = info;
		idx++;
		if (idx == 16) idx = 0;
		p_global->next_info_idx = idx;
	}
	void unlockinfo() {
		unsigned char idx;
		for (idx = 0; idx < 16; idx++) {
			if (info_list[idx] != 0xFF)
				info_list[idx] &= ~0x80;
		}
		p_global->last_info &= ~0x80;
	}
	void nextinfo() {
		do {
			unsigned char idx = p_global->last_info_idx;
			info_list[idx] = 0;
			idx++;
			if (idx == 16) idx = 0;
			p_global->last_info_idx = idx;
			p_global->last_info = info_list[idx];
		} while (p_global->last_info == 0xFF);
	}
	void delinfo(unsigned char info) {
		unsigned char idx;
		for (idx = 0; idx < 16; idx++) {
			if ((info_list[idx] & ~0x80) == info) {
				info_list[idx] = 0xFF;
				if (idx == p_global->last_info_idx)
					nextinfo();
				break;
			}
		}
	}
	void updateinfolist() {
		int idx;
		for (idx = 0; idx < 16; idx++)
			info_list[idx] = 0;
	}
	void init_globals() {
		gameIcons[16].cursor_id |= 0x8000;

		p_global->areaNum = Areas::arMo;
		p_global->areaVisitCount = 1;
		p_global->menuItemIdLo = 0;
		p_global->menuItemIdHi = 0;
		p_global->randomNumber = 0;
		p_global->gameTime = 0;
		p_global->gameDays = 0;
		p_global->chrono = 0;
		p_global->eloiDepartureDay = 0;
		p_global->roomNum = 259;
		p_global->newRoomNum = 0;
		p_global->phaseNum = 0;
		p_global->metPersonsMask1 = 0;
		p_global->party = 0;
		p_global->partyOutside = 0;
		p_global->metPersonsMask2 = 0;
		p_global->__UNUSED_1C = 0;
		p_global->phaseActionsCount = 0;
		p_global->curAreaFlags = 0;
		p_global->curItemsMask = 0;
		p_global->curPowersMask = 0;
		p_global->curPersoItems = 0;
		p_global->curPersoPowers = 0;
		p_global->wonItemsMask = 0;
		p_global->wonPowersMask = 0;
		p_global->stepsToFindAppleFast = 0;
		p_global->stepsToFindAppleNormal = 0;
		p_global->roomPersoItems = 0;
		p_global->roomPersoPowers = 0;
		p_global->gameFlags = 0;
		p_global->curVideoNum = 0;
		p_global->morkusSpyVideoNum1 = 89;
		p_global->morkusSpyVideoNum2 = 88;
		p_global->morkusSpyVideoNum3 = 83;
		p_global->morkusSpyVideoNum4 = 94;
		p_global->newMusicType = MusicType::mtDontChange;
		p_global->ff_43 = 0;
		p_global->videoSubtitleIndex = 0;
		p_global->partyInstruments = 0;
		p_global->monkGotRing = 0;
		p_global->chrono_on = 0;
		p_global->curRoomFlags = 0;
		p_global->endGameFlag = 0;
		p_global->last_info = 0;
		p_global->autoDialog = 0;
		p_global->worldTyrannSighted = 0;
		p_global->ff_4D = 0;
		p_global->ff_4E = 0;
		p_global->worldGaveGold = 0;
		p_global->worldHasTriceraptors = 0;
		p_global->worldHasVelociraptors = 0;
		p_global->worldHasTyrann = 0;
		p_global->ff_53 = 0;
		p_global->ff_54 = 0;
		p_global->ff_55 = 0;
		p_global->ff_56 = 0;
		p_global->textToken1 = 0;
		p_global->textToken2 = 0;
		p_global->eloiHaveNews = 0;
		p_global->dialogFlags = 0;
		p_global->curAreaType = 0;
		p_global->curCitadelLevel = 0;
		p_global->newLocation = 0;
		p_global->prevLocation = 0;
		p_global->curPersoFlags = 0;
		p_global->ff_60 = 0;
		p_global->eventType = EventType::etEvent5;
		p_global->ff_62 = 0;
		p_global->curObjectId = 0;
		p_global->curObjectFlags = 0;
		p_global->ff_65 = 1;
		p_global->roomPersoType = 0;
		p_global->roomPersoFlags = 0;
		p_global->narratorSequence = 0;
		p_global->ff_69 = 0;
		p_global->ff_6A = 0;
		p_global->fresqNumber = 0;
		p_global->ff_6C = 0;
		p_global->ff_6D = 0;
		p_global->labyrinthDirections = 0;
		p_global->labyrinthRoom = 0;
		p_global->curPersoAnimPtr = 0;
		p_global->perso_img_bank = 0;
		p_global->roomImgBank = 0;
		p_global->persoBackgroundBankIdx = 55;
		p_global->ff_D4 = 0;
		p_global->fresqWidth = 0;
		p_global->fresqImgBank = 0;
		p_global->ff_DA = 0;
		p_global->ff_DC = 0;
		p_global->room_x_base = 0;
		p_global->ff_E0 = 0;
		p_global->dialogType = 0;
		p_global->ff_E4 = 0;
		p_global->currentMusicNum = 0;
		p_global->textNum = 0;
		p_global->travelTime = 0;
		p_global->ff_EC = 0;
		p_global->displayFlags = DisplayFlags::dfFlag1;
		p_global->oldDisplayFlags = 1;
		p_global->drawFlags = 0;
		p_global->ff_F1 = 0;
		p_global->ff_F2 = 0;
		p_global->menuFlags = 0;
		p_global->ff_F5 = 0;
		p_global->ff_F6 = 0;
		p_global->ff_F7 = 0;
		p_global->ff_F8 = 0;
		p_global->ff_F9 = 0;
		p_global->ff_FA = 0;
		p_global->animationFlags = 0;
		p_global->__UNUSED_FC = 0;
		p_global->giveobj1 = 0;
		p_global->giveobj2 = 0;
		p_global->giveobj3 = 0;
		p_global->ff_100 = 0;
		p_global->roomVidNum = 0;
		p_global->ff_102 = 0;
		p_global->ff_103 = 0;
		p_global->roomBgBankNum = 0;
		p_global->valleyVidNum = 0;
		p_global->updatePaletteFlag = 0;
		p_global->inventoryScrollPos = 0;
		p_global->obj_count = 0;
		p_global->ff_109 = 0;
		p_global->textBankIndex = 69;
		p_global->cita_area_num = 0;
		p_global->ff_113 = 0;
		p_global->lastSalNum = 0;
		p_global->__UNUSED_70 = 0;
		p_global->dialog_ptr = 0;
		p_global->tape_ptr = tapes;
		p_global->next_dialog_ptr = 0;
		p_global->narrator_dialog_ptr = 0;
		p_global->last_dialog_ptr = 0;
		p_global->nextRoomIcon = 0;
		p_global->phraseBufferPtr = 0;
		p_global->__UNUSED_90 = 0;
		p_global->__UNUSED_94 = 0;
		p_global->room_ptr = 0;
		p_global->area_ptr = 0;
		p_global->last_area_ptr = 0;
		p_global->cur_area_ptr = 0;
		p_global->cita_area_firstRoom = 0;
		p_global->perso_ptr = 0;
		p_global->room_perso = 0;
		p_global->last_info_idx = 0;
		p_global->next_info_idx = 0;
		p_global->iconsIndex = 16;
		p_global->persoSpritePtr = 0;
		p_global->numGiveObjs = 0;

		rect_31C7A.sy = 0;          //TODO: unused?
		rect_31C7A.sx = 0;
		rect_31C7A.ex = 320 - 1;
		rect_31C7A.ey = 200 - 1;

		initrect();

		underSubtitlesScreenRect.sy = 0;
		underSubtitlesScreenRect.sx = subtitles_x_scr_margin;
		underSubtitlesScreenRect.ex = subtitles_x_scr_margin + subtitles_x_width - 1;
		underSubtitlesScreenRect.ey = 176 - 1;

		underSubtitlesBackupRect.sy = 0;
		underSubtitlesBackupRect.sx = subtitles_x_scr_margin;
		underSubtitlesBackupRect.ex = subtitles_x_scr_margin + subtitles_x_width - 1;
		underSubtitlesBackupRect.ey = 60 - 1;
	}
	void initrect() {
		underTopBarScreenRect.sy = 0;
		underTopBarScreenRect.sx = 0;
		underTopBarScreenRect.ex = 320 - 1;
		underTopBarScreenRect.ey = 16 - 1;

		underTopBarBackupRect.sy = 0;
		underTopBarBackupRect.sx = 0;
		underTopBarBackupRect.ex = 320 - 1;
		underTopBarBackupRect.ey = 16 - 1;

		underBottomBarScreenRect.sy = 176;
		underBottomBarScreenRect.sx = 0;
		underBottomBarScreenRect.ex = 320 - 1;
		underBottomBarScreenRect.ey = 200 - 1;  //TODO: original bug? this cause crash in copyrect (this, underBottomBarBackupRect)

		underBottomBarBackupRect.sy = 16;
		underBottomBarBackupRect.sx = 0;
		underBottomBarBackupRect.ex = 320 - 1;
		underBottomBarBackupRect.ey = 40 - 1;
	}
	void closesalle() {
		if (p_global->displayFlags & DisplayFlags::dfPanable) {
			p_global->displayFlags &= ~DisplayFlags::dfPanable;
			resetscroll();
		}
	}
	void afsalle1(room_t *room) {
		unsigned char *ptr = (unsigned char *)GetElem(sal_buf, room->ff_0 - 1);
		ptr++;
		for (;;) {
			unsigned char b0, b1;
			short index, x, y, ex, ey;
			b0 = *ptr++;
			b1 = *ptr++;
			index = (b1 << 8) | b0;
			if (index == -1)
				break;
			if (index > 0) {
				x = *ptr++ | (((b1 & 0x2) >> 1) << 8);      //TODO: check me
				y = *ptr++;
				ptr++;
				index &= 0x1FF;
				if (!(p_global->displayFlags & 0x80)) {
					if (index == 1 || p_global->ff_F7)
						noclipax_avecnoir(index - 1, x, y);
				}
				p_global->ff_F7 = 0;
				continue;
			}
			if (b1 & 0x40) {
				if (b1 & 0x20) {
					char addIcon = 0;
					icon_t *icon = p_global->nextRoomIcon;
					if (b0 < 4) {
						if (p_global->room_ptr->exits[b0])
							addIcon = 1;
					} else if (b0 > 229) {
						if (p_global->partyOutside & (1 << (b0 - 230)))
							addIcon = 1;
					} else if (b0 >= 100) {
						debug("add object %d", b0 - 100);
						if (objecthere(b0 - 100)) {
							addIcon = 1;
							p_global->ff_F7 = -1;
						}
					} else
						addIcon = 1;
					if (addIcon) {
						icon->action_id = b0;
						icon->object_id = b0;
						icon->cursor_id = kActionCursors[b0];
						x = PLE16(ptr);
						ptr += 2;
						y = PLE16(ptr);
						ptr += 2;
						ex = PLE16(ptr);
						ptr += 2;
						ey = PLE16(ptr);
						ptr += 2;
						x += p_global->room_x_base;
						ex += p_global->room_x_base;
						debug("add hotspot at %3d:%3d - %3d:%3d, action = %d", x, y, ex, ey, b0);
#ifdef EDEN_DEBUG
						for (int iii = x; iii < ex; iii++) p_mainview_buf[y * 640 + iii] = p_mainview_buf[ey * 640 + iii] = (iii % 2) ? 0 : 255;
						for (int iii = y; iii < ey; iii++) p_mainview_buf[iii * 640 + x] = p_mainview_buf[iii * 640 + ex] = (iii % 2) ? 0 : 255;
#endif
						icon->sx = x;
						icon->sy = y;
						icon->ex = ex;
						icon->ey = ey;
						p_global->nextRoomIcon = ++icon;
						icon->sx = -1;
					} else
						ptr += 8;
				} else
					ptr += 8;
			} else
				ptr += 8;
		}
	}
	void afsalle() {
		room_t *room = p_global->room_ptr;
		p_global->displayFlags = DisplayFlags::dfFlag1;
		p_global->room_x_base = 0;
		p_global->roomBgBankNum = room->background;
		if (room->flags & RoomFlags::rf08) {
			p_global->displayFlags |= DisplayFlags::dfFlag80;
			if (room->flags & RoomFlags::rfPanable) {
				p_global->displayFlags |= DisplayFlags::dfPanable;
				p_global->ff_F4 = 0;
				rundcurs();
				sauvefrises();
				use_bank(room->bank - 1);
				noclipax_avecnoir(0, 0, 16);
				use_bank(room->bank);
				noclipax_avecnoir(0, 320, 16);
				afsalle1(room);
				p_global->room_x_base = 320;
				afsalle1(room + 1);
			} else
				afsalle1(room);
		} else {
			//TODO: roomImgBank is garbage here!
			debug("drawroom: room 0x%X using bank %d", p_global->roomNum, p_global->roomImgBank);
			use_bank(p_global->roomImgBank);
			afsalle1(room);
			assert(ScreenView.pitch == 320);
		}
	}
	void aflieu() {
		no_perso();
		if (!pomme_q) {
			p_global->iconsIndex = 16;
			p_global->autoDialog = 0;
		}
		p_global->nextRoomIcon = &gameIcons[28];
		afsalle();
		needPaletteUpdate = 1;
	}
	void loadsal(short num) {
		if (num == p_global->lastSalNum)
			return;
		p_global->lastSalNum = num;
		loadfile(num + 419, sal_buf);
	}
	void specialoutside() {
		if (p_global->last_area_ptr->type == AreaType::atValley && (p_global->party & PersonMask::pmLeader))
			perso_ici(5);
	}
	void specialout() {
		if (p_global->gameDays - p_global->eloiDepartureDay > 2) {
			if (eloirevientq() == 1)
				eloirevient();
		}
		if (p_global->phaseNum >= 32 && p_global->phaseNum < 48)
			if (p_global->newLocation == 9 || p_global->newLocation == 4 || p_global->newLocation == 24) {
				kPersons[PER_MESSAGER].roomNum = 263;
				return;
			}
		if (p_global->phaseNum == 434)
			if (p_global->newLocation == 5) {
				reste_ici(4);
				kPersons[PER_BOURREAU].roomNum = 264;
				return;
			}
		if (p_global->phaseNum < 400)
			if ((p_global->gameFlags & GameFlags::gfFlag4000) && p_global->prevLocation == 1
			        && (p_global->party & PersonMask::pmEloi) && p_global->curAreaType == AreaType::atValley)
				eloipart();
		if (p_global->phaseNum == 386)
			if (p_global->prevLocation == 1
			        && (p_global->party & PersonMask::pmEloi) && p_global->areaNum == Areas::arCantura)
				eloipart();
	}
	void specialin() {
		if (!(p_global->party & PersonMask::pmEloi) && (p_global->partyOutside & PersonMask::pmEloi) && (p_global->roomNum & 0xFF) == 1) {
			suis_moi(5);
			p_global->eloiHaveNews = 1;
		}
		if (p_global->roomNum == 288)
			p_global->gameFlags |= GameFlags::gfFlag100 | GameFlags::gfFlag2000;
		if (p_global->roomNum == 3075 && p_global->phaseNum == 546) {
			incphase1();
			if (p_global->curItemsMask & 0x2000) { // Morkus' tablet
				bars_out();
				playhnm(92);
				gameRooms[129].exits[0] = 0;
				gameRooms[129].exits[2] = 1;
				p_global->roomNum = 3074;
				kPersons[PER_MANGO].roomNum = 3074;
				p_global->eventType = EventType::etEvent5;
				maj_salle(p_global->roomNum);
				return;
			}
			p_global->narratorSequence = 53;
		}
		if (p_global->roomNum == 1793 && p_global->phaseNum == 336)
			eloipart();
		if (p_global->roomNum == 259 && p_global->phaseNum == 129)
			p_global->narratorSequence = 12;
		if (p_global->roomNum >= 289 && p_global->roomNum < 359)
			p_global->labyrinthDirections = kLabyrinthPath[(p_global->roomNum & 0xFF) - 33];
		if (p_global->roomNum == 305 && p_global->prevLocation == 103)
			p_global->gameFlags &= ~GameFlags::gfFlag2000;
		if (p_global->roomNum == 304 && p_global->prevLocation == 105)
			p_global->gameFlags &= ~GameFlags::gfFlag2000;
		if (p_global->phaseNum < 226) {
			if (p_global->roomNum == 842)
				p_global->gameFlags |= GameFlags::gfFlag2;
			if (p_global->roomNum == 1072)
				p_global->gameFlags |= GameFlags::gfFlag4;
			if (p_global->roomNum == 1329)
				p_global->gameFlags |= GameFlags::gfFlag8000;
		}
	}
	void animpiece() {
		room_t *room = p_global->room_ptr;
		if (p_global->roomVidNum && p_global->ff_100 != 0xFF) {
			if (p_global->valleyVidNum || !room->level || (room->flags & RoomFlags::rfHasCitadel)
			        || room->level == p_global->ff_100) {
				bars_out();
				p_global->updatePaletteFlag = 16;
				if (!p_global->narratorSequence & 0x80) //TODO: bug? !() @ 100DC
					p_global->ff_102 = 0;
				if (!needToFade)
					needToFade = room->flags & RoomFlags::rf02;
				playhnm(p_global->roomVidNum);
				return;
			}
		}
		p_global->ff_F1 &= ~RoomFlags::rf04;
	}
	void getdino(room_t *room) {
		assert(tab_2CEF0[4] == 0x25);
		perso_t *perso = &kPersons[PER_UNKN_18C];
		short *tab;
		unsigned char persoType, r27;
		short bank;
		room->flags &= ~0xC;
		for (; perso->roomNum != 0xFFFF; perso++) {
			if (perso->flags & PersonFlags::pf80)
				continue;
			if (perso->roomNum != p_global->roomNum)
				continue;
			persoType = perso->flags & PersonFlags::pfTypeMask;
			if (persoType == PersonFlags::pftVelociraptor)
				delinfo(p_global->cita_area_num + ValleyNews::vnVelociraptorsIn);
			if (persoType == PersonFlags::pftTriceraptor)
				delinfo(p_global->cita_area_num + ValleyNews::vnTriceraptorsIn);
			perso->flags |= PersonFlags::pf20;
			tab = tab_2CF70;
			if (p_global->areaNum != Areas::arUluru && p_global->areaNum != Areas::arTamara)
				tab = tab_2CEF0;
			r27 = (room->flags & 0xC0) >> 2;    //TODO: check me (like pc)
			persoType = perso->flags & PersonFlags::pfTypeMask;
			if (persoType == PersonFlags::pftTyrann)
				persoType = 13;
			r27 |= (persoType & 7) << 1;    //TODO: check me 13 & 7 = ???
			tab += r27;
			p_global->roomVidNum = *tab++;
			bank = *tab;
			if (bank & 0x8000) {
				bank &= ~0x8000;
				room->flags |= RoomFlags::rf08;
			}
			room->flags |= RoomFlags::rf04 | RoomFlags::rf02;
			p_global->roomImgBank = bank;
			break;
		}
	}
	room_t *getsalle(short loc) { //TODO: unsigned char?
		debug("get room for %X, starting from %d, looking for %X", loc, p_global->area_ptr->firstRoomIndex, p_global->partyOutside);
		room_t *room = &gameRooms[p_global->area_ptr->firstRoomIndex];
		loc &= 0xFF;
		for (;; room++) {
			for (; room->location != loc; room++) {
				if (room->ff_0 == 0xFF)
					return 0;
			}
			if (p_global->partyOutside == room->party || room->party == 0xFFFF)
				break;
		}
		debug("found room: party = %X, bank = %X", room->party, room->bank);
		p_global->roomImgBank = room->bank;
		p_global->labyrinthRoom = 0;
		if (p_global->roomImgBank > 104 && p_global->roomImgBank <= 112)
			p_global->labyrinthRoom = p_global->roomImgBank - 103;
		if (p_global->valleyVidNum)
			p_global->roomVidNum = p_global->valleyVidNum;
		else
			p_global->roomVidNum = room->video;
		if ((room->flags & 0xC0) == RoomFlags::rf40 || (room->flags & RoomFlags::rf01))
			getdino(room);
		if (room->flags & RoomFlags::rfHasCitadel) {
			delinfo(p_global->areaNum + ValleyNews::vnCitadelLost);
			delinfo(p_global->areaNum + ValleyNews::vnTyrannIn);
			delinfo(p_global->areaNum + ValleyNews::vnTyrannLost);
			delinfo(p_global->areaNum + ValleyNews::vnVelociraptorsLost);
		}
		if (istyran(p_global->roomNum))
			p_global->gameFlags |= GameFlags::gfFlag10;
		else
			p_global->gameFlags &= ~GameFlags::gfFlag10;
		return room;
	}
	void initlieu(short roomNum) {
		area_t *area;
		p_global->gameFlags |= GameFlags::gfFlag4000;
		gameIcons[18].cursor_id |= 0x8000;
		p_global->last_area_ptr = p_global->area_ptr;
		p_global->area_ptr = &kAreasTable[((roomNum >> 8) & 0xFF) - 1];
		area = p_global->area_ptr;
		area->visitCount++;
		p_global->areaVisitCount = area->visitCount;
		p_global->curAreaFlags = area->flags;
		p_global->curAreaType = area->type;
		p_global->curCitadelLevel = area->citadelLevel;
		if (p_global->curAreaType == AreaType::atValley)
			gameIcons[18].cursor_id &= ~0x8000;
		loadsal(area->salNum);
	}
	void maj2() {
		char r9, r30;
		room_t *room = p_global->room_ptr;  //TODO: unused
		aflieu();
		assert(ScreenView.pitch == 320);
		if (p_global->roomNum == 273 && p_global->prevLocation == 18)
			p_global->ff_102 = 1;
		if (p_global->eventType == EventType::etEventC) {
			afftopscr();
			showobjects();
		}
		FRDevents();
		assert(ScreenView.pitch == 320);
		r9 = 0;
		if (p_global->curAreaType == AreaType::atValley && !(p_global->displayFlags & DisplayFlags::dfPanable))
			r9 = 1;
		r30 = r9;   //TODO: ^^ inlined func?
		if (p_global->ff_102 || p_global->ff_103)
			afficher();
		else if (p_global->ff_F1 == (RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01)) {
			blackbars();
			effet1();
		} else if (p_global->ff_F1 && !(p_global->ff_F1 & RoomFlags::rf04) && !r30) {
			if (!(p_global->displayFlags & DisplayFlags::dfPanable))
				blackbars();
			else if (p_global->valleyVidNum)
				blackbars();
			effet1();
		} else if (r30 && !(p_global->ff_F1 & RoomFlags::rf04))
			effetpix();
		else
			afficher128();
		musique();
		if (p_global->eventType != EventType::etEventC) {
			afftopscr();
			showobjects();
		}
		bars_in();
		showevents();
		p_global->labyrinthDirections = 0;
		specialin();
	}
	void majsalle1(short roomNum) {
		room_t *room = getsalle(roomNum & 0xFF);
		p_global->room_ptr = room;
		debug("DrawRoom: room 0x%X, arg = 0x%X", p_global->roomNum, roomNum);
		p_global->curRoomFlags = room->flags;
		p_global->ff_F1 = room->flags;
		animpiece();
		p_global->ff_100 = 0;
		maj2();
	}
	void maj_salle(unsigned short roomNum) {
		setpersohere();
		majsalle1(roomNum);
	}
	void initbuf() {
#define ALLOC(ptr, size, typ) if(!((ptr) = (typ*)malloc(size))) quit_flag = 1;
		ALLOC(bigfile_header, 0x10000, pak_t);
		ALLOC(gameRooms, 0x4000, room_t);
		ALLOC(gameIcons, 0x4000, icon_t);
		ALLOC(bank_data_buf, 0x10000, unsigned char);
		ALLOC(p_global, sizeof(*p_global), global_t);
		ALLOC(sal_buf, 2048, void);
		ALLOC(gameConditions, 0x4800, unsigned char);
		ALLOC(gameDialogs, 0x2800, unsigned char);
		ALLOC(gamePhrases, 0x10000, unsigned char);
		ALLOC(main_bank_buf, 0x9400, unsigned char);
		ALLOC(glow_buffer, 0x2800, unsigned char);
		ALLOC(gameFont, 0x900, unsigned char);
		ALLOC(gameLipsync, 0x205C, unsigned char);
		ALLOC(music_buf, 0x140000, unsigned char);
#undef ALLOC
	}
	void freebuf() {
		free(bigfile_header);
		free(gameRooms);
		free(gameIcons);
		free(bank_data_buf);
		free(p_global);
		free(sal_buf);
		free(gameConditions);
		free(gameDialogs);
		free(gamePhrases);
		free(main_bank_buf);
		free(glow_buffer);
		free(gameFont);
		free(gameLipsync);
		free(music_buf);
	}
	void openwindow() {
		p_underBarsView = CLView_New(320, 40);
		p_underBarsView->norm.width = 320;

		p_view2 = CLView_New(32, 32);
		p_view2_buf = p_view2->p_buffer;

		p_subtitlesview = CLView_New(subtitles_x_width, 60);
		p_subtitlesview_buf = p_subtitlesview->p_buffer;

		p_underSubtitlesView = CLView_New(subtitles_x_width, 60);
		p_underSubtitlesView_buf = p_underSubtitlesView->p_buffer;

		p_mainview = CLView_New(640, 200);
		p_mainview->norm.width = 320;
		CLBlitter_FillView(p_mainview, 0xFFFFFFFF);
		CLView_SetSrcZoomValues(p_mainview, 0, 0);
		CLView_SetDisplayZoomValues(p_mainview, 640, 400);
		CLScreenView_CenterIn(p_mainview);
		p_mainview_buf = p_mainview->p_buffer;

		mouse_x_center = p_mainview->norm.dst_left + p_mainview->norm.width / 2;
		mouse_y_center = p_mainview->norm.dst_top + p_mainview->norm.height / 2;
		CLMouse_SetPosition(mouse_x_center, mouse_y_center);
		CLMouse_Hide();

		curs_x = 320 / 2;
		curs_y = 200 / 2;
	}
	void EmergencyExit() {
		SysBeep(1);
	}
public:
	void main() {
		debug("global size is %X", (size_t)(&((global_t *)0)->save_end));
		if ((size_t)(&((global_t *)0)->__UNUSED_70) != 0x70) { // let's be more optimistic
			// great, you broke the dialog system. expect all nasty stuff now
			assert(0);
		}



		CRYOLib_Init();
		CRYOLib_InstallExitPatch();
		CRYOLib_SetDebugMode(0);
		word_378CE = 0;
		CRYOLib_MinimalInit();
		CRYOLib_TestConfig();
		CRYOLib_ManagersInit();
		CLFile_SetFilter(1, 'EDNS', 0, 0, 0);
//	CRYOLib_InstallEmergencyExit(EmergencyExit);
		CRYOLib_SetupEnvironment();
		CLHNM_SetupSound(5, 0x2000, 8, 11025 * 65536.0 , 0);
		CLHNM_SetForceZero2Black(1);
		CLHNM_SetupTimer(12.5);
		voiceSound = CLSoundRaw_New(0, 11025 * 65536.0, 8, 0);
		hnmsound_ch = CLHNM_GetSoundChannel();
		music_channel = CLSoundChannel_New(0);
		CLSound_SetWantsDesigned(1);

		initbuf();
		openbigfile();
		openwindow();
		loadpermfiles();

		if (!quit_flag) {
			LostEdenMac_InitPrefs();
			init_cube();
			p_mainview->doubled = doubled;
			while (!quit_flag2) {
				init_globals();
				quit_flag3 = 0;
				normalCursor = 1;
				torchCursor = 0;
				curs_keepy = -1;
				curs_keepx = -1;
				CLDesktop_TestOpenFileAtStartup();
				if (!gameLoaded)
					intro();
				edmain();
				if (quit_flag)
					goto quit;
				startmusique(1);
				blackbars();
				afficher();
				fadetoblack(3);
				ClearScreen();
				playhnm(95);
				if (p_global->endGameFlag == 50) {
					loadrestart();
					gameLoaded = 0;
				}
				fademusica0(2);
				CLSoundChannel_Stop(music_channel);
				CLSoundGroup_Free(mus_queue_grp);
				musicPlaying = 0;
				mus_queue_grp = 0;
			}
//		LostEdenMac_SavePrefs();
		}
quit:
		;
		fadetoblack(4);
		closebigfile();
		freebuf();
		CRYOLib_RestoreEnvironment();
		CRYOLib_ManagersDone();
		CRYOLib_Done();
		CRYOLib_RemoveExitPatch();
	}
private:
	void edmain() {
		//TODO
		entergame();
		while (!quit_flag && !quit_flag3 && p_global->endGameFlag != 50) {
			if (!gameStarted) {
				// if in demo mode, reset game after a while
				demoCurrentTicks = TimerTicks;
				if (demoCurrentTicks - demoStartTicks > 3000) {
					rundcurs();
					afficher();
					fademusica0(2);
					fadetoblack(3);
					CLBlitter_FillScreenView(0);
					CLBlitter_FillView(p_mainview, 0);
					CLSoundChannel_Stop(music_channel);
					CLSoundGroup_Free(mus_queue_grp);
					musicPlaying = 0;
					mus_queue_grp = 0;
					intro();
					entergame();
				}
			}
			rundcurs();
			musicspy();
			FRDevents();
			narrateur();
			evenchrono();
			if (p_global->drawFlags & DrawFlags::drDrawInventory) showobjects();
			if (p_global->drawFlags & DrawFlags::drDrawTopScreen) afftopscr();
			if ((p_global->displayFlags & DisplayFlags::dfPanable) && (p_global->displayFlags != DisplayFlags::dfPerson)) scrollpano();
			if ((p_global->displayFlags & DisplayFlags::dfMirror) && (p_global->displayFlags != DisplayFlags::dfPerson)) scrollmiroir();
			if ((p_global->displayFlags & DisplayFlags::dfFresques) && (p_global->displayFlags != DisplayFlags::dfPerson)) scrollfresques();
			if (p_global->displayFlags & DisplayFlags::dfFlag2) noclicpanel();
			if (animationActive) anim_perso();
			update_cursor();
			afficher();
		}
	}
	void intro() {
#if 0
		return;
#endif

		short speed = 0;
		if (!machine_speed) {
			if (!word_378CC)
				speed = 4;
			else {
				if (CLComputer_Has68030())
					speed = 1;
				if (CLComputer_Has68040())
					speed = 2;
			}
			if (speed == 2)
				if (testcdromspeed())
					speed++;
			machine_speed = speed;
		}
		if (machine_speed == 1) {
			doubled = 0;
			p_mainview->doubled = doubled;
			if (ScreenView.width < 640 || ScreenView.height < 400)
				allow_doubled = 0;
		}
		if (machine_speed < 3)
			playhnm(98);
		else {
			CLSoundChannel_Stop(hnmsound_ch);
			CLHNM_CloseSound();
			CLHNM_SetupSound(5, 0x2000, 16, 22050 * 65536.0, 0);
			hnmsound_ch = CLHNM_GetSoundChannel();
			playhnm(2012);
		}
		playhnm(171);
		CLBlitter_FillScreenView(0);
		specialTextMode = 0;
		if (machine_speed < 3)
			playhnm(170);
		else {
			playhnm(2001);
			CLSoundChannel_Stop(hnmsound_ch);
			CLHNM_CloseSound();
			CLHNM_SetupSound(5, 0x2000, 8, 11025 * 65536.0, 0);
			hnmsound_ch = CLHNM_GetSoundChannel();
		}
	}
	char testcdromspeed() {
		return 1;
	}
	void entergame() {
		char flag = 0;
		currentTime = TimerTicks / 100;
		p_global->gameTime = currentTime;
		demoStartTicks = TimerTicks;
		gameStarted = 0;
		if (!gameLoaded) {
			p_global->roomNum = 279;
			p_global->areaNum = Areas::arMo;
			p_global->ff_100 = -1;
			initlieu(p_global->roomNum);
			p_global->currentMusicNum = 0;
			startmusique(1);
		} else {
			flag = p_global->autoDialog == 0xFF;    //TODO
			initafterload();
			lastMusicNum = p_global->currentMusicNum;   //TODO: ???
			p_global->currentMusicNum = 0;
			startmusique(lastMusicNum);
			p_global->inventoryScrollPos = 0;
			gameStarted = 1;
		}
		showobjects();
		afftopscr();
		sauvefrises();
		showBlackBars = 1;
		p_global->ff_102 = 1;
		maj_salle(p_global->roomNum);
		if (flag) {
			p_global->iconsIndex = 4;
			p_global->autoDialog = -1;
			parle_moi();
		}
	}
	void signon(char *s) {
	}
	void testPommeQ() {
		char key;
		if (!CLKeyboard_HasCmdDown())
			return;
		key = CLKeyboard_GetLastASCII();
		if (key == 'Q' || key == 'q')
			if (!pomme_q)
				pomme_q = 1;
	}
	void FRDevents() {
		short dx, dy, max_y;
		CLKeyboard_Read();
		if (allow_doubled) {
			if (CLKeyboard_IsScanCodeDown(0x30)) { //TODO: const
				if (!keybd_held) {
					doubled = !doubled;
					p_mainview->doubled = doubled;
					CLBlitter_FillScreenView(0);
					keybd_held = 1;
				}
			} else
				keybd_held = 0;
		}
		CLMouse_GetPosition(&mouse_x, &mouse_y);
		dx = mouse_x - mouse_x_center;
		dy = mouse_y - mouse_y_center;
		CLMouse_SetPosition(mouse_x_center, mouse_y_center);
		curs_x += dx;
		if (curs_x < 4)
			curs_x = 4;
		if (curs_x > 292)
			curs_x = 292;
		curs_y += dy;
		max_y = p_global->displayFlags == DisplayFlags::dfFlag2 ? 190 : 170;
		if (curs_y < 4)
			curs_y = 4;
		if (curs_y > max_y)
			curs_y = max_y;
		curs_x_pan = curs_x;
		if (curs_y >= 10 && curs_y <= 164 && !(p_global->displayFlags & DisplayFlags::dfFresques))
			curs_x_pan += scroll_pos;
		if (normalCursor) {
			current_cursor = 0;
			current_spot = scan_icon_list(curs_x_pan + curs_center, curs_y + curs_center, p_global->iconsIndex);
			if (current_spot)
				current_cursor = current_spot->cursor_id;
		}
		if (curs_center == 0 && current_cursor != 53) {
			curs_center = 11;
			curs_x -= 11;
		}
		if (curs_center == 11 && current_cursor == 53) {
			curs_center = 0;
			curs_x += 11;
		}
		if (p_global->displayFlags & DisplayFlags::dfPanable) {
			//TODO: current_spot may be zero (due to scan_icon_list failure) if cursor slips between hot areas.
			//fix me here or above?
			if (current_spot) { // ok, plug it here
				current_spot2 = current_spot;
				affrepereadam(current_spot2->action_id - 14);
			}
		}
		if (p_global->displayFlags == DisplayFlags::dfFlag2 && current_spot)
			current_spot2 = current_spot;
		if (p_global->displayFlags & DisplayFlags::dfFresques) {
			if (fresqTalk)
				restaurefondbulle();
			if (current_cursor == 9 && !torchCursor) {
				rundcurs();
				torchCursor = 1;
				glow_x = -1;
			}
			if (current_cursor != 9 && torchCursor == 1) {
				unglow();
				torchCursor = 0;
				curs_saved = 0;
			}
		}
		if (CLMouse_IsDown()) {
			if (!mouse_held) {
				mouse_held = 1;
				gameStarted = 1;
				mouse();
			}
		} else
			mouse_held = 0;
		if (p_global->displayFlags != DisplayFlags::dfFlag2) {
			if (--inventoryScrollDelay <= 0) {
				if (p_global->obj_count > 9 && curs_y > 164) {
					if (curs_x > 284 && p_global->inventoryScrollPos + 9 < p_global->obj_count) {
						p_global->inventoryScrollPos++;
						inventoryScrollDelay = 20;
						showobjects();
					}
					if (curs_x < 30 && p_global->inventoryScrollPos != 0) {
						p_global->inventoryScrollPos--;
						inventoryScrollDelay = 20;
						showobjects();
					}
				}
			}
		}
		if (inventoryScrollDelay < 0)
			inventoryScrollDelay = 0;
		if (!pomme_q) {
			testPommeQ();
			if (pomme_q) {
				PommeQ();
				return;     //TODO: useless
			}
		}
	}
	icon_t *scan_icon_list(short x, short y, short index) {
		icon_t *icon;
		for (icon = &gameIcons[index]; icon->sx >= 0; icon++) {
			if (icon->cursor_id & 0x8000)
				continue;
#if 0
			// MAC version use this check. Same check is present in PC version, but never used
			// Because of x >= clause two adjacent rooms has 1-pixel wide dead zone between them
			// On valley view screens if cursor slips in this zone a crash in FRDevents occurs
			// due to lack of proper checks
			if (x < icon->ff_0 || x >= icon->ff_4
			        || y < icon->ff_2 || y >= icon->ff_6)
#else
			// PC version has this check inlined in FRDevents
			// Should we keep it or fix edge coordinates in afroom() instead?
			if (x < icon->sx || x > icon->ex
			        || y < icon->sy || y > icon->ey)
#endif
				continue;
			return icon;
		}
		return 0;
	}
	void update_cursor() {
		if (++word_2C300 > 3)
			word_2C300 = 0;
		if (!word_2C300) {
			word_2C304++;
			word_2C302++;
		}
		if (word_2C304 > 8)
			word_2C304 = 0;
		if (word_2C302 > 4)
			word_2C302 = 0;
		if (!torchCursor) {
			use_main_bank();
			sundcurs(curs_x + scroll_pos, curs_y);
			if (current_cursor != 53 && current_cursor < 10) //TODO: cond
				moteur();
			else
				noclipax(current_cursor, curs_x + scroll_pos, curs_y);
			glow_x = 1;
		} else {
			use_bank(117);
			if (curs_x > 294)
				curs_x = 294;
			unglow();
			glow(word_2C302);
			noclipax(word_2C304, curs_x + scroll_pos, curs_y);
			if (fresqTalk)
				af_subtitle();
		}
	}
	void mouse() {
		static void (EdenGameImpl::*mouse_actions[])() = {
			&EdenGameImpl::goto_nord,
			&EdenGameImpl::goto_est,
			&EdenGameImpl::goto_sud,
			&EdenGameImpl::goto_ouest,
			&EdenGameImpl::plaquemonk,
			&EdenGameImpl::fresquesgraa,
			&EdenGameImpl::pushpierre,
			&EdenGameImpl::tetesquel,
			&EdenGameImpl::tetemomie,
			&EdenGameImpl::goto_nord,
			&EdenGameImpl::roiparle1,
			&EdenGameImpl::roiparle2,
			&EdenGameImpl::roiparle3,
			&EdenGameImpl::gotohall,
			&EdenGameImpl::demitourlabi,
			&EdenGameImpl::squelmoorkong,
			&EdenGameImpl::gotonido,
			&EdenGameImpl::voirlac,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::final,
			&EdenGameImpl::goto_nord,
			&EdenGameImpl::goto_sud,
			&EdenGameImpl::visiter,
			&EdenGameImpl::dinosoufle,
			&EdenGameImpl::fresqueslasc,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			&EdenGameImpl::gotoval,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::getprisme,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::getoeuf,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::getchampb,
			&EdenGameImpl::getchampm,
			&EdenGameImpl::getcouteau,
			&EdenGameImpl::getnidv,
			&EdenGameImpl::getnido,
			&EdenGameImpl::getor,
			nullptr,
			&EdenGameImpl::ret,
			&EdenGameImpl::getsoleil,
			&EdenGameImpl::getcorne,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			&EdenGameImpl::ret,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::getplaque,
			&EdenGameImpl::clicplanval,
			&EdenGameImpl::finfresques,
			&EdenGameImpl::choisir,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::roi,
			&EdenGameImpl::dina,
			&EdenGameImpl::thoo,
			&EdenGameImpl::monk,
			&EdenGameImpl::bourreau,
			&EdenGameImpl::messager,
			&EdenGameImpl::mango,
			&EdenGameImpl::eve,
			&EdenGameImpl::azia,
			&EdenGameImpl::mammi,
			&EdenGameImpl::gardes,
			&EdenGameImpl::fisher,
			&EdenGameImpl::dino,
			&EdenGameImpl::tyran,
			&EdenGameImpl::morkus,
			&EdenGameImpl::ret,
			&EdenGameImpl::parle_moi,
			&EdenGameImpl::adam,
			&EdenGameImpl::takeobject,
			&EdenGameImpl::putobject,
			&EdenGameImpl::clictimbre,
			&EdenGameImpl::dinaparle,
			&EdenGameImpl::close_perso,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			&EdenGameImpl::generique,
			&EdenGameImpl::choixsubtitle,
			&EdenGameImpl::EdenQuit,
			&EdenGameImpl::restart,
			&EdenGameImpl::cancel2,
			&EdenGameImpl::testvoice,
			&EdenGameImpl::reglervol,
			&EdenGameImpl::load,
			&EdenGameImpl::save,
			&EdenGameImpl::cliccurstape,
			&EdenGameImpl::playtape,
			&EdenGameImpl::stoptape,
			&EdenGameImpl::rewindtape,
			&EdenGameImpl::forwardtape,
			&EdenGameImpl::confirmyes,
			&EdenGameImpl::confirmno,
			&EdenGameImpl::gotocarte
		};

		if (!(current_spot = scan_icon_list(curs_x_pan + curs_center,
		                                    curs_y + curs_center, p_global->iconsIndex)))
			return;
		current_spot2 = current_spot;
		debug("invoking mouse action %d", current_spot->action_id);
		if (mouse_actions[current_spot->action_id])
			(this->*mouse_actions[current_spot->action_id])();
	}
////// film.c
	void showfilm(char arg1) {
		short playing;
		CLHNM_Prepare2Read(p_hnmcontext, 0);
		CLHNM_ReadHeader(p_hnmcontext);
		if (p_global->curVideoNum == 92) {
			p_hnmcontext->header.flag2 = 0;
			CLSoundChannel_SetVolumeLeft(hnmsound_ch, 0);
			CLSoundChannel_SetVolumeRight(hnmsound_ch, 0);
		}
		playing = 1;
		if (CLHNM_GetVersion(p_hnmcontext) != 4)
			return;
		CLHNM_AllocMemory(p_hnmcontext);
		p_hnmview = CLView_New(p_hnmcontext->header.width, p_hnmcontext->header.height);
		CLView_SetSrcZoomValues(p_hnmview, 0, 0);
		CLView_SetDisplayZoomValues(p_hnmview, p_hnmcontext->header.width * 2, p_hnmcontext->header.height * 2);
		CLScreenView_CenterIn(p_hnmview);
		p_hnmview_buf = p_hnmview->p_buffer;
		if (arg1) {
			p_hnmview->norm.height = 160;
			p_hnmview->zoom.height = 320;   //TODO: width??
			p_hnmview->norm.dst_top = p_mainview->norm.dst_top + 16;
			p_hnmview->zoom.dst_top = p_mainview->zoom.dst_top + 32;
		}
		CLHNM_SetFinalBuffer(p_hnmcontext, p_hnmview->p_buffer);
		p_hnmview->doubled = doubled;
		do {
			hnm_position = CLHNM_GetFrameNum(p_hnmcontext);
			CLHNM_WaitLoop(p_hnmcontext);
			playing = CLHNM_NextElement(p_hnmcontext);
			if (specialTextMode)
				bullehnm();
			else
				musicspy();
			CLBlitter_CopyView2Screen(p_hnmview);
			assert(ScreenView.pitch == 320);
			CLKeyboard_Read();
			if (allow_doubled) {
				if (CLKeyboard_IsScanCodeDown(0x30)) { //TODO: const
					if (!keybd_held) {
						doubled = !doubled;
						p_hnmview->doubled = doubled;   //TODO: but mainview ?
						CLBlitter_FillScreenView(0);
						keybd_held = 1;
					}
				} else
					keybd_held = 0;
			}
			if (arg1) {
				if (CLMouse_IsDown()) {
					if (!mouse_held) {
						mouse_held = 1;
						videoCanceled = 1;
					}
				} else
					mouse_held = 0;
			}
		} while (playing && !videoCanceled);
		CLView_Free(p_hnmview);
		CLHNM_DeallocMemory(p_hnmcontext);
	}
	void playhnm(short num) {
		perso_t *perso;
		short oldDialogType;
		p_global->curVideoNum = num;
		if (num != 2001 && num != 2012 && num != 98 && num != 171) {
			unsigned char oldMusicType = p_global->newMusicType;
			p_global->newMusicType = MusicType::mtEvent;
			musique();
			musicspy();
			p_global->newMusicType = oldMusicType;
		}
		p_global->videoSubtitleIndex = 1;
		if (specialTextMode) {
			perso = p_global->perso_ptr;
			oldDialogType = p_global->dialogType;
			prechargephrases(num);
			fademusica0(1);
			CLSoundChannel_Stop(music_channel);
		}
		showVideoSubtitle = 0;
		videoCanceled = 0;
		shnmfl(num);
		CLHNM_Reset(p_hnmcontext);
		CLHNM_FlushPreloadBuffer(p_hnmcontext);
		if (needToFade) {
			fadetoblack(4);
			ClearScreen();
			needToFade = 0;
		}
		if (num == 2012 || num == 98 || num == 171)
			showfilm(0);
		else
			showfilm(1);
		curs_keepx = curs_keepy = -1;
		p_mainview->doubled = doubled;
		if (specialTextMode) {
			mus_fade_flags = 3;
			musicspy();
			p_global->perso_ptr = perso;
			p_global->dialogType = oldDialogType;
			specialTextMode = 0;
		}
		if (videoCanceled)
			p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
		if (p_global->curVideoNum == 167)
			p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
		if (p_global->curVideoNum == 104)
			p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
		if (p_global->curVideoNum == 102)
			p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
		if (p_global->curVideoNum == 77)
			p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
		if (p_global->curVideoNum == 149)
			p_global->ff_F1 = RoomFlags::rf40 | RoomFlags::rf04 | RoomFlags::rf01;
	}
	void bullehnm() {
		short *frames, *frames_start, frame, num;
		perso_t *perso;
		switch (p_global->curVideoNum) {
		case 170:
			frames = tab_2D24C;
			perso = &kPersons[PER_UNKN_156];
			break;
		case 83:
			frames = tab_2D28E;
			perso = &kPersons[PER_MORKUS];
			break;
		case 88:
			frames = tab_2D298;
			perso = &kPersons[PER_MORKUS];
			break;
		case 89:
			frames = tab_2D2AA;
			perso = &kPersons[PER_MORKUS];
			break;
		case 94:
			frames = tab_2D2C4;
			perso = &kPersons[PER_MORKUS];
			break;
		default:
			return;
		}
		frames_start = frames;
		while ((frame = *frames++) != -1) {
			if ((frame & ~0x8000) == hnm_position)
				break;
		}
		if (frame == -1) {
			if (showVideoSubtitle)
				af_subtitlehnm();
			return;
		}
		if (frame & 0x8000)
			showVideoSubtitle = 0;
		else {
			p_global->videoSubtitleIndex = (frames - frames_start) / 2 + 1;
			p_global->perso_ptr = perso;
			p_global->dialogType = DialogType::dtInspect;
			num = (perso->id << 3) | p_global->dialogType;
			dialoscansvmas((dial_t *)GetElem(gameDialogs, num));
			showVideoSubtitle = 1;
		}
		if (showVideoSubtitle)
			af_subtitlehnm();
	}
////// sound.c
	void musique() {
		unsigned char flag, hidx, lidx, mus;
		dial_t *dial;
		if (p_global->newMusicType == MusicType::mtDontChange)
			return;
		dial = (dial_t *)GetElem(gameDialogs, 128);
		for (;; dial++) {
			if (dial->flags == -1 && dial->condNumLow == -1)
				return;
			flag = dial->flags;
			hidx = (dial->textCondHiMask & 0xC0) >> 6;
			lidx = dial->condNumLow;            //TODO: fixme - unsigned = signed
			if (flag & 0x10)
				hidx |= 4;
			if (testcondition(((hidx << 8) | lidx) & 0x7FF))
				break;
		}
		mus = dial->textNumLow;
		p_global->newMusicType = MusicType::mtDontChange;
		if (mus != 0 && mus != 2 && mus < 50)
			startmusique(mus);
	}
	void startmusique(unsigned char num) {
		short seq_size, pat_size, freq;
		if (num == p_global->currentMusicNum)
			return;
		if (musicPlaying) {
			fademusica0(1);
			CLSoundChannel_Stop(music_channel);
			CLSoundGroup_Free(mus_queue_grp);
		}
		loadmusicfile(num);
		p_global->currentMusicNum = num;
		mus_sequence_ptr = music_buf + 32;  //TODO: rewrite it properly
		seq_size = PLE16(music_buf + 30);
		mus_patterns_ptr = music_buf + 30 + seq_size;
		pat_size = PLE16(music_buf + 27);
		mus_samples_ptr = music_buf + 32 + 4 + pat_size;
		freq = PLE16(mus_samples_ptr - 2);
		if (freq == 166)
			mus_queue_grp = CLSoundGroup_New(3, 0, 8, 7.225344e8, 0);
		else
			mus_queue_grp = CLSoundGroup_New(3, 0, 8, 1.4450688e9, 0);
		musicSequencePos = 0;
		mus_vol_left = p_global->pref_10C[0];
		mus_vol_right = p_global->pref_10C[1];
		CLSoundChannel_SetVolumeLeft(music_channel, mus_vol_left);
		CLSoundChannel_SetVolumeRight(music_channel, mus_vol_right);
	}
	void musicspy() {
		unsigned char patnum, *patptr;
		int ofs, len;
		if (!mus_queue_grp)
			return;
		mus_vol_left = p_global->pref_10C[0];
		mus_vol_right = p_global->pref_10C[1];
		if (mus_fade_flags & 3)
			fademusicup();
		if (personTalking && !hnmsound_ch->numSounds)
			mus_fade_flags = 3;
		if (music_channel->numSounds < 3) {
			patnum = mus_sequence_ptr[musicSequencePos];
			if (patnum == 0xFF) {
				// rewind
				musicSequencePos = 0;
				patnum = mus_sequence_ptr[musicSequencePos];
			}
			musicSequencePos++;
			patptr = mus_patterns_ptr + patnum * 6;
			ofs = patptr[0] + (patptr[1] << 8) + (patptr[2] << 16);
			len = patptr[3] + (patptr[4] << 8) + (patptr[5] << 16);
			CLSoundGroup_AssignDatas(mus_queue_grp, mus_samples_ptr + ofs, len, 0);
			CLSoundGroup_PlayNextSample(mus_queue_grp, music_channel);
			musicPlaying = 1;
		}
	}
	int loadmusicfile(short num) {
		short res;
		long numread;
		pakfile_t *file = &bigfile_header->files[num + 435];
		long size = PLE32(&file->size);
		long offs = PLE32(&file->offs);
		CLFile_SetPosition(h_bigfile, 1, offs);
		numread = size;
		if (numread > 0x140000)     //TODO: const
			numread = 0x140000;
		CLFile_Read(h_bigfile, music_buf, &numread);
		return size;
	}
	void persovox() {
		short vol_l, vol_r, step_l, step_r;
		short num = p_global->textNum;
		if (p_global->textBankIndex != 1)
			num += 565;
		if (p_global->textBankIndex == 3)
			num += 707;
		voiceSamplesSize = ssndfl(num);
		vol_l = p_global->pref_110[0];
		vol_r = p_global->pref_110[1];
		step_l = -1;
		if (music_channel->volumeLeft < vol_l)
			step_l = 1;
		step_r = -1;
		if (music_channel->volumeRight < vol_r)
			step_r = 1;
		do {
			if (vol_l != music_channel->volumeLeft)
				CLSoundChannel_SetVolumeLeft(music_channel, music_channel->volumeLeft + step_l);
			if (vol_r != music_channel->volumeRight)
				CLSoundChannel_SetVolumeRight(music_channel, music_channel->volumeRight + step_r);
		} while (music_channel->volumeLeft != vol_l || music_channel->volumeRight != vol_r);
		vol_l = p_global->pref_10E[0];
		vol_r = p_global->pref_10E[1];
		CLSoundChannel_SetVolumeLeft(hnmsound_ch, vol_l);
		CLSoundChannel_SetVolumeRight(hnmsound_ch, vol_r);
		CLSound_SetWantsDesigned(0);
		CLSoundRaw_AssignBuffer(voiceSound, voiceSamplesBuffer, 0, voiceSamplesSize);
		CLSoundChannel_Play(hnmsound_ch, voiceSound);
		personTalking = 1;
		mus_fade_flags = 0;
		last_anim_ticks = TimerTicks;
	}
	void endpersovox() {
		restaurefondbulle();
		if (personTalking) {
			CLSoundChannel_Stop(hnmsound_ch);
			personTalking = 0;
			mus_fade_flags = 3;
		}
		if (soundAllocated) {
			CLMemory_Free(voiceSamplesBuffer);
			soundAllocated = 0;
		}
	}
	void fademusicup() {
		short vol;
		if (mus_fade_flags & 2) {
			vol = music_channel->volumeLeft;
			if (vol < mus_vol_left) {
				vol += 8;
				if (vol > mus_vol_left)
					vol = mus_vol_left;
			} else {
				vol -= 8;
				if (vol < mus_vol_left)
					vol = mus_vol_left;
			}
			CLSoundChannel_SetVolumeLeft(music_channel, vol);
			if (vol == mus_vol_left)
				mus_fade_flags &= ~2;
		}
		if (mus_fade_flags & 1) {
			vol = music_channel->volumeRight;
			if (vol < mus_vol_right) {
				vol += 8;
				if (vol > mus_vol_right)
					vol = mus_vol_right;
			} else {
				vol -= 8;
				if (vol < mus_vol_right)
					vol = mus_vol_right;
			}
			CLSoundChannel_SetVolumeRight(music_channel, vol);
			if (vol == mus_vol_right)
				mus_fade_flags &= ~1;
		}
	}
	void fademusica0(short delay) {
		short volume;
		while ((volume = CLSoundChannel_GetVolume(music_channel)) > 2) {
			volume -= 2;
			if (volume < 2)
				volume = 2;
			CLSoundChannel_SetVolume(music_channel, volume);
			wait(delay);
		}
	}
//// obj.c
	object_t *getobjaddr(short id) {
		int i;
		for (i = 0; i < MAX_OBJECTS; i++)
			if (objects[i].id == id)
				break;
		return objects + i;
	}
	void countobjects() {
		short index = 0;
		unsigned char total = 0;
		int i;
		for (i = 0; i < MAX_OBJECTS; i++) {
			short count = objects[i].count;
#ifdef EDEN_DEBUG
			count = 1;
			goto show_all_objects;  //DEBUG
#endif
			if (count == 0)
				continue;
			if (objects[i].flags & ObjectFlags::ofInHands)
				count--;
show_all_objects:
			;
			if (count) {
				total += count;
				while (count--)
					own_objects[index++] = objects[i].id;
			}
		}
		p_global->obj_count = total;
	}
	void showobjects() {
		short i, total, index;
		icon_t *icon = &gameIcons[19];
		p_global->drawFlags &= ~(DrawFlags::drDrawInventory | DrawFlags::drDrawFlag2);
		countobjects();
		total = p_global->obj_count;
		for (i = 9; i--; icon++) {
			if (total) {
				icon->cursor_id &= ~0x8000;
				total--;
			} else
				icon->cursor_id |= 0x8000;
		}
		use_main_bank();
		noclipax(55, 0, 176);
		icon = &gameIcons[19];
		total = p_global->obj_count;
		index = p_global->inventoryScrollPos;
		for (i = 9; total-- && i--; icon++) {
			char obj = own_objects[index++];
			icon->object_id = obj;
			noclipax(obj + 9, icon->sx, 178);
		}
		needPaletteUpdate = 1;
		if ((p_global->displayFlags & DisplayFlags::dfMirror) || (p_global->displayFlags & DisplayFlags::dfPanable)) {
			sauvefrisesbas();
			scroll();
		}
	}
	void winobject(short id) {
		object_t *object = getobjaddr(id);
		object->flags |= ObjectFlags::ofFlag1;
		object->count++;
		p_global->curItemsMask |= object->itemMask;
		p_global->wonItemsMask |= object->itemMask;
		p_global->curPowersMask |= object->powerMask;
		p_global->wonPowersMask |= object->powerMask;
	}
	void loseobject(short id) {
		object_t *object = getobjaddr(id);
		if (object->count > 0)
			object->count--;
		if (!object->count) {
			object->flags &= ~ObjectFlags::ofFlag1;
			p_global->curItemsMask &= ~object->itemMask;
			p_global->curPowersMask &= ~object->powerMask;
		}
		p_global->curObjectId = 0;
		p_global->curObjectFlags = 0;
		p_global->curObjectCursor = 9;
		gameIcons[16].cursor_id |= 0x8000;
		object->flags &= ~ObjectFlags::ofInHands;
		normalCursor = 1;
		current_cursor = 0;
		torchCursor = 0;
	}
	void lostobject() {
		byte_30AFE = 1;
		if (p_global->curObjectId)
			loseobject(p_global->curObjectId);
	}
	char objecthere(short id) {
		object_t *object = getobjaddr(id);
		for (pCurrentObjectLocation = &kObjectLocations[object->locations]; *pCurrentObjectLocation != -1; pCurrentObjectLocation++) {
			if (*pCurrentObjectLocation == p_global->roomNum)
				return 1;
		}
		return 0;
	}
	void objectmain(short id) {
		object_t *object = getobjaddr(id);
		gameIcons[16].cursor_id &= ~0x8000;
		p_global->curObjectId = object->id;
		p_global->curObjectCursor = p_global->curObjectId + 9;
		object->flags |= ObjectFlags::ofInHands;
		p_global->curObjectFlags = object->flags;
		current_cursor = p_global->curObjectId + 9;
		normalCursor = 0;
	}
	void getobject(short id) {
		room_t *room = p_global->room_ptr;
		if (p_global->curObjectId)
			return;
		if (!objecthere(id))
			return;
		*pCurrentObjectLocation |= 0x8000;
		objectmain(id);
		winobject(id);
		showobjects();
		p_global->roomImgBank = room->bank;
		p_global->roomVidNum = room->video;
		aflieu();
	}
	void putobject() {
		object_t *object;
		if (!p_global->curObjectId)
			return;
		gameIcons[16].cursor_id |= 0x8000;
		object = getobjaddr(p_global->curObjectId);
		p_global->curObjectCursor = 9;
		p_global->curObjectId = 0;
		p_global->curObjectFlags = 0;
		object->flags &= ~ObjectFlags::ofInHands;
		p_global->next_dialog_ptr = 0;
		byte_30B00 = 0;
		p_global->dialogType = DialogType::dtTalk;
		showobjects();
		normalCursor = 1;
	}
	void newobject(short id, short arg2) {
		object_t *object = getobjaddr(id);
		short e, *t = &kObjectLocations[object->locations];
		while ((e = *t) != -1) {
			e &= ~0x8000;
			if ((e >> 8) == arg2)
				*t = e;
			t++;
		}
	}
	void giveobjectal(short id) {
		if (id == Objects::obKnife)
			kObjectLocations[2] = 0;
		if (id == Objects::obApple)
			p_global->stepsToFindAppleNormal = 0;
		if (id >= Objects::obEyeInTheStorm && id < (Objects::obRiverThatWinds + 1) && p_global->roomPersoType == PersonFlags::pftVelociraptor) {
			//TODO: fix that cond above
			object_t *object = getobjaddr(id);
			p_global->room_perso->powers &= ~object->powerMask;
		}
		winobject(id);
	}
	void giveobject() {
		unsigned char id;
		id = p_global->giveobj1;
		if (id) {
			p_global->giveobj1 = 0;
			giveobjectal(id);
		}
		id = p_global->giveobj2;
		if (id) {
			p_global->giveobj2 = 0;
			giveobjectal(id);
		}
		id = p_global->giveobj3;
		if (id) {
			p_global->giveobj3 = 0;
			giveobjectal(id);
		}
	}
	void takeobject() {
		objectmain(current_spot2->object_id);
		p_global->next_dialog_ptr = 0;
		byte_30B00 = 0;
		p_global->dialogType = 0;
		if (p_global->inventoryScrollPos)
			p_global->inventoryScrollPos--;
		showobjects();
	}
////
	void newchampi() {
		if (objects[Objects::obShroom - 1].count == 0) {
			newobject(Objects::obShroom, p_global->cita_area_num);
			newobject(Objects::obBadShroom, p_global->cita_area_num);
		}
	}
	void newnidv() {
		short *ptr;
		object_t *obj;
		room_t *room = p_global->cita_area_firstRoom;
		if (objects[Objects::obNest - 1].count)
			return;
		obj = getobjaddr(Objects::obNest);
		for (ptr = kObjectLocations + obj->locations; *ptr != -1; ptr++) {
			if ((*ptr & ~0x8000) >> 8 != p_global->cita_area_num)
				continue;
			*ptr &= ~0x8000;
			for (; room->ff_0 != 0xFF; room++) {
				if (room->location == (*ptr & 0xFF)) {
					room->bank = 279;
					room->ff_0 = 9;
					room++;
					room->bank = 280;
					return;
				}
			}
		}
	}
	void newnido() {
		short *ptr;
		object_t *obj;
		room_t *room = p_global->cita_area_firstRoom;
		if (objects[Objects::obFullNest - 1].count)
			return;
		if (objects[Objects::obNest - 1].count)
			return;
		obj = getobjaddr(Objects::obFullNest);
		for (ptr = kObjectLocations + obj->locations; *ptr != -1; ptr++) {
			if ((*ptr & ~0x8000) >> 8 != p_global->cita_area_num)
				continue;
			*ptr &= ~0x8000;
			for (; room->ff_0 != 0xFF; room++) {
				if (room->location == (*ptr & 0xFF)) {
					room->bank = 277;
					room->ff_0 = 9;
					room++;
					room->bank = 278;
					return;
				}
			}
		}
	}
	void newor() {
		if (objects[Objects::obGold - 1].count == 0) {
			newobject(Objects::obGold, p_global->cita_area_num);
		}
	}
	void gotopanel() {
		if (pomme_q)
			byte_31D64 = p_global->autoDialog == 0xFF;  //TODO: check me
		no_palette = 0;
		p_global->iconsIndex = 85;
		p_global->perso_ptr = 0;
		p_global->drawFlags |= DrawFlags::drDrawMenu;
		p_global->displayFlags = DisplayFlags::dfFlag2;
		p_global->menuFlags = 0;
		affpanel();
		fadetoblack(3);
		afftoppano();
		CLBlitter_CopyView2Screen(p_mainview);
		CLPalette_Send2Screen(global_palette, 0, 256);
		curs_x = 320 / 2;
		curs_y = 200 / 2;
		CLMouse_SetPosition(mouse_x_center, mouse_y_center);
	}
	void noclicpanel() {
		unsigned char num;
		if (p_global->menuFlags & MenuFlags::mfFlag4) {
			depcurstape();
			return;
		}
		if (p_global->drawFlags & DrawFlags::drDrawFlag8)
			return;
		if (p_global->menuFlags & MenuFlags::mfFlag1) {
			changervol();
			return;
		}
		if (current_spot2 >= &gameIcons[119]) {
			debug("noclic: objid = %4X, glob3,2 = %2X %2X", current_spot2, p_global->menuItemIdHi, p_global->menuItemIdLo);
			if (current_spot2->object_id == (p_global->menuItemIdLo + p_global->menuItemIdHi) << 8) //TODO: check me
				return;
		} else {
			int idx = current_spot2 - &gameIcons[105];
			if (idx == 0) {
				p_global->menuItemIdLo = 1;
				num = 1;
				goto skip;
			}
			num = idx & 0x7F + 1;
			if (num >= 5)
				num = 1;
			if (num == p_global->ff_43)
				return;
			p_global->ff_43 = 0;
		}
		num = p_global->menuItemIdLo;
		p_global->menuItemIdLo = current_spot2->object_id & 0xFF;
skip:
		;
		p_global->menuItemIdHi = (current_spot2->object_id & 0xFF00) >> 8;
		debug("noclic: new glob3,2 = %2X %2X", p_global->menuItemIdHi, p_global->menuItemIdLo);
		affresult();
		num &= 0xF0;
		if (num != 0x30)
			num = p_global->menuItemIdLo & 0xF0;
		if (num == 0x30)
			affcurseurs();
	}
	void generique() {
		int oldmusic;
		blackbars();
		afficher();
		fadetoblack(3);
		ClearScreen();
		oldmusic = p_global->currentMusicNum;
		playhnm(95);
		affpanel();
		afftoppano();
		needPaletteUpdate = 1;
		startmusique(oldmusic);
	}
	void cancel2() {
		afftopscr();
		showobjects();
		p_global->iconsIndex = 16;
		p_global->drawFlags &= ~DrawFlags::drDrawMenu;
		gametomiroir(1);
	}
	void testvoice() {
		char res;   //TODO: useless?
		short num;
		p_global->fresqNumber = 0;
		p_global->perso_ptr = kPersons;
		p_global->dialogType = DialogType::dtInspect;
		num = (kPersons[0].id << 3) | p_global->dialogType;
		res = dialoscansvmas((dial_t *)GetElem(gameDialogs, num));
		restaurefondbulle();
		af_subtitle();
		persovox();
		waitendspeak();
		endpersovox();
		p_global->ff_CA = 0;
		p_global->dialogType = DialogType::dtTalk;
	}
	void load() {
		char name[132];
		unsigned char oldMusic, talk;
		gameLoaded = 0;
		oldMusic = p_global->currentMusicNum;   //TODO: from ush to byte?!
		fademusica0(1);
		desktopcolors();
		FlushEvents(-1, 0);
//	if(OpenDialog(0, 0)) //TODO: write me
		{
			// TODO
			strcpy(name, "edsave1.000");
			loadgame(name);
		}
		CLMouse_Hide();
		CLBlitter_FillScreenView(-1);
		fadetoblack(3);
		CLBlitter_FillScreenView(0);
		if (!gameLoaded) {
			mus_fade_flags = 3;
			musicspy();
			needPaletteUpdate = 1;
			return;
		}
		if ((oldMusic & 0xFF) != p_global->currentMusicNum) { //TODO: r30 is uns char/bug???
			oldMusic = p_global->currentMusicNum;
			p_global->currentMusicNum = 0;
			startmusique(oldMusic);
		} else {
			mus_fade_flags = 3;
			musicspy();
		}
		talk = p_global->autoDialog == 0xFF;    //TODO check me
		initafterload();
		fadetoblack(3);
		CLBlitter_FillScreenView(0);
		CLBlitter_FillView(p_mainview, 0);
		afftopscr();
		p_global->inventoryScrollPos = 0;
		showobjects();
		maj_salle(p_global->roomNum);
		if (talk) {
			p_global->iconsIndex = 4;
			p_global->autoDialog = -1;
			parle_moi();
		}

	}
	void initafterload() {
		p_global->perso_img_bank = 0;
		p_global->lastSalNum = 0;
		loadsal(p_global->area_ptr->salNum);
		gameIcons[18].cursor_id |= 0x8000;
		if (p_global->curAreaType == AreaType::atValley)
			gameIcons[18].cursor_id &= ~0x8000;
		kPersoRoomBankTable[30] = 27;
		if (p_global->phaseNum >= 352)
			kPersoRoomBankTable[30] = 26;
		animateTalking = 0;
		animationActive = 0;
		p_global->ff_100 = 0;
		p_global->eventType = EventType::etEventC;
		p_global->valleyVidNum = 0;
		p_global->drawFlags &= ~DrawFlags::drDrawMenu;
		currentTime = TimerTicks / 100;
		p_global->gameTime = currentTime;
		if (p_global->roomPersoType == PersonFlags::pftTyrann)
			chronoon(3000);
		saved_repadam.x = -1;
		saved_repadam.y = -1;
	}
	void save() {
		char name[260];
		fademusica0(1);
		desktopcolors();
		FlushEvents(-1, 0);
		//SaveDialog(byte_37150, byte_37196->ff_A);
		//TODO
		strcpy(name, "edsave1.000");
		savegame(name);
		CLMouse_Hide();
		CLBlitter_FillScreenView(0xFFFFFFFF);
		fadetoblack(3);
		CLBlitter_FillScreenView(0);
		mus_fade_flags = 3;
		musicspy();
		needPaletteUpdate = 1;
	}
	void desktopcolors() {
		fadetoblack(3);
		CLBlitter_FillScreenView(-1);
		CLPalette_BeSystem();
		CLMouse_Show();
	}
	void panelrestart() {
		unsigned char curmus, curlng;
		gameLoaded = 0;
		curmus = p_global->currentMusicNum;
		curlng = p_global->pref_language;
		loadrestart();
		p_global->pref_language = curlng;
		if (!gameLoaded) //TODO always?
			return;
		p_global->perso_img_bank = 0;
		p_global->lastSalNum = 0;
		loadsal(p_global->area_ptr->salNum);
		p_global->displayFlags = DisplayFlags::dfFlag1;
		gameIcons[18].cursor_id |= 0x8000;
		if (p_global->curAreaType == AreaType::atValley)
			gameIcons[18].cursor_id &= ~0x8000;
		kPersoRoomBankTable[30] = 27;
		if (p_global->phaseNum >= 352)
			kPersoRoomBankTable[30] = 26;
		animateTalking = 0;
		animationActive = 0;
		p_global->ff_100 = 0;
		p_global->eventType = 0;
		p_global->valleyVidNum = 0;
		p_global->drawFlags &= ~DrawFlags::drDrawMenu;
		p_global->inventoryScrollPos = 0;
		saved_repadam.x = -1;
		saved_repadam.y = -1;
		if (curmus != p_global->currentMusicNum) {
			curmus = p_global->currentMusicNum;
			p_global->currentMusicNum = 0;
			startmusique(curmus);
		}
		fadetoblack(3);
		CLBlitter_FillScreenView(0);
		CLBlitter_FillView(p_mainview, 0);
		afftopscr();
		showobjects();
		sauvefrises();
		showBlackBars = 1;
		maj_salle(p_global->roomNum);
	}
	void reallyquit() {
		quit_flag3 = 1; //TODO: byte
		quit_flag2 = 1;
	}
	void confirmer(char mode, char yesId) {
		p_global->iconsIndex = 119;
		gameIcons[119].object_id = yesId;
		confirmMode = mode;
		use_bank(65);
		noclipax(12, 117, 74);
		curs_x = 156;
		if (pomme_q)
			curs_x = 136;
		curs_y = 88;
	}
	void confirmyes() {
		affpanel();
		p_global->iconsIndex = 85;
		switch (confirmMode) {
		case 1:
			panelrestart();
			break;
		case 2:
			reallyquit();
			break;
		}
	}
	void confirmno() {
		affpanel();
		p_global->iconsIndex = 85;
		pomme_q = 0;
	}
	void restart() {
		confirmer(1, current_spot2->object_id);
	}
	void EdenQuit() {
		confirmer(2, current_spot2->object_id);
	}
	void choixsubtitle() {
		unsigned char lang = current_spot2->object_id & 0xF;
		if (lang == p_global->pref_language)
			return;
		if (lang > 5)
			return;
		p_global->pref_language = lang;
		langbuftopanel();
		afflangue();
	}
	void reglervol() {
		unsigned char *valptr = &p_global->pref_10C[current_spot2->object_id & 7];
		curs_y = 104 - ((*valptr >> 2) & 0x3F); // TODO: check me
		cur_slider_value_ptr = valptr;
		p_global->menuFlags |= MenuFlags::mfFlag1;
		if (current_spot2->object_id & 8)
			p_global->menuFlags |= MenuFlags::mfFlag2;
		cur_slider_x = current_spot2->sx;
		cur_slider_y = curs_y;
	}
	void changervol() {
		short delta;
		if (mouse_held) {
			limitezonecurs(cur_slider_x - 1, cur_slider_x + 3, 40, 110);
			delta = cur_slider_y - curs_y;
			if (delta == 0)
				return;
			newvol(cur_slider_value_ptr, delta);
			if (p_global->menuFlags & MenuFlags::mfFlag2)
				newvol(cur_slider_value_ptr + 1, delta);
			cursbuftopanel();
			affcurseurs();
			cur_slider_y = curs_y;
		} else
			p_global->menuFlags &= ~(MenuFlags::mfFlag1 | MenuFlags::mfFlag2);
	}
	void newvol(unsigned char *volptr, short delta) {
		short vol = *volptr / 4;
		vol += delta;
		if (vol < 0)
			vol = 0;
		if (vol > 63)
			vol = 63;
		*volptr = vol * 4;
		CLSoundChannel_SetVolumeLeft(music_channel, p_global->pref_10C[0]); //TODO: this val only?
		CLSoundChannel_SetVolumeRight(music_channel, p_global->pref_10C[1]);
	}
	void playtape() {
		unsigned short oldRoomNum, oldParty;
		unsigned char oldBack;
		perso_t *oldPerso;
		if (p_global->menuItemIdHi & 8)
			p_global->tape_ptr++;
		for (;; p_global->tape_ptr++) {
			if (p_global->tape_ptr == &tapes[MAX_TAPES]) {
				p_global->tape_ptr--;
				stoptape();
				return;
			}
			if (p_global->tape_ptr->textNum)
				break;
		}
		p_global->menuFlags |= MenuFlags::mfFlag8;
		p_global->drawFlags &= ~DrawFlags::drDrawMenu;
		oldRoomNum = p_global->roomNum;
		oldParty = p_global->party;
		oldBack = p_global->roomBgBankNum;
		oldPerso = p_global->perso_ptr;
		p_global->party = p_global->tape_ptr->party;
		p_global->roomNum = p_global->tape_ptr->roomNum;
		p_global->roomBgBankNum = p_global->tape_ptr->bgBankNum;
		p_global->dialog_ptr = p_global->tape_ptr->dialog;
		p_global->perso_ptr = p_global->tape_ptr->perso;
		endpersovox();
		affcurstape();
		if (p_global->perso_ptr != oldPerso
		        || p_global->roomNum != lastTapeRoomNum) {
			lastTapeRoomNum = p_global->roomNum;
			p_global->curPersoAnimPtr = 0;
			p_global->ff_CA = 0;
			p_global->perso_img_bank = -1;
			anim_perfin();
			load_perso_cour();
		}
		af_fondperso();
		p_global->textNum = p_global->tape_ptr->textNum;
		my_bulle();
		getdatasync();
		showpersopanel();
		persovox();
		p_global->roomBgBankNum = oldBack;
		p_global->party = oldParty;
		p_global->roomNum = oldRoomNum;
	}
	void rewindtape() {
		if (p_global->tape_ptr > tapes) {
			p_global->tape_ptr--;
			p_global->menuFlags &= ~MenuFlags::mfFlag8;
			affcurstape();
		}
	}
	void depcurstape() {
		int idx;
		tape_t *tape;
		if (mouse_held) {
			limitezonecurs(95, 217, 179, 183);
			idx = (curs_x - 97);
			if (idx < 0) idx = 0;
			idx /= 8;
			tape = tapes + idx;
			if (tape >= tapes + 16) tape = tapes + 16 - 1;
			if (tape != p_global->tape_ptr) {
				p_global->tape_ptr = tape;
				affcurstape();
				p_global->menuFlags &= ~MenuFlags::mfFlag8;
			}
		} else
			p_global->menuFlags &= ~MenuFlags::mfFlag4;
	}
	void affcurstape() {
		int x;
		if (p_global->drawFlags & DrawFlags::drDrawFlag8)
			no_palette = 1;
		use_bank(65);
		noclipax(2, 0, 176);
		x = (p_global->tape_ptr - tapes) * 8 + 97;
		gameIcons[112].sx = x - 3;
		gameIcons[112].ex = x + 3;
		noclipax(5, x, 179);
		no_palette = 0;
	}
	void forwardtape() {
		if (p_global->tape_ptr < tapes + 16) {
			p_global->tape_ptr++;
			p_global->menuFlags &= ~MenuFlags::mfFlag8;
			affcurstape();
		}
	}
	void stoptape() {
		if (!(p_global->drawFlags & DrawFlags::drDrawFlag8))
			return;
		p_global->menuFlags &= ~MenuFlags::mfFlag8;
		p_global->drawFlags &= ~DrawFlags::drDrawFlag8;
		p_global->menuFlags |= MenuFlags::mfFlag10;
		p_global->iconsIndex = 85;
		p_global->perso_ptr = 0;
		lastTapeRoomNum = 0;
		endpersovox();
		fin_perso();
		affpanel();
		afftoppano();
		needPaletteUpdate = 1;
	}
	void cliccurstape() {
		p_global->menuFlags |= MenuFlags::mfFlag4;
	}
	void paneltobuf() {
		setRS1(0, 16, 320 - 1, 169 - 1);
		setRD1(320, 16, 640 - 1, 169 - 1);
		CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
	}
	void cursbuftopanel() {
		setRS1(434, 40, 525 - 1, 111 - 1);
		setRD1(114, 40, 205 - 1, 111 - 1);
		CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
	}
	void langbuftopanel() {
		setRS1(328, 42, 407 - 1, 97 - 1);
		setRD1(8, 42,  87 - 1, 97 - 1);
		CLBlitter_CopyViewRect(p_mainview, p_mainview, &rect_src, &rect_dst);
	}
	void affpanel() {
		use_bank(65);
		noclipax(0, 0, 16);
		paneltobuf();
		afflangue();
		affcurseurs();
		affcurstape();
	}
	void afflangue() {
		use_bank(65);
		if (p_global->pref_language < 0 //TODO: never happens
		        || p_global->pref_language > 5)
			return;
		noclipax(6,  8, p_global->pref_language * 9 + 43);  //TODO: * FONT_HEIGHT
		noclipax(7, 77, p_global->pref_language * 9 + 44);
	}
	void affcursvol(short x, short vol1, short vol2) {
		short slider = 3;
		if (lastMenuItemIdLo && (lastMenuItemIdLo & 9) != 1) //TODO check me
			slider = 4;
		noclipax(slider, x, 104 - vol1);
		slider = 3;
		if ((lastMenuItemIdLo & 9) != 0)
			slider = 4;
		noclipax(slider, x + 12, 104 - vol2);
	}
	void affcurseurs() {
		use_bank(65);
		if (p_global->drawFlags & DrawFlags::drDrawFlag8)
			return;
		curseurselect(48);
		affcursvol(114, p_global->pref_10C[0] / 4, p_global->pref_10C[1] / 4);
		curseurselect(50);
		affcursvol(147, p_global->pref_10E[0] / 4, p_global->pref_10E[1] / 4);
		curseurselect(52);
		affcursvol(179, p_global->pref_110[0] / 4, p_global->pref_110[1] / 4);
	}
	void curseurselect(int itemId) {
		lastMenuItemIdLo = p_global->menuItemIdLo;
		if ((lastMenuItemIdLo & ~9) != itemId)
			lastMenuItemIdLo = 0;
	}
	void afftoppano() {
		noclipax(1, 0, 0);
	}
	void affresult() {
		short num;
		restaurefondbulle();
		p_global->perso_ptr = &kPersons[19];
		p_global->dialogType = DialogType::dtInspect;
		num = (kPersons[19].id << 3) | p_global->dialogType;
		if (dialoscansvmas((dial_t *)GetElem(gameDialogs, num)))
			af_subtitle();
		p_global->ff_CA = 0;
		p_global->dialogType = DialogType::dtTalk;
		p_global->perso_ptr = 0;
	}
	void limitezonecurs(short xmin, short xmax, short ymin, short ymax) {
		if (curs_x < xmin) curs_x = xmin;
		if (curs_x > xmax) curs_x = xmax;
		if (curs_y < ymin) curs_y = ymin;
		if (curs_y > ymax) curs_y = ymax;
	}
	void PommeQ() {
		icon_t *icon = &gameIcons[85];
		if (p_global->displayFlags & DisplayFlags::dfFresques) {
			torchCursor = 0;
			curs_saved = 1;
			if (p_global->displayFlags & DisplayFlags::dfPerson)
				close_perso();
			p_global->displayFlags = DisplayFlags::dfFlag1;
			resetscroll();
			p_global->ff_100 = -1;
			maj_salle(p_global->roomNum);
		}
		if (p_global->displayFlags & DisplayFlags::dfPerson)
			close_perso();
		if (p_global->displayFlags & DisplayFlags::dfPanable)
			resetscroll();
		if (p_global->displayFlags & DisplayFlags::dfMirror)
			resetscroll();
		if (p_global->drawFlags & DrawFlags::drDrawFlag8)
			stoptape();
		if (personTalking)
			endpersovox();
		p_global->ff_103 = 0;
		p_global->ff_102 = 0;
		putobject();
		current_cursor = 53;
		if (p_global->displayFlags != DisplayFlags::dfFlag2)
			gotopanel();
		current_spot2 = icon + 7;   //TODO
		EdenQuit();
	}
	void habitants(perso_t *perso) {
		char persType = perso->flags & PersonFlags::pfTypeMask; //TODO rename
		if (persType && persType != PersonFlags::pfType2) {
			p_global->room_perso = perso;
			p_global->roomPersoType = persType;
			p_global->roomPersoFlags = perso->flags;
			p_global->roomPersoItems = perso->items;
			p_global->roomPersoPowers = perso->powers;
			p_global->partyOutside |= perso->party;
			if (p_global->roomPersoType == PersonFlags::pftTriceraptor)
				delinfo(p_global->areaNum + ValleyNews::vnTriceraptorsIn);
			else if (p_global->roomPersoType == PersonFlags::pftVelociraptor)
				delinfo(p_global->areaNum + ValleyNews::vnVelociraptorsIn);
		} else if (!(perso->flags & PersonFlags::pfInParty))
			p_global->partyOutside |= perso->party;
	}
	void suiveurs(perso_t *perso) {
		char persType = perso->flags & PersonFlags::pfTypeMask;
		if (persType == 0 || persType == PersonFlags::pfType2) {
			if (perso->flags & PersonFlags::pfInParty)
				p_global->party |= perso->party;
		}
	}
	void evenements(perso_t *perso) {
		if (p_global->ff_113)
			return;
		if (perso >= &kPersons[PER_UNKN_18C])
			return;
		if (!dialo_even(perso))
			return;
		p_global->ff_113++;
		p_global->oldDisplayFlags = 1;
		perso = p_global->perso_ptr;
		init_perso_ptr(perso);
		if (!(perso->party & PersonMask::pmLeader))
			p_global->ff_60 = -1;
		p_global->eventType = 0;
	}
	void followme(perso_t *perso) {
		if (perso->flags & PersonFlags::pfTypeMask)
			return;
		if (perso->flags & PersonFlags::pfInParty)
			perso->roomNum = destinationRoom;
	}
	void rangermammi(perso_t *perso, room_t *room) {
		room_t *found_room;
		if (!(perso->party & PersonMask::pmLeader))
			return;
		for (; room->ff_0 != 0xFF; room++) {
			if (room->flags & RoomFlags::rfHasCitadel) {
				found_room = room;
				break;
			}
			if (room->party != 0xFFFF && (room->party & PersonMask::pmLeader))
				found_room = room;  //TODO: no brk?
		}
		if (!found_room)    //TODO not zeroed?
			return;
		perso->roomNum &= ~0xFF;
		perso->roomNum |= found_room->location;
		perso->flags &= ~PersonFlags::pfInParty;
		p_global->party &= ~perso->party;
	}
	void perso_ici(short action) {
		perso_t *perso = &kPersons[PER_UNKN_156];
//	room_t *room = p_global->last_area_ptr->room_ptr;    //TODO: compiler opt bug? causes access to zero ptr??? last_area_ptr == 0
		switch (action) {
		case 0:
			suiveurs(perso);
			break;
		case 1:
			habitants(perso);
			break;
		case 3:
			evenements(perso);
			break;
		case 4:
			followme(perso);
			break;
		case 5:
			rangermammi(perso, p_global->last_area_ptr->citadelRoom);
			break;
		}
		perso = kPersons;
		do {
			if (perso->roomNum == p_global->roomNum && !(perso->flags & PersonFlags::pf80)) {
				switch (action) {
				case 0:
					suiveurs(perso);
					break;
				case 1:
					habitants(perso);
					break;
				case 3:
					evenements(perso);
					break;
				case 4:
					followme(perso);
					break;
				case 5:
					rangermammi(perso, p_global->last_area_ptr->citadelRoom);
					break;
				}
			}
			perso++;
		} while (perso->roomNum != 0xFFFF);
	}
	void setpersohere() {
		debug("setpersohere, perso is %d", p_global->perso_ptr - kPersons);
		p_global->partyOutside = 0;
		p_global->party = 0;
		p_global->room_perso = 0;
		p_global->roomPersoType = 0;
		p_global->roomPersoFlags = 0;
		perso_ici(1);
		perso_ici(0);
		if (p_global->roomPersoType == PersonFlags::pftTyrann) delinfo(p_global->areaNum + ValleyNews::vnTyrannIn);
		if (p_global->roomPersoType == PersonFlags::pftTriceraptor) delinfo(p_global->areaNum + ValleyNews::vnTriceraptorsIn);
		if (p_global->roomPersoType == PersonFlags::pftVelociraptor) {
			delinfo(p_global->areaNum + ValleyNews::vnTyrannIn);
			delinfo(p_global->areaNum + ValleyNews::vnTyrannLost);
			delinfo(p_global->areaNum + ValleyNews::vnVelociraptorsLost);
		}
	}
	void faire_suivre(short roomNum) {
		destinationRoom = roomNum;
		perso_ici(4);
	}
	void suis_moi5() {
		debug("adding person %d to party", p_global->perso_ptr - kPersons);
		p_global->perso_ptr->flags |= PersonFlags::pfInParty;
		p_global->perso_ptr->roomNum = p_global->roomNum;
		p_global->party |= p_global->perso_ptr->party;
		p_global->drawFlags |= DrawFlags::drDrawTopScreen;
	}
	void suis_moi(short index) {
		perso_t *old_perso = p_global->perso_ptr;
		p_global->perso_ptr = &kPersons[index];
		suis_moi5();
		p_global->perso_ptr = old_perso;
	}
	void reste_ici5() {
		debug("removing person %d from party", p_global->perso_ptr - kPersons);
		p_global->perso_ptr->flags &= ~PersonFlags::pfInParty;
		p_global->partyOutside |= p_global->perso_ptr->party;
		p_global->party &= ~p_global->perso_ptr->party;
		p_global->drawFlags |= DrawFlags::drDrawTopScreen;
	}
	void reste_ici(short index) {
		perso_t *old_perso = p_global->perso_ptr;
		p_global->perso_ptr = &kPersons[index];
		reste_ici5();
		p_global->perso_ptr = old_perso;
	}
	void eloipart() {
		reste_ici(5);
		p_global->gameFlags &= ~GameFlags::gfFlag4000;
		kPersons[PER_MESSAGER].roomNum = 0;
		p_global->partyOutside &= ~kPersons[PER_MESSAGER].party;
		if (p_global->roomNum == 2817)
			chronoon(3000);
		p_global->eloiDepartureDay = p_global->gameDays;
		p_global->eloiHaveNews = 0;
		unlockinfo();
	}
	char eloirevientq() {
		if (p_global->phaseNum < 304)
			return 1;
		if (p_global->phaseNum <= 353)
			return 0;
		if (p_global->phaseNum == 370)
			return 0;
		if (p_global->phaseNum == 384)
			return 0;
		if (p_global->areaNum != Areas::arShandovra)
			return 1;
		if (p_global->phaseNum < 480)
			return 0;
		return 1;
	}
	void eloirevient() {
		if (p_global->area_ptr->type == AreaType::atValley && !kPersons[PER_MESSAGER].roomNum)
			kPersons[PER_MESSAGER].roomNum = (p_global->roomNum & 0xFF00) + 1;
	}
//// phase.c
	void incphase1() {
		static phase_t phases[] = {
			{ 65, &EdenGameImpl::dialautoon },
			{ 113, &EdenGameImpl::phase113 },
			{ 129, &EdenGameImpl::dialautoon },
			{ 130, &EdenGameImpl::phase130 },
			{ 161, &EdenGameImpl::phase161 },
			{ 211, &EdenGameImpl::dialautoon },
			{ 226, &EdenGameImpl::phase226 },
			{ 257, &EdenGameImpl::phase257 },
			{ 353, &EdenGameImpl::phase353 },
			{ 369, &EdenGameImpl::phase369 },
			{ 371, &EdenGameImpl::phase371 },
			{ 385, &EdenGameImpl::phase385 },
			{ 386, &EdenGameImpl::dialonfollow },
			{ 418, &EdenGameImpl::phase418 },
			{ 433, &EdenGameImpl::phase433 },
			{ 434, &EdenGameImpl::phase434 },
			{ 449, &EdenGameImpl::dialautoon },
			{ 497, &EdenGameImpl::dialautoon },
			{ 513, &EdenGameImpl::phase513 },
			{ 514, &EdenGameImpl::phase514 },
			{ 529, &EdenGameImpl::phase529 },
			{ 545, &EdenGameImpl::phase545 },
			{ 561, &EdenGameImpl::phase561 },
			{ -1, nullptr }
		};

		phase_t *phase = phases;
		p_global->phaseNum++;
		debug("!!! next phase - %4X , room %4X", p_global->phaseNum, p_global->roomNum);
		p_global->phaseActionsCount = 0;
		for (; phase->id != -1; phase++) {
			if (p_global->phaseNum == phase->id) {
				(this->*phase->disp)();
				break;
			}
		}
	}
	void incphase() {
		incphase1();
	}
	void phase113() {
		reste_ici(1);
		kPersons[PER_DINA].roomNum = 274;
	}
	void phase130() {
		dialautoon();
		reste_ici(3);
	}
	void phase161() {
		area_t *area = p_global->area_ptr;
		suis_moi(9);
		kPersons[PER_MAMMI].flags |= PersonFlags::pf10;
		area->flags |= AreaFlags::afFlag1;
		p_global->curAreaFlags |= AreaFlags::afFlag1;
	}
	void phase226() {
		newobject(16, 3);
		newobject(16, 4);
		newobject(16, 5);
	}
	void phase257() {
		gameIcons[127].cursor_id &= ~0x8000;
		p_global->persoBackgroundBankIdx = 58;
		dialautooff();
	}
	void phase353() {
		reste_ici(1);
		kPersons[PER_DINA].roomNum = 0;
		kTabletView[1] = 88;
	}
	void phase369() {
		suis_moi(5);
		p_global->narratorSequence = 2;
		gameRooms[334].exits[0] = 134;
		gameRooms[335].exits[0] = 134;
	}
	void phase371() {
		eloirevient();
		gameIcons[128].cursor_id &= ~0x8000;
		gameIcons[129].cursor_id &= ~0x8000;
		gameIcons[127].cursor_id |= 0x8000;
		p_global->persoBackgroundBankIdx = 59;
		gameRooms[334].exits[0] = -1;
		gameRooms[335].exits[0] = -1;
		gameIcons[123].object_id = 9;
		gameIcons[124].object_id = 26;
		gameIcons[125].object_id = 42;
		gameIcons[126].object_id = 56;
	}
	void phase385() {
		dialautooff();
		eloirevient();
		p_global->next_info_idx = 0;
		p_global->last_info_idx = 0;
		updateinfolist();
		p_global->last_info = 0;
	}
	void phase418() {
		loseobject(Objects::obHorn);
		dialautoon();
		suis_moi(4);
	}
	void phase433() {
		dialautoon();
		kPersons[PER_MAMMI_4].flags &= ~PersonFlags::pf80;
		kPersons[PER_BOURREAU].flags &= ~PersonFlags::pf80;
		setpersohere();
		p_global->chrono_on = 0;
		p_global->chrono = 0;
	}
	void phase434() {
		p_global->roomNum = 275;
		gameRooms[16].bank = 44;
		gameRooms[18].bank = 44;
		gameIcons[132].cursor_id &= ~0x8000;
		p_global->persoBackgroundBankIdx = 61;
		gameRooms[118].exits[2] = -1;
		abortdial();
		gameRooms[7].bank = 322;
		reste_ici(7);
		reste_ici(3);
		reste_ici(5);
		reste_ici(18);
		reste_ici(4);
		p_global->drawFlags |= DrawFlags::drDrawTopScreen;
	}
	void phase513() {
		p_global->last_dialog_ptr = 0;
		byte_30AFE = 0;
		dialautoon();
	}
	void phase514() {
		gameRooms[123].exits[2] = 1;
	}
	void phase529() {
		gameIcons[133].cursor_id &= ~0x8000;
		p_global->persoBackgroundBankIdx = 63;
	}
	void phase545() {
	}
	void phase561() {
		p_global->narratorSequence = 10;
	}
	void bigphase1() {
		static void (EdenGameImpl::*bigphases[])() = {
			&EdenGameImpl::phase16,
			&EdenGameImpl::phase32,
			&EdenGameImpl::phase48,
			&EdenGameImpl::phase64,
			&EdenGameImpl::phase80,
			&EdenGameImpl::phase96,
			&EdenGameImpl::phase112,
			&EdenGameImpl::phase128,
			&EdenGameImpl::phase144,
			&EdenGameImpl::phase160,
			&EdenGameImpl::phase176,
			&EdenGameImpl::phase192,
			&EdenGameImpl::phase208,
			&EdenGameImpl::phase224,
			&EdenGameImpl::phase240,
			&EdenGameImpl::phase256,
			&EdenGameImpl::phase272,
			&EdenGameImpl::phase288,
			&EdenGameImpl::phase304,
			&EdenGameImpl::phase320,
			&EdenGameImpl::phase336,
			&EdenGameImpl::phase352,
			&EdenGameImpl::phase368,
			&EdenGameImpl::phase384,
			&EdenGameImpl::phase400,
			&EdenGameImpl::phase416,
			&EdenGameImpl::phase432,
			&EdenGameImpl::phase448,
			&EdenGameImpl::phase464,
			&EdenGameImpl::phase480,
			&EdenGameImpl::phase496,
			&EdenGameImpl::phase512,
			&EdenGameImpl::phase528,
			&EdenGameImpl::phase544,
			&EdenGameImpl::phase560
		};

		short   phase = (p_global->phaseNum & ~3) + 0x10;   //TODO: check me
		debug("!!! big phase - %4X", phase);
		p_global->phaseActionsCount = 0;
		p_global->phaseNum = phase;
		if (phase > 560)
			return;
		phase >>= 4;
		(this->*bigphases[phase - 1])();
	}
	void bigphase() {
		if (!(p_global->dialog_ptr->flags & DialogFlags::dfSpoken))
			bigphase1();
	}
	void phase16() {
		dialautoon();
	}
	void phase32() {
		word_31E7A &= ~0x8000;
	}
	void phase48() {
		gameRooms[8].exits[1] = 22;
		dialautoon();
	}
	void phase64() {
		suis_moi(1);
		kPersons[PER_MESSAGER].roomNum = 259;
	}
	void phase80() {
		kPersons[PER_THOO].roomNum = 0;
	}
	void phase96() {
	}
	void phase112() {
		giveobject();
	}
	void phase128() {
		suis_moi(1);
		giveobject();
	}
	void phase144() {
		suis_moi(5);
		gameRooms[113].video = 0;
		gameRooms[113].bank = 317;
	}
	void phase160() {
	}
	void phase176() {
		dialonfollow();
	}
	void phase192() {
		area_t *area = p_global->area_ptr;
		suis_moi(10);
		kPersons[PER_MAMMI_1].flags |= PersonFlags::pf10;
		dialautoon();
		area->flags |= AreaFlags::afFlag1;
		p_global->curAreaFlags |= AreaFlags::afFlag1;
	}
	void phase208() {
		eloirevient();
	}
	void phase224() {
		gameIcons[126].cursor_id &= ~0x8000;
		p_global->persoBackgroundBankIdx = 57;
		dialautooff();
	}
	void phase240() {
		area_t *area = p_global->area_ptr;
		suis_moi(11);
		kPersons[PER_MAMMI_2].flags |= PersonFlags::pf10;
		area->flags |= AreaFlags::afFlag1;
		p_global->curAreaFlags |= AreaFlags::afFlag1;
	}
	void phase256() {
		dialautoon();
	}
	void phase272() {
		dialautoon();
		p_global->eloiHaveNews = 0;
	}
	void phase288() {
		oui();
		kPersons[PER_MANGO].roomNum = 0;
		reste_ici(6);
		suis_moi(5);
		p_global->narratorSequence = 8;
	}
	void phase304() {
		area_t *area = p_global->area_ptr;
		suis_moi(7);
		suis_moi(14);
		kPersons[PER_MAMMI_5].flags |= PersonFlags::pf10;
		dialautoon();
		area->flags |= AreaFlags::afFlag1;
		p_global->curAreaFlags |= AreaFlags::afFlag1;
	}
	void phase320() {
		dialonfollow();
	}
	void phase336() {
		gameRooms[288].exits[0] = 135;
		gameRooms[289].exits[0] = 135;
		loseobject(p_global->curObjectId);
		dialautoon();
	}
	void phase352() {
		kPersoRoomBankTable[30] = 26;
		kPersons[PER_EVE].bank = 9;
		kPersons[PER_EVE].targetLoc = 8;
		suiveurs_list[13].image = 2;
		dialautoon();
		gameRooms[288].exits[0] = -1;
		gameRooms[289].exits[0] = -1;
		gameRooms[288].flags &= ~RoomFlags::rf02;
		gameRooms[289].flags &= ~RoomFlags::rf02;
	}
	void phase368() {
		reste_ici(7);
		dialautoon();
		kPersons[PER_MESSAGER].roomNum = 1811;
		kPersons[PER_DINA].roomNum = 1607;
	}
	void phase384() {
		area_t *area = p_global->area_ptr;
		suis_moi(7);
		reste_ici(1);
		dialautoon();
		area->flags |= AreaFlags::afFlag1;
		p_global->curAreaFlags |= AreaFlags::afFlag1;
		eloipart();
	}
	void phase400() {
		dialonfollow();
		kPersons[PER_ROI].roomNum = 0;
		kPersons[PER_MONK].roomNum = 259;
		p_global->eloiHaveNews = 0;
		kObjectLocations[20] = 259;
	}
	void phase416() {
		suis_moi(3);
		gameIcons[130].cursor_id &= ~0x8000;
		p_global->persoBackgroundBankIdx = 60;
		gameRooms[0].exits[0] = 138;
	}
	void phase432() {
		p_global->narratorSequence = 3;
		kPersons[PER_MAMMI_4].flags |= PersonFlags::pf80;
		kPersons[PER_BOURREAU].flags |= PersonFlags::pf80;
		kPersons[PER_MESSAGER].roomNum = 257;
		gameRooms[0].exits[0] = -1;
		p_global->drawFlags |= DrawFlags::drDrawTopScreen;
	}
	void phase448() {
		dialautoon();
		eloipart();
	}
	void phase464() {
		p_global->area_ptr->flags |= AreaFlags::afFlag1;
		p_global->curAreaFlags |= AreaFlags::afFlag1;
		kPersons[PER_MAMMI_6].flags |= PersonFlags::pf10;
		suis_moi(8);
		p_global->cita_area_num = p_global->areaNum;
		naitredino(8);
	}
	void phase480() {
		giveobject();
		newvallee();
		eloirevient();
		kTabletView[1] = 94;
	}
	void phase496() {
		dialautoon();
		p_global->last_dialog_ptr = 0;
		byte_30AFE = 0;
	}
	void phase512() {
		reste_ici(3);
		reste_ici(7);
		reste_ici(8);
		reste_ici(18);
	}
	void phase528() {
		p_global->narratorSequence = 11;
		suis_moi(3);
		suis_moi(5);
		suis_moi(7);
		suis_moi(8);
		suis_moi(18);
	}
	void phase544() {
		eloipart();
		dialautoon();
		reste_ici(8);
		reste_ici(18);
	}
	void phase560() {
		kPersons[PER_MESSAGER].roomNum = 3073;
		gameRooms[127].exits[1] = 0;
	}
//// saveload.c
	void savegame(char *name) {
//	filespec_t fs;
//	file_t handle;
		long size;
//	CLFile_MakeStruct(0, 0, name, &fs);
//	CLFile_Create(&fs);
//	CLFile_SetFinderInfos(&fs, 'EDNS', 'LEDN');
//	CLFile_Open(&fs, 3, handle);

		Common::OutSaveFile *handle = g_system->getSavefileManager()->openForSaving(name);
		if (!handle)
			return;

#define CLFile_Write(h, ptr, size) \
	debug("writing 0x%lX bytes", *size); \
	h->write(ptr, *size);

		vavaoffsetout();
		size = (char *)(&p_global->save_end) - (char *)(p_global);
		CLFile_Write(handle, p_global, &size);
		size = (char *)(&gameIcons[134]) - (char *)(&gameIcons[123]);
		CLFile_Write(handle, &gameIcons[123], &size);
		lieuoffsetout();
		size = (char *)(&kAreasTable[12]) - (char *)(&kAreasTable[0]);
		CLFile_Write(handle, &kAreasTable[0], &size);
		size = (char *)(&gameRooms[423]) - (char *)(&gameRooms[0]);
		CLFile_Write(handle, &gameRooms[0], &size);
		size = (char *)(&objects[42]) - (char *)(&objects[0]);
		CLFile_Write(handle, &objects[0], &size);
		size = (char *)(&kObjectLocations[45]) - (char *)(&kObjectLocations[0]);
		CLFile_Write(handle, &kObjectLocations[0], &size);
		size = (char *)(&suiveurs_list[14]) - (char *)(&suiveurs_list[13]);
		CLFile_Write(handle, &suiveurs_list[13], &size);
		size = (char *)(&kPersons[55]) - (char *)(&kPersons[0]);
		CLFile_Write(handle, &kPersons[0], &size);
		bandeoffsetout();
		size = (char *)(&tapes[16]) - (char *)(&tapes[0]);
		CLFile_Write(handle, &tapes[0], &size);
		size = (char *)(&kTabletView[6]) - (char *)(&kTabletView[0]);
		CLFile_Write(handle, &kTabletView[0], &size);
		size = (char *)(&gameDialogs[10240]) - (char *)(&gameDialogs[0]); //TODO: const size 10240
		CLFile_Write(handle, &gameDialogs[0], &size);

		delete handle;

#undef CLFile_Write

//	CLFile_Close(handle);

		vavaoffsetin();
		lieuoffsetin();
		bandeoffsetin();

		debug("* Game saved to %s", name);
	}
	void loadrestart() {
		assert(0);  //TODO: this won't work atm - all snapshots are BE
		long offs = 0;
		long size;
		size = (char *)(&p_global->save_end) - (char *)(p_global);
		loadpartoffile(2495, p_global, offs, size);
		offs += size;
		vavaoffsetin();
		size = (char *)(&gameIcons[134]) - (char *)(&gameIcons[123]);
		loadpartoffile(2495, &gameIcons[123], offs, size);
		offs += size;
		size = (char *)(&kAreasTable[12]) - (char *)(&kAreasTable[0]);
		loadpartoffile(2495, &kAreasTable[0], offs, size);
		offs += size;
		lieuoffsetin();
		size = (char *)(&gameRooms[423]) - (char *)(&gameRooms[0]);
		loadpartoffile(2495, &gameRooms[0], offs, size);
		offs += size;
		size = (char *)(&objects[42]) - (char *)(&objects[0]);
		loadpartoffile(2495,  &objects[0], offs, size);
		offs += size;
		size = (char *)(&kObjectLocations[45]) - (char *)(&kObjectLocations[0]);
		loadpartoffile(2495,  &kObjectLocations[0], offs, size);
		offs += size;
		size = (char *)(&suiveurs_list[14]) - (char *)(&suiveurs_list[13]);
		loadpartoffile(2495,  &suiveurs_list[13], offs, size);
		offs += size;
		size = (char *)(&kPersons[55]) - (char *)(&kPersons[0]);
		loadpartoffile(2495,  &kPersons[0], offs, size);
		offs += size;
		size = (char *)(&tapes[16]) - (char *)(&tapes[0]);
		loadpartoffile(2495,  &tapes[0], offs, size);
		offs += size;
		bandeoffsetin();
		size = (char *)(&kTabletView[6]) - (char *)(&kTabletView[0]);
		loadpartoffile(2495, &kTabletView[0], offs, size);
		offs += size;
		size = (char *)(&gameDialogs[10240]) - (char *)(&gameDialogs[0]); //TODO: const size 10240
		loadpartoffile(2495,  &gameDialogs[0], offs, size);
		gameLoaded = 1;
	}
	void loadgame(char *name) {
//	filespec_t fs;
//	file_t handle;
		long size;
//	CLFile_MakeStruct(0, 0, name, &fs);
//	CLFile_Open(&fs, 3, handle);

		Common::InSaveFile *handle = g_system->getSavefileManager()->openForLoading(name);
		if (!handle)
			return;

#define CLFile_Read(h, ptr, size) \
	h->read(ptr, *size);

		size = (char *)(&p_global->save_end) - (char *)(p_global);
		CLFile_Read(handle, p_global, &size);
		vavaoffsetin();
		size = (char *)(&gameIcons[134]) - (char *)(&gameIcons[123]);
		CLFile_Read(handle, &gameIcons[123], &size);
		size = (char *)(&kAreasTable[12]) - (char *)(&kAreasTable[0]);
		CLFile_Read(handle, &kAreasTable[0], &size);
		lieuoffsetin();
		size = (char *)(&gameRooms[423]) - (char *)(&gameRooms[0]);
		CLFile_Read(handle, &gameRooms[0], &size);
		size = (char *)(&objects[42]) - (char *)(&objects[0]);
		CLFile_Read(handle, &objects[0], &size);
		size = (char *)(&kObjectLocations[45]) - (char *)(&kObjectLocations[0]);
		CLFile_Read(handle, &kObjectLocations[0], &size);
		size = (char *)(&suiveurs_list[14]) - (char *)(&suiveurs_list[13]);
		CLFile_Read(handle, &suiveurs_list[13], &size);
		size = (char *)(&kPersons[55]) - (char *)(&kPersons[0]);
		CLFile_Read(handle, &kPersons[0], &size);
		size = (char *)(&tapes[16]) - (char *)(&tapes[0]);
		CLFile_Read(handle, &tapes[0], &size);
		bandeoffsetin();
		size = (char *)(&kTabletView[6]) - (char *)(&kTabletView[0]);
		CLFile_Read(handle, &kTabletView[0], &size);
		size = (char *)(&gameDialogs[10240]) - (char *)(&gameDialogs[0]); //TODO: const size 10240
		CLFile_Read(handle, &gameDialogs[0], &size);

		delete handle;
#undef CLFile_Read

//	CLFile_Close(handle);
		gameLoaded = 1;
		debug("* Game loaded from %s", name);
	}
#define NULLPTR (void*)0xFFFFFF
#define OFSOUT(val, base, typ) if(val)      (val) = (typ*)((char*)(val) - (size_t)(base)); else (val) = (typ*)NULLPTR;
#define OFSIN(val, base, typ) if((void*)(val) != NULLPTR)   (val) = (typ*)((char*)(val) + (size_t)(base)); else (val) = 0;
	void vavaoffsetout() {
		OFSOUT(p_global->dialog_ptr, gameDialogs, dial_t);
		OFSOUT(p_global->next_dialog_ptr, gameDialogs, dial_t);
		OFSOUT(p_global->narrator_dialog_ptr, gameDialogs, dial_t);
		OFSOUT(p_global->last_dialog_ptr, gameDialogs, dial_t);
		OFSOUT(p_global->tape_ptr, tapes, tape_t);
		OFSOUT(p_global->nextRoomIcon, gameIcons, icon_t);
		OFSOUT(p_global->room_ptr, gameRooms, room_t);
		OFSOUT(p_global->cita_area_firstRoom, gameRooms, room_t);
		OFSOUT(p_global->area_ptr, kAreasTable, area_t);
		OFSOUT(p_global->last_area_ptr, kAreasTable, area_t);
		OFSOUT(p_global->cur_area_ptr, kAreasTable, area_t);
		OFSOUT(p_global->perso_ptr, kPersons, perso_t);
		OFSOUT(p_global->room_perso, kPersons, perso_t);
	}
	void vavaoffsetin() {
		OFSIN(p_global->dialog_ptr, gameDialogs, dial_t);
		OFSIN(p_global->next_dialog_ptr, gameDialogs, dial_t);
		OFSIN(p_global->narrator_dialog_ptr, gameDialogs, dial_t);
		OFSIN(p_global->last_dialog_ptr, gameDialogs, dial_t);
		OFSIN(p_global->tape_ptr, tapes, tape_t);
		OFSIN(p_global->nextRoomIcon, gameIcons, icon_t);
		OFSIN(p_global->room_ptr, gameRooms, room_t);
		OFSIN(p_global->cita_area_firstRoom, gameRooms, room_t);
		OFSIN(p_global->area_ptr, kAreasTable, area_t);
		OFSIN(p_global->last_area_ptr, kAreasTable, area_t);
		OFSIN(p_global->cur_area_ptr, kAreasTable, area_t);
		OFSIN(p_global->perso_ptr, kPersons, perso_t);
		OFSIN(p_global->room_perso, kPersons, perso_t);
	}
	void lieuoffsetout() {
		int i;
		for (i = 0; i < 12; i++) {
			OFSOUT(kAreasTable[i].citadelRoom, gameRooms, room_t);
		}
	}
	void lieuoffsetin() {
		int i;
		for (i = 0; i < 12; i++) {
			OFSIN(kAreasTable[i].citadelRoom, gameRooms, room_t);
		}
	}
	void bandeoffsetout() {
		int i;
		for (i = 0; i < 16; i++) {
			OFSOUT(tapes[i].perso, kPersons, perso_t);
			OFSOUT(tapes[i].dialog, gameDialogs, dial_t);
		}
	}
	void bandeoffsetin() {
		int i;
		for (i = 0; i < 16; i++) {
			OFSIN(tapes[i].perso, kPersons, perso_t);
			OFSIN(tapes[i].dialog, gameDialogs, dial_t);
		}
	}
//// cond.c

	unsigned char *code_ptr;

	char testcondition(short index) {
		char end = 0;
		unsigned char op;
		unsigned short value, value2;
		unsigned short stack[32], *sp = stack, *sp2;
		unsigned short ofs;
		assert(index > 0);
		code_ptr = (unsigned char *)GetElem(gameConditions, (index - 1));
		do {
			value = cher_valeur();
			for (;;) {
				op = *code_ptr++;
				if (op == 0xFF) {
					end = 1;
					break;
				}
				if ((op & 0x80) == 0) {
					value2 = cher_valeur();
					value = operation(op, value, value2);
				} else {
					assert(sp < stack + 32);
					*sp++ = value;
					*sp++ = op;
					break;
				}
			}
		} while (!end);

		if (sp != stack) {
			*sp++ = value;
			sp2 = stack;
			value = *sp2++;
			do {
				op = *sp2++;
				value2 = *sp2++;
				value = operation(op, value, value2);
			} while (sp2 != sp);
		}
//	if (value)
		debug("cond %d(-1) returns %s", index, value ? "TRUE" : "false");
//	if (index == 402) debug("(glob_61.b == %X) & (glob_12.w == %X) & (glob_4C.b == %X) & (glob_4E.b == %X)", p_global->eventType, p_global->phaseNum, p_global->worldTyrannSighted, p_global->ff_4E);
		return value != 0;
	}
	unsigned short opera_add(unsigned short v1, unsigned short v2)  {
		return v1 + v2;
	}
	unsigned short opera_sub(unsigned short v1, unsigned short v2)  {
		return v1 - v2;
	}
	unsigned short opera_and(unsigned short v1, unsigned short v2)  {
		return v1 & v2;
	}
	unsigned short opera_or(unsigned short v1, unsigned short v2)   {
		return v1 | v2;
	}
	unsigned short opera_egal(unsigned short v1, unsigned short v2)     {
		return v1 == v2 ? -1 : 0;
	}
	unsigned short opera_petit(unsigned short v1, unsigned short v2)    {
		return v1 < v2 ? -1 : 0;    //TODO: all comparisons are unsigned!
	}
	unsigned short opera_grand(unsigned short v1, unsigned short v2)    {
		return v1 > v2 ? -1 : 0;
	}
	unsigned short opera_diff(unsigned short v1, unsigned short v2)     {
		return v1 != v2 ? -1 : 0;
	}
	unsigned short opera_petega(unsigned short v1, unsigned short v2)   {
		return v1 <= v2 ? -1 : 0;
	}
	unsigned short opera_graega(unsigned short v1, unsigned short v2)   {
		return v1 >= v2 ? -1 : 0;
	}
	unsigned short opera_faux(unsigned short v1, unsigned short v2)     {
		return 0;
	}
	unsigned short operation(unsigned char op, unsigned short v1, unsigned short v2) {
		static unsigned short(EdenGameImpl::*operations[16])(unsigned short, unsigned short) = {
			&EdenGameImpl::opera_egal,
			&EdenGameImpl::opera_petit,
			&EdenGameImpl::opera_grand,
			&EdenGameImpl::opera_diff,
			&EdenGameImpl::opera_petega,
			&EdenGameImpl::opera_graega,
			&EdenGameImpl::opera_add,
			&EdenGameImpl::opera_sub,
			&EdenGameImpl::opera_and,
			&EdenGameImpl::opera_or,
			&EdenGameImpl::opera_faux,
			&EdenGameImpl::opera_faux,
			&EdenGameImpl::opera_faux,
			&EdenGameImpl::opera_faux,
			&EdenGameImpl::opera_faux,
			&EdenGameImpl::opera_faux
		};
		return (this->*operations[(op & 0x1F) >> 1])(v1, v2);
	}
	unsigned short cher_valeur() {
		unsigned short val;
		unsigned char typ = *code_ptr++;
		if (typ < 0x80) {
			unsigned char ofs = *code_ptr++;
			if (typ == 1)
				val = *(unsigned char *)(ofs + (unsigned char *)p_global);
			else
				val = *(unsigned short *)(ofs + (unsigned char *)p_global);
		} else if (typ == 0x80)
			val = *code_ptr++;
		else {
			val = PLE16(code_ptr);
			code_ptr += 2;
		}
		return val;
	}
	void monbreak() {
		assert(0);
		signon(" coucou");
	}
	void ret() {
	}
//// cube.c
	short tabcos[361 * 2];
	int dword_32424, dword_32428, dword_3242C;
	int dword_32430, dword_32434, dword_32438;
	int dword_3243C, dword_32440, dword_32444;
	short word_32448;
	short word_3244A, word_3244C;
	float flt_32450, flt_32454;
	cube_t cube;
	short curs_cur_map;
	short lines[200 * 8];
	unsigned char cube_texture[0x4000];
	int cube_faces;
	long curs_old_tick, curs_new_tick;

	void make_tabcos() {
		int i;
		for (i = 0; i < 361; i++) {
			tabcos[i * 2] = (int)(cos(3.1416 * i / 180.0) * 255.0);
			tabcos[i * 2 + 1] = (int)(sin(3.1416 * i / 180.0) * 255.0);
		}
	}
	void make_matrice_fix() {
		short r30, r28, r29;
		r30 = word_3244C;
		r28 = word_3244A;
		r29 = word_32448;

		dword_32424 = (tabcos[r29 * 2] * tabcos[r28 * 2]) >> 8;
		dword_32430 = (tabcos[r29 * 2 + 1] * tabcos[r28 * 2]) >> 8;
		dword_3243C = -tabcos[r28 * 2 + 1];
		dword_32428 = ((-tabcos[r29 * 2 + 1] * tabcos[r30 * 2]) >> 8)
		              + ((tabcos[r30 * 2 + 1] * ((tabcos[r29 * 2] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
		dword_32434 = ((tabcos[r29 * 2] * tabcos[r30 * 2]) >> 8)
		              + ((tabcos[r30 * 2 + 1] * ((tabcos[r29 * 2 + 1] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
		dword_32440 = (tabcos[r28 * 2] * tabcos[r30 * 2 + 1]) >> 8;
		dword_3242C = ((tabcos[r29 * 2 + 1] * tabcos[r30 * 2 + 1]) >> 8)
		              + ((tabcos[r30 * 2] * ((tabcos[r29 * 2] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
		dword_32438 = ((-tabcos[r29 * 2] * tabcos[r30 * 2 + 1]) >> 8)
		              + ((tabcos[r30 * 2] * ((tabcos[r29 * 2 + 1] * tabcos[r28 * 2 + 1]) >> 8)) >> 8);
		dword_32444 = (tabcos[r28 * 2] * tabcos[r30 * 2]) >> 8;
	}
	void projection_fix(cube_t *cube, int n) {
		int i;
		for (i = 0; i < n; i++) {
			int r24, r25, r26, r27, r28, r29;
			r28 = cube->vertices[i * 4];
			r27 = cube->vertices[i * 4 + 1];
			r26 = cube->vertices[i * 4 + 2];

			r25 = dword_32424 * r28 + dword_32428 * r27 + dword_3242C * r26 + (int)(flt_32454 * 256.0f);
			r24 = dword_32430 * r28 + dword_32434 * r27 + dword_32438 * r26 + (int)(flt_32450 * 256.0f);
			r29 = dword_3243C * r28 + dword_32440 * r27 + dword_32444 * r26 + (int)(flt_2DF7C * 256.0f);

			r29 >>= 8;
			if (r29 == -256)
				r29++;
			cube->projection[i * 4    ] = r25 / (r29 + 256) + curs_x + 14 + scroll_pos;
			cube->projection[i * 4 + 1] = r24 / (r29 + 256) + curs_y + 14;
			cube->projection[i * 4 + 2] = r29;

//		assert(cube->projection[i * 4] < 640);
//		assert(cube->projection[i * 4 + 1] < 200);
		}
	}
	void init_cube() {
		NEWcharge_map(2493, cube_texture);
		NEWcharge_objet_mob(&cube, 2494, cube_texture);
		make_tabcos();
	}
	void moteur() {
		Eden_dep_and_rot();
		make_matrice_fix();
		projection_fix(&cube, cube_faces);
		affiche_objet(&cube);
	}
	void affiche_objet(cube_t *cube) {
		int i;
		for (i = 0; i < cube->num; i++)
			affiche_polygone_mapping(cube, cube->faces[i]);
	}
	void NEWcharge_map(int file_id, unsigned char *buffer) {
		int i;
		loadpartoffile(file_id, buffer, 32, 256 * 3);

		for (i = 0; i < 256; i++) {
			color3_t color;
			color.r = buffer[i * 3] << 8;
			color.g = buffer[i * 3 + 1] << 8;
			color.b = buffer[i * 3 + 2] << 8;
			CLPalette_SetRGBColor(global_palette, i, &color);
		}
		CLPalette_Send2Screen(global_palette, 0, 256);

		loadpartoffile(file_id, buffer, 32 + 256 * 3, 0x4000);
	}
	void NEWcharge_objet_mob(cube_t *cube, int file_id, unsigned char *texptr) {
		int i, j, count2;
		char *tmp1, *next, error;
		cubeface_t **tmp4;
		short *vertices, *projection;
		tmp1 = (char *)malloc(454);
		loadpartoffile(file_id, tmp1, 0, 454);
		next = tmp1;
		cube_faces = next_val(&next, &error);
		vertices = (short *)malloc(cube_faces * 4 * sizeof(*vertices));
		projection = (short *)malloc(cube_faces * 4 * sizeof(*projection));
		for (i = 0; i < cube_faces; i++) {
			vertices[i * 4] = next_val(&next, &error);
			vertices[i * 4 + 1] = next_val(&next, &error);
			vertices[i * 4 + 2] = next_val(&next, &error);
		}
		count2 = next_val(&next, &error);
		tmp4 = (cubeface_t **)malloc(count2 * sizeof(*tmp4));
		for (i = 0; i < count2; i++) {
			char textured;
			tmp4[i] = (cubeface_t *)malloc(sizeof(cubeface_t));
			tmp4[i]->tri = 3;
			textured = next_val(&next, &error);
			tmp4[i]->ff_5 = next_val(&next, &error);
			tmp4[i]->indices = (unsigned short *)malloc(3 * sizeof(*tmp4[i]->indices));
			tmp4[i]->uv = (short *)malloc(3 * 2 * sizeof(*tmp4[i]->uv));
			for (j = 0; j < 3; j++) {
				tmp4[i]->indices[j] = next_val(&next, &error);
				if (textured) {
					tmp4[i]->uv[j * 2] = next_val(&next, &error);
					tmp4[i]->uv[j * 2 + 1] = next_val(&next, &error);
				}
			}
			if (textured) {
				tmp4[i]->ff_4 = 3;
				tmp4[i]->texptr = texptr;
			} else
				tmp4[i]->ff_4 = 0;
		}
		free(tmp1);
		cube->num = count2;
		cube->faces = tmp4;
		cube->projection = projection;
		cube->vertices = vertices;
	}
	static int next_val(char **ptr, char *error) {
		char c = 0;
		char *p = *ptr;
		int val = strtol(p, 0, 10);
		while ((*p >= '0' && *p <= '9' && *p != 0) || *p == '-') p++;
		while ((*p == 13 || *p == 10 || *p == ',' || *p == ' ') && *p) c = *p++;
		*error = c == 10;
		*ptr = p;
		return val;
	}
	void selectmap(short num) {
		int i, j;
		short k, x, y;
		char mode;
		curs_cur_map = num;
		k = 0;
		mode = tab_2E138[num];
		x = (num & 7) * 32;
		y = (num & 0x18) * 4;
		for (i = 0; i < 6 * 2; i++)
			for (j = 0; j < 3; j++) {
				cube.faces[i]->uv[j * 2    ] = x + cube_texcoords[mode][k];
				k++;
				cube.faces[i]->uv[j * 2 + 1] = y + cube_texcoords[mode][k];
				k++;
			}
	}
	void Eden_dep_and_rot() {
		short curs;
		curs = current_cursor;
		if (normalCursor && (p_global->drawFlags & DrawFlags::drDrawFlag20))
			curs = 10;
		selectmap(curs);
		curs_new_tick = TickCount();
		if (curs_new_tick - curs_old_tick < 1)
			return;
		curs_old_tick = curs_new_tick;
		switch (current_cursor) {
		case 0:
			word_3244C = (word_3244C + 2) % 360;
			word_3244A = (word_3244A + 2) % 360;
			restoreZDEP();
			break;
		case 1:
			word_3244C = 0;
			word_3244A -= 2;
			if (word_3244A < 0)
				word_3244A += 360;
			restoreZDEP();
			break;
		case 2:
			word_3244C = (word_3244C + 2) % 360;
			word_3244A = 0;
			restoreZDEP();
			break;
		case 3:
			word_3244C -= 2;
			if (word_3244C < 0)
				word_3244C += 360;
			word_3244A = 0;
			restoreZDEP();
			break;
		case 4:
			word_3244C = 0;
			word_3244A = (word_3244A + 2) % 360;
			restoreZDEP();
			break;
		case 5:
			word_3244C = 0;
			word_3244A = 0;
			flt_2DF7C += flt_2DF84;
			if ((flt_2DF7C < -3600.0 + flt_2DF80) || flt_2DF7C > flt_2DF80)
				flt_2DF84 = -flt_2DF84;
			break;
		case 6:
			word_3244C = 0;
			word_3244A = 0;
			flt_2DF7C = flt_2DF80;
			break;
		case 7:
			word_3244C -= 2;
			if (word_3244C < 0)
				word_3244C += 360;
			word_3244A = 0;
			restoreZDEP();
			break;
		case 8:
			word_3244C = 0;
			word_3244A = 0;
			flt_2DF7C = flt_2DF80;
			break;
		case 9:
			word_3244C = 0;
			word_3244A = 0;
			flt_2DF7C = flt_2DF80;
			break;
		}
	}
	void restoreZDEP() {
		flt_2DF84 = 200.0;
		if (flt_2DF7C < flt_2DF80)
			flt_2DF7C += flt_2DF84;
		if (flt_2DF7C > flt_2DF80)
			flt_2DF7C -= flt_2DF84;
	}
	void affiche_polygone_mapping(cube_t *cube, cubeface_t *face) {
		short r20, r30, r26, r31, r19, r18, /*r25,*/ r24, ymin, ymax, v46, v48, v4A, v4C, v4E, v50;
		short *uv;
		unsigned short r25;
		unsigned short *indices = face->indices;
		int r17, r29;
		r29 = indices[0] * 4;
		v46 = cube->projection[r29];
		v48 = cube->projection[r29 + 1];

		r29 = indices[1] * 4;
		v4A = cube->projection[r29];
		v4C = cube->projection[r29 + 1];

		r29 = indices[2] * 4;
		v4E = cube->projection[r29];
		v50 = cube->projection[r29 + 1];

		if ((v4C - v48) * (v4E - v46) - (v50 - v48) * (v4A - v46) > 0)
			return;

		uv = face->uv;
		ymin = 200; // min y
		ymax = 0;   // max y
		r29 = indices[0] * 4;
		r20 = cube->projection[r29];
		r30 = cube->projection[r29 + 1];
		r19 = *uv++;
		r18 = *uv++;
		indices++;
		for (r17 = 0; r17 < face->tri - 1; r17++, indices++) {
			r29 = indices[0] * 4;
			r26 = cube->projection[r29];
			r31 = cube->projection[r29 + 1];
			r25 = *uv++;    //TODO: unsigned
			r24 = *uv++;    //TODO: unsigned
			if (r30 < ymin)
				ymin = r30;
			if (r30 > ymax)
				ymax = r30;
			if (r31 < ymin)
				ymin = r31;
			if (r31 > ymax)
				ymax = r31;
			trace_ligne_mapping(r20, r30, r26, r31, r19, r18, r25, r24, lines);
			r20 = r26;
			r30 = r31;
			r19 = r25;
			r18 = r24;
		}
		r29 = face->indices[0] * 4;
		r26 = cube->projection[r29];
		r31 = cube->projection[r29 + 1];
		uv = face->uv;
		r25 = *uv++;    //TODO: this is unsigned
		r24 = *uv;      //TODO: this is signed
		if (r30 < ymin)
			ymin = r30;
		if (r30 > ymax)
			ymax = r30;
		if (r31 < ymin)
			ymin = r31;
		if (r31 > ymax)
			ymax = r31;
		trace_ligne_mapping(r20, r30, r26, r31, r19, r18, r25, r24, lines);
		affiche_ligne_mapping(ymin, ymax, p_mainview->p_buffer, face->texptr);
	}
	void trace_ligne_mapping(short r3, short r4, short r5, short r6, short r7, short r8, short r9, short r10, short *lines) {
		short t;
		short r26;
		int r30, r29, r28, r23, r24, r25;
		int i;
		r26 = r6 - r4;
		if (r26 <= 0) {
			if (r26 == 0) {
				lines += r4 * 8;
				if (r5 - r3 > 0) {
					lines[0] = r3;
					lines[1] = r5;
					lines[4] = r7;
					lines[5] = r9;
					lines[6] = r8;
					lines[7] = r10;
				} else {
					lines[0] = r5;
					lines[1] = r3;
					lines[4] = r9;
					lines[5] = r7;
					lines[6] = r10;
					lines[7] = r8;
				}
				return;
			}
			t = r3;
			r3 = r5;
			r5 = t;
			t = r7;
			r7 = r9;
			r9 = t;
			t = r8;
			r8 = r10;
			r10 = t;
			lines += r6 * 8;
			r26 = -r26;
		} else
			lines += r4 * 8 + 1;    //TODO wha???

		r30 = r3 << 16;
		r29 = r7 << 16;
		r28 = r8 << 16;

		r25 = ((r5 - r3) << 16) / r26;
		r24 = ((r9 - r7) << 16) / r26;
		r23 = ((r10 - r8) << 16) / r26;

		for (i = 0; i < r26; i++) {
			lines[0] = r30 >> 16;
			lines[4] = r29 >> 16;
			lines[6] = r28 >> 16;

			r30 += r25;
			r29 += r24;
			r28 += r23;
			lines += 8;
		}
	}
	void affiche_ligne_mapping(short r3, short r4, unsigned char *target, unsigned char *texture) {
		short r21, r20, r26, r25, r29, r28, len;
		int r22;
		unsigned short r31, r30;
		short height = r4 - r3;
		unsigned char *trg, *trg_line = p_mainview->p_buffer + r3 * 640;    //TODO: target??
		short *line = &lines[r3 * 8];
//	debug("curs: beg draw %d - %d", r3, r4);
		for (r22 = height; r22; r22--, line += 8, trg_line += 640) {
			r29 = line[0];
			r28 = line[1];
			len = r28 - r29;
			if (len < 0)
				break;
			if (len == 0)
				continue;
//	debug("curs: lin draw %d", r4 - height);
			r31 = line[4] << 8;
			r30 = line[6] << 8;

			r21 = line[5] - line[4];
			r20 = line[7] - line[6];

			r26 = (r21 << 8) / len;
			r25 = (r20 << 8) / len;
			trg = trg_line + r29;
#if 1
			while (r29++ < r28) {
				*trg++ = texture[(r30 & 0xFF00) | (r31 >> 8)];
				r31 += r26;
				r30 += r25;
			}
#endif
		}
	}
////// macgame.c
//void MyDlgHook()  {  }
//void PrepareReply()  {  }
	short OpenDialog(void *arg1, void *arg2) {
		//TODO
		return 0;
	}
//void SaveDialog()  {  }
//void LostEdenMac_SavePrefs()  {  }
//void LostEdenMac_LoadPrefs()  {  }
	void LostEdenMac_InitPrefs() {
		p_global->pref_language = 1;
		doubled = 0;    // TODO: set to 1
		p_global->pref_10C[0] = 192;
		p_global->pref_10C[1] = 192;
		p_global->pref_10E[0] = 255;
		p_global->pref_10E[1] = 255;
		p_global->pref_110[0] = 32;
		p_global->pref_110[1] = 32;
	}
//void MacGame_DoAbout()  {  }
//void MacGame_DoAdjustMenus()  {  }
//void LostEdenMac_DoPreferences()  {  }
//void MacGame_DoSave()  {  }
//void MacGame_DoMenuCommand()  {  }
//void MacGame_DoOpen()  {  }
//void MacGame_DoSaveAs()  {  }

};  // class EdenGameImpl

EdenGameImpl LostEden;

void EdenGame::main() {
	LostEden.main();
}

}   // namespace Cryo
