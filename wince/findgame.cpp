#ifdef _WIN32_WCE

// Browse directories to locate SCUMM games

#include "stdafx.h"
#include <Winuser.h>
#include "resource.h"

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
		 "Indiana Jones 3 (new)", 
	     "Buggy, unplayable", 
		 "indy3", "", "", 
		 "indy3",
	     0 
	},
	{	 
		 "Zak Mc Kracken (new)",
		 "Buggy, unplayable",
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
		 "Buggy, playable a bit",
		 "loomcd", "", "",
		 "loomcd",
		 0
	},
	{
		 "Monkey Island 1 (VGA)",
		 "Working well, perhaps completable ?",
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
		 "Completable,glitches,no music",
		 "", "SAMNMAX.000", "SAMNMAX.001",
		 "samnmax",
		 1
	},
	{
		 "Sam & Max demo",
		 "Completable, minor glitches, no music",
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

char gamesFound[MAX_GAMES];
unsigned char listIndex[MAX_GAMES];
InstalledScummGame gamesInstalled[MAX_GAMES];
int installedGamesNumber;
HWND hwndDlg;

void setFindGameDlgHandle(HWND x) {
	hwndDlg = x;
}

bool loadGameSettings() {
	HKEY			hkey;
	DWORD			disposition;
	DWORD			keyType, keySize, dummy;
	int				index;
	int				i;
	unsigned char	references[MAX_PATH];

	if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\PocketSCUMM"), 
		 0, NULL, 0, 0, NULL, &hkey, &disposition) == ERROR_SUCCESS) {

		 keyType = REG_DWORD;
		 keySize = sizeof(DWORD);
		 if (RegQueryValueEx(hkey, TEXT("GamesInstalled"), NULL, &keyType, 
						 (unsigned char*)&dummy, &keySize) == ERROR_SUCCESS) 
					index = dummy;
		 else
					return FALSE;

		 installedGamesNumber = index;

		 keyType = REG_BINARY;
		 keySize = index;
		 if (RegQueryValueEx(hkey, TEXT("GamesReferences"), NULL, &keyType, 
						 references, &keySize) != ERROR_SUCCESS)
					return FALSE;

		 for (i=0; i<index; i++)
			 gamesFound[references[i]] = 1;

		 keyType = REG_SZ;
		 for (i=0; i<index; i++) {
			 char work[100];
			 TCHAR keyname[100];

			 gamesInstalled[i].reference = references[i];
			 keySize = MAX_PATH;
			 sprintf(work, "GamesDirectory%d", i);			 
			 MultiByteToWideChar(CP_ACP, 0, work, strlen(work) + 1, keyname, sizeof(keyname));
			 if (RegQueryValueEx(hkey, keyname, NULL, &keyType, (unsigned char*)gamesInstalled[i].directory, &keySize) != ERROR_SUCCESS)
				 return FALSE;			 
		 }
		 
		 RegCloseKey(hkey);
		 displayFoundGames();
		 return TRUE;
	 }
	else
		return FALSE;
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

void startFindGame() {
	TCHAR			fileName[MAX_PATH];
	TCHAR			*tempo;
	int				i = 0;
	int		    	index = 0;
	HKEY			hkey;
	DWORD			disposition, keyType, keySize, dummy;
	unsigned char	references[MAX_GAMES];

	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Scanning, please wait"));

	SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);

	memset(gamesFound, 0, MAX_GAMES);
	GetModuleFileName(NULL, fileName, MAX_PATH);
	tempo = wcsrchr(fileName, '\\');
	*tempo = '\0';
	*(tempo + 1) = '\0';
	installedGamesNumber = 0;

	findGame(fileName);

	// Display the results
	index = displayFoundGames();

	// Save the results in the registry
	SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Saving the results"));

	for (i=0; i<index; i++)
		references[i] = gamesInstalled[i].reference;

	if(RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\PocketSCUMM"), 
		 0, NULL, 0, 0, NULL, &hkey, &disposition) == ERROR_SUCCESS) {

		 keyType = REG_DWORD;
		 keySize = sizeof(DWORD);
		 dummy = index;
		 RegSetValueEx(hkey, TEXT("GamesInstalled"), 0, keyType, (unsigned char*)&dummy, keySize);
		 keyType = REG_BINARY;
		 keySize = index;	
		 RegSetValueEx(hkey, TEXT("GamesReferences"), 0, keyType, references, 
						keySize);	
		 keyType = REG_SZ;
		 for (i=0; i<index; i++) {
			 char work[100];
			 TCHAR keyname[100];

			 sprintf(work, "GamesDirectory%d", i);
			 MultiByteToWideChar(CP_ACP, 0, work, strlen(work) + 1, keyname, sizeof(keyname));
			 keySize = (wcslen(gamesInstalled[i].directory) + 1) * 2;
			 RegSetValueEx(hkey, keyname, 0, keyType, (unsigned char*)gamesInstalled[i].directory, keySize);				 
		 }
		 
		 RegCloseKey(hkey);
	 }

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