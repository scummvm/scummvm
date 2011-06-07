#include	<general.h>
#include	<cfile.h>
#include	<lz.h>
#include	<wind.h>
#include	<boot.h>
#include	<dbf.h>
#include	<values.h>
#include	<errno.h>
#include	<intregs.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	"c:\work\instaluj.cpp\ar.h"
#include	<io.h>
#include	<bios.h>
#include	<dos.h>
#include	<dir.h>
#include	<fcntl.h>
#include	<ctype.h>

#define		MAJOR		1
#define		MINOR		70
#define		CLONE_SIG	""
#define		MaxTextLines	100
#define		BUF_SIZ		K(16)
#define		DISK_C		2

#define		ENCRYPT		0x01
#define		PUTBOOT		0x02
#define		PRGNUMB		0x04
#define		ALL_FLAGS	(ENCRYPT | PUTBOOT | PRGNUMB)
#define		PRGNUMLEN	12

#define		CycPerMin	1092

//extern	word	_stklen	= K(8);

extern	int	InitHook	(int drv);
extern	int	CopyHook	(char b);

static	char	Signature[] = "JBW Install Manager";

static	char
#ifdef	PL
  CREDIR_TXT[]	= " Tworzenie katalogu ",
  DIRWARN_TXT[]	= "Katalog docelowy istnieje. Grozi utrata danych! Kontynuujemy?",
  MKDIRFAIL_TXT[]= "Stworzenie docelowego katalogu jest niewykonalne! Przerywam instalowanie.",
  CHDIR_TXT[]	= " Zmiana katalogu na ",
  CHDIRFAIL_TXT[]= "Zmiana katalogu jest niewykonalna! Przerywam instalowanie.",
  CHDRV_TXT[]	= "Zmiana dysku na  ",
  CHDRVFAIL_TXT[]= " jest niewykonalna. Przerywam instalowanie.",
  DRVSEL_TXT[]	= " Wskazanie dysku, gdzie zostanie zainstalowany ",
  NODRV_TXT[]	= "Brak dysku do zainstalowania programu ",
  SELDRVP_TXT[]	= "Wybierz dysk do zainstalowania programu ",
  SELDIR_TXT[]	= " Podaj katalog, gdzie zostanie umieszczony ",
  DESTDIR_TXT[]	= "Katalog docelowy",
  NOCDRV_TXT[]	= "Brak dysku C:! Przerywam instalowanie.",
  PATHINF_TXT[]	= " W wyniku modyfikacji polecenia PATH szybciej uruchomisz ",
  AEASK1[]	= "Daj pozwolenie na niewielkie zmiany w pliku C:\\AUTOEXEC.BAT",
  AEASK_MNU[]	= "Pozwalam   Zabraniam",
  AEASK2[]	= "Dokonasz w pliku C:\\AUTOEXEC.BAT poszerzenia listy PATH o ",
  AEASK3[]	= "Wpiszesz do pliku C:\\AUTOEXEC.BAT polecenie PATH ",
  AENAK_TXT[]	= "Na skutek Twej nieprzejednanej postawy przerywam instalowanie!",
  AEPROT_TXT[]	= "Brak praw do zapisu pliku AUTOEXEC.BAT! Przerywam instalowanie.",
  AEFAIL_TXT[]	= "Niepowodzenie przetwarzania pliku AUTOEXEC.BAT! Przerywam instalowanie.",
  TMPFILE_TXT[]	= " Tworzenie pomocniczego pliku ",
  TMPFAIL_TXT[]	= "Niepowodzenie tworzenia pliku ",
  NOTXTRAM_TXT[]= "Brak RAM na bufor tekstu!",
  NOVFILE_TXT[]	= "Brak wskazanego pliku tekstowego!",
  VIEWREAD_TXT[]= " Czytanie pliku ",
  VIEWKEYS_TXT[]= " ³ Klawisze: [] [] [PgUp] [PgDn] [Esc]",
  FLOPFAIL_TXT[]= "Niepowodzenie odczytu dyskietki. Przerywam instalowanie.",
  TWICE_TXT[]	= "Uwaga: ta dyskietka jest wykorzystywana nie po raz pierwszy! Rezygnujesz?"
#else
  CREDIR_TXT[]	= " Creating directory ",
  DIRWARN_TXT[]	= "Target directory exists. Files may be overwriten! Continue?",
  MKDIRFAIL_TXT[]= "Target directory creation failed! Installation terminated.",
  CHDIR_TXT[]	= " Changing directory: ",
  CHDIRFAIL_TXT[]= "Changing directory failed! Installation terminated.",
  CHDRV_TXT[]	= "Changing drive:  ",
  CHDRVFAIL_TXT[]= " failed. Installation terminated.",
  DRVSEL_TXT[]	= " Select target disk for ",
  NODRV_TXT[]	= "There is no drive to install ",
  SELDRVP_TXT[]	= "Select drive to install ",
  SELDIR_TXT[]	= " Type target directory for ",
  DESTDIR_TXT[]	= "Target directory",
  NOCDRV_TXT[]	= "There is no drive C:! Installation terminated.",
  PATHINF_TXT[]	= " After updating PATH statement you can get faster ",
  AEASK1[]	= "Please confirm slight changes in file C:\\AUTOEXEC.BAT",
  AEASK_MNU[]	= "Accept   Reject",
  AEASK2[]	= "Will you add to PATH list in your C:\\AUTOEXEC.BAT: ",
  AEASK3[]	= "Will you add to your C:\\AUTOEXEC.BAT: ",
  AENAK_TXT[]	= "Your negative answers caused installation termination!",
  AEPROT_TXT[]	= "File AUTOEXEC.BAT is write protected! Installation terminated.",
  AEFAIL_TXT[]	= "AUTOEXEC.BAT processing failed! Installation terminated.",
  TMPFILE_TXT[]	= " Creating temporary file: ",
  TMPFAIL_TXT[]	= "File creation failed: ",
  NOTXTRAM_TXT[]= "No RAM for text buffer!",
  NOVFILE_TXT[]	= "Can\'t find text file!",
  VIEWREAD_TXT[]= " File reading ",
  VIEWKEYS_TXT[]= " ³ Active keys: [] [] [PgUp] [PgDn] [Esc]",
  FLOPFAIL_TXT[]= "Unable to read diskette. Installation terminated.",
  TWICE_TXT[]	= "Note that diskette is used not the first time! Do you resign?"
#endif
;

static	char	PrgNumOrd[2][PRGNUMLEN/2] = { { 0, 1,  4, 2,  6, 8},
					      { 7, 5, 11, 3, 10, 9} };


static	char	ProgDrv[MAXDRIVE];
static	char	ProgDir[MAXDIR];
static	char	ProgNam[MAXFILE];
static	char	IniName[MAXPATH];
static	char	OldDir[MAXPATH];

static	int	OldDrv;
static	Boolean	OldMono;
static	word *	OldScr;
static	Wind *	DeskWind = NULL;
static	Wind *	ViewWind = NULL;
static	Wind *	StatWind;
static	Wind *	TitleWind;
static	Wind *	TextWind;

static	CFILE *	OFile = NULL;
static	dword	Space = 0;
static	long	OldSecret = 0;
static	long	Mask = 0;
static	int	Delay = 1000;
static	Boolean	Chain;
static	Boolean	DirWarn = TRUE;
static	Boolean	BigView = FALSE;
static	Boolean	DosScreen = TRUE;
static	Boolean	Installed = FALSE;
static	Boolean	NumberTaken = FALSE;
static	Boolean	Completed = FALSE;
static	byte	Mode = 0;
static	char	Title[10][73];
static	char	Text[10][73];
static	char	Mazovia[19] = "†‘’¤¢ž¦§•œ¥£˜ ¡";
static	char	Polskie[19] = "†‘’¤¢ž¦§•œ¥£˜ ¡";
static	char	Program[73] = "";
static	char	Name[73] = "";
static	char	PrgNumStr[PRGNUMLEN+1];
	Boot *	BootSec = NULL;

static	int	_0 = 0, _1 = 1, _9 = 9, _72 = 72;
static	int	_1001 = 1001, _3601 = 3601;
static	byte	_128 = 128;

#define		CC(bank,reg)	(Colors[0][bank]+reg)
#define		CM(bank,reg)	(Colors[1][bank]+reg)

Boolean	WriteStatText	(Wind *, char *, int);
void	WarnM		(char *);
void	Quit		(Boolean);

void	CheckSpace	(void);
void	Wait		(void);
void	ShowTitle	(void);
void	ShowTitleR	(void);
void	ShowText	(void);
void	ShowTextR	(void);
void	HideTitle	(void);
void	HideText	(void);
void	GetAnyKey	(void);
void	Menu		(void);
void	SetPol		(void);
void	Open		(void);
void	Close		(void);
void	Copy		(void);
void	CopyP		(void);
void	Unpack		(void);
void	Erase		(void);
void	GetDrive	(void);
void	GetDir		(void);
void	ChangeDir	(void);
void	MakeDir		(void);
void	SaveDir		(void);
void	RestoreDir	(void);
void	InsertPath	(void);
void	MakeBatch	(void);
void	IfProc		(void);
void	ElseProc	(void);
void	EndIfProc	(void);
void	Command		(void);
void	ViewText	(void);
void	IniChain	(void);
void	TakeMask	(void);

#ifdef		ADD
#include	"usr_fun.c"
#endif

#define		Inf3		(InfTab[3])
#define		Inf3Adr		((void*)IniChain)

static	IniItem InfTab[] = {

  { FALSE, TypeProc,  "*If",            (void*)IfProc,    NULL,    NULL },
  { FALSE, TypeProc,  "*Else",          (void*)ElseProc,  NULL,    NULL },
  { FALSE, TypeProc,  "*EndIf",         (void*)EndIfProc, NULL,    NULL },
  { FALSE, TypeProc,  "*Chain",         Inf3Adr,          NULL,    NULL },

#ifdef		ADD
#include	"usr_tab.c"
#endif

  { FALSE, TypeProc,  "*CheckSpace",    (void*)CheckSpace,NULL,    NULL },
  { FALSE, TypeProc,  "*Wait",          (void*)Wait,      NULL,    NULL },
  { FALSE, TypeProc,  "*ShowTitle?",    (void*)ShowTitle, NULL,    NULL },
  { FALSE, TypeProc,  "*ShowTitle",     (void*)ShowTitleR,NULL,    NULL },
  { FALSE, TypeProc,  "*ShowText?",     (void*)ShowText,  NULL,    NULL },
  { FALSE, TypeProc,  "*ShowText",      (void*)ShowTextR, NULL,    NULL },
  { FALSE, TypeProc,  "*HideTitle",     (void*)HideTitle, NULL,    NULL },
  { FALSE, TypeProc,  "*HideText",      (void*)HideText,  NULL,    NULL },
  { FALSE, TypeProc,  "*Menu",          (void*)Menu,      NULL,    NULL },
  { FALSE, TypeProc,  "*Polskie",       (void*)SetPol,    NULL,    NULL },
  { FALSE, TypeProc,  "*GetAnyKey",     (void*)GetAnyKey, NULL,    NULL },
  { FALSE, TypeProc,  "*Erase",         (void*)Erase,     NULL,    NULL },
  { FALSE, TypeProc,  "*Open",          (void*)Open,      NULL,    NULL },
  { FALSE, TypeProc,  "*Close",         (void*)Close,     NULL,    NULL },
  { FALSE, TypeProc,  "*CopyP",         (void*)CopyP,     NULL,    NULL },
  { FALSE, TypeProc,  "*Copy",          (void*)Copy,      NULL,    NULL },
  { FALSE, TypeProc,  "*Expand",        (void*)Unpack,    NULL,    NULL },
  { FALSE, TypeProc,  "*GetDrive",      (void*)GetDrive,  NULL,    NULL },
  { FALSE, TypeProc,  "*GetDir",        (void*)GetDir,    NULL,    NULL },
  { FALSE, TypeProc,  "*ChangeDir",     (void*)ChangeDir, NULL,    NULL },
  { FALSE, TypeProc,  "*MakeDir",       (void*)MakeDir,   NULL,    NULL },
  { FALSE, TypeProc,  "*SaveDir",       (void*)SaveDir,   NULL,    NULL },
  { FALSE, TypeProc,  "*RestoreDir",    (void*)RestoreDir,NULL,    NULL },
  { FALSE, TypeProc,  "*InsertPath",    (void*)InsertPath,NULL,    NULL },
  { FALSE, TypeProc,  "*MakeBatch",     (void*)MakeBatch, NULL,    NULL },
  { FALSE, TypeProc,  "*Command",       (void*)Command,   NULL,    NULL },
  { FALSE, TypeProc,  "*View",          (void*)ViewText,  NULL,    NULL },

  { FALSE, TypeBool,  "Monochrome=",    &Mono,        NULL,        NULL },
  { FALSE, TypeInt,   "ProtectDelay=",  &ProtectDelay,&_0,         &_3601 },
  { FALSE, TypeInt,   "ProtectSpeed=",  &ProtectSpeed,&_1,         &_1001 },
  { FALSE, TypeString,"FrameNormal=",   &FrDes,       &_9,         &_9 },
  { FALSE, TypeString,"FrameBold=",     &(FrDes[9]),  &_9,         &_9 },

  { FALSE, TypeString,"Title1=",        Title[0],     &_72,        &_72 },
  { FALSE, TypeString,"Title2=",        Title[1],     &_72,        &_72 },
  { FALSE, TypeString,"Title3=",        Title[2],     &_72,        &_72 },
  { FALSE, TypeString,"Title4=",        Title[3],     &_72,        &_72 },
  { FALSE, TypeString,"Title5=",        Title[4],     &_72,        &_72 },
  { FALSE, TypeString,"Title6=",        Title[5],     &_72,        &_72 },
  { FALSE, TypeString,"Title7=",        Title[6],     &_72,        &_72 },
  { FALSE, TypeString,"Title8=",        Title[7],     &_72,        &_72 },
  { FALSE, TypeString,"Title9=",        Title[8],     &_72,        &_72 },
  { FALSE, TypeString,"Title10=",       Title[9],     &_72,        &_72 },

  { FALSE, TypeString,"Text1=",         Text[0],      &_72,        &_72 },
  { FALSE, TypeString,"Text2=",         Text[1],      &_72,        &_72 },
  { FALSE, TypeString,"Text3=",         Text[2],      &_72,        &_72 },
  { FALSE, TypeString,"Text4=",         Text[3],      &_72,        &_72 },
  { FALSE, TypeString,"Text5=",         Text[4],      &_72,        &_72 },
  { FALSE, TypeString,"Text6=",         Text[5],      &_72,        &_72 },
  { FALSE, TypeString,"Text7=",         Text[6],      &_72,        &_72 },
  { FALSE, TypeString,"Text8=",         Text[7],      &_72,        &_72 },
  { FALSE, TypeString,"Text9=",         Text[8],      &_72,        &_72 },
  { FALSE, TypeString,"Text10=",        Text[9],      &_72,        &_72 },

  { FALSE, TypeString,"Program=",       Program,      &_72,        &_72 },
  { FALSE, TypeString,"Name=",          Name,         &_72,        &_72 },
  { FALSE, TypeLong,  "Space=",         &Space,       NULL,        NULL },
  { FALSE, TypeInt,   "Delay=",         &Delay,       &_1,         NULL },
  { FALSE, TypeBool,  "DirWarn=",       &DirWarn,     NULL,        NULL   },
  { FALSE, TypeBool,  "DosScreen=",     &DosScreen,   NULL,        NULL   },
  { FALSE, TypeBool,  "BigView=",       &BigView,     NULL,        NULL   },

  { FALSE, TypeByte, "ColorStdFrmNor=",CC(STD,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorStdFrmBol=",CC(STD,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorStdFldNor=",CC(STD,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorStdFldBol=",CC(STD,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorStdSelNor=",CC(STD,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorStdSelBol=",CC(STD,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "ColorMnuFrmNor=",CC(MNU,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorMnuFrmBol=",CC(MNU,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorMnuFldNor=",CC(MNU,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorMnuFldBol=",CC(MNU,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorMnuSelNor=",CC(MNU,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorMnuSelBol=",CC(MNU,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "ColorHlpFrmNor=",CC(HLP,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorHlpFrmBol=",CC(HLP,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorHlpFldNor=",CC(HLP,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorHlpFldBol=",CC(HLP,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorHlpSelNor=",CC(HLP,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorHlpSelBol=",CC(HLP,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "ColorWarFrmNor=",CC(WAR,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorWarFrmBol=",CC(WAR,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorWarFldNor=",CC(WAR,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorWarFldBol=",CC(WAR,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorWarSelNor=",CC(WAR,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorWarSelBol=",CC(WAR,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "ColorDskFrmNor=",CC(DSK,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorDskFrmBol=",CC(DSK,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorDskFldNor=",CC(DSK,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorDskFldBol=",CC(DSK,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "ColorDskSelNor=",CC(DSK,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "ColorDskSelBol=",CC(DSK,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "MonoStdFrmNor=", CM(STD,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoStdFrmBol=", CM(STD,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoStdFldNor=", CM(STD,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoStdFldBol=", CM(STD,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoStdSelNor=", CM(STD,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoStdSelBol=", CM(STD,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "MonoMnuFrmNor=", CM(MNU,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoMnuFrmBol=", CM(MNU,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoMnuFldNor=", CM(MNU,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoMnuFldBol=", CM(MNU,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoMnuSelNor=", CM(MNU,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoMnuSelBol=", CM(MNU,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "MonoHlpFrmNor=", CM(HLP,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoHlpFrmBol=", CM(HLP,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoHlpFldNor=", CM(HLP,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoHlpFldBol=", CM(HLP,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoHlpSelNor=", CM(HLP,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoHlpSelBol=", CM(HLP,SEL_H), &_0,         &_128 },

  { FALSE, TypeByte, "MonoWarFrmNor=", CM(WAR,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoWarFrmBol=", CM(WAR,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoWarFldNor=", CM(WAR,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoWarFldBol=", CM(WAR,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoWarSelNor=", CM(WAR,SEL_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoWarSelBol=", CM(WAR,SEL_N), &_0,         &_128 },

  { FALSE, TypeByte, "MonoDskFrmNor=", CM(DSK,FRM_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoDskFrmBol=", CM(DSK,FRM_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoDskFldNor=", CM(DSK,FLD_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoDskFldBol=", CM(DSK,FLD_H), &_0,         &_128 },
  { FALSE, TypeByte, "MonoDskSelNor=", CM(DSK,SEL_N), &_0,         &_128 },
  { FALSE, TypeByte, "MonoDskSelBol=", CM(DSK,SEL_H), &_0,         &_128 },

  { FALSE, TypeBool, NULL,             NULL,          NULL,        NULL },
  };





static char PolConvr (char c)
{
  char *p = strchr(Mazovia, c);
  return (p) ? Polskie[p-Mazovia] : c;
}




static void SaveDir (void)
{
  OldDrv = getdisk();
  getcwd(OldDir, sizeof(OldDir));
}





static void RestoreDir (void)
{
  setdisk(OldDrv);
  chdir(OldDir);
}






static Boolean WriteStatText (Wind *W, char *txt, int mode)
{
  struct bu { struct bu *nx; char tx[1]; } *b;
  int wid = W->Wid;
  Boolean was = W->Body.Near != NULL;

  switch (mode)
    {
      case -1 : if (! was) return FALSE;
		b = (bu *) W->Body.Near; W->Body.Near = b->nx;
		free(b); break;
      case  0 : if (was) break;
      case +1 : if ((b = (bu *) malloc(sizeof(void *) + wid)) == NULL) return FALSE;
		b->nx = (bu *) W->Body.Near; W->Body.Near = b;
		if (! was) W->Flags.Allocated = TRUE;
		break;
    }
  b = (bu *) W->Body.Near;
  if (b == NULL) W->Flags.Allocated = FALSE;
  else if (txt != NULL)
    {
      int len = strlen(txt);
      if (wid < len) len = wid;
      memset(b->tx, ' ', wid);
      memcpy(b->tx, txt, len);
      MemConvr(b->tx, len);
    }
  SetRefreshWind(W);
  Refresh();
  return TRUE;
}






Boolean WarnQ (char *s, int color)
{
  #ifdef PL
  static char m1[] = " Sytuacja awaryjna! Nie odpowiadaj pochopnie, zajrzyj do instrukcji.";
  static char m2[] = " Wybierz [T] lub [N]. Nie odpowiadaj bez zastanowienia.";
  static char mq[] = "Tak    Nie";
  #else
  static char m1[] = " Troubles! Do not answer without consideration.";
  static char m2[] = " Select [Y] or [N]. Do not answer without consideration.";
  static char mq[] = "Yes    No";
  #endif
  int q;
  WriteStatText(StatWind, ((color==WAR) ? m1 : m2), 1);
  q = Question((ColorBank) color, s, mq);
  WriteStatText(StatWind, NULL, -1);
  return (q == 0);
}






static void WarnM (char *s)
{
  Critic = TRUE;
  #ifdef PL
  WriteStatText(StatWind, " Sytuacja awaryjna! Przykro mi, kontynuowanie instalacji jest bezcelowe.", 1);
  #else
  WriteStatText(StatWind, " Installation failed.", 1);
  #endif
  Question(WAR, s, "Ok");
  WriteStatText(StatWind, NULL, -1);
  Critic = FALSE;
}






static void Restore (void)
{
  SetMouseFun(NULL, 0);
  RestoreBreak();
  SetDesk(FALSE);
  RestoreScreen(OldScr);
}




static void Quit (Boolean q)
{
  static Boolean active = FALSE;

  if (! active)
    {
       active = TRUE;
       if (q)
	 {
	   #ifdef PL
	   WriteStatText(StatWind, " [Tak] spowoduje definitywne przerwanie instalacji, [Nie] oznacza kontynuowanie", 1);
	   q = WarnQ("Zaniechanie instalowania?", WAR);
	   #else
	   WriteStatText(StatWind, " [Yes] = abort, [No] = continue", 1);
	   q = WarnQ("Cancel installation?", WAR);
	   #endif
	   WriteStatText(StatWind, NULL, -1);
	   active = FALSE;
	   if (! q) return;
	 }
       if (Mode & PUTBOOT)
	 {
	   if (! Completed) BootSec->Secret = OldSecret;
	   WriteBoot(*ProgDrv-'A', BootSec);
	 }
       Close();
       Restore();
       exit(0);
    }
}





static void AskForNumber (void)
{
  if ((Mode & PRGNUMB) && ! NumberTaken)
    {
      char buf[PRGNUMLEN+1] = "", *s;
      int i;

      #ifdef PL
      WriteStatText(StatWind, " Przepisz numer programu z etykiety oryginalnej dyskietki (nie pomyl 0 z O)", 1);
      s = GetString(buf, PRGNUMLEN, 1, "Numer", STD);
      #else
      WriteStatText(StatWind, " Read serial number printed on disk label and type it here", 1);
      s = GetString(buf, PRGNUMLEN, 1, "Number", STD);
      #endif
      if (s == NULL) goto bad_num;
      memcpy(PrgNumStr, s, PRGNUMLEN);
      PrgNumStr[PRGNUMLEN] = '\0';
      for (i = 0; i < ArrayCount(PrgNumOrd[0]); i ++)
	{
	  char *p = &s[PrgNumOrd[0][i]],
	       *q = &s[PrgNumOrd[1][i]];
	  *p = *p + *q;
	  *q = *p - *q;
	  *p = *p - *q;
	}
      for (i = 0; i < PRGNUMLEN; i ++)
	{
	  int c = s[i];
	  c = toupper(c) - '0';
	  if (c > 9)
	    {
	      c -= 'A'-('9'+1);
	      if (c < 10) goto bad_num;
	    }
	  if (c < 0 || c > 31) goto bad_num;
	  if (i > 5) c += 32-(16-9+i);
	  c %= 32;
	  s[i] = c;
	}
      BootSec->Secret = 0L;
      for (i = 0; i < PRGNUMLEN/2; i ++)
	{
	  if (s[i] != s[i+6])
	    {
	      bad_num:
	      #ifdef PL
	      WarnM("To nie jest poprawny numer. Przerywam instalowanie.");
	      #else
	      WarnM("Incorrect number. Installation terminated.");
	      #endif
	      Quit(FALSE);
	    }
	  BootSec->Secret = (BootSec->Secret << 5) + s[i];
	}
      if (OldSecret) if (BootSec->Secret != OldSecret) goto bad_num;
      NumberTaken = TRUE;
      WriteStatText(StatWind, NULL, -1);
    }
  if (Mode & PUTBOOT)
    if (! WriteBoot(*ProgDrv-'A', BootSec))
      {
	#ifdef PL
	WarnM("Dyskietka zabezpieczona! Przerywam instalowanie.");
	#else
	WarnM("Disk is write-protected! Installation terminated.");
	#endif
	Quit(FALSE);
      }
}





//------------------------------------------------------------------------






static void CheckSpace (void)
{
  struct dfree sp;
  int drv = getdisk();
  #ifdef PL
  char m[81] = "Niedostatek miejsca na dysku, potrzeba co najmniej ";
  #else
  char m[81] = "Not enough free disk space, need at least ";
  #endif
  char * m1 = m+28;

  ltoa(Space, m+strlen(m), 10);
  strcat(m, " Kb");

  getdfree(drv+1, &sp);
  if (sp.df_sclus == 0xFFFF)
    {
      #ifdef PL
      strcpy(m1, ". Kontynuujemy?");
      #else
      strcpy(m1, ". Continue?");
      #endif
      if (! WarnQ(m, WAR)) Quit(FALSE);
    }
  else if (((dword) sp.df_avail * sp.df_bsec)/(1024/sp.df_sclus) < Space)
    {
      WarnM(m);
      Quit(FALSE);
    }
}






static void Wait (void)
{
  if (Delay) delay(Delay);
  Refresh();
}




static void GetAnyKey (void)
{
  #ifdef PL
  WriteStatText(StatWind, " Przeczytaj tekst (czekam na dowolny klawisz)", 1);
  #else
  WriteStatText(StatWind, " Read above, then press any key", 1);
  #endif
  ClearKeyboard();
  GetKey();
  WriteStatText(StatWind, NULL, -1);
}





static void SetPol (void)
{
  memcpy(Polskie, Name, 18);
}




static void HideTitle (void)
{
  int i;
  for (i = 0; i < ArrayCount(Title); i ++) Title[i][0] = '\0';
  if (TitleWind != NULL)
    {
      CloseWind(TitleWind);
      TitleWind = NULL;
    }
  Refresh();
}




static void HideText (void)
{
  int i;
  for (i = 0; i < ArrayCount(Text); i ++) Text[i][0] = '\0';
  if (TextWind != NULL)
    {
      CloseWind(TextWind);
      TextWind = NULL;
    }
  Refresh();
}




static void ShowTitleR (void)
{
  ShowTitle();
  Refresh();
}





static void ShowTitle (void)
{
  int i, wid, hig;

  wid = 0; hig = 0;
  for (i = 0; i < ArrayCount(Title); i ++)
    {
      int l = strlen(Title[i]);
      if (l) hig = i+1;
      wid = Max(wid, l+2);
    }
  if (hig > 0)
    {
      if (TitleWind != NULL) CloseWind(TitleWind);
      i = ((MaxScrHig / 2) - (hig + 1) - ZoomTop) / 3;
      TitleWind = MakeWind(0, ZoomTop+i, wid+1, ZoomTop+i+hig+1, HLP);
      MoveWindHorz(TitleWind, -1);
      TitleWind->Flags.Movable = TRUE;
      for (i = 0; i < hig; i ++)
	{
	  WriteWindTextConvr(TitleWind, 1, i, Title[i]);
	}
      ShowWind(TitleWind);
    }
}




static void ShowText (void)
{
  int i, wid, hig;

  wid = 0; hig = 0;
  for (i = 0; i < ArrayCount(Text); i ++)
    {
      int l = strlen(Text[i]);
      if (l) hig = i+1;
      wid = Max(wid, l+2);
    }
  if (hig > 0)
    {
      if (TextWind != NULL) CloseWind(TextWind);
      i = ((MaxScrHig / 2) - (hig + 1) - (MaxScrHig-1-ZoomBot)) / 3;
      TextWind = MakeWind(0, ZoomBot-i-(hig+1), wid+1, ZoomBot-i, MNU);
      MoveWindHorz(TextWind, -1);
      TextWind->Flags.Movable = TRUE;
      for (i = 0; i < hig; i ++)
	{
	  WriteWindTextConvr(TextWind, 1, i, Text[i]);
	}
      ShowWind(TextWind);
    }
}





static void ShowTextR (void)
{
  ShowText();
  Refresh();
}




static void Menu (void)
{
  int i;
  char * t = strdup(Name);
  if (t)
    {
      StrConvr(t);
      ShowText();
      TextWind->ShowProc = ShowMenuChar;
      TextWind->ReptProc = RepaintMenuWind;
      SetTitle(TextWind, Name, -1);
      #ifdef PL
      WriteStatText(StatWind, " Wybierz klawiszami [] i [], zaakceptuj przy pomocy [Enter]", 1);
      #else
      WriteStatText(StatWind, " Select with [], [], confirm with [Enter]", 1);
      #endif
      while ((i = MenuChoice(TextWind)) < 0) /* nop */ ;
      memcpy(Name, Text[i], sizeof(Name));
      WriteStatText(StatWind, NULL, -1);
      CloseWind(TextWind);
      TextWind = NULL;
      free(t);
    }
}






static void CheckFile (const char * pat)
{
  while (! CFILE::Exist(pat))
    {
      #ifdef PL
      static char wrong[] = "To nie jest odpowiednia dyskietka!";
      #else
      static char wrong[] = "Incorrect disk!";
      #endif
      Wind * w = MakeWind(0, 0, strlen(wrong) + 5, 4, WAR);
      WriteWindText(w, 2, 1, wrong);
      MoveWind(w, -1, -1);
      w->Flags.Movable = TRUE;
      ShowWind(w);
      GetAnyKey();
      CloseWind(w);
      Refresh();
    }
}






static void Close (void)
{
  if (OFile)
    {
      delete OFile;
      OFile = NULL;
    }
}





static void Open (void)
{
  char nam[MAXFILE+MAXEXT], ext[MAXEXT];

  AskForNumber();
  Close();
  fnsplit(strupr(Name), NULL, NULL, nam, ext);
  strcat(nam, ext);

  OFile = new CFILE(nam, WRI);

  if (OFile == NULL)
    {
      #ifdef PL
      WarnM("Brak RAM do otwarcia pliku! Przerywam instalowanie.");
      #else
      WarnM("Not enough RAM to open file! Installation aborted.");
      #endif
      Quit(FALSE);
    }
  if (OFile->Error)
    {
      #ifdef PL
      WarnM("Niepowodzenie otwarcia pliku! Przerywam instalowanie.");
      #else
      WarnM("Open failed! Installation aborted.");
      #endif
      Quit(FALSE);
    }
}





		CFILE * file;
static		char	bar[] = "²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²²°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°";
static		long	size;
static		Wind *	wind = NULL;





static void SetEcho (CFILE * cf)
{
  file = cf;
  if (wind)
    {
      Image(wind) = bar;
      SetRefreshWind(wind);
      Refresh();
      CloseWind(wind);
      wind = NULL;
    }
  else
    {
      size = cf->Size();
      wind = CreateWind(StatWind->Rgt+1 - sizeof(bar)/2, StatWind->Top, StatWind->Rgt, StatWind->Top, MNU, FALSE, Boolean FALSE);
      Image(wind) = bar+sizeof(bar)/2;
      ShowWind(wind);
      Refresh();
    }
}




static void Echo (void)
{
  Image(wind) = bar+sizeof(bar)/2-(word)(file->Mark() * (sizeof(bar)/2) / size);
  SetRefreshWind(wind);
  Refresh();
}






static void Copy0 (Boolean pol)
{
  extern int CheckBuff(char c);
  char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE+MAXEXT], ext[MAXEXT];
  long siz;
  int i;
  word len /*fd, ft*/;
  Boolean ok = FALSE;
  byte * Buf;

  AskForNumber();
  Buf = new byte[BUF_SIZ];
  if (Buf == NULL)
    {
      #ifdef PL
      WarnM("Brak RAM do kopiowania! Przerywam instalowanie.");
      #else
      WarnM("No enough RAM for copy! Installation aborted.");
      #endif
      Quit(FALSE);
    }
  i = fnsplit(strupr(Name), drv, dir, nam, ext);
  #ifdef PL
  strcpy(pat, " Kopiowanie pliku ");
  #else
  strcpy(pat, " Copying file ");
  #endif
  strcat(pat, Name);
  WriteStatText(StatWind, pat, 1);
  fnmerge(pat,	(i & DRIVE)     ? drv : ProgDrv,
		(i & DIRECTORY) ? dir : ProgDir,
		nam, ext);

  CheckFile(pat);
  CFILE IFile(pat);
  if (IFile.Error == 0)
    {
      Boolean single = (OFile == NULL);
      siz = IFile.Size();
      if (single)
	{
	  OFile = new CFILE(strcat(nam, ext), WRI);
	}
      if (OFile->Error == 0)
	{
	  SetEcho(&IFile);
	  do
	    {
	      word n;
	      Echo();
	      len = IFile.Read(Buf, BUF_SIZ);
	      if (IFile.Error) break;
	      if (len)
		{
		  char * p, * q;
		  int i;
		  if (Mode & ENCRYPT)
		    {
		      for (i = 0; i < BUF_SIZ; i += 4)
			{
			  *(long *)(&Buf[i]) ^= Mask;
			  asm rol word ptr Mask+0,1
			  asm ror word ptr Mask+2,1
			}
		    }
		  if (pol)
		    for (q = (p = Buf) + len; p < q; p ++) *p = PolConvr(*p);
		  for (q = (p = Buf) + len; p < q; p ++) *p = CopyHook(*p);
		  siz -= (n = OFile->Write(Buf, len));
		  if (OFile->Error || n != len) break;
		}
	    }
	  while (len == BUF_SIZ);
	  Echo();
	  SetEcho(NULL);
	  OFile->Flush();
	  OFile->SetTime(IFile.Time());
	  ok = (siz == 0);
	}
      if (single && OFile) Close();
    }

  delete[] Buf;

  if (! ok)
    {
      #ifdef PL
      WarnM("Kopiowanie pliku jest niewykonalne! Przerywam instalowanie.");
      #else
      WarnM("Unable to copy file! Installation aborted.");
      #endif
      Quit(FALSE);
    }
  WriteStatText(StatWind, NULL, -1);
}





static void Copy (void)
{
  Copy0(FALSE);
}




static void CopyP (void)
{
  Copy0(TRUE);
}



ulong origsize, compsize;  /* global */

static uchar buffer[DICSIZ];



static void Unpack (void)
{
  char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE+MAXEXT], ext[MAXEXT];
  int i;
  Boolean ok = FALSE;

  AskForNumber();
  i = fnsplit(strupr(Name), drv, dir, nam, ext);
  #ifdef PL
  strcpy(pat, " Kopiowanie pliku ");
  #else
  strcpy(pat, " Copying file ");
  #endif
  strcat(pat, Name);
  WriteStatText(StatWind, pat, 1);
  fnmerge(pat,	(i & DRIVE)     ? drv : ProgDrv,
		(i & DIRECTORY) ? dir : ProgDir,
		nam, ext);

  CheckFile(pat);
  CFILE IFile(pat);

  if (IFile.Error == 0)
    {
      Boolean single = (OFile == NULL);
      if (single)
	{
	  OFile = new CFILE(strcat(nam, ext), WRI);
	}
      if (OFile->Error == 0)
	{
	  SetEcho(&IFile);
	  IFile.Read(&origsize, 2*sizeof(origsize));
	  decode_start();

	  while (origsize != 0)
	    {
	      uint n = (uint)((origsize > DICSIZ) ? DICSIZ : origsize);
	      decode(n, buffer);
	      OFile->Write(buffer, n);
	      Echo();
	      origsize -= n;
	    }
	  ok = origsize == 0;
	  Echo();
	  SetEcho(NULL);
	  OFile->Flush();
	  OFile->SetTime(IFile.Time());
	  if (ok) ok = (IFile.Error == 0 && OFile->Error == 0);
	}
      if (single && OFile) Close();
    }

  if (! ok)
    {
      #ifdef PL
      WarnM("Kopiowanie pliku jest niewykonalne! Przerywam instalowanie.");
      #else
      WarnM("Unable to copy file! Installation aborted.");
      #endif
      Quit(FALSE);
    }
  WriteStatText(StatWind, NULL, -1);
}




static void Command (void)
{
  char com[80];
  int rc;

  #ifdef PL
  strcpy(com, " Wykonanie komendy DOS-u \"");
  #else
  strcpy(com, " Executing DOS command \"");
  #endif
  strcat(com, strupr(Name));
  strcat(com, "\"");
  WriteStatText(StatWind, com, 1);
  if (DosScreen) RestoreScreen(OldScr);
  rc = system(Name);
  if (DosScreen) OldScr = SaveScreen();
  SetRefreshDesk();
  Refresh();
  if (rc != 0)
    {
      #ifdef PL
      WarnM("Niepowodzenie wykonania komendy DOS-u! Przerywam instalowanie.");
      #else
      WarnM("DOS command failed! Installation aborted.");
      #endif
      Quit(FALSE);
    }
  WriteStatText(StatWind, NULL, -1);
}






static void Erase (void)
{
  char m[80];

  if (IsFile(Name))
    {
      #ifdef PL
      strcpy(m, " Usuwanie pliku ");
      #else
      strcpy(m, " Erasing file ");
      #endif
      strcat(m, strupr(Name));
      WriteStatText(StatWind, m, 1);
      if (unlink(Name) != 0)
	{
	  #ifdef PL
	  WarnM("Usuwanie pliku jest niewykonalne! Przerywam instalowanie.");
	  #else
	  WarnM("Erasing failed! Installation terminated.");
	  #endif
	  Quit(FALSE);
	}
      WriteStatText(StatWind, NULL, -1);
    }
}





static Boolean MakeAnyDir (const char *s)
{
  char pat[MAXPATH], drv[MAXDRIVE], dir[MAXDIR];

  fnsplit(s, drv, dir, NULL, NULL);
  if (strlen(dir) > 1)
    {
      char cur[MAXPATH];
      int i;
      getcwd(cur, sizeof(cur));
      fnmerge(pat, drv, dir, NULL, NULL);
      i = chdir(CutPathTrailer(pat));
      chdir(cur);
      if (i != 0)
	{
	  if (strcmp(s, pat) == 0) return FALSE;
	  if (! MakeAnyDir(pat)) return FALSE;
	}
    }
  return (mkdir(s) == 0);
}





static void MakeDir (void)
{
  char m[80];
  int i;

  AskForNumber();
  CutTrailingBS(strupr(Name));
  strcpy(m, CREDIR_TXT);
  strcat(m, Name);
  WriteStatText(StatWind, m, 1);
  i = fnsplit(Name, NULL, NULL, NULL, NULL);
  if (i & DRIVE) setdisk(*Name - 'A');
  if (chdir(Name) == 0)
    {
      if (DirWarn)
	{
	  if (! WarnQ(DIRWARN_TXT, WAR)) Quit(FALSE);
	}
    }
  else
    {
      if (! MakeAnyDir(Name))
	{
	  WarnM(MKDIRFAIL_TXT);
	  Quit(FALSE);
	}
      else chdir(Name);
    }
  WriteStatText(StatWind, NULL, -1);
}




static void ChangeDir (void)
{
  char m[80];
  int i;

  CutTrailingBS(strupr(Name));
  strcpy(m, CHDIR_TXT);
  strcat(m, Name);
  WriteStatText(StatWind, m, 1);
  i = fnsplit(Name, NULL, NULL, NULL, NULL);
  if (i & DRIVE) setdisk(*Name - 'A');
  if (chdir(Name) != 0)
    {
      WarnM(CHDIRFAIL_TXT);
      Quit(FALSE);
    }
  WriteStatText(StatWind, NULL, -1);
}





static void ChgDsk (int d)
{
  char m[81];
  setdisk(d);
  if (getdisk() != d)
    {
      strcpy(m, CHDRV_TXT);
      m[strlen(m)-1] = *Name;
      strcat(m, CHDRVFAIL_TXT);
      WarnM(m);
      Quit(FALSE);
    }
}



static void GetDrive (void)
{
  char m[80], q[80];
  word md, dd = getdisk(), d;
  Wind *w;
  int ml, ql, i, n, cho;

  AskForNumber();
  strcpy(m, DRVSEL_TXT);
  strcat(m, Program);
  WriteStatText(StatWind, m, 1);
  strcpy(q, " ");
  _dos_setdrive(0, &md);
  n = 0;
  cho = 0;
  for (i = DISK_C; i < md; i ++)
    {
      setdisk(i);
      d = getdisk();
      setdisk(dd);
      if (d == i && i != *ProgDrv-'A')
	{
	  strcat(q, "A ");
	  q[strlen(q)-2] += i;
	  if (d == dd) cho = n;
	  ++ n;
	}
    }
  if (n == 0)
    {
      strcpy(m, NODRV_TXT);
      strcat(m, Program);
      WarnM(m);
      Quit(FALSE);
    }
  strcpy(m, SELDRVP_TXT);
  strcat(m, Program);
  ml = strlen(m);
  ql = strlen(q);
  i = Max(ml, ql) + 4;

  SetRefreshWind(TopWind());
  w = MakeWind(0, 0, i-1, 4, MNU);
  w->Flags.Movable = TRUE;
  CenterWind(w);
  WriteWindTextConvr(w, i/2 - ml/2 - 1, 0, m);
  WriteWindTextConvr(w, i/2 - ql/2 - 1, 2, q);
  w->Y = 2;
  w->Vp = cho;
  ShowWind(w);
  while ((i = HorzChoice(w)) < 0) /* nop */ ;
  CloseWind(w);
  if (Name[0] == '\0') Name[1] = '\0';
  ChgDsk((*Name = q[2*i+1]) - 'A');
  WriteStatText(StatWind, NULL, -1);
}






static void GetDir (void)
{
  char m[80];
  char drv[MAXDRIVE], dir[MAXDIR], nam[MAXFILE], ext[MAXEXT];
  word md, dd = getdisk(), d;
  int i, n = -1, n0 = -1;

  AskForNumber();

  _dos_setdrive(0, &md);	// drive count
  for (i = DISK_C; i < md; i ++)
    {
      setdisk(i);
      d = getdisk();
      setdisk(dd);
      if (d == i && i != *ProgDrv-'A')
	{
	  if (d == dd) n = d;
	  if (n0 < 0) n0 = d;
	}
    }
  if (n < 0) n = n0;
  if (n < 0)
    {
      strcpy(m, NODRV_TXT);
      strcat(m, Program);
      WarnM(m);
      Quit(FALSE);
    }

  strcpy(m, SELDIR_TXT);
  strcat(m, Program);
  WriteStatText(StatWind, m, 1);

  if (Name[1] == ':') Name[0] = 'A' + n;

  while (GetString(strupr(Name), MAXDIR, 1, DESTDIR_TXT, STD) == NULL)
    /* nop */;
  fnsplit(Name, drv, dir, nam, ext);
  fnmerge(Name, drv, dir, nam, ext);
  CutTrailingBS(Name);
  ChgDsk(*Name - 'A');
  WriteStatText(StatWind, NULL, -1);
}





static void InsertPath (void)
{
  FILE *hf0 = NULL, *hf1 = NULL;
  char oldC[MAXDIR], line[512], pat[] = "PATH=";
  int dd = getdisk();
  int i, n;
  Boolean ok = FALSE, pa = FALSE;

  AskForNumber();
  strupr(Name);
  ///--- locate AUTOEXEC.BAT
  setdisk(DISK_C);
  if (getdisk() != DISK_C)
    {
      WarnM(NOCDRV_TXT);
      Quit(FALSE);
    }
  getcwd(oldC, sizeof(oldC));
  chdir("\\");
  if (!IsFile("AUTOEXEC.BAT"))
    if ((hf1 = fopen("AUTOEXEC.BAT", "wt")) == NULL) goto ip_exit;
    else
      {
	if (fputs("@PATH ", hf1) == EOF) goto ip_exit;
	if (fputs(Name, hf1) == EOF) goto ip_exit;
	if (fputs("\n", hf1) == EOF) goto ip_exit;
	ok = TRUE; goto ip_exit;
      }

  ///--- search for PATH=...
  hf0 = fopen("AUTOEXEC.BAT", "rt");
  if (hf0 == NULL) goto ip_exit;
  strupr(Name);
  while (! ok)
    {
      if (fgets(line, sizeof(line), hf0) == NULL) break;
      strupr(line);
      n = strlen(line);
      for (i = 0; i < n; i ++) if (! IsWhite(line[i])) break;
      if (line[i] == '@') ++ i;
      if (memicmp(line+i, "SET", 3) == 0 && IsWhite(line[i+3]))
	{
	  pat[4] = '=';
	  for (i += 4; i < n; i ++) if (! IsWhite(line[i])) break;
	}
      else pat[4] = ' ';
      if (memicmp(line+i, pat, 5) == 0)
	{
	  char *p = strstr(line+i+5, Name);
	  pa = TRUE;
	  if (p != NULL) if (memchr("; \t\n", p[strlen(Name)], 5) != NULL)
	    {
	      ok = TRUE;
	      goto ip_exit;
	    }
	}
    }
  fclose(hf0);

  strcpy(line, PATHINF_TXT);
  strcat(line, Program);
  WriteStatText(StatWind, line, 1);
  i = Question(MNU, AEASK1, AEASK_MNU);
  WriteStatText(StatWind, NULL, -1);
  if (i != 0)
    {
      strcpy(line, (pa) ? AEASK2
			: AEASK3);
      strcat(line, Name);
      if (strlen(line) > 72) strcpy(line+66, "...");
      if (pa) strcat(line, ";");
      strcat(line, " ?");
      if (WarnQ(line, WAR))
	{
	  ok = TRUE;
	  goto ip_exit;
	}
      WarnM(AENAK_TXT);
      Quit(FALSE);
    }
  if (!IsWritable("AUTOEXEC.BAT"))
    {
      WarnM(AEPROT_TXT);
      Quit(FALSE);
    }

  if (IsFile("AUTOEXEC.BAK")) if (unlink("AUTOEXEC.BAK") != 0) goto ip_exit;
  if (rename("AUTOEXEC.BAT", "AUTOEXEC.BAK") == 0)
    {
      if ((hf0 = fopen("AUTOEXEC.BAK", "rt")) != NULL)
	{
	  if ((hf1 = fopen("AUTOEXEC.BAT", "wt")) != NULL)
	    {
	      if (! pa)
		{
		  if (fputs("@PATH ", hf1) == EOF) goto ip_exit;
		  if (fputs(Name, hf1) == EOF) goto ip_exit;
		  if (fputs("\n", hf1) == EOF) goto ip_exit;
		}
	      while (TRUE)
		{
		  if (fgets(line, sizeof(line), hf0) == NULL) break;
		  if (pa)
		    {
		      n = strlen(line);
		      for (i = 0; i < n; i ++) if (! IsWhite(line[i])) break;
		      if (line[i] == '@') ++ i;
		      if (memicmp(line+i, "SET", 3) == 0 && IsWhite(line[i+3]))
			{
			  pat[4] = '=';
			  for (i += 4; i < n; i ++) if (! IsWhite(line[i])) break;
			}
		      else pat[4] = ' ';
		      if (memicmp(line+i, pat, 5) == 0)
			{
			  if (strstr(line+i+5, Name) == NULL)
			    {
			      i += 5;
			      if (fputs("PATH ", hf1) == EOF) goto ip_exit;
			      if (fputs(Name, hf1) == EOF) goto ip_exit;
			      if (fputs(";", hf1) == EOF) goto ip_exit;
			      if (fputs(line+i, hf1) == EOF) goto ip_exit;
			      continue;
			    }
			}
		    }
		  if (fputs(line, hf1) == EOF) goto ip_exit;
		}
	      ok = TRUE;
	    }
	}
    }
  ip_exit:
  if (hf0 != NULL) fclose(hf0);
  if (hf1 != NULL) fclose(hf1);
  chdir(oldC);
  setdisk(dd);
  if (! ok)
    {
      WarnM(AEFAIL_TXT);
      Quit(FALSE);
    }
}






static void MakeBatch (void)
{
  FILE *hf = NULL;
  char m[MAXPATH+10];

  AskForNumber();
  strcpy(m, TMPFILE_TXT);
  strcat(m, strupr(Name));
  WriteStatText(StatWind, m, 1);
  hf = fopen(Name, "wt");
  if (hf == NULL)
    {
      bat_error:
      strcpy(m, TMPFAIL_TXT);
      strcat(m, Name);
      WarnM(m);
      fclose(hf);
      Quit(FALSE);
    }

  if (fputs("@ECHO OFF\n", hf) == EOF) goto bat_error;
  strcpy(m, "A:\n");
  *m += getdisk();
  if (fputs(m, hf) == EOF) goto bat_error;
  strcpy(m, "CD ");
  getcwd(m + 3, sizeof(m) - 4);
  strcat(m, "\n");
  if (fputs(m, hf) == EOF) goto bat_error;
  strcpy(m, Program);
  strcat(m, "\n");
  if (fputs(m, hf) == EOF) goto bat_error;
  fclose(hf);
  WriteStatText(StatWind, NULL, -1);
}





static void IfProc (void)
{
  char * p = strdup(Name);
  if (p)
    {
      StrConvr(p);
      Inf3.Addr = (WarnQ(p, MNU)) ? Inf3Adr : NULL;
      free(p);
    }
}




static void ElseProc (void)
{
  Inf3.Addr = (Inf3.Addr == NULL) ? Inf3Adr : NULL;
}




static void EndIfProc (void)
{
  Inf3.Addr = Inf3Adr;
}





static void IniChain (void)
{
  strcpy(IniName, strupr(Name));
  Chain = TRUE;
}





static void ViewText (void)
{
  static EditStat ES = {
			 //FALSE,	// EditMode
			 TRUE,		// InsMode;
			 FALSE,		// Updated;
			 FALSE,		// Valid;
			 0,		// LineCount;
			 0, 0,		// Line, Column;
			 0, 0,		// S, Y;
			 BUF_SIZ,	// BufSiz;
			 128,		// Wrap;
			 NULL,		// Buff;
			 NULL,		// Tail;
			 NULL,		// Here;
			 {NULL},	// LineTab[MaxSc_Hig];
			 {0},		// LgthTab[MaxSc_Hig];
			 //NULL,	// *ToFind;
			 NULL,		// (*XlatProc)(int);
			 NULL,		// (*StatProc)(Wind *);
			 NULL,		// *User;
		       };
  int f, len;
  char m[256];

  ClearKeyboard();
  ES.Buff = new byte[BUF_SIZ];
  if (ES.Buff == NULL)
    {
      WarnM(NOTXTRAM_TXT);
      Quit(FALSE);
    }
  else ES.Tail = ES.Buff;
  if ((f = open(Name, O_RDONLY | O_TEXT)) < 0)
    {
      bad_view:
      WarnM(NOVFILE_TXT);
      Quit(FALSE);
    }

  len = read(f, ES.Buff, MAXINT);
  close(f);
  if (len <= 0) goto bad_view;

  strcpy(m, VIEWREAD_TXT);
  strcat(m, strupr(Name));
  strcat(m, VIEWKEYS_TXT);
  WriteStatText(StatWind, m, 1);
  if (BigView)
    {
      ViewWind = CreateWind(0, ZoomTop, MaxScrWid-1, ZoomBot, STD, FALSE, FALSE);
    }
  else
    {
      ViewWind = CreateWind(0, ZoomBot/2, MaxScrWid-1, ZoomBot, STD, FALSE, TRUE);
      ViewWind->Flags.Zoomable = TRUE;
      ViewWind->Flags.Movable = TRUE;
      ViewWind->Flags.VSizeable = TRUE;
      ViewWind->Flags.HSizeable = TRUE;
      SetTitle(ViewWind, Name, -1);
    }
  ViewWind->ShowProc = ShowEditChar;
  ViewWind->ReptProc = EditReptProc;
  ViewWind->Body.Near = &ES;
  ViewWind->Body.Near = &ES;
  ES.Tail = ES.Buff+len;
// ???  ES.BufSiz = MAXINT;
  EditInit(ViewWind);
  ShowWind(ViewWind);
  while (TRUE)
    {
      if (GetKey() == Esc) break;
      EditKey(ViewWind);
    }
  CloseWind(ViewWind);
  ViewWind = NULL;
  delete[] ES.Buff;
  WriteStatText(StatWind, NULL, -1);
}



//------------------------------------------------------




static word ShowStatChar (void)
{
  _SI += sizeof(void *);
  asm	add	SI,[BX].Body.Near
  asm	mov	AL,[SI]
  asm	mov	AH,[BX].Color+FLD_N
  return _AX;
}





static void MyIdle (void)
{
  if (OldMono != Mono)
    {
      SetRefreshDesk();
      OldMono = Mono;
    }
}







static void Init (void)
{
  //// preserve screen image
  OldScr = SaveScreen();

  //// load setup
  GetScrSiz();

  //// set desk, trap vectors
  DeskWind = SetDesk(TRUE);
  DisableBreak();
  SetMouseFun(StdMouseFun, MouseAllFlags);
}






static Keys MyKey (Keys k)
{
  switch (k)
    {
      case F6        : Mono = ! Mono; break;
      case F9        : ProtectTime = 0L; k = NoKey; break;
      case Esc       :
      case F10       :
      case MouseRight: if (! Critic && TopWind() != ViewWind)
			 {
			   Critic = TRUE;
			   Quit(TRUE);
			   Critic = FALSE;
			   k = NoKey;
			 }
		       break;
    }
  return WindKeyProc(k);
}






//------------------------------------------------------------------------


void main (int i, char **arg)
{
  word ShowWindChar(void);
  //i = coreleft();
  //i = (int) farcoreleft();

  printf("\n%s   version %d\.%02d%s"
  #ifdef EVA
				   "ú"
  #endif
				     "\n", Signature, MAJOR, MINOR, CLONE_SIG);
  printf("Copyright (c) 1993 Janusz B. Wi$niewski\n");
  //cout<<"\n"<<Signature<<"   version "<<MAJOR<<"."<<MINOR<<CLONE_SIG<<"\n";
  //cout<<"Copyright (c) 1993 Janusz B. Wi$niewski\n";

  SaveDir();
  i = fnsplit(arg[0], ProgDrv, ProgDir, ProgNam, NULL);
  if ((i & DRIVE) == 0)
    {
      strcpy(ProgDrv, "A:");
      *ProgDrv += getdisk();
    }
  if ((i & DIRECTORY) == 0)
    {
      *ProgDir = '\\';
      getcurdir(*ProgDrv - 'A' + 1, ProgDir+1);
    }

  *ProgDrv = toupper(*ProgDrv);
  fnmerge(IniName, ProgDrv, ProgDir, ProgNam, ".INF");

  Init();
  OldMono = Mono;
  SetIdleProc(MyIdle);
  SetKeyProc(MyKey);
  SetConvrProc(PolConvr);
  SetSaveProc(MakeWorm);
  EOL_Ch = ' ';

//  SetHelpProc(MyHelp);

  ///// Make Status Window
  StatWind = CreateWind(0, MaxScrHig-1, MaxScrWid-1, MaxScrHig-1, HLP, FALSE, FALSE);
  StatWind->ShowProc = ShowStatChar;
  -- ZoomBot;
  ShowWind(StatWind);

  ClearKeyboard();
  strcpy(Title[0], Signature);
  WriteStatText(StatWind, "", 0);

  if ((BootSec = ReadBoot(*ProgDrv-'A')) == NULL)
    {
      WarnM(FLOPFAIL_TXT);
      Quit(FALSE);
    }
  OldSecret = BootSec->Secret;
  Mode = (CheckBoot(BootSec)) ? 0 : BootSec->BootFlags;
  if ((Mode & (PRGNUMB | PUTBOOT)) && OldSecret) Installed = TRUE;
  Mask = BootSec->Serial;
  if (Installed)
    {
      #ifdef ADD
	ShowLaw();
      #endif
      if (WarnQ(TWICE_TXT, WAR))
	Quit(FALSE);
    }
  HideText();
  if (! InitHook(*ProgDrv-'A'))
    {
      WarnM(FLOPFAIL_TXT);
      Quit(FALSE);
    }
  do
    {
      Chain = FALSE;
      LoadIni(IniName, InfTab, TRUE);
      Mode &= ~PUTBOOT;
    }
  while (Chain);
  Completed = TRUE;
  Quit(FALSE);
}
