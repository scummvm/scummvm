class GameDetecter {
public:
	int detectMain(int argc, char **argv);
	void parseCommandLine(int argc, char **argv);
	bool detectGame(void);
	char *getGameName(void);

	bool _fullScreen;
	uint16 _debugMode;
	uint16 _noSubtitles;
	uint16 _bootParam;
	unsigned int _scale;
	char *_gameDataPath;
	int _gameTempo;
	void *_soundEngine;
	int _midi_driver;
	int _videoMode;
	char *_exe_name;
	byte _gameId;
	const char *_gameText;
	uint32 _features;
	uint16 _soundCardType;

};
