//////////////////////////////////////////////////////////////////////////////
// emudebug.cpp                                                             //
//////////////////////////////////////////////////////////////////////////////
/*
	EmuDebug by Rafael Vuijk (aka Dark Fader)

	see emudebug.txt for more info
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <conio.h>

//////////////////////////////////////////////////////////////////////////////
// Pragmas                                                                  //
//////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "user32")

//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
#define VER					"1.02"

//////////////////////////////////////////////////////////////////////////////
// Variables                                                                //
//////////////////////////////////////////////////////////////////////////////
// set in InitDebug
HWND hDebugWnd = 0;
HWND hEmuWnd = 0;
HANDLE hProcess = 0;

// set in ScanBuffer
void *debugBufferBeginAddr = 0;
void *debugBufferEndAddr = 0;
void *debugBufferDataAddr = 0;
int debugBufferDataSize = 0;
char *debugBufferData = 0;		// temp. data

// default options
int minDebugBufferSize = 31;
int maxDebugBufferSize = 16*1024;
int pollInterval = 10;
int priorityClass = NORMAL_PRIORITY_CLASS;
char windowClass[256] = "BOYCOTTADVANCE";			// :)
char windowTitle[256] = "BoycottAdvance - ";
bool waitForKey = false;

//////////////////////////////////////////////////////////////////////////////
// InitDebug                                                                //
//////////////////////////////////////////////////////////////////////////////
int InitDebug()
{
	// minimize debug window
	//ShowWindow(hDebugWnd, SW_MINIMIZE); //ph0x

restart:
	printf("Searching debugging target...");

	char *pWindowClass = windowClass[0] ? windowClass : 0;
	char *pWindowTitle = windowTitle[0] ? windowTitle : 0;

	// loop
	while (1)
	{
		hEmuWnd = FindWindow(pWindowClass, pWindowTitle);
		//if (!hEmuWnd) { printf("Can't find window!\n"); return -1; }
		if (hEmuWnd) break;

		if (kbhit() && (getch() == 27)) return -1;	// abort?
		Sleep(20);
	}

	DWORD processId = 0;
	GetWindowThreadProcessId(hEmuWnd, &processId);
	if (!processId) { printf("Can't get process ID!\n"); return -1; }

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
	if (!hProcess) { printf("Can't open process!\n"); return -1; }

	SetPriorityClass(hProcess, priorityClass);		// set process priority class

	printf(" done.\n");

	int bufferSize = 1*1024*1024;	// start with 1MB or so
	char *buffer = new char[bufferSize];	// temp ReadProcessMemory buffer

	if (!waitForKey) printf("Searching debug buffer...");

	// loop
	while (1)
	{
		if (waitForKey)
		{
			printf("Press any key to begin searching for debug buffer...");
			getch();
			printf("\n");
			printf("Searching debug buffer...");
		}

		DWORD exitCode;
		if (!GetExitCodeProcess(hProcess, &exitCode)) { printf("\n"); goto restart; }
		if (exitCode != STILL_ACTIVE) { printf("\n"); goto restart; }

		bool something = false;		// some data found?
		MEMORY_BASIC_INFORMATION mbi;

		unsigned int addr;
		for (addr=0; VirtualQueryEx(hProcess, (void*)addr, &mbi, sizeof(mbi)); addr = (unsigned int)mbi.BaseAddress + mbi.RegionSize)
		{
			//printf("base=%08X, size=%d, protect=%08X, type=%08X\n", mbi.BaseAddress, mbi.RegionSize, mbi.Protect, mbi.Type);
			if (mbi.Type == MEM_PRIVATE)	// type=00020000
			if (mbi.Protect == PAGE_READWRITE)	// protect=00000004
			{
				if (mbi.RegionSize > bufferSize)
				{
					delete buffer;
					bufferSize = mbi.RegionSize * 3/2;
					buffer = new char[bufferSize];
				}
				
				if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer, mbi.RegionSize, NULL))
				{
					something = true;
					for (unsigned int i=0; i<mbi.RegionSize; i += minDebugBufferSize-2)
					{
						if (buffer[i] == ' ')	// might be somewhere in the buffer
						{
							//printf("scan left\n");
							// scan to left
							int left = i;
							while (buffer[left] == ' ') if (--left <= 0) { continue; }	// nothing left
							if (buffer[left] != '{') { continue; }	// nope, wrong start

							//printf("scan right\n");
							// scan to right
							int right = i;
							while (buffer[right] == ' ') if (++right >= mbi.RegionSize) { i = right; continue; }	// nothing left
							if (buffer[right] != '}') { i = right; continue; }	// nope, wrong end

							// alloc new temp. debug buffer with max debug buffer length
							debugBufferDataSize = right - left + 1;
							//printf("debugBufferDataSize = %d\n", debugBufferDataSize);
							if (
								(debugBufferDataSize >= minDebugBufferSize) &&		// minimum size
								(debugBufferDataSize <= maxDebugBufferSize) &&		// maximum size
								(*(unsigned int *)(buffer + left - 8) == 0xEDEBEDEB) &&	// start
								(*(unsigned int *)(buffer + left - 4) == 0xEDEBEDEB)		// end
							)
							{
								// remember addresses
								debugBufferBeginAddr = (void *)((int)mbi.BaseAddress + left - 8);
								debugBufferEndAddr = (void *)((int)mbi.BaseAddress + left - 4);
								debugBufferDataAddr = (void *)((int)mbi.BaseAddress + left - 0);

								// allocate temporary buffer
								if (debugBufferData) delete debugBufferData;
								debugBufferData = new char[debugBufferDataSize];

								// start debugging
								int n = 0;
								WriteProcessMemory(hProcess, debugBufferBeginAddr, &n, sizeof(n), NULL);
								WriteProcessMemory(hProcess, debugBufferEndAddr, &n, sizeof(n), NULL);

								// show done
								printf(" done.\n");
								delete buffer;

								//printf("base=%08X, size=%d, protect=%08X, type=%08X\n", mbi.BaseAddress, mbi.RegionSize, mbi.Protect, mbi.Type);

								// do things to activate/show debugger
								ShowWindow(hEmuWnd, SW_RESTORE); //ph0x
								SetActiveWindow(hEmuWnd); //ph0x
								SetForegroundWindow(hEmuWnd); //ph0x
								FlashWindow(hDebugWnd, TRUE);
								SetWindowPos(hDebugWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

								return 0;	// ok
							}
						}	// ' '
					}	// for
				}	// ReadProcessMemory
				else
				{
					// can't read memory anymore
					//printf("\n"); goto restart;
				}
				//printf("\n");
			}	// type
		}	// for VirtualQueryEx

		if (waitForKey) printf("\n");

		//if (!addr) { printf("\n"); goto restart; }	// no VirtualQueryEx data

		//if (!something) { printf("\n"); goto restart; }	// invalid process or something

		if (kbhit() && (getch() == 27)) break;	// abort
		Sleep(20);
	}	// while

	delete buffer;

	return -1;
}

//////////////////////////////////////////////////////////////////////////////
// ShowHelp                                                                 //
//////////////////////////////////////////////////////////////////////////////
void ShowHelp()
{
	printf("EmuDebug "VER" by Rafael Vuijk (aka Dark Fader)\n\n");
	printf("Flags:\n");
	printf("  -h -?      Show this help.\n");
	printf("  -b         Set emulator process to below priority class.\n");
	printf("  -i         Set emulator process to idle priority class.\n");
	printf("  -p<n>      Set polling interval in milliseconds.\n");
	printf("  -c[<n>]    Window class name to find. Default: \"BOYCOTTADVANCE\".\n");
	printf("             You can use MS Spy++ or something to find this.\n");
	printf("  -t[<n>]    Window title name to find. Default: \"BoycottAdvance - \".\n");
	printf("  -s<n>      Set mininum debug buffer size to look for.\n");
	printf("  -k         Wait for a key to commence searching.\n");
	printf("\n");
	printf("Some 'good' working examples:\n");
	printf("  emudebug -i -p100 -s127\n");
	printf("  emudebug -p20 -k -b -c\"\" -t\"VGBA-Windows 1.1r\" -s63\n");
}

//////////////////////////////////////////////////////////////////////////////
// main                                                                     //
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	// check parameters
	for (int a=1; a<argc; a++)
	{
		if (argv[a][0] == '-')
		switch (argv[a][1])
		{
			case 'h': case '?': ShowHelp(); return 0;
			case 'i': priorityClass = IDLE_PRIORITY_CLASS; break;
			//case 'b': priorityClass = BELOW_NORMAL_PRIORITY_CLASS; break; //ph0x
			case 'p': pollInterval = strtoul(argv[a]+2, NULL, 0); break;
			case 'c': strcpy(windowClass, argv[a]+2); break;
			case 't': strcpy(windowTitle, argv[a]+2); break;
			case 's': minDebugBufferSize = strtoul(argv[a]+2, NULL, 0); break;
			case 'k': waitForKey = true; break;
			default: printf("Unknown uption: %c\n", argv[a][1]); break;
		}
	}

	// find debug window
	SetConsoleTitle("EmuDebug Console");
	hDebugWnd = FindWindow(NULL, "EmuDebug Console");

	// search for debug buffer
	if (InitDebug()) return -1;

	// do debug console
	int boostPollInterval = pollInterval;
	while (1)
	{
		// check keyboard input
		if (kbhit())
		{
			char c = getch();
			if (c == 27)	// escape
			{
				// try to close emulator & minimize debug window
				SendMessage(hEmuWnd, WM_CLOSE, 0, 0); 
				SendMessage(hDebugWnd, WM_CLOSE, 0, 0); //ph0x
				//SetWindowPos(hDebugWnd, HWND_TOP, 0,0,0,0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE);
				//ShowWindow(hDebugWnd, SW_MINIMIZE); //ph0x
			}
		}

		// get begin/end from debug buffer
		int end = 0;
		if (!ReadProcessMemory(hProcess, (void *)debugBufferEndAddr, &end, sizeof(end), NULL))
		{
			// re-init debug after failure
			if (InitDebug()) break;
			continue;
		}

		int begin = 0;
		ReadProcessMemory(hProcess, (void *)debugBufferBeginAddr, &begin, sizeof(begin), NULL);

		// some data?
		if (begin != end)
		{
			unsigned int nextBegin;
			while (1)	//begin != end)
			{
				int begin = end;
				ReadProcessMemory(hProcess, (void *)debugBufferBeginAddr, &begin, sizeof(begin), NULL);
				if (begin == end) break;	// no more data
				nextBegin = begin + 1;
				if (nextBegin >= debugBufferDataSize) nextBegin = 0;
				char c;
				ReadProcessMemory(hProcess, (void *)((int)debugBufferDataAddr + begin), &c, 1, NULL);
				putchar(c);
				begin = nextBegin;
				WriteProcessMemory(hProcess, debugBufferBeginAddr, &begin, sizeof(begin), NULL);
			}

			// boost poll interval
			boostPollInterval /= 2;
		}
		else
		{
			// slow down again
			if (boostPollInterval == 0) boostPollInterval = 1; else boostPollInterval *= 2;
			if (boostPollInterval > pollInterval) boostPollInterval = pollInterval;
		}

		// poll interval
		Sleep(boostPollInterval);
	}

	// clean up
	if (debugBufferData) delete debugBufferData;
	CloseHandle(hProcess);

	return 0;
}
