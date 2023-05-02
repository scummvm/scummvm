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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/t2d/expr.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/globvar.h"


#define MAX_VISEMA  1000

#define MIN_VISEMA_LENG 70

#define RandomInt(Max) (rand()%(Max+1))
#define RandomFloat(Max) ((float)rand()/MAX_RAND)*Max)

#define VIS_DEFAULT_TIME    -10
#define VIS_NULL            -999

#define WAVE_TOLLERANCE     15

#define MAXCICLES 10000
#define CICLECONTROL(CV) { CV++; if (CV>MAXCICLES) break; }

#define VIS_PAUSE_VAR   30

//#define PROVAMODE

namespace Watchmaker {

struct SilbsStruct {
	const char *Text;
	int  Res[3];
} Silbs[] = {
	{ " ",      {14, -1, -1} },
	{ ",",      {14, -1, -1} },
	{ "'",      {-1, -1, -1} },
	{ ";",      {14, -1, -1} },
	{ ".",       {0, 14,  0} },
	{ ":",       {0, -1, -1} },
	{ "?",       {0, 14,  0} },
	{ "!",       {0, 14,  0} },
	{ "-",      {14, -1, -1} },
	{ "<",      {-1, -1, -1} },
	{ ">",      {-1, -1, -1} },
	{ "a",      {13, -1, -1} },
	{ "ae",     {11, -1, -1} },
	{ "b",       {1, -1, -1} },
	{ "bb",      {1, -1, -1} },
	{ "c",       {9, -1, -1} },
	{ "cc",      {9, -1, -1} },
	{ "ch",      {8, -1, -1} },
	{ "ck",      {9, -1, -1} },
	{ "ce",      {7, -1, -1} },
	{ "ci",      {7, -1, -1} },
	{ "d",       {7, -1, -1} },
	{ "dd",      {7, -1, -1} },
	{ "det",    {10,  7, -1} },
	{ "ded",    {10,  7, -1} },
	{ "e",      {11, -1, -1} },
	{ "5e",     {11, -1, -1} },
	{ "ea",     {10, -1, -1} },
	{ "ee",     {10, -1, -1} },
	{ "e|",     {10, -1, -1} },
	{ "$e|",    {-1, -1, -1} },
	{ "ed|",     {7, -1, -1} },
	{ "en|",     {7, -1, -1} },
	{ "et|",     {7, -1, -1} },
	{ "ened|",   {7, -1, -1} },
	{ "f",       {4, -1, -1} },
	{ "ff",      {4, -1, -1} },
	{ "g",       {9, -1, -1} },
	{ "gp",      {1, -1, -1} },
	{ "gb",      {1, -1, -1} },
	{ "gm",      {1, -1, -1} },
	{ "gg",      {9, -1, -1} },
	{ "gh",     {-1, -1, -1} },
	{ "h",      {-1, -1, -1} },
	{ "i",      {10, -1, -1} },
	{ "j",       {8, -1, -1} },
	{ "ja",      {8, -1, -1} },
	{ "je",      {8, -1, -1} },
	{ "ji",      {8, -1, -1} },
	{ "jo",      {8, -1, -1} },
	{ "ju",      {8, -1, -1} },
	{ "k",       {9, -1, -1} },
	{ "kk",      {9, -1, -1} },
	{ "l",       {6, -1, -1} },
	{ "ll",      {6, -1, -1} },
	{ "m",       {1, -1, -1} },
	{ "mm",      {1, -1, -1} },
	{ "n",       {7, -1, -1} },
	{ "nn",      {7, -1, -1} },
	{ "nd",      {7, -1, -1} },
	{ "ng",      {9, -1, -1} },
	{ "nt",      {9, -1, -1} },
	{ "ntr",     {9, -1, -1} },
	{ "nce|",    {9, -1, -1} },
	{ "o",      {12, -1, -1} },
	{ "oo",      {2,  2, -1} },
	{ "ou",     {12, -1, -1} },
	{ "or",     {13, -1, -1} },
	{ "ou|",     {2, -1, -1} },
	{ "one",     {7, 11,  1} },
	{ "oul",     {2, -1, -1} },
	{ "p",       {1, -1, -1} },
	{ "pp",      {1, -1, -1} },
	{ "q",       {9,  2, -1} },
	{ "qq",      {9,  2, -1} },
	{ "rr",      {3, -1, -1} },
	{ "|r",      {3, -1, -1} },
	{ "%r",      {7, -1, -1} },
	{ "$r�",   {3, -1, -1} },
	{ "�r%",  {-1, -1, -1} },
	{ "s",       {7, -1, -1} },
	{ "ss",      {7, -1, -1} },
	{ "si",      {8, -1, -1} },
	{ "sh",      {8, -1, -1} },
	{ "some",    {2, 11,  7} },
	{ "t",       {7, -1, -1} },
	{ "tt",      {7, -1, -1} },
	{ "th",      {5, -1, -1} },
	{ "ted",    {10,  7, -1} },
	{ "tet",    {10,  7, -1} },
	{ "tp",      {1, -1, -1} },
	{ "tm",      {1, -1, -1} },
	{ "tb",      {1, -1, -1} },
	{ "u",      {11, -1, -1} },
	{ "uh",      {9, -1, -1} },
	{ "v",       {4, -1, -1} },
	{ "vv",      {4, -1, -1} },
	{ "w",       {2, -1, -1} },
	{ "ww",      {2, -1, -1} },
	{ "wa",      {2, 12, -1} },
	{ "x",       {7, -1, -1} },
	{ "xx",      {7, -1, -1} },
	{ "y",       {9, -1, -1} },
	{ "y|",      {8, -1, -1} },
	{ "z",       {7, -1, -1} },
	{ "zz",      {7, -1, -1} }

};

//Spiegazione caratteri:
// � vocale
// $ consonante
// % qualcosa
// | fine parola
// i numeri all'inizio o alla fine indicano i visemi precedenti o successivi
// La priorit� � data dalla posizione, il piu in alto viene scelto

//Il visema 14 � un random fra la pausa con occhi aperti (0) e quella con occhi chiusi (14)
//Il visema 15 � un random fra la pausa veloce con occhi aperti (11) e quella con occhi chiusi (15)

#define NumSilbs 106

struct VisemaBufStruct {
	int Visema;     //Id del visema
	int TimeLeng;   //Lunghezza in milliseconda
};

struct VisemaBufStruct VisemaBuf[MAX_VISEMA];
int risFaces[MAX_VISEMA]; //Buffer dove verranno registrati i visemi della frase
int TotTime;        //Durata della frase in millisecondi
int TimeXVis;       //Tempo in millisecondi per ogni visema
int NumVis;         //Numero dei visemi

int VisDefaultTime = 75;

bool VisemaInitialized = false;

//Gestione Wave
#ifndef ER_MEM
#define ER_MEM     0xe000
#endif

#ifndef ER_CANNOTOPEN
#define ER_CANNOTOPEN   0xe100
#endif

#ifndef ER_NOTWAVEFILE
#define ER_NOTWAVEFILE   0xe101
#endif

#ifndef ER_CANNOTREAD
#define ER_CANNOTREAD   0xe102
#endif

#ifndef ER_CORRUPTWAVEFILE
#define ER_CORRUPTWAVEFILE 0xe103
#endif

#ifndef ER_CANNOTWRITE
#define ER_CANNOTWRITE  0xe104
#endif

//Fine Gestione Wave

//Controlla se un carattere rappresenta una vocale
bool Vocale(char C) {
	C = tolower(C);
	if ((C == 'a') | (C == 'e') | (C == 'i') | (C == 'o') | (C == 'u'))
		return true;
	else    return false;
}

//Da carattere a intero
int ctoi(char C) {
	char Text[2];
	Text[0] = C;
	Text[1] = 0;

	return atoi(Text);
}

//Restituisce la lunghezza reale di un silb
//Praticamente restituisce la lunghezza - i caratteri speciali
int TrueSilbLeng(int Silb) {
	int Leng = 0;
	unsigned int i;

	for (i = 0; i < strlen(Silbs[Silb].Text); i++) {
		warning("TODO: Properly fix the special character handling in TrueSilbLeng");
		if ((Silbs[Silb].Text[i] != '|') && // (Silbs[Silb].Text[i]!='�') &&
		        (Silbs[Silb].Text[i] != '$') && (Silbs[Silb].Text[i] != '%') &&
		        ((Silbs[Silb].Text[i] < '0') || (Silbs[Silb].Text[i] > '9'))) Leng++;
	}

	return Leng;
}


//Riconosce i silbs in una stringa
bool SilbRecon(char *Text, int tp, int LastVis, int NewSilb, int *CurrentSilb) {
	int TextLen = strlen(Text);
	int NewSilbLeng = strlen(Silbs[NewSilb].Text);
	int CurrentSilbLeng = 0;
	int i;
	bool Equal;

	if (*CurrentSilb != -1) CurrentSilbLeng = strlen(Silbs[*CurrentSilb].Text);

	if (NewSilbLeng <= CurrentSilbLeng) return FALSE;


	for (i = 0; i < NewSilbLeng; i++) {
		Equal = false;

		switch (Silbs[NewSilb].Text[i]) {
		case '|':
			if ((i == 0) && ((tp == 0) || ((tolower(Text[tp - 1] < 'a')) && (tolower(Text[tp - 1] > 'z'))))) Equal = TRUE;
			if ((i != 0) && ((tp == TextLen) || ((tolower(Text[tp]) < 'a') || (tolower(Text[tp]) > 'z')))) Equal = TRUE;
			break;

		case 0xA3: // '£' // TODO: Create a proper constant for the Pound symbol.
			if ((i == 0) && (tp > 0) && (Vocale(Text[tp - 1]))) Equal = TRUE;
			if ((i != 0) && (Vocale(Text[tp]))) {
				Equal = TRUE;
				tp++;
			}
			break;

		case '$':
			if ((i == 0) && (tp > 0) && (!Vocale(Text[tp - 1]))) Equal = TRUE;
			if ((i != 0) && (!Vocale(Text[tp]))) {
				Equal = TRUE;
				tp++;
			}
			break;

		case '%':
			if ((i == 0) && (tolower(Text[tp - 1]) >= 'a') && (tolower(Text[tp - 1]) <= 'z')) Equal = TRUE;
			if ((i != 0) && (tolower(Text[tp]) >= 'a') && (tolower(Text[tp]) <= 'z')) {
				Equal = TRUE;
				tp++;
			}
			break;

		default:
			if ((Silbs[NewSilb].Text[i] >= '0') && (Silbs[NewSilb].Text[i] <= '9')) {
				if (LastVis == ctoi(Silbs[NewSilb].Text[i]))
					Equal = TRUE;
				else    Equal = false;
			} else {
				if (tolower(Text[tp]) == Silbs[NewSilb].Text[i]) Equal = TRUE;
				tp++;
			}
			break;
		}

		if (Equal == FALSE) return FALSE;
	}

	*CurrentSilb = NewSilb;

	return TRUE;
}

//Riconosce le pause in un file wave
int WavePauseRecon(int32 /*n*/, int /*PauseBuf*/[100][5]) {
	warning("STUBBED: WavePauseRecon");
#if 0
	WAVEFORMATEX *pwfxInfo;
	MMCKINFO ckIn;
	MMCKINFO ckInRIFF;
	HMMIO hmmioIn;
	UINT cbActualRead;
	BYTE *Buffer;
	int Size;
	int i;
	int Pausa, NPause = 0, PausaStart = -1, PausaEnd = -1;
	int Play, PlayStart = -1, PlayEnd = -1;
	int Var = WAVE_TOLLERANCE;
	int MinLeng = 300;
	int IgnoreLeng = 10;
	int Centro = 0;

	char FileName[MAX_PATH];
	sprintf(FileName, "%ss%04d.wav", WmSpeechDir, n);

	//Per prove
#ifdef PROVAMODE
	if (strcmp(Sound[wPROVA].name, "") == 0) return -1;
	if (WaveOpenFile(Sound[wPROVA].name,    &hmmioIn, &pwfxInfo, &ckInRIFF) != 0) return -1;
	* /
#else
	if (WaveOpenFile(FileName,  &hmmioIn, &pwfxInfo, &ckInRIFF) != 0) return -1;
#endif
	if (WaveStartDataRead(&hmmioIn, &ckIn, &ckInRIFF) != 0) return -1;
	Size = ckIn.cksize;
	Buffer = (BYTE *)VirtualAlloc(NULL, Size, MEM_COMMIT, PAGE_READWRITE);
	if (Buffer == NULL) return -1;
	if (WaveReadFile(hmmioIn, Size, Buffer, &ckIn, &cbActualRead) != 0) return -1;

	Centro = (1 << pwfxInfo->wBitsPerSample) / 2;

	Pausa = -999;
	Play = -999;

	for (i = 0; i < Size; i++) {
		if ((Buffer[i] >= Centro - Var) && (Buffer[i] <= Centro + Var)) {
			//Pausa
			if (PausaStart == -1) PausaStart = i;
			Pausa = i;
		} else {
			//Play
			if (PlayStart == -1) PlayStart = i;
			Play = i;

			if (i == Pausa + IgnoreLeng) {
				//FinePausa
				PausaEnd = i - IgnoreLeng;
				if (PausaEnd - PausaStart > MinLeng) {
					PauseBuf[NPause][0] = (int)(((double)PausaStart / pwfxInfo->nSamplesPerSec) * 1000);
					PauseBuf[NPause][1] = (int)(((double)PausaEnd / pwfxInfo->nSamplesPerSec) * 1000);
					NPause++;
				}
				PausaStart = -1;
			}
		}
	}

	if (PausaStart != -1) {
		PausaEnd = Size;
		PauseBuf[NPause][0] = (int)(((double)PausaStart / pwfxInfo->nSamplesPerSec) * 1000);
		PauseBuf[NPause][1] = (int)(((double)PausaEnd / pwfxInfo->nSamplesPerSec) * 1000);
		NPause++;
	}

	i = pwfxInfo->nSamplesPerSec;

	WaveCloseReadFile(&hmmioIn, &pwfxInfo);

	return (int)((double)Size / i * 1000.0);
#else
	return 0;
#endif
}

//Restituisce il tempo del discorso a un determinato visema
int VisemaBufTimeLeng(struct VisemaBufStruct *Buf, int Leng) {
	int i;
	int TimeLeng = 0;

	for (i = 0; i < Leng; i++) {
		if (Buf[i].TimeLeng == VIS_NULL) continue;

		if (Buf[i].TimeLeng == VIS_DEFAULT_TIME)
			TimeLeng += VisDefaultTime;
		else TimeLeng += Buf[i].TimeLeng;
	}

	return TimeLeng;
}

//Riconosce le pause probabili in una lista di visemi rappresentanti una frase
bool ProbPauseRecon(int ProbPause[100][5]) {
	int LastVis = -2;
	int i;
	int NPause = 0;

	LastVis = -1;
	for (i = 0; i < NumVis; i++) {
		if ((VisemaBuf[i].Visema == 0) || (VisemaBuf[i].Visema == 14) || (VisemaBuf[i].Visema == 15)) {
			if ((LastVis != 0) && (LastVis != 14) && (LastVis != 15)) {
				ProbPause[NPause][0] = VisemaBufTimeLeng(VisemaBuf, i);
				ProbPause[NPause][2] = i;
			}
		} else {
			if ((LastVis == 0) || (LastVis == 14) || (LastVis == 15)) {
				ProbPause[NPause][1] = VisemaBufTimeLeng(VisemaBuf, i);
				ProbPause[NPause++][3] = i;
			}
		}

		LastVis = VisemaBuf[i].Visema;
	}
	if ((LastVis == 0) || (LastVis == 14) || (LastVis == 15)) {
		ProbPause[NPause][1] = VisemaBufTimeLeng(VisemaBuf, i);
		ProbPause[NPause++][3] = i;
	}

	return TRUE;
}

//Restituisce la distanza vera in numero dei visemi
//Praticamente la distanza - il numero dei visemi speciali
int TrueVisemaDist(struct VisemaBufStruct */*Buf*/, int Start, int End) {
	int i;
	int Leng = 0;

	for (i = Start; i < End; i++) {
		if (VisemaBuf[i].TimeLeng != VIS_NULL) Leng++;
	}

	return Leng;
}

// Sovrappone le pause probabili con le pause del wave, e calcola di conseguenza
// il TimeLeng dei visemi.
bool PauseOnPause(int32 n) {
	int LastVis = -2;
	int i, j;
	int Rnd;
	int DistCentro;
	int ProbPause[500][5];  //Posizione probabile delle pause
	int WavePause[500][5];  //(TimeStart, TimeEnd, VisStart, VisEnd, Misc)
//	int NPause=0;

	//Inizializzazione Buffers
	for (i = 0; i < 100; i++) {
		ProbPause[i][0] = -1;
		ProbPause[i][1] = -1;
		ProbPause[i][2] = -1;
		ProbPause[i][3] = -1;
		ProbPause[i][4] = -1;
		WavePause[i][0] = -1;
		WavePause[i][1] = -1;
		WavePause[i][2] = -1;
		WavePause[i][3] = -1;
		WavePause[i][4] = -1;
	}
	i = -1;

	//Riconoscimento pause nel Wave
	i = WavePauseRecon(n, WavePause);
	if (i == -1) return FALSE;

	VisDefaultTime = i / (NumVis - 2);

	//Riconoscimento pause probabili
	ProbPauseRecon(ProbPause);

	//Sovrapposizione pause
	j = 0;
	while (WavePause[j][0] != -1) {
		i = 0;
		DistCentro = -1;
		while (ProbPause[i][0] != -1) {
			if (((ProbPause[i][0] > WavePause[j][0] - VIS_PAUSE_VAR) && (ProbPause[i][1] < WavePause[j][1] + VIS_PAUSE_VAR)) ||
			        ((ProbPause[i][0] < WavePause[j][0] + VIS_PAUSE_VAR) && (ProbPause[i][1] > WavePause[j][0] - VIS_PAUSE_VAR) && (ProbPause[i][1] < WavePause[j][1] + VIS_PAUSE_VAR)) ||
			        ((ProbPause[i][0] > WavePause[j][0] - VIS_PAUSE_VAR) && (ProbPause[i][0] < WavePause[j][1] + VIS_PAUSE_VAR) && (ProbPause[i][1] > WavePause[j][1] - VIS_PAUSE_VAR)) ||
			        ((ProbPause[i][0] < WavePause[j][0] + VIS_PAUSE_VAR) && (ProbPause[i][1] > WavePause[j][1] - VIS_PAUSE_VAR))) {
				/*if (DistCentro==-1)
				{
				    WavePause[j][2]=ProbPause[i][2];
				    WavePause[j][3]=ProbPause[i][3];
				    DistCentro=abs((ProbPause[i][0]+ProbPause[i][1])/2-(WavePause[j][0]-WavePause[j][1])/2);
				}
				else
				{
				    Rnd=abs((ProbPause[i][0]+ProbPause[i][1])/2-(WavePause[j][0]-WavePause[j][1])/2);
				    if (Rnd<DistCentro)
				    {
				        DistCentro=Rnd;
				        WavePause[j][2]=ProbPause[i][2];
				        WavePause[j][3]=ProbPause[i][3];
				    }
				}
				break;*/

				/*if (DistCentro==-1)
				{
				    if (WavePause[j][4]==-1)
				    {
				        WavePause[j][2]=ProbPause[i][2];
				        WavePause[j][3]=ProbPause[i][3];
				        DistCentro=abs((ProbPause[i][0]+ProbPause[i][1])/2-(WavePause[j][0]-WavePause[j][1])/2);
				        WavePause[j][4]=DistCentro;
				    }
				    else
				    {
				        DistCentro=abs((ProbPause[i][0]+ProbPause[i][1])/2-(WavePause[j][0]-WavePause[j][1])/2);
				        if (DistCentro<WavePause[j][4])
				        {
				            WavePause[j][2]=ProbPause[i][2];
				            WavePause[j][3]=ProbPause[i][3];
				            WavePause[j][4]=DistCentro;
				        }
				    }
				}
				else
				{
				    Rnd=abs((ProbPause[i][0]+ProbPause[i][1])/2-(WavePause[j][0]-WavePause[j][1])/2);
				    if (Rnd<DistCentro)
				    {
				        WavePause[j][2]=ProbPause[i][2];
				        WavePause[j][3]=ProbPause[i][3];
				        DistCentro=Rnd;
				        WavePause[j][4]=DistCentro;
				    }
				}*/

				if (DistCentro == -1) {
					WavePause[j][2] = ProbPause[i][2];
					WavePause[j][3] = ProbPause[i][3];
					DistCentro = abs((ProbPause[i][0] + ProbPause[i][1]) / 2 - (WavePause[j][0] + WavePause[j][1]) / 2);
					//DistCentro=abs(ProbPause[i][0]-WavePause[j][0]);
					WavePause[j][4] = i;
					ProbPause[i][4] = j;
				} else {
					Rnd = abs((ProbPause[i][0] + ProbPause[i][1]) / 2 - (WavePause[j][0] + WavePause[j][1]) / 2);
					//Rnd=abs(ProbPause[i][0]-WavePause[j][0]);
					if (Rnd < DistCentro) {
						WavePause[j][2] = ProbPause[i][2];
						WavePause[j][3] = ProbPause[i][3];
						ProbPause[WavePause[j][4]][4] = -1;
						WavePause[j][4] = i;
						ProbPause[i][4] = j;
						DistCentro = Rnd;
					}
				}
			}
			i++;
		}
		j++;
	}
	//Fine Sovrapposizione pause

	//Controllo pause non trovate
	i = 0;
	while (ProbPause[i][0] != -1) {
		j = 0;
		DistCentro = false;
		while (WavePause[j][0] != -1) {
			if ((WavePause[j][2] == ProbPause[i][2]) && (WavePause[j][3] == ProbPause[i][3])) {
				DistCentro = TRUE;
				break;
			}
			j++;
		}
		if (!DistCentro) {
			for (j = ProbPause[i][2]; j < ProbPause[i][3]; j++)
				VisemaBuf[j].TimeLeng = VIS_NULL;
		}
		i++;
	}
	//Fine Controllo pause non trovate

	//Calcolo TimeLeng dei visemi in base alle pause trovate
	j = 0;
	LastVis = 0;
	while (WavePause[j][0] != -1) {
		if (WavePause[j][2] == -1) {
			j++;
			continue;
		}

		if (TrueVisemaDist(VisemaBuf, LastVis, WavePause[j][2]) > 0) {
			TimeXVis = (WavePause[j][0] - VisemaBufTimeLeng(VisemaBuf, LastVis)) / (TrueVisemaDist(VisemaBuf, LastVis, WavePause[j][2]));

			//Correzione Velocit�
			i = 0;
			while ((TimeXVis < MIN_VISEMA_LENG) && (TimeXVis != 0)) {
				CICLECONTROL(i);
				error("TODO: Randomness");
#if 0
				Rnd = LastVis + RandomInt(WavePause[j][2] - LastVis - 1);
#endif
				if (VisemaBuf[Rnd].Visema != 0)
					VisemaBuf[Rnd].TimeLeng = VIS_NULL;
				if (TrueVisemaDist(VisemaBuf, LastVis, WavePause[j][2]) == 0) break;
				TimeXVis = (WavePause[j][0] - VisemaBufTimeLeng(VisemaBuf, LastVis)) / (TrueVisemaDist(VisemaBuf, LastVis, WavePause[j][2]));
			}
			//Fine Correzione Velocit�

			for (i = LastVis; i < WavePause[j][2]; i++) {
				if (VisemaBuf[i].TimeLeng != VIS_NULL)
					VisemaBuf[i].TimeLeng = TimeXVis;
			}
		}

		if (TrueVisemaDist(VisemaBuf, WavePause[j][2], WavePause[j][3]) > 0) {
			TimeXVis = (WavePause[j][1] - WavePause[j][0]) / (TrueVisemaDist(VisemaBuf, WavePause[j][2], WavePause[j][3]));

			for (i = WavePause[j][2]; i < WavePause[j][3]; i++) {
				if (VisemaBuf[i].TimeLeng != VIS_NULL)
					VisemaBuf[i].TimeLeng = TimeXVis;
			}
		}

		LastVis = WavePause[j][3];

		j++;
	}
	//Fine Calcolo TimeLeng dei visemi in base alle pause trovate

	return TRUE;
}

//Funzione principale del lip-sync
//Riconosce i silb, trova le pause, le sovrappone, e setta le variabili globali
int VisemaRecon(int32 n) {
	char Text[1000];
	int RPos = 0;
	int Leng;
	int LastVis = -2;
	int tp = 0; //Text Position
	int i;
	int Silb = -1;
	int Rnd;
	bool Pensiero = false;
	//int NPause=0;

	strcpy(Text, Sentence[n]);
	Leng = strlen(Text);

	VisemaInitialized = false;

	for (i = 0; i < MAX_VISEMA; i++) {
		VisemaBuf[i].Visema = 0;
		VisemaBuf[i].TimeLeng = 0;
	}

	if ((Text[0] == '<') && (Text[Leng - 1] == '>')) Pensiero = TRUE;

	// FIXME: condition never happen
	//if (Text==NULL) return -1;
	error("TODO: Randomness");
#if 0
	srand((unsigned int)time(NULL));
#endif
	VisemaBuf[RPos].Visema = 0;
	VisemaBuf[RPos++].TimeLeng = VIS_NULL;

	while (tp < Leng) {
		for (i = 0; i < NumSilbs; i++) {
			SilbRecon(Text, tp, LastVis, i, &Silb);
		}

		if (Silb == -1) tp++;
		else {
			for (i = 0; i < 3; i++) {
				if (!Pensiero)
					switch (Silbs[Silb].Res[i]) {
					case -1:
						break;
					case  0:
						VisemaBuf[RPos].Visema = 0;
						VisemaBuf[RPos++].TimeLeng = VIS_DEFAULT_TIME;
						LastVis = 0;
						break;
					case 14:
						/*if (RandomInt(100)<80)    VisemaBuf[RPos].Visema=0;
						else                    VisemaBuf[RPos].Visema=14;*/
						VisemaBuf[RPos].Visema = 0;
						VisemaBuf[RPos++].TimeLeng = VIS_DEFAULT_TIME;
						LastVis = 0;
						break;
					case 15:
						error("TODO: Randomness");
#if 0
						Rnd = RandomInt(100);
#endif
						if (Rnd < 15)      VisemaBuf[RPos].Visema = 15;
						else if (Rnd < 30) VisemaBuf[RPos].Visema = 15;
						VisemaBuf[RPos++].TimeLeng = VIS_DEFAULT_TIME;
						LastVis = 0;
						break;
					default:
						VisemaBuf[RPos].Visema = Silbs[Silb].Res[i];
						VisemaBuf[RPos++].TimeLeng = VIS_DEFAULT_TIME;
						LastVis = Silbs[Silb].Res[i];
						break;
					} else if (Silbs[Silb].Res[i] != -1) {
					VisemaBuf[RPos].Visema = 0;
					VisemaBuf[RPos++].TimeLeng = VIS_NULL;
				}

			}

			tp += TrueSilbLeng(Silb);

			Silb = -1;
		}
	}

	VisemaBuf[RPos].Visema = 0;
	VisemaBuf[RPos++].TimeLeng = VIS_NULL;

	NumVis = RPos;

	PauseOnPause(n);

	TotTime = NumVis * VisDefaultTime;
	TimeXVis = TotTime / NumVis;

	VisemaInitialized = TRUE;

	return TotTime;
}

//Restituisce il visema corrispondente a un determinato tempo
int32 VisemaTimeRecon(int32 Time) {
	if (Time < 0) return 0;
	if (!VisemaInitialized) return 0;

#ifdef PROVAMODE
	static bool Partito = FALSE;
	if (Time == 0) Partito = FALSE;

	if ((!Partito) && (Time > 0)) {
		Partito = TRUE;
		StartSound(wPROVA);
	}
#endif

	for (int i = 0; i < NumVis; i++) {
		if ((Time >= VisemaBufTimeLeng(VisemaBuf, i)) && (Time < VisemaBufTimeLeng(VisemaBuf, i + 1)))
			return VisemaBuf[i].Visema;
	}

	return VisemaBuf[NumVis - 1].Visema;
}

} // End of namespace Watchmaker
