#include "app.h"
#include "stdafx.h"

bool App::Init() {
	// Load all SDL subsystems and the TrueType font subsystem
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize SDL", "Please install libsdl2", NULL);
		return false;
	}

	if (TTF_Init() == -1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize sdl_ttf", "Please install libsdl2_ttf", NULL);
		return false;
	}

	int flags = IMG_Init(IMG_INIT_PNG);
	if (flags != IMG_INIT_PNG) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to initialize PNG support", "Please install libsdl2_image", NULL);
		return false;
	}

	// Load paths for important files
	gFilePath.Load("res/paths.xml");

	// Initialize Steam
	// SteamAPI_Init();

	// Load the settings corresponding to the latest version
	{
		const std::string DEFAULT_FILENAME = "res/settings.xml";

		std::string filename = gFilePath.appdata;
		filename += "settings.xml";

		using namespace boost::filesystem;
		if (!is_regular_file(filename)) {
			// The other file does not exist, just use the default file
			LoadSettings(DEFAULT_FILENAME);
		} else {
			// We are using the other file, check if it is up to date or not
			if (Version(DEFAULT_FILENAME) > Version(filename)) {
				// The game has been updated to a different control scheme, use the default file
				LoadSettings(DEFAULT_FILENAME);
			} else {
				// The version set by the player is fine, just use that
				LoadSettings(filename);
			}
		}
	}

	SDL_DisplayMode d;
	if (SDL_GetDesktopDisplayMode(0, &d) == 0) {
		// Store the default desktop values before starting our own screen
		gScreenSettings.desktop.w = d.w;
		gScreenSettings.desktop.h = d.h;
	}

	// Set up the screen - use windowed mode at start in order to prevent loss of texture on context switching
	gWindow = SDL_CreateWindow("Unrest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							   gScreenSettings.cur.w, gScreenSettings.cur.h, SDL_WINDOW_SHOWN);

	if (gWindow == nullptr)
		return false;

	// Set the window icon
	SDL_Surface *icon = IMG_Load(gFilePath.icon.c_str());
	SDL_SetWindowIcon(gWindow, icon);
	SDL_FreeSurface(icon);

	// Create the renderer
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_PRESENTVSYNC);

	if (gRenderer == nullptr) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to create renderer", ":(", NULL);
		return false;
	}

	// Initialize and load input
	pyrodactyl::input::gInput.Init();

	// Disable the SDL stock cursor and screen-saver
	SDL_ShowCursor(SDL_DISABLE);
	SDL_DisableScreenSaver();

	// Initial check for controllers on the system
	pyrodactyl::input::gInput.AddController();

	gScreenSettings.in_game = false;
	return true;
}

void App::Run() {
	// State IDs
	GameStateID CurrentStateID = GAMESTATE_NULL, NextStateID = GAMESTATE_TITLE;
	bool ShouldChangeState = true;

	// Set the current game state object
	GameState *CurrentState = NULL;
	Timer fps;
	SDL_Event Event;
	int fpscount = 0, fpsval = 1, lasts = 0;

	// While the user hasn't quit - This is the main game loop
	while (CurrentStateID != GAMESTATE_EXIT) {
		// Start the frame timer
		fps.Start();

		// Change state if needed
		if (ShouldChangeState) {
			// Delete the current state
			delete CurrentState;

			if (NextStateID == GAMESTATE_EXIT)
				break;

			// Change the state
			switch (NextStateID) {
			case GAMESTATE_TITLE:
				CurrentState = new Splash();
				gScreenSettings.in_game = false;

				// Now apply all settings - except resolution because that's already set at the start
				gScreenSettings.SetFullscreen();
				gScreenSettings.SetWindowBorder();
				gScreenSettings.SetVsync();
				gScreenSettings.SetGamma();
				gScreenSettings.SetMouseTrap();
				break;

			case GAMESTATE_MAIN_MENU:
				CurrentState = new MainMenu();
				gScreenSettings.in_game = false;
				break;

			case GAMESTATE_NEW_GAME:
				CurrentState = new Game();
				gScreenSettings.in_game = true;
				break;

			case GAMESTATE_LOAD_GAME:
				if (boost::filesystem::exists(pyrodactyl::ui::gLoadMenu.SelectedPath()))
					CurrentState = new Game(pyrodactyl::ui::gLoadMenu.SelectedPath());
				else
					CurrentState = new Game();

				gScreenSettings.in_game = true;
				break;

			default:
				// Encountering an undefined state, exit with an error code
				return;
			}

			// Change the current state ID
			CurrentStateID = NextStateID;

			// NULL the next state ID
			NextStateID = GAMESTATE_NULL;

			// No need to change state until further notice
			ShouldChangeState = false;
		}

		// Do state InternalEvents
		CurrentState->InternalEvents(ShouldChangeState, NextStateID);

		while (SDL_PollEvent(&Event)) {
			// Do state Event handling
			CurrentState->HandleEvents(Event, ShouldChangeState, NextStateID);

			// If the user has closed the window or pressed ALT+F4
			if (Event.type == SDL_QUIT || (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_F4 && Event.key.keysym.mod & KMOD_ALT)) {
				// Save the program state to file
				CurrentState->AutoSave();

				// Quit the program
				ShouldChangeState = true;
				NextStateID = GAMESTATE_EXIT;
			} else if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_RETURN && Event.key.keysym.mod & KMOD_ALT) {
				// Toggle full-screen if user presses ALT+ENTER
				gScreenSettings.fullscreen = !gScreenSettings.fullscreen;
				gScreenSettings.SetFullscreen();
			} else if (Event.type == SDL_KEYUP && Event.key.keysym.scancode == SDL_SCANCODE_GRAVE && Event.key.keysym.mod & KMOD_CTRL)
				GameDebug = !GameDebug;
			pyrodactyl::input::gInput.HandleController(Event);
		}

		// Do we have to reposition our interface?
		if (gScreenSettings.change_interface) {
			CurrentState->SetUI();
			gScreenSettings.change_interface = false;
		}

		// Do state Drawing
		CurrentState->Draw();

		if (GameDebug) {
			if (SDL_GetTicks() - lasts > 1000) {
				lasts = SDL_GetTicks();
				fpsval = fpscount;
				fpscount = 1;
			} else
				++fpscount;

			if (CurrentStateID >= 0)
				pyrodactyl::text::gTextManager.Draw(0, 0, NumberToString(fpsval), 0);
		}

		// Update the screen
		SDL_RenderPresent(gRenderer);

		// Cap the frame rate
		if (fps.Ticks() < 1000u / gScreenSettings.fps)
			SDL_Delay((1000u / gScreenSettings.fps) - fps.Ticks());
	}
}

void App::LoadSettings(const std::string &filename) {
	XMLDoc settings(filename);
	if (settings.ready()) {
		rapidxml::xml_node<char> *node = settings.Doc()->first_node("settings");
		if (NodeValid(node)) {
			// Load the version
			LoadNum(gScreenSettings.version, "version", node);

			// Load screen settings
			if (NodeValid("screen", node))
				gScreenSettings.Load(node->first_node("screen"));

			// Start the sound subsystem
			pyrodactyl::music::gMusicManager.Load(node);
		}
	}
}

App::~App() {
	// SteamAPI_Shutdown();

	pyrodactyl::image::gImageManager.Quit();
	pyrodactyl::music::gMusicManager.Quit();
	pyrodactyl::text::gTextManager.Quit();
	pyrodactyl::input::gInput.Quit();
	gLoadScreen.Quit();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);

	// Quit SDL_ttf
	TTF_Quit();

	// Quit the audio stuff
	Mix_CloseAudio();
	Mix_Quit();

	// Quit SDL_image
	IMG_Quit();

	// Quit SDL
	SDL_Quit();
}