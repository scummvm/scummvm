#include	"config.h"
#include	"sound.h"
#include	"vmenu.h"
#include	"text.h"
#include	"cge.h"

/*
   51=wska§ typ posiadanej karty d¦wi‘kowej
   52=wybierz numer portu dla karty d¦wi‘kowej
   53=wybierz numer przerwania dla karty d¦wi‘kowej
   54=wybierz numer kana’u DMA dla karty d¦wi‘kowej
   55=wybierz numer portu dla General MIDI
   55=konfiguracja karty d¦wi‘kowej
*/
#define		STYPE_TEXT	51
#define		SPORT_TEXT	52
#define		SIRQ_TEXT	53
#define		SDMA_TEXT	54
#define		MPORT_TEXT	55
#define		MENU_TEXT	56

#define		NONE_TEXT	60
#define		SB_TEXT		61
#define		SBM_TEXT	62
#define		GUS_TEXT	63
#define		GUSM_TEXT	64
#define		MIDI_TEXT	65
#define		AUTO_TEXT	66

#define		DETECT		0xFFFF


void		NONE		(void);
void		SB		(void);
void		SBM		(void);
void		GUS		(void);
void		GUSM		(void);
void		MIDI		(void);
void		AUTO		(void);
void		SetPortD	(void);
void		SetPortM	(void);
void		SetIRQ		(void);
void		SetDMA		(void);


static int DevName[] = { NONE_TEXT, SB_TEXT, SBM_TEXT,
			 GUS_TEXT, GUSM_TEXT,
			 MIDI_TEXT, AUTO_TEXT };

static CHOICE DevMenu[]={ { NULL, NONE  },
			  { NULL, SB    },
			  { NULL, SBM   },
			  { NULL, GUS   },
			  { NULL, GUSM  },
			  { NULL, MIDI  },
			  { NULL, AUTO  },
			  { NULL, NULL  } };


static CHOICE DigiPorts[]={ { " 210h", SetPortD },
			    { " 220h", SetPortD },
			    { " 230h", SetPortD },
			    { " 240h", SetPortD },
			    { " 250h", SetPortD },
			    { " 260h", SetPortD },
			    { "AUTO ", SetPortD },
			    { NULL,   NULL     } };

static CHOICE MIDIPorts[]={ { " 220h", SetPortM },
			    { " 230h", SetPortM },
			    { " 240h", SetPortM },
			    { " 250h", SetPortM },
			    { " 300h", SetPortM },
			    { " 320h", SetPortM },
			    { " 330h", SetPortM },
			    { " 340h", SetPortM },
			    { " 350h", SetPortM },
			    { " 360h", SetPortM },
			    { "AUTO ", SetPortM },
			    { NULL,   NULL     } };

static CHOICE BlsterIRQ[]={ { "IRQ  2", SetIRQ },
			    { "IRQ  5", SetIRQ },
			    { "IRQ  7", SetIRQ },
			    { "IRQ 10", SetIRQ },
			    { "AUTO  ", SetIRQ },
			    { NULL,   NULL     } };

static CHOICE GravisIRQ[]={ { "IRQ  2", SetIRQ },
			    { "IRQ  5", SetIRQ },
			    { "IRQ  7", SetIRQ },
			    { "IRQ 11", SetIRQ },
			    { "IRQ 12", SetIRQ },
			    { "IRQ 15", SetIRQ },
			    { "AUTO  ", SetIRQ },
			    { NULL,   NULL     } };

static CHOICE GravisDMA[]={ { "DMA 1", SetDMA },
			    { "DMA 3", SetDMA },
			    { "DMA 5", SetDMA },
			    { "DMA 6", SetDMA },
			    { "DMA 7", SetDMA },
			    { "AUTO ", SetDMA },
			    { NULL,   NULL    } };

static CHOICE BlsterDMA[]={ { "DMA 0", SetDMA },
			    { "DMA 1", SetDMA },
			    { "DMA 3", SetDMA },
			    { "AUTO ", SetDMA },
			    { NULL,   NULL    } };




void SelectSound (void)
{
  int i;
  Sound.Close();
  if (VMENU::Addr) SNPOST_(SNKILL, -1, 0, VMENU::Addr);
  Inf(Text[STYPE_TEXT]);
  Talk->Goto(Talk->X, FONT_HIG/2);
  for (i = 0; i < ArrayCount(DevName); i ++)
    DevMenu[i].Text = Text[DevName[i]];
  (new VMENU(DevMenu, SCR_WID/2, Talk->Y+Talk->H+TEXT_VM+FONT_HIG))->SetName(Text[MENU_TEXT]);
}




static void Reset (void)
{
  SNDDrvInfo.DBASE = SNDDrvInfo.DIRQ = SNDDrvInfo.DDMA = SNDDrvInfo.MBASE = DETECT;
}





static word deco (const char * str, word (*dco)(const char *))
{
  while (*str && ! IsDigit(*str)) ++ str;
  if (*str) return dco(str);
  else return DETECT;
}




static word ddeco (const char * str)
{
  return deco(str, atow);
}




static word xdeco (const char * str)
{
  return deco(str, xtow);
}




static	CHOICE *	Cho;
static	int		Hlp;

static void SNSelect (void)
{
  Inf(Text[Hlp]);
  Talk->Goto(Talk->X, FONT_HIG / 2);
  (new VMENU(Cho, SCR_WID/2, Talk->Y+Talk->H+TEXT_VM+FONT_HIG))->SetName(Text[MENU_TEXT]);
}




static void Select (CHOICE * cho, int hlp)
{
  Cho = cho;
  Hlp = hlp;
  SNPOST(SNEXEC, -1, 0, (void *) SNSelect);
}






static void NONE (void)
{
  SNDDrvInfo.DDEV = DEV_QUIET;
  SNDDrvInfo.MDEV = DEV_QUIET;
  Sound.Open();
}



static void SB (void)
{
  SNDDrvInfo.DDEV = DEV_SB;
  SNDDrvInfo.MDEV = DEV_SB;
  Reset();
  Select(DigiPorts, SPORT_TEXT);
}



static void SBM (void)
{
  SNDDrvInfo.DDEV = DEV_SB;
  SNDDrvInfo.MDEV = DEV_GM;
  Reset();
  Select(DigiPorts, SPORT_TEXT);
}


static void GUS (void)
{
  SNDDrvInfo.DDEV = DEV_GUS;
  SNDDrvInfo.MDEV = DEV_GUS;
  Reset();
  Select(DigiPorts, SPORT_TEXT);
}



static void GUSM (void)
{
  SNDDrvInfo.DDEV = DEV_GUS;
  SNDDrvInfo.MDEV = DEV_GM;
  Reset();
  Select(DigiPorts, SPORT_TEXT);
}


static void MIDI (void)
{
  SNDDrvInfo.DDEV = DEV_QUIET;
  SNDDrvInfo.MDEV = DEV_GM;
  SNDDrvInfo.MBASE = DETECT;
  Select(MIDIPorts, MPORT_TEXT);
}



static void AUTO (void)
{
  SNDDrvInfo.DDEV = DEV_AUTO;
  SNDDrvInfo.MDEV = DEV_AUTO;
  Reset();
  Sound.Open();
}






static void SetPortD (void)
{
  SNDDrvInfo.DBASE = xdeco(DigiPorts[VMENU::Recent].Text);
  Select((SNDDrvInfo.DDEV == DEV_SB) ? BlsterIRQ : GravisIRQ, SIRQ_TEXT);
}



static void SetPortM (void)
{
  SNDDrvInfo.MBASE = xdeco(MIDIPorts[VMENU::Recent].Text);
  Sound.Open();
}




static void SetIRQ (void)
{
  SNDDrvInfo.DIRQ = ddeco(((SNDDrvInfo.DDEV == DEV_SB) ? BlsterIRQ : GravisIRQ)[VMENU::Recent].Text);
  Select((SNDDrvInfo.DDEV == DEV_SB) ? BlsterDMA : GravisDMA, SDMA_TEXT);
}




static void SetDMA (void)
{
  SNDDrvInfo.DDMA = ddeco(((SNDDrvInfo.DDEV == DEV_SB) ? BlsterDMA : GravisDMA)[VMENU::Recent].Text);
  if (SNDDrvInfo.MDEV != SNDDrvInfo.DDEV) Select(MIDIPorts, MPORT_TEXT);
  else Sound.Open();
}
