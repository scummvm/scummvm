/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifdef _WIN32_WCE

// Browse directories to locate SCUMM games

#include "stdafx.h"
#include <assert.h>
#include <Winuser.h>
#include <Winnls.h>
#include "resource.h"
#include "scumm.h"
#include "config-file.h"

extern Config *scummcfg;

#define MAX_GAMES 20

struct ScummGame {
	const char *gamename;
	const char *description;
	const char *directory;
	const char *check_file_1;
	const char *check_file_2;
	const char *filename;
	unsigned char next_demo;
};

struct InstalledScummGame {
	unsigned char reference;
	TCHAR directory[MAX_PATH];
};

static const ScummGame GameList[] = {
	{	
		 "Simon The Sorcerer 1 (dos)",
		 "Completable",
		 "", "1631.VGA", "GAMEPC",
		 "simon1dos",
		 0
	},
	{	 
		 "Simon The Sorcerer 1 (win)",
		 "Completable",
		 "", "SIMON.GME", "GAMEPC",
		 "simon1win",
		 0
	},
	{	 
		 "Simon The Sorcerer 2 (win)",
		 "To be tested",
		 "", "SIMON2.GME", "GSPTR30",
		 "simon2win",
		 0
	},
	{ 
		 "Indiana Jones 3 (new)", 
	     "Buggy, playable a bit", 
		 "indy3", "", "", 
		 "indy3",
	     0 
	},
	{	 
		 "Zak Mc Kracken (new)",
		 "Completable",
		 "zak256", "", "",
		 "zak256",
		 0
	},
	{
		 "Loom (old)",
		 "Not working",
		 "loom", "", "",
		 "loom",
		 0
	},
	{
		 "Monkey Island 1 (EGA)",
		 "Not tested",
		 "monkeyEGA", "", "",
		 "monkeyEGA",
		 0
	},
	{
		 "Loom (VGA)",
		 "Completable, MP3 audio",
		 "loomcd", "", "",
		 "loomcd",
		 0
	},
	{
		 "Monkey Island 1 (VGA)",
		 "Completable, MP3 music",
		 "", "MONKEY.000", "MONKEY.001",
		 "monkey",
		 0
	},
	{
		 "Monkey Island 2 (VGA)",
		 "Completable",
		 "", "MONKEY2.000", "MONKEY2.001",
		 "monkey2",
		 0
	},
	{
		 "Indiana Jones 4",
		 "Completable",
		 "", "ATLANTIS.000", "ATLANTIS.001",
		 "atlantis",
		 1
	},
	{
		 "Indiana Jones 4 demo",
		 "Completable",
		 "", "PLAYFATE.000", "PLAYFATE.001",
		 "playfate",
		 0
	},
	{
		 "Day of the Tentacle",
		 "Completable",
		 "", "TENTACLE.000", "TENTACLE.001",
		 "tentacle",
		 1
	},
	{
		 "Day of the Tentacle demo",
		 "Completable",
		 "", "DOTTDEMO.000", "DOTTDEMO.001",
		 "dottdemo",
		 0
	},
	{	
		 "Sam & Max",
		 "Completable, music glitches",
		 "", "SAMNMAX.000", "SAMNMAX.001",
		 "samnmax",
		 1
	},
	{
		 "Sam & Max demo",
		 "Completable",
		 "", "SNMDEMO.000", "SNMDEMO.001",
		 "snmdemo",
		 0
	},
	{
		 "Full Throttle",
		 "Partially working",
		 "", "FT.LA0", "FT.LA1",
		 "ft",
		 0
	},
	{
		 "The Dig",
		 "Partially working",
		 "", "DIG.LA0", "DIG.LA1",
		 "dig",
		 0
	},
	{
		 NULL, NULL, NULL, NULL, NULL, NULL, 0
	}
};

void findGame(TCHAR*);
int displayFoundGames(void);
void doScan();
void startFindGame();

char gamesFound[MAX_GAMES];
unsigned char listIndex[MAX_GAMES];
InstalledScummGame gamesInstalled[MAX_GAMES];
int installedGamesNumber;
HWND hwndDlg;
TCHAR basePath[MAX_PATH];
TCHAR old_basePath[MAX_PATH];
BOOL prescanning;

BOOL isPrescanning() {
	return prescanning;
}

void setFindGameDlgHandle(HWND x) {
	hwndDlg = x;
}

bool loadGameSettings() {
	int				index;
	int				i;
	const char		*current;

	prescanning = FALSE;

	current = scummcfg->get("GamesInstalled", "wince");
	if (!current)
		return FALSE;
	index = atoi(current);

	installedGamesNumber = index;

	current = scummcfg->get("GamesReferences", "wince");
	if (!current)
		return FALSE;
	for (i=0; i<index; i++) {
		char x[6];
		int j;

		memset(x, 0, sizeof(x));
		memcpy(x, current + 3 * i, 2);
		sscanf(x, "%x", &j);
		gamesFound[j] = 1;
		gamesInstalled[i].reference = j;
	}

	current = scummcfg->get("BasePath", "wince");
	if (!current)
		return FALSE;
	MultiByteToWideChar(CP_ACP, 0, current, strlen(current) + 1, basePath, sizeof(basePath));

	for (i=0; i<index; i++) {
		char keyName[100];

		sprintf(keyName, "GamesDirectory%d", i);
		current = scummcfg->get(keyName, "wince");
		if (!current)
			return FALSE;
		MultiByteToWideChar(CP_ACP, 0, current, strlen(current) + 1, gamesInstalled[i].directory, sizeof(gamesInstalled[i].directory));
	}

	displayFoundGames();

	return TRUE;
}

int countGameReferenced(int reference, int *infos) {
	int i;
	int number = 0;

	for (i=0; i<installedGamesNumber; i++)
		if (gamesInstalled[i].reference == reference) 
			infos[number++] = i;

	return number;
}

int displayFoundGames() {

	int i;
	int index = 0;

	for (i = 0; i< MAX_GAMES; i++) {
		ScummGame current_game;
		char    work[400];
		TCHAR	desc[400];
		int		numberReferenced;
		int		infos[10];
		int		j;

		current_game = GameList[i];
		if (!current_game.filename)
			break;
		if (!gamesFound[i])
			continue;
		
		numberReferenced = countGameReferenced(i, infos);

		for (j=0; j<numberReferenced; j++) {
			if (numberReferenced != 1)
				sprintf(work, "%s (%d)", current_game.gamename, j + 1);
			else
				strcpy(work, current_game.gamename);
			MultiByteToWideChar(CP_ACP, 0, work, strlen(work) + 1, desc, sizeof(desc));
			SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_ADDSTRING, 0, (LPARAM)desc);
			listIndex[index++] = infos[j];
		}
	}

	return index;

}

void changeScanPath() {
	int item;
	TCHAR path[MAX_PATH];

	item = SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_GETCURSEL, 0, 0);
	if (item == LB_ERR)
		return;

	SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_GETTEXT, item, (LPARAM)path);

	if (wcscmp(path, TEXT("..")) != 0) {
		wcscat(basePath, TEXT("\\"));
		wcscat(basePath, path);
	}
	else {
		TCHAR *work;
		
		work = wcsrchr(basePath, '\\');
		*work = 0;
		*(work + 1) = 0;
	}

	doScan();
}

void doScan() {
	WIN32_FIND_DATA	 desc;
	TCHAR			 searchPath[MAX_PATH];
	HANDLE			 x;

	SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);

	if (wcslen(basePath) != 0)
		SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_ADDSTRING, 0, (LPARAM)TEXT(".."));

	wsprintf(searchPath, TEXT("%s\\*"), basePath);

	x = FindFirstFile(searchPath, &desc);
	if (x == INVALID_HANDLE_VALUE)
		return;
	if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		TCHAR *work;

		work = wcsrchr(desc.cFileName, '\\');
		SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), 
			LB_ADDSTRING, 0, (LPARAM)(work ? work + 1 : desc.cFileName));
	}
	while (FindNextFile(x, &desc))
		if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		TCHAR *work;

		work = wcsrchr(desc.cFileName, '\\');
		SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), 
			LB_ADDSTRING, 0, (LPARAM)(work ? work + 1 : desc.cFileName));
	}	
	FindClose(x);
}

void startScan() {
	prescanning = TRUE;
	wcscpy(old_basePath, basePath);
	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Choose the games root directory"));
	SetDlgItemText(hwndDlg, IDC_SCAN, TEXT("OK"));
	SetDlgItemText(hwndDlg, IDC_GAMEDESC, TEXT(""));
	ShowWindow(GetDlgItem(hwndDlg, IDC_PLAY), SW_HIDE);
	doScan();
}

void endScanPath() {
	prescanning = FALSE;
	SetDlgItemText(hwndDlg, IDC_SCAN, TEXT("Scan"));
	ShowWindow(GetDlgItem(hwndDlg, IDC_PLAY), SW_SHOW);
	startFindGame();
}

void abortScanPath() {
	prescanning = FALSE;
	wcscpy(basePath, old_basePath);
	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT(""));
	SetDlgItemText(hwndDlg, IDC_SCAN, TEXT("Scan"));	
	SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);
	ShowWindow(GetDlgItem(hwndDlg, IDC_PLAY), SW_SHOW);
	displayFoundGames();
}

void startFindGame() {
	//TCHAR			fileName[MAX_PATH];
	//TCHAR			*tempo;
	int				i = 0;
	int		    	index = 0;
	char			tempo[1024];
	char		    workdir[MAX_PATH];	

	prescanning = FALSE;

	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Scanning, please wait"));

	SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);

	memset(gamesFound, 0, MAX_GAMES);
	/*
	GetModuleFileName(NULL, fileName, MAX_PATH);
	tempo = wcsrchr(fileName, '\\');
	*tempo = '\0';
	*(tempo + 1) = '\0';
	*/
	installedGamesNumber = 0;

	//findGame(fileName);
	findGame(basePath);

	// Display the results
	index = displayFoundGames();

	// Save the results in the registry
	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Saving the results"));

	scummcfg->set("GamesInstalled", index, "wince");

	tempo[0] = '\0';
	for (i=0; i<index; i++) {
		char x[3];
		sprintf(x, "%.2x ", gamesInstalled[i].reference);
		strcat(tempo, x);
	}	

	scummcfg->set("GamesReferences", tempo, "wince");

	WideCharToMultiByte(CP_ACP, 0, basePath, wcslen(basePath) + 1, workdir, sizeof(workdir), NULL, NULL);

	scummcfg->set("BasePath", workdir, "wince");

	for (i=0; i<index; i++) {
		char keyName[100];

		sprintf(keyName, "GamesDirectory%d", i);
		WideCharToMultiByte(CP_ACP, 0, gamesInstalled[i].directory, wcslen(gamesInstalled[i].directory) + 1, workdir, sizeof(workdir), NULL, NULL);
		scummcfg->set(keyName, workdir, "wince");
	}

	scummcfg->flush();

	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Scan finished"));

}

void getSelectedGame(int result, char *id, TCHAR *directory) {
	ScummGame game;

	game = GameList[gamesInstalled[listIndex[result]].reference];
	strcpy(id, game.filename);
	wcscpy(directory, gamesInstalled[listIndex[result]].directory);
}

void displayGameInfo() {
	int item;	
	TCHAR work[400];
	ScummGame game;

	item = SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_GETCURSEL, 0, 0);
	if (item == LB_ERR)
		return;

	game = GameList[gamesInstalled[listIndex[item]].reference];
	wcscpy(work, TEXT("File path : ..."));	
	wcscat(work, wcsrchr(gamesInstalled[listIndex[item]].directory, '\\'));			
	SetDlgItemText(hwndDlg, IDC_FILEPATH, work);
	MultiByteToWideChar(CP_ACP, 0, game.description, strlen(game.description) + 1, work, sizeof(work));
	SetDlgItemText(hwndDlg, IDC_GAMEDESC, work);	
}

void findGame(TCHAR *directory) {
	TCHAR			 fileName[MAX_PATH];
	TCHAR			 newDirectory[MAX_PATH];
	WIN32_FIND_DATA	 desc;
	HANDLE			 x;
	int				 i;

	// Check for games in the current directory

	//MessageBox(NULL, directory, TEXT("Current"), MB_OK);

	for (i = 0 ; i < MAX_GAMES ; i++) {
		ScummGame current_game;

		current_game = GameList[i];
		if (!current_game.filename)
			break;

		if (strlen(current_game.directory)) {
			// see if the last directory matches
			TCHAR	*work;
			char	curdir[MAX_PATH];

			
			work = wcsrchr(directory, '\\');
			WideCharToMultiByte(CP_ACP, 0, work + 1, wcslen(work + 1) + 1, curdir, sizeof(curdir), NULL, NULL);
			if (stricmp(curdir, current_game.directory) == 0) {
				
				//MessageBox(NULL, TEXT("Match directory !"), TEXT("..."), MB_OK);
				
				gamesFound[i] = 1;
				gamesInstalled[installedGamesNumber].reference = i;
				wcscpy(gamesInstalled[installedGamesNumber].directory, directory);
				installedGamesNumber++;
			}
		}
		else
		{
			TCHAR	work[MAX_PATH];
			TCHAR	checkfile[MAX_PATH];


			MultiByteToWideChar(CP_ACP, 0, current_game.check_file_1, strlen(current_game.check_file_1) + 1, checkfile, sizeof(checkfile));
			wsprintf(work, TEXT("%s\\%s"), directory, checkfile);
			//MessageBox(NULL, work, TEXT("Checking file"), MB_OK);

			if (GetFileAttributes(work) == 0xFFFFFFFF)
				continue;

			//MessageBox(NULL, TEXT("Check OK"), TEXT("Checking file"), MB_OK);
			MultiByteToWideChar(CP_ACP, 0, current_game.check_file_2, strlen(current_game.check_file_2) + 1, checkfile, sizeof(checkfile));
			wsprintf(work, TEXT("%s\\%s"), directory, checkfile);			
			if (GetFileAttributes(work) == 0xFFFFFFFF)
				continue;
			
			//MessageBox(NULL, TEXT("Match file !"), TEXT("..."), MB_OK);
			gamesFound[i] = 1;
			gamesInstalled[installedGamesNumber].reference = i;
			wcscpy(gamesInstalled[installedGamesNumber].directory, directory);
			installedGamesNumber++;

		}
	}

	// Recurse

	wsprintf(fileName, TEXT("%s\\*"), directory);

	x = FindFirstFile(fileName, &desc);
	if (x == INVALID_HANDLE_VALUE)
		return;
	if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		wsprintf(newDirectory, TEXT("%s\\%s"), directory, desc.cFileName);
		findGame(newDirectory);
	}
	while (FindNextFile(x, &desc))
		if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		wsprintf(newDirectory, TEXT("%s\\%s"), directory, desc.cFileName);
		findGame(newDirectory);
	}	
	FindClose(x);
}

#endif