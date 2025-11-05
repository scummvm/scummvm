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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "image/pcx.h"
#include "image/png.h"

#include "pelrock.h"
#include "pelrock/console.h"
#include "pelrock/detection.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

PelrockEngine *g_engine;

PelrockEngine::PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																				 _gameDescription(gameDesc), _randomSource("Pelrock") {
	g_engine = this;
}

PelrockEngine::~PelrockEngine() {
	delete _screen;
}

uint32 PelrockEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String PelrockEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error PelrockEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 400);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, 60);

	init();
	if (shouldPlayIntro == false) {
		stateGame = GAME;
	} else {
		stateGame = INTRO;
		playIntro();
	}

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_MOUSEMOVE) {
				mouseX = e.mouse.x;
				mouseY = e.mouse.y;
				debug(3, "Mouse moved to (%d,%d)", mouseX, mouseY);
			}
		}
		checkMouseHover();
		frames();
		_screen->update();
		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void PelrockEngine::init() {
	CursorMan.setDefaultArrowCursor();
	CursorMan.showMouse(true);
	if (gameInitialized == false) {
		pixelsShadows = new byte[640 * 400];
		// 		sabeUsarElLibroMagico = 0;
		// 		magicWords = false;
		// 		apagaLaLuz = false;
		// 		showUsingObject = -1;
		// 		prevWhichScreen = -1;
		// 		whichScreen = -1;
		gameInitialized = true;

		// 		tutorial.init();

		prevDirX = 0;
		prevDirY = 0;
		dirAlfred = 2;

		objectToShow = "";

		// 		factX = (float) widthScreen / (float) 640.0f;
		// 		factY = (float) heightScreen / (float) 400.0f;

		// xAlfred = (186 * factX);
		// yAlfred = (307 * factY);
		// xAlfred = 186;
		// yAlfred = 307;

		// 		for (int i = 0; i < 14; i++)
		// 		{
		// 			myPestanas[i].x = (int) ((float) myPestanas[i].x * factX);
		// 			myPestanas[i].y = (int) ((float) myPestanas[i].y * factY);
		// 			myPestanas[i].w = (int) ((float) myPestanas[i].w * factX);
		// 			myPestanas[i].h = (int) ((float) myPestanas[i].h * factY);
		// 		}

		// 		movingAlfred = ALFRED_STOPPED;

		// 		listUsedBranchs = new LinkedList<Integer>();
		// 		listUsedBranchs.clear();

		// 		listRects = new LinkedList<Integer>();
		// 		listRects.clear();

		// 		timeIddle = SystemClock.uptimeMillis();

		// 		myListLibros = new LinkedList<libros>();
		// 		myListLibros.clear();

		// 		myListDesactAnims = new LinkedList<dactAnims>();
		// 		myListDesactAnims.clear();

		// 		myListChangesConvs = new LinkedList<changesConvs>();
		// 		myListChangesConvs.clear();

		// 		myListPegas = new LinkedList<pegas>();
		// 		myListPegas.clear();

		// 		myListObjects = new LinkedList<objects>();
		// 		myListObjects.clear();

		// 		myListUsingObjects = new LinkedList<Integer>();
		// 		myListUsingObjects.clear();

		// 		isPersonajeTalking = -1;

		// 		stanteriaABuscar = -1;

		// 		selectedUsingObject = false;

		// 		currentTrack = "";

		// 		mainVolumen = 50;
		// 		mainTextSpeed = 50;

		// 		extraDataToSave = new byte[98];

		// 		leeLibros();
		// 		myoverlay.setCredits();
		loadAnims();
		// 		loadOtherBitmaps();
		setScreen(2, 2);
		// setScreen(0, 2);
		// 		valSound1 = 0;
		// 		valSound2 = 0;
		// 		valSound3 = 0;

		// 		loadExtraTextsToTranslate();

		// 		loadTutorialText();

		// 		if (myMainActivity == null)
		// 			myMainActivity = this;// new AlfredActivity();

		// 		this.addContentView(myoverlay,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						android.view.ViewGroup.LayoutParams.FILL_PARENT,
		// 						android.view.ViewGroup.LayoutParams.FILL_PARENT));

		// 		settingsLayout = new RelativeLayout(myContext);
		// 		settingsLayout
		// 				.setLayoutParams(new android.view.ViewGroup.LayoutParams(
		// 						android.view.ViewGroup.LayoutParams.FILL_PARENT,
		// 						android.view.ViewGroup.LayoutParams.FILL_PARENT));

		// 		settingsElem1 = new SeekBar(myContext);
		// 		settingsElem2 = new SeekBar(myContext);
		// 		settingsElem3 = new Spinner(myContext);
		// 		settingsElem4 = new Button(myContext);

		// 		String[] items = new String[myLanguages.length];// {"English",
		// 														// "Spanish",
		// 														// "Three"};
		// 		for (i = 0; i < myLanguages.length; i++)
		// 		{
		// 			items[i] = new String();
		// 			items[i] = myLanguages[i].nShow;
		// 		}

		// 		mySpinAdapter<String> adapter = new mySpinAdapter<String>(myContext,
		// 				items);

		// 		settingsElem3.setAdapter(adapter);

		// 		settingsElem3.setSelection(whichLanguageIndex);

		// 		String stringTutorial = "";
		// 		String namet = AlfredActivity.destinationFolder + "/tutorial.kk";
		// 		File ft = new File(namet);

		// 		if (ft.exists() == true)
		// 		{
		// 			byte b[] = tools.readFile(namet);
		// 			b[0]++;

		// 			if (b[0] >= 3)
		// 			{
		// 				b[0] = 3;
		// 				playTutorial = false;
		// 				stringTutorial = "OFF";
		// 			} else
		// 			{
		// 				playTutorial = true;
		// 				stringTutorial = "ON";
		// 			}

		// 			tools.createFile(namet, b);

		// 		} else
		// 		{
		// 			byte b[] = new byte[1];

		// 			b[0] = 0;

		// 			tools.createFile(namet, b);
		// 			playTutorial = true;
		// 			stringTutorial = "ON";
		// 		}

		// 		String finalText = "Tutorial " + stringTutorial;

		// 		settingsElem4.setText(finalText);
		// 		settingsElem4.postInvalidate();
		// 		settingsElem4.setBackgroundColor(0x00000000);

		// 		settingsElem1.setOnSeekBarChangeListener(
		// 				new SeekBar.OnSeekBarChangeListener()
		// 				{

		// 					public void onProgressChanged(SeekBar seekBar,
		// 							int progress, boolean fromTouch)
		// 					{
		// 						setVolumen(progress);
		// 					}

		// 					public void onStartTrackingTouch(SeekBar seekBar)
		// 					{
		// 					}

		// 					public void onStopTrackingTouch(SeekBar seekBar)
		// 					{
		// 					}
		// 				});
		// 		settingsElem2.setOnSeekBarChangeListener(
		// 				new SeekBar.OnSeekBarChangeListener()
		// 				{

		// 					public void onProgressChanged(SeekBar seekBar,
		// 							int progress, boolean fromTouch)
		// 					{
		// 						// mainTextSpeed=progress;
		// 						factTimeToShow = 10 + (100 - progress);
		// 					}

		// 					public void onStartTrackingTouch(SeekBar seekBar)
		// 					{
		// 					}

		// 					public void onStopTrackingTouch(SeekBar seekBar)
		// 					{
		// 					}
		// 				});

		// 		settingsElem3.setOnItemSelectedListener(new OnItemSelectedListener()
		// 		{

		// 			@Override
		// 			public void onItemSelected(AdapterView<?> arg0, View arg1,
		// 					int arg2, long arg3)
		// 			{

		// 				// trying to avoid undesired spinner selection changed
		// 				// event, a known problem
		// 				if (m_intSpinnerInitiCount < NO_OF_EVENTS)
		// 				{
		// 					m_intSpinnerInitiCount++;
		// 				} else
		// 				{
		// 					whichLanguage = myLanguages[arg2].nInternal;
		// 					whichLanguageInt = arg2;
		// 					loadExtraTextsToTranslate();
		// 					setScreen(whichScreen, dirAlfred);
		// 					leeLibros();
		// 					myoverlay.setCredits();
		// 					loadTutorialText();
		// 				}
		// 			}

		// 			@Override
		// 			public void onNothingSelected(AdapterView<?> arg0)
		// 			{

		// 			}

		// 		});

		// 		// set up list view
		// 		//////////////////
		// 		saveLoadLayout = new LinearLayout(myContext);
		// 		saveLoadLayout
		// 				.setLayoutParams(new android.view.ViewGroup.LayoutParams(
		// 						android.view.ViewGroup.LayoutParams.FILL_PARENT,
		// 						android.view.ViewGroup.LayoutParams.FILL_PARENT));

		// 		saveLoadListView = new ListView(myContext);

		// 		saveLoadListView.setOnItemClickListener(new OnItemClickListener()
		// 		{

		// 			@Override
		// 			public void onItemClick(AdapterView<?> arg0, View arg1,
		// 					final int arg2, long arg3)
		// 			{

		// 				if (saveOrLoad == 0)// save
		// 				{

		// 					AlertDialog.Builder alert = new AlertDialog.Builder(
		// 							myContext);

		// 					alert.setTitle("Dreamtripper");
		// 					alert.setMessage(extraThingsToTranslate[4]);
		// 					alert.setIcon(R.drawable.alfred);
		// 					final EditText input = new EditText(myContext);
		// 					String pp = nombrePartidas[arg2];
		// 					if (pp.compareTo(extraThingsToTranslate[7]) == 0)
		// 						pp = "";
		// 					input.setText(pp);
		// 					alert.setView(input);

		// 					alert.setPositiveButton("Ok",
		// 							new DialogInterface.OnClickListener()
		// 							{
		// 								public void onClick(DialogInterface dialog,
		// 										int whichButton)
		// 								{
		// 									grabaPartida(arg2,
		// 											input.getText().toString());
		// 									tools.showMessage(myContext,
		// 											"Dreamtripper",
		// 											extraThingsToTranslate[6],
		// 											"OK");
		// 									getNamesPartidas();
		// 								}
		// 							});

		// 					alert.setNegativeButton("Cancel",
		// 							new DialogInterface.OnClickListener()
		// 							{
		// 								public void onClick(DialogInterface dialog,
		// 										int whichButton)
		// 								{
		// 								}
		// 							});

		// 					alert.show();

		// 				} else
		// 				{// load

		// 					cargaPartida(arg2);

		// 				}

		// 			}

		// 		});

		// 		saveLoadLayout.addView(saveLoadListView,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
		// 						android.view.ViewGroup.LayoutParams.WRAP_CONTENT));

		// 		settingsLayout.addView(settingsElem1,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						(int) ((float) 180 * factX),
		// 						(int) ((float) 50 * factY)));

		// 		settingsLayout.addView(settingsElem2,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						(int) ((float) 180 * factX),
		// 						(int) ((float) 50 * factY)));

		// 		settingsLayout.addView(settingsElem3,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						(int) ((float) 120 * factX),
		// 						(int) ((float) 50 * factY)));

		// 		settingsLayout.addView(settingsElem4,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						(int) ((float) 130 * factX),
		// 						(int) ((float) 50 * factY)));

		// 		ViewGroup.MarginLayoutParams mlp2 = (ViewGroup.MarginLayoutParams) settingsElem1
		// 				.getLayoutParams();
		// 		mlp2.setMargins((int) ((float) 300 * factX),
		// 				(int) ((float) 195 * factY),
		// 				(int) ((float) (640 - 460) * factX),
		// 				(int) ((float) (400 - 245) * factY));

		// 		ViewGroup.MarginLayoutParams mlp3 = (ViewGroup.MarginLayoutParams) settingsElem2
		// 				.getLayoutParams();
		// 		mlp3.setMargins((int) ((float) 300 * factX),
		// 				(int) ((float) 235 * factY),
		// 				(int) ((float) (640 - 460) * factX),
		// 				(int) ((float) (400 - 285) * factY));

		// 		ViewGroup.MarginLayoutParams mlp4 = (ViewGroup.MarginLayoutParams) settingsElem3
		// 				.getLayoutParams();
		// 		mlp4.setMargins((int) ((float) 340 * factX),
		// 				(int) ((float) 285 * factY),
		// 				(int) ((float) (640 - 460) * factX),
		// 				(int) ((float) (400 - 325) * factY));

		// 		ViewGroup.MarginLayoutParams mlp5 = (ViewGroup.MarginLayoutParams) settingsElem4
		// 				.getLayoutParams();
		// 		mlp5.setMargins((int) ((float) 210 * factX),
		// 				(int) ((float) 285 * factY),
		// 				(int) ((float) (640 - 360) * factX),
		// 				(int) ((float) (400 - 325) * factY));

		// 		ViewGroup.MarginLayoutParams mlp = (ViewGroup.MarginLayoutParams) saveLoadListView
		// 				.getLayoutParams();
		// 		mlp.setMargins((int) ((float) 223 * factX),
		// 				(int) ((float) 200 * factY),
		// 				(int) ((float) (640 - 466) * factX),
		// 				(int) ((float) (400 - 312) * factY));

		// 		settingsElem1.setProgress(mainVolumen);
		// 		settingsElem2.setProgress(mainTextSpeed);

		// 		this.addContentView(saveLoadLayout,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
		// 						android.view.ViewGroup.LayoutParams.WRAP_CONTENT));

		// 		this.addContentView(settingsLayout,
		// 				new android.view.ViewGroup.LayoutParams(
		// 						android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
		// 						android.view.ViewGroup.LayoutParams.WRAP_CONTENT));

		// 		settingsElem4.setOnClickListener(new OnClickListener()
		// 		{

		// 			@Override
		// 			public void onClick(View arg0)
		// 			{

		// 				String stringTutorial = "";
		// 				String namet = AlfredActivity.destinationFolder
		// 						+ "/tutorial.kk";

		// 				byte b[] = tools.readFile(namet);
		// 				if (playTutorial == false)// (b[0]==0)
		// 				{
		// 					b[0] = 0;
		// 					playTutorial = true;
		// 					stringTutorial = "ON";

		// 					tutorial.init();

		// 				} else
		// 				{
		// 					b[0] = 4;
		// 					playTutorial = false;
		// 					stringTutorial = "OFF";
		// 				}

		// 				tools.createFile(namet, b);

		// 				String fiinalText = "Tutorial " + stringTutorial;

		// 				settingsElem4.setText(fiinalText);
		// 				settingsElem4.postInvalidate();

		// 			}

		// 		});

		// 		hideList();
		// 		hideSettings();

		// 		myExtraText = new extraText();

		// 		String name = AlfredActivity.destinationFolder + "/test.kk";
		// 		File f = new File(name);
		// 		if (f.exists() == true)
		// 		{
		// 			byte b[] = tools.readFile(name);

		// 			if (b[0] < 3)
		// 			{
		// 				b[0]++;
		// 				playIntro = true;
		// 			} else
		// 				playIntro = false;

		// 			tools.createFile(name, b);

		// 		} else
		// 		{
		// 			byte b[] = new byte[1];

		// 			b[0] = 0;
		// 			tools.createFile(name, b);
		// 			playIntro = true;
		// 		}

		// 		// playIntro=true;
		// 		if (playIntro == false)
		// 		{
		// 			stateGame = GAME;
		// 		} else
		// 		{

		// 			stateGame = INTRO;
		// 			varCheckRealTime = 0;
		// 			getWindow().setFormat(PixelFormat.TRANSLUCENT);
		// 			videoHolder = new myVideoView(this);

		// 			videoHolder.setWH(widthScreen, heightScreen);

		// 			// videoHolder.setLayoutParams(params)

		// 			Uri video = Uri.parse("android.resource://" + getPackageName()
		// 					+ "/" + R.raw.intro2); // do not add any extension
		// 			videoHolder.setVideoURI(video);

		// 			LinearLayout.LayoutParams paramsVideo = new LinearLayout.LayoutParams(
		// 					widthScreen, heightScreen);

		// 			videoOverlay vo = new videoOverlay(myContext, assetManager,
		// 					videoHolder);

		// 			this.addContentView(videoHolder, paramsVideo);// new
		// 															// android.view.ViewGroup.LayoutParams(paramsVideo));

		// 			this.addContentView(vo, paramsVideo);// new
		// 													// android.view.ViewGroup.LayoutParams(paramsVideo));

		// 			// superTime=SystemClock.uptimeMillis();
		// 			videoHolder.start();

		// 			AlfredActivity.playTrack(22, false);

		// 			videoHolder.setOnTouchListener(new OnTouchListener()
		// 			{
		// 				@Override
		// 				public boolean onTouch(View arg0, MotionEvent arg1)
		// 				{
		// 					videoHolder.setVisibility(View.GONE);
		// 					videoHolder.stopPlayback();

		// 					AlfredActivity.myHandler
		// 							.sendEmptyMessage(AlfredActivity.INTRO_END);

		// 					return false;
		// 				}

		// 			});

		// 			videoHolder.setOnCompletionListener(new OnCompletionListener()
		// 			{

		// 				@Override
		// 				public void onCompletion(MediaPlayer arg0)
		// 				{
		// 					wait2(4000);
		// 				}

		// 			});

		// 		}
	}
}

void PelrockEngine::playIntro() {
}

void PelrockEngine::loadAnims() {
	loadMainCharacterAnims();
	// try
	// {

	// 	// andar, acciones y hablar...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred1.png");

	// 	for (int i = 0; i < 60; i++)
	// 	{
	// 		alfredBitmap[i] = Bitmap.createBitmap(mainAlfredAnim, i * 51, 0,
	// 				51, 102);
	// 	}

	// 	mainAlfredAnim.recycle();

	// 	// andar por el tunel
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred13.png");

	// 	for (int i = 0; i < 18; i++)
	// 	{
	// 		alfredBitmap2[i] = Bitmap.createBitmap(mainAlfredAnim, i * 130,
	// 				0, 130, 55);
	// 	}

	// 	mainAlfredAnim.recycle();

	// 	// animacion de peinarse mirando a la izquierda...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred2.png");

	// 	for (int i = 0; i < 11; i++)
	// 		alfredBitmapExtra1[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 51, 0, 51, 102);

	// 	mainAlfredAnim.recycle();

	// 	// animacion de peinarse mirando a la derecha...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred3.png");

	// 	for (int i = 0; i < 11; i++)
	// 		alfredBitmapExtra2[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 51, 0, 51, 102);

	// 	mainAlfredAnim.recycle();

	// 	// animacion de leer...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred4.png");

	// 	for (int i = 0; i < 10; i++)
	// 		alfredBitmapExtra3[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 51, 0, 51, 102);

	// 	// animacion de electrocutarse...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred5.png");

	// 	for (int i = 0; i < 8; i++)
	// 		alfredBitmapExtra4[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 82, 0, 82, 58);

	// 	// animacion de cocodrilo...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/crocodillo.png");

	// 	for (int i = 0; i < 14; i++)
	// 		alfredBitmapExtra5[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 171, 0, 171, 109);

	// 	// animacion de escondite...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred6.png");

	// 	for (int i = 0; i < 12; i++)
	// 		alfredBitmapExtra6[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 113, 0, 113, 103);

	// 	// animacion de bajada al tunel...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred7.png");

	// 	for (int i = 0; i < 11; i++)
	// 		alfredBitmapExtra7[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 33, 0, 33, 72);

	// 	// animacion de subida del tunel...
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred8.png");

	// 	for (int i = 0; i < 9; i++)
	// 		alfredBitmapExtra8[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 33, 0, 33, 72);

	// 	// animacion de escapada del tunel (llega al zoco)
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred9.png");

	// 	for (int i = 0; i < 16; i++)
	// 		alfredBitmapExtra9[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 158, 0, 158, 115);

	// 	// animacion de puesta del muneco hinchable
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred10.png");

	// 	for (int i = 0; i < 17; i++)
	// 		alfredBitmapExtra10[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 177, 0, 177, 124);

	// 	// animacion de magia (1)
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/magia1.png");

	// 	for (int i = 0; i < 11; i++)
	// 		alfredBitmapExtra11[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 98, 0, 98, 138);

	// 	// animacion de magia (2)
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/magia2.png");

	// 	for (int i = 0; i < 12; i++)
	// 		alfredBitmapExtra12[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 98, 0, 98, 138);

	// 	// animacion de pedras
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred11.png");

	// 	for (int i = 0; i < 7; i++)
	// 		alfredBitmapExtra13[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 208, 0, 208, 102);

	// 	// animacion de desnudo
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred12.png");

	// 	for (int i = 0; i < 4; i++)
	// 		alfredBitmapExtra14[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 51, 0, 51, 102);

	// 	// animacion de despertarse
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred14.png");

	// 	for (int i = 0; i < 14; i++)
	// 		alfredBitmapExtra15[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 71, 0, 71, 66);

	// 	// animacion de tirar piedra
	// 	mainAlfredAnim = getBitmapFromAsset("animaciones/alfred15.png");

	// 	for (int i = 0; i < 4; i++)
	// 		alfredBitmapExtra16[i] = Bitmap.createBitmap(mainAlfredAnim,
	// 				i * 71, 0, 71, 101);

	// 	mainAlfredAnim.recycle();

	// } catch (IOException e)
	// {
	// }
}

const int EXPECTED_SIZE = 640 * 400;
size_t decompress_rle_block(const uint8_t *data, size_t data_size, uint32_t offset, uint32_t size, uint8_t **out_data) {
	// Check for uncompressed markers
	if (size == 0x8000 || size == 0x6800) {
		*out_data = (uint8_t *)malloc(size);
		memcpy(*out_data, data + offset, size);
		return size;
	}

	// RLE compressed
	*out_data = (uint8_t *)malloc(EXPECTED_SIZE * 2); // Allocate enough space
	size_t result_size = 0;

	uint32_t pos = offset;
	uint32_t end = offset + size;

	while (pos + 2 <= end && pos + 2 <= data_size) {
		// Check for BUDA marker
		if (pos + 4 <= data_size &&
			data[pos] == 'B' && data[pos + 1] == 'U' &&
			data[pos + 2] == 'D' && data[pos + 3] == 'A') {
			break;
		}

		uint8_t count = data[pos];
		uint8_t value = data[pos + 1];

		for (int i = 0; i < count; i++) {
			(*out_data)[result_size++] = value;
		}

		pos += 2;
	}

	return result_size;
}

void PelrockEngine::getPalette(Common::File *roomFile, int roomOffset, byte *palette) {
	// get palette
	int paletteOffset = roomOffset + (11 * 8);
	roomFile->seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile->readUint32LE();
	uint32 size = roomFile->readUint32LE();

	roomFile->seek(offset, SEEK_SET);

	roomFile->read(palette, size);
	for (int i = 0; i < 256; i++) {
		palette[i * 3] = palette[i * 3] << 2;
		palette[i * 3 + 1] = palette[i * 3 + 1] << 2;
		palette[i * 3 + 2] = palette[i * 3 + 2] << 2;
	}
}

void PelrockEngine::getBackground(Common::File *roomFile, int roomOffset, byte *background) {
	roomFile->seek(0, SEEK_SET);
	// get screen
	size_t combined_size = 0;
	size_t uncompressed_size = 0;
	for (int pair_idx = 0; pair_idx < 8; pair_idx++) {
		uint32_t pair_offset = roomOffset + (pair_idx * 8);
		if (pair_offset + 8 > roomFile->size())
			continue;

		roomFile->seek(pair_offset, SEEK_SET);
		uint32_t offset = roomFile->readUint32LE();
		uint32_t size = roomFile->readUint32LE();
		uncompressed_size += size;

		if (offset > 0 && size > 0 && offset < roomFile->size()) {
			byte *data = new byte[size];
			roomFile->seek(offset, SEEK_SET);
			roomFile->read(data, size);
			uint8_t *block_data = NULL;
			size_t block_size = decompress_rle_block(data, size, 0, size, &block_data);

			memcpy(background + combined_size, block_data, block_size);
			combined_size += block_size + 1;
			free(block_data);
			delete[] data;
		}
	}
}

Common::List<AnimSet> PelrockEngine::getRoomAnimations(Common::File *roomFile, int roomOffset) {
	uint32_t pair_offset = roomOffset + (8 * 8);
	roomFile->seek(pair_offset, SEEK_SET);
	uint32_t offset = roomFile->readUint32LE();
	uint32_t size = roomFile->readUint32LE();

	byte *data = new byte[size];
	roomFile->seek(offset, SEEK_SET);
	roomFile->read(data, size);

	unsigned char *pic = new byte[10000 * 10000];
	if (offset > 0 && size > 0) {
		decompress_rle_block(data, size, 0, size, &pic);
	} else {
		return Common::List<AnimSet>();
	}
	Common::List<AnimSet> anims = Common::List<AnimSet>();
	uint32_t spriteEnd = offset + size;
	uint32_t metadata_start = spriteEnd + 108;
	uint32_t picOffset = 0;
	for (int i = 0; i < 10; i++) {
		uint32_t animOffset = metadata_start + (i * 44);
		roomFile->seek(animOffset, SEEK_SET);

		int16 x = roomFile->readSint16LE();
		int16 y = roomFile->readSint16LE();
		byte w = roomFile->readByte();
		byte h = roomFile->readByte();
		roomFile->skip(2); // reserved
		int secAnimCount = roomFile->readByte();
		roomFile->skip(1);
		byte frames = 0;
		for (int i = 0; i < secAnimCount; i++) {
			frames += roomFile->readByte();
		}
		if (w > 0 && h > 0 && frames > 0) {
			AnimSet anim;
			anim.x = x;
			anim.y = y;
			anim.w = w;
			anim.h = h;
			anim.nframes = frames;
			uint32_t needed = anim.w * anim.h * anim.nframes;
			anim.animData = new byte[needed];
			Common::copy(pic + picOffset, pic + picOffset + needed, anim.animData);
			picOffset += needed;
			debug("Anim %d: x=%d y=%d w=%d h=%d nframes=%d", i, anim.x, anim.y, anim.w, anim.h, anim.nframes);
			anims.push_back(anim);
		}
	}
	return anims;
}

Common::List<WalkBox> PelrockEngine::loadWalkboxes(Common::File *roomFile, int roomOffset) {
	uint32_t pair10_offset_pos = roomOffset + (10 * 8);
	roomFile->seek(pair10_offset_pos, SEEK_SET);
	// roomFile->skip(4);
	uint32_t pair10_data_offset = roomFile->readUint32LE();
	uint32_t pair10_size = roomFile->readUint32LE();

	uint32_t walkbox_countOffset = pair10_data_offset + 0x213;
	roomFile->seek(walkbox_countOffset, SEEK_SET);
	byte walkbox_count = roomFile->readByte();
	debug("Walkbox count: %d", walkbox_count);
	uint32_t walkbox_offset = pair10_data_offset + 0x218;
	Common::List<WalkBox> walkboxes;
	for (int i = 0; i < walkbox_count; i++) {
		uint32_t box_offset = walkbox_offset + i * 9;
		roomFile->seek(box_offset, SEEK_SET);
		int16 x1 = roomFile->readSint16LE();
		int16 y1 = roomFile->readSint16LE();
		int16 w = roomFile->readSint16LE();
		int16 h = roomFile->readSint16LE();
		byte flags = roomFile->readByte();
		debug("Walkbox %d: x1=%d y1=%d w=%d h=%d", i, x1, y1, w, h);
		WalkBox box;
		box.x = x1;
		box.y = y1;
		box.w = w;
		box.h = h;
		box.flags = flags;
		walkboxes.push_back(box);
	}
	return walkboxes;
}

void PelrockEngine::loadHotspots(Common::File *roomFile, int roomOffset) {
	uint32_t pair10_offset_pos = roomOffset + (10 * 8);
	roomFile->seek(pair10_offset_pos, SEEK_SET);
	// roomFile->skip(4);
	uint32_t pair10_data_offset = roomFile->readUint32LE();
	uint32_t pair10_size = roomFile->readUint32LE();
	uint32_t count_offset = pair10_data_offset + 0x47a;
	roomFile->seek(count_offset, SEEK_SET);
	byte hotspot_count = roomFile->readByte();
	uint32_t hotspot_data_start = pair10_data_offset + 0x47c;
	Common::List<HotSpot> hotspots;
	for (int i = 0; i < hotspot_count; i++) {
		uint32_t obj_offset = hotspot_data_start + i * 9;
		roomFile->seek(obj_offset, SEEK_SET);
		byte obj_bytes[9];
		roomFile->read(obj_bytes, 9);
		byte type_byte = obj_bytes[0];
		HotSpot spot;
		spot.x = obj_bytes[1] | (obj_bytes[2] << 8);
		spot.y = obj_bytes[3] | (obj_bytes[4] << 8);
		spot.w = obj_bytes[5];
		spot.h = obj_bytes[6];
		spot.extra = obj_bytes[7] | (obj_bytes[8] << 8);
		// debug("Hotspot %d: type=%d x=%d y=%d w=%d h=%d extra=%d", i, type_byte, spot.x, spot.y, spot.w, spot.h, spot.extra);
		hotspots.push_back(spot);
	}
	_hotspots = hotspots;
}

void PelrockEngine::loadMainCharacterAnims() {
	Common::File alfred3;
	if (!alfred3.open(Common::Path("ALFRED.3"))) {
		error("Could not open ALFRED.3");
		return;
	}
	int alfred3Size = alfred3.size();
	unsigned char *bufferFile = (unsigned char *)malloc(alfred3Size);
	alfred3.seek(0, SEEK_SET);
	alfred3.read(bufferFile, alfred3Size);
	alfred3.close();

	int index = 0;
	int index3 = 0;
	uint32_t capacity = 3060 * 102;
	unsigned char *pic = new unsigned char[capacity];
	decompress_rle_block(bufferFile, alfred3Size, 0, alfred3Size, &pic);
	memcpy(standingAnim, pic, 3060 * 102);
}

void PelrockEngine::frames() {
	for (uint32_t y = 0; y < kAlfredFrameHeight; y++) {
		for (uint32_t x = 0; x < kAlfredFrameWidth; x++) {
			unsigned int src_pos = (curAlfredFrame * kAlfredFrameHeight * kAlfredFrameWidth) + (y * kAlfredFrameWidth) + x;
			if (standingAnim[src_pos] != 255)
				_screen->setPixel(x + xAlfred, y + yAlfred, standingAnim[src_pos]);
		}
	}
	_screen->markAllDirty();
	_screen->update();
}

void PelrockEngine::checkMouseHover() {
	for (Common::List<HotSpot>::iterator i = _hotspots.begin(); i != _hotspots.end(); i++) {
		if (mouseX >= i->x && mouseX <= (i->x + i->w) &&
			mouseY >= i->y && mouseY <= (i->y + i->h)) {
			// _currentHotspot = &(*i);
			debug("Hotspot at (%d,%d) size (%d,%d) extra %d", i->x, i->y, i->w, i->h, i->extra);
			return;
		}
	}
}

void PelrockEngine::setScreen(int number, int dir) {

	Common::File roomFile;
	if (!roomFile.open(Common::Path("ALFRED.1"))) {
		error("Could not open ALFRED.1");
		return;
	}

	int roomOffset = number * kRoomStructSize;

	byte *palette = new byte[256 * 3];
	getPalette(&roomFile, roomOffset, palette);

	int paletteOffset = roomOffset + (11 * 8);
	roomFile.seek(paletteOffset, SEEK_SET);
	uint32 offset = roomFile.readUint32LE();

	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	byte *background = new byte[640 * 400];
	getBackground(&roomFile, roomOffset, background);
	for (int i = 0; i < 640; i++) {
		for (int j = 0; j < 400; j++) {
			_screen->setPixel(i, j, background[j * 640 + i]);
		}
	}
	Common::List<AnimSet> anims = getRoomAnimations(&roomFile, roomOffset);
	int num = 0;
	for (Common::List<AnimSet>::iterator i = anims.begin(); i != anims.end(); i++) {
		byte *frame = new byte[i->w * i->h];

		Common::copy(i->animData, i->animData + (i->w * i->h), frame);

		for (int y = 0; y < i->h; y++) {
			for (int x = 0; x < i->w; x++) {
				unsigned int src_pos = (y * i->w) + x;
				int xPos = i->x + x;
				int yPos = i->y + y;
				if (frame[src_pos] != 255 && xPos > 0 && yPos > 0 && xPos < 640 && yPos < 400)
					_screen->setPixel(xPos, yPos, frame[src_pos]);
			}
		}
	}
	loadHotspots(&roomFile, roomOffset);
	Common::List<WalkBox> walkboxes = loadWalkboxes(&roomFile, roomOffset);
	int walkboxCount = 0;
	for (Common::List<WalkBox>::iterator i = walkboxes.begin(); i != walkboxes.end(); i++) {
		// _screen->fillRect(Common::Rect(i->x, i->y, i->x + i->w, i->y + i->h), 255);
		_screen->drawLine(i->x, i->y, i->x + i->w, i->y, 0 + walkboxCount);
		_screen->drawLine(i->x, i->y + i->h, i->x + i->w, i->y + i->h, 0 + walkboxCount);
		_screen->drawLine(i->x, i->y, i->x, i->y + i->h, 0 + walkboxCount);
		_screen->drawLine(i->x + i->w, i->y, i->x + i->w, i->y + i->h, 0 + walkboxCount);
		walkboxCount++;
	}
	_screen->markAllDirty();
	roomFile.close();
	delete[] background;
	delete[] palette;
}

void PelrockEngine::setScreenJava(int s, int dir) {
	screenReady = false;
	dirAlfred = dir;

	// 		vueltaALaCarcel = false;
	// 		timeToGoToPiss = -1;
	// 		myoverlay.shakeScreen = false;
	// 		walkingAgain = false;

	// 		myoverlay.indexCicleColores = 0;

	// 		stringToShowOnTutorial = "";

	// 		magicWords = false;

	// 		myMovingAlfredThread.isOtherTalking = false;
	// 		isPersonajeTalking = -1;
	// 		myMovingAlfredThread.showTextNowOtros = false;

	// 		// pantalla................
	// 		if (screenBitmap != null)
	// 		{
	// 			screenBitmap.recycle();
	// 			screenBitmap = null;
	// 		}

	prevWhichScreen = whichScreen;

	whichScreen = s;

	// 		objetos.getObjectList(assetManager, s);
	// 		objectToShow = "";

	Common::String nameScreen = Common::String::format("pantallas/pantalla%d.png", s);
	Common::String nameCamino = Common::String::format("caminos/pantalla%d.txt", s);

	Common::File screen;
	if (!screen.open(Common::Path(nameScreen))) {
		error("Could not find pantalla!");
	}
	decoder->loadStream(screen);
	Graphics::Palette palette = decoder->getPalette();
	g_system->getPaletteManager()->setPalette(palette);
	const Graphics::Surface *surf = decoder->getSurface();
	if (!surf)
		error("No surface");

	g_engine->_screen->blitFrom(*surf);

	_screen->markAllDirty();

	// 		Config c = null;
	// 		try
	// 		{
	// 			screenBitmap = getBitmapFromAsset(nameScreen);

	// 			c = screenBitmap.getConfig();
	// 			screenBitmap = screenBitmap.copy(c, true);
	// 		} catch (IOException e)
	// 		{
	// 		}

	Common::String nameColorsx = Common::String::format("colors%d.png", s);
	Common::String nameColors = "colors/" + nameColorsx;
	// 		try
	// 		{
	// 			screenColors = getBitmapFromAsset(nameColors);
	// 		} catch (IOException e)
	// 		{
	// 			screenColors = null;
	// 		}
	// 		if (screenColors != null)
	// 			setFakePalette(nameColorsx, screenColors);

	Common::String nameShadows = Common::String::format("shadows/shadows_%d.png", s); // <-- fixed: include s
	Common::File shadows;
	if (shadows.open(Common::Path(nameShadows)) != Common::kNoError) {
		error("Error opening shadows: %s", nameShadows.c_str());
		return;
	}
	if (decoder->loadStream(shadows) != Common::kNoError) {
		error("Decoder failed loading shadows: %s", nameShadows.c_str());
		return;
	}
	const Graphics::Surface *shadowSurf = decoder->getSurface();
	if (!shadowSurf) {
		error("No shadow surface for %s", nameShadows.c_str());
		return;
	}
	Common::copy((byte *)shadowSurf->getPixels(), (byte *)shadowSurf->getPixels() + 640 * 400, pixelsShadows);

	// 		try
	// 		{
	// 			shadowColors = getBitmapFromAsset(nameShadows);
	// 		} catch (IOException e)
	// 		{
	// 			shadowColors = null;
	// 		}

	// 		shadowColors.getPixels(pixelsShadows, 0, 640, 0, 0, 640, 400);

	Common::File caminos;
	if (!caminos.open(Common::Path(nameCamino))) {
		error("Cant find camino");
	}

	// 		// caminos................
	// 		BufferedReader br = null;
	// 		List<String> wordList = new ArrayList<String>();

	// 		try
	// 		{
	// 			br = new BufferedReader(
	// 					new InputStreamReader(assetManager.open(nameCamino)));
	// 			String word;
	// 			while ((word = br.readLine()) != null)
	// 				wordList.add(word);
	// 		} catch (IOException e)
	// 		{
	// 		}

	// 		paths.caminos = new defCam();

	// 		int k = wordList.size();

	// 		List<rectCam> provList = new LinkedList<rectCam>();
	// 		provList.clear();

	// 		List<conex> provListCon = new LinkedList<conex>();
	// 		provListCon.clear();

	// 		List<anims> provListAnims = new LinkedList<anims>();
	// 		provListAnims.clear();

	// 		myScale = null;

	// 		scale aux4 = null;

	// 		String line;
	// 		int ind = -1;
	// 		int prevInd = 0;
	// 		int which = 0;

	// 		for (int i = 0; i < k; i++)
	// 		{

	// 			line = wordList.get(i);

	// 			if (line.compareTo("paths") == 0)
	// 				which = 0;
	// 			else if (line.compareTo("conexions") == 0)
	// 				which = 1;
	// 			else if (line.compareTo("anims") == 0)
	// 				which = 2;
	// 			else if (line.compareTo("scale") == 0)
	// 				which = 3;
	// 			else
	// 			{

	// 				switch (which)
	// 				{
	// 				case 0:

	// 					rectCam aux = new rectCam();

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux.x = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factX);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux.y = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factY);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux.w = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factX);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux.h = (int) (Float.parseFloat(line.substring(prevInd))
	// 							* factY);
	// 					prevInd = ind + 1;

	// 					ind = -1;
	// 					prevInd = 0;

	// 					provList.add(aux);

	// 				break;

	// 				case 1:

	// 					conex aux2 = new conex();

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.x = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factX);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.y = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factY);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.which = (int) (Integer
	// 							.parseInt(line.substring(prevInd, ind)));
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.ready = (int) (Integer
	// 							.parseInt(line.substring(prevInd, ind)));
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.nx = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factX);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.ny = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factY);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux2.ndir = (int) (Integer
	// 							.parseInt(line.substring(prevInd)));
	// 					prevInd = ind + 1;

	// 					ind = -1;
	// 					prevInd = 0;

	// 					provListCon.add(aux2);
	// 				break;

	// 				case 2:

	// 					anims aux3 = new anims();

	// 					aux3.activated = true;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux3.x = (int) (Integer
	// 							.parseInt(line.substring(prevInd, ind)) * factX);
	// 					aux3.backupX = aux3.x;
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux3.y = (int) (Integer
	// 							.parseInt(line.substring(prevInd, ind)) * factY);
	// 					aux3.backupY = aux3.y;
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux3.moving = (boolean) (Boolean
	// 							.parseBoolean((line.substring(prevInd, ind))));
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux3.frames = (int) (Integer
	// 							.parseInt(line.substring(prevInd, ind)));
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					if (ind == -1)
	// 						aux3.speed = (int) (Integer
	// 								.parseInt(line.substring(prevInd)));
	// 					else
	// 						aux3.speed = (int) (Integer
	// 								.parseInt(line.substring(prevInd, ind)));
	// 					prevInd = ind + 1;

	// 					aux3.whichFrame = 0;
	// 					aux3.nSpeed = 0;

	// 					if (ind != -1)// otros datos en las animaciones...
	// 					{
	// 						int kk;
	// 						if (aux3.moving == false)// los datos extras son de
	// 													// animaciones 'combinadas'
	// 						{

	// 							ind = line.indexOf(',', ind + 1);
	// 							kk = (int) (Integer
	// 									.parseInt(line.substring(prevInd, ind)));
	// 							prevInd = ind + 1;

	// 							aux3.animParts = new parts[kk];
	// 							aux3.time = 0;
	// 							aux3.whichPart = 0;

	// 							for (int j = 0; j < kk; j++)
	// 							{
	// 								aux3.animParts[j] = new parts();

	// 								ind = line.indexOf(',', ind + 1);
	// 								aux3.animParts[j].nframes = (int) (Integer
	// 										.parseInt(
	// 												line.substring(prevInd, ind)));
	// 								prevInd = ind + 1;

	// 								ind = line.indexOf(',', ind + 1);
	// 								if (ind == -1)
	// 									aux3.animParts[j].duration = (int) (Integer
	// 											.parseInt(line.substring(prevInd)));
	// 								else
	// 									aux3.animParts[j].duration = (int) (Integer
	// 											.parseInt(line.substring(prevInd,
	// 													ind)));

	// 								prevInd = ind + 1;

	// 							}

	// 						} else
	// 						{

	// 							ind = line.indexOf(',', ind + 1);
	// 							kk = (int) (Integer
	// 									.parseInt(line.substring(prevInd, ind)));
	// 							prevInd = ind + 1;

	// 							aux3.incAnims = new movements[kk];

	// 							aux3.whichMovement = 0;
	// 							aux3.isMovingAlready = false;

	// 							ind = line.indexOf(',', ind + 1);
	// 							aux3.startMoving = (int) (Integer
	// 									.parseInt(line.substring(prevInd, ind)));
	// 							prevInd = ind + 1;

	// 							for (int j = 0; j < kk; j++)
	// 							{
	// 								aux3.incAnims[j] = new movements();

	// 								ind = line.indexOf(',', ind + 1);
	// 								aux3.incAnims[j].ix = (float) (Float.parseFloat(
	// 										line.substring(prevInd, ind)));
	// 								prevInd = ind + 1;

	// 								ind = line.indexOf(',', ind + 1);
	// 								aux3.incAnims[j].iy = (float) (Float.parseFloat(
	// 										line.substring(prevInd, ind)));
	// 								prevInd = ind + 1;

	// 								ind = line.indexOf(',', ind + 1);
	// 								if (ind == -1)
	// 									aux3.incAnims[j].duration = (int) (Integer
	// 											.parseInt(line.substring(prevInd)));
	// 								else
	// 									aux3.incAnims[j].duration = (int) (Integer
	// 											.parseInt(line.substring(prevInd,
	// 													ind)));
	// 								prevInd = ind + 1;

	// 								aux3.incAnims[j].nduration = 0;

	// 							}

	// 						}

	// 					}

	// 					ind = -1;
	// 					prevInd = 0;

	// 					provListAnims.add(aux3);

	// 				break;

	// 				case 3:

	// 					aux4 = new scale();

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux4.miny = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factY);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux4.factmin = (int) (Integer
	// 							.parseInt(line.substring(prevInd, ind)));
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux4.maxy = (int) (Float
	// 							.parseFloat(line.substring(prevInd, ind)) * factY);
	// 					prevInd = ind + 1;

	// 					ind = line.indexOf(',', ind + 1);
	// 					aux4.factmax = (int) (Integer
	// 							.parseInt(line.substring(prevInd)));
	// 					prevInd = ind + 1;

	// 					prevInd += 0;
	// 				break;
	// 				}

	// 			}

	// 		}

	// 		// myAnims

	// 		int realPaths = provList.size();

	// 		paths.caminos.cams = new rectCam[realPaths];

	// 		int t;
	// 		for (t = 0; t < realPaths; t++)
	// 		{
	// 			paths.caminos.cams[t] = new rectCam();
	// 			paths.caminos.cams[t] = provList.get(t);
	// 		}

	// 		int realConex = provListCon.size();

	// 		myConex = new conex[realConex];
	// 		for (t = 0; t < realConex; t++)
	// 		{
	// 			myConex[t] = new conex();
	// 			myConex[t] = provListCon.get(t);
	// 		}

	// 		int realAnims = provListAnims.size();

	// 		bitmapAnims = new Bitmap[realAnims];

	// 		try
	// 		{
	// 			myAnims = new anims[realAnims];
	// 			for (t = 0; t < realAnims; t++)
	// 			{

	// 				myAnims[t] = new anims();
	// 				myAnims[t] = provListAnims.get(t);

	// 				bitmapAnims[t] = getBitmapFromAsset(
	// 						"animaciones/anim" + String.valueOf(whichScreen)
	// 								+ String.valueOf(t) + ".png");

	// 				myAnims[t].realWFrame = bitmapAnims[t].getWidth();
	// 				myAnims[t].realHFrame = bitmapAnims[t].getHeight();

	// 				myAnims[t].w = (int) ((float) bitmapAnims[t].getWidth()
	// 						* factX);
	// 				myAnims[t].h = (int) ((float) bitmapAnims[t].getHeight()
	// 						* factY);

	// 				myAnims[t].predOrder = t;
	// 			}
	// 		} catch (Exception e)
	// 		{
	// 		}

	// 		for (int i = 0; i < realPaths; i++)
	// 		{
	// 			paths.caminos.cams[i].vecinos = new LinkedList<Integer>();
	// 			paths.caminos.cams[i].index = i;
	// 			paths.caminos.cams[i].marked = false;

	// 			for (int j = 0; j < realPaths; j++)
	// 			{
	// 				if ((i != j) && (paths.esVecino(i, j)))
	// 					paths.caminos.cams[i].vecinos.add(j);
	// 			}
	// 		}

	// 		if (aux4 != null)
	// 		{
	// 			myScale = aux4;
	// 		}

	// 		checkPegatinas(whichScreen);
	// 		checkObjects(whichScreen);

	// 		myConversaciones.loadConversations(whichScreen);

	// 		// a ver si alguna conversacion esta cambiada
	// 		if (myConversaciones.mytalkingAnims[whichScreen].myPersonajes != null)
	// 		{

	// 			int x = myConversaciones.mytalkingAnims[whichScreen].myPersonajes.length;

	// 			for (int i = 0; i < x; i++)
	// 				myConversaciones.mytalkingAnims[whichScreen].myPersonajes[i].currentConversationEach = 0;

	// 		}

	// 		int n = myListChangesConvs.size();
	// 		for (int i = 0; i < n; i++)
	// 		{
	// 			changesConvs aux = new changesConvs();
	// 			aux = myListChangesConvs.get(i);

	// 			if (whichScreen == aux.screen)
	// 			{
	// 				myConversaciones.mytalkingAnims[whichScreen].myPersonajes[aux.which].currentConversationEach = aux.n;// .setConversation(whichScreen,
	// 																														// aux.which,
	// 																														// aux.n);
	// 			}
	// 		}

	// 		myoverlay.isFadedOut = false;

	// 		// a ver si se ha desactivado alguna animacion
	// 		n = myListDesactAnims.size();

	// 		for (int i = 0; i < n; i++)
	// 		{
	// 			dactAnims aux = new dactAnims();
	// 			aux = myListDesactAnims.get(i);

	// 			if (whichScreen == aux.screen)
	// 				myAnims[aux.which].activated = false;
	// 		}

	// 		// personajes pe[];
	// 		personajes pe[] = AlfredActivity.myConversaciones.mytalkingAnims[whichScreen].myPersonajes;
	// 		// lista de cosas sueltas a hacer...

	// 		pintaPeriodicoDandoVueltas = false;
	// 		int j;
	// 		switch (whichScreen)
	// 		{

	// 		case 51:
	// 			desactivaAnims(51, 0);
	// 			desactivaAnims(51, myAnims.length - 1);
	// 			resetDiosesTime();
	// 		break;

	// 		case 52:
	// 			desactivaAnims(52, 0);
	// 			desactivaAnims(52, 1);
	// 			desactivaAnims(52, 2);
	// 			desactivaAnims(52, 3);
	// 			desactivaAnims(52, 4);
	// 			desactivaAnims(52, 5);
	// 			ponPegatina(52, 145);
	// 			desactivaAnims(52, myAnims.length - 1);
	// 			resetDiosesTime();
	// 		break;

	// 		case 53:
	// 			desactivaAnims(53, 0);
	// 			desactivaAnims(53, myAnims.length - 1);
	// 			resetDiosesTime();
	// 		break;

	// 		case 54:
	// 			desactivaAnims(54, 0);
	// 			desactivaAnims(54, myAnims.length - 1);
	// 			resetDiosesTime();
	// 		break;

	// 		case 0:
	// 			if ((readExtraVariable(VUELTA_A_EMPEZAR) == 1)
	// 					&& (readExtraVariable(FROM_INTRO) == 0))
	// 			{
	// 				setExtraVariables(AlfredActivity.VUELTA_A_EMPEZAR, 2);
	// 				myoverlay.setAnimExtra(14, 15, 21, 71, 66, 0, 0);
	// 				myMovingAlfredThread.showTextNow = true;
	// 				whichObject = 0;
	// 				textToShow = extraThingsToTranslate[117];

	// 			}

	// 		break;

	// 		case 48:

	// 			AlfredActivity.myMovingAlfredThread.saySomethingOther(
	// 					AlfredActivity.extraThingsToTranslate[115], 0);
	// 			setExtraVariables(AlfredActivity.A_POR_LA_PRINCESA, 1);

	// 			desactivaAnims(48, 7);

	// 		break;

	// 		case 44:

	// 			if (readExtraVariable(AlfredActivity.PIEDRA_FAKE_MOJADA) == 3)
	// 				updateConexion(55, 1);

	// 		break;

	// 		case 41:
	// 			j = getOrderAnims(2);
	// 			myAnims[j].activated = false;

	// 			j = getOrderAnims(3);
	// 			myAnims[j].activated = false;

	// 			if (readExtraVariable(AlfredActivity.GUARDIAS_BORRACHOS) == 1)
	// 				updateConexion(43, 1);

	// 		break;

	// 		case 40:

	// 			finalObject = 0;
	// 			whichObject = 0;
	// 			actionToDo = 1;
	// 			myMovingAlfredThread.stateOfTalking = 0;
	// 			gotoPos(290, 370, 3);
	// 			whichAction = objetos.HABLAR;
	// 			afterMovingAlfred = ALFRED_TALKING;
	// 			posibilityOfFakeAnswer = false;
	// 			varCheckRealTime = -1;

	// 		break;

	// 		case 39:

	// 			j = getOrderAnims(0);
	// 			myAnims[j].activated = false;

	// 			j = getOrderAnims(7);
	// 			myAnims[j].activated = false;

	// 			j = getOrderAnims(8);
	// 			myAnims[j].activated = false;

	// 			j = getOrderAnims(9);
	// 			myAnims[j].activated = false;

	// 			j = getOrderAnims(10);
	// 			myAnims[j].activated = false;

	// 		break;

	// 		case 34:
	// 			if (readExtraVariable(AlfredActivity.VIGILANTE_PAJEANDOSE) == 1)
	// 			{
	// 				int i = getOrderAnims(0);
	// 				myAnims[i].activated = false;
	// 				updateConexion(35, 1);

	// 			}
	// 		break;

	// 		case 21:

	// 			if ((prevWhichScreen == 36)
	// 					&& (readExtraVariable(AlfredActivity.PIRAMIDE_JODIDA) == 1))
	// 			{
	// 				updateConexion(36, 0);
	// 				AlfredActivity.ponPegatina(21, 79);
	// 				AlfredActivity.addPegatina(21, 79);
	// 			}

	// 		break;

	// 		case 36:

	// 			byte z = readExtraVariable(AlfredActivity.PIRAMIDE_JODIDA);
	// 			if (z >= 3)
	// 				setExtraVariables(AlfredActivity.PIRAMIDE_JODIDA, 1);

	// 			int i;

	// 			if (readExtraVariable(AlfredActivity.PIRAMIDE_JODIDA) == 1) // se ha
	// 																		// cargado
	// 																		// la
	// 																		// piramide
	// 			{

	// 				i = getOrderAnims(0);
	// 				myAnims[i].x = (int) (328.0f * factX);
	// 				myAnims[i].y = (int) (209.0f * factY);
	// 				myAnims[i].moving = false;
	// 				myAnims[i].incAnims = null;// new movements[1];

	// 				myAnims[i].speed = 1000000;

	// 				i = getOrderAnims(2);
	// 				myAnims[i].activated = true;
	// 				myAnims[i].speed = 16;
	// 			} else// if (prevWhichScreen==21)//no ha hecho nada
	// 			{
	// 				i = getOrderAnims(0);

	// 				if (prevWhichScreen == 21)
	// 				{
	// 					myAnims[i].activated = true;
	// 					myAnims[i].speed = 1000000;
	// 					updateConexion(37, 0);
	// 				} else
	// 				{
	// 					myAnims[i].activated = false;
	// 					updateConexion(37, 1);
	// 				}

	// 				i = getOrderAnims(2);
	// 				myAnims[i].activated = false;

	// 			}

	// 		break;

	// 		case 26:

	// 			mypersonajes = pe[0];

	// 			if (myListUsingObjects.contains(82) == false)// no tiene dinero
	// 				mypersonajes.setConversation(26, 0, 0);
	// 			else
	// 				mypersonajes.setConversation(26, 0, 1);

	// 			mypersonajes = pe[1];
	// 			mypersonajes.setConversation(26, 1, 0);

	// 			if ((AlfredActivity
	// 					.readExtraVariable(AlfredActivity.A_LA_CARCEL) == 1)
	// 					&& (prevWhichScreen == 27)
	// 					&& (AlfredActivity.readExtraVariable(
	// 							AlfredActivity.SE_HA_PUESTO_EL_MUNECO) == 0))
	// 			{
	// 				mypersonajes = pe[1];
	// 				mypersonajes.setConversation(26, 1, 1);
	// 				// myAnims[0].activated=true;
	// 				finalObject = 1;
	// 				whichObject = 1;
	// 				actionToDo = 1;
	// 				myMovingAlfredThread.stateOfTalking = 0;
	// 				gotoPos(314, 240, 3);
	// 				whichAction = objetos.HABLAR;// .h
	// 				afterMovingAlfred = ALFRED_TALKING;
	// 				posibilityOfFakeAnswer = false;
	// 				varCheckRealTime = -1;
	// 			}
	// 			if ((AlfredActivity
	// 					.readExtraVariable(AlfredActivity.A_LA_CARCEL) == 1)
	// 					&& (prevWhichScreen == 27)
	// 					&& (AlfredActivity.readExtraVariable(
	// 							AlfredActivity.SE_HA_PUESTO_EL_MUNECO) == 1))
	// 			{
	// 				mypersonajes = pe[1];
	// 				mypersonajes.setConversation(26, 1, 2);
	// 			}

	// 		break;

	// 		case 27:
	// 			if (prevWhichScreen == 33)// viene del tunel
	// 			{
	// 				xAlfred = 90.0f * factX;
	// 				yAlfred = 380.0f * factY;
	// 				myoverlay.setAnimExtra(16, 9, 12, 158, 115, 0, 0);
	// 			}

	// 			// mypersonajes = pe[0];

	// 			if (myListUsingObjects.contains(82) == false)
	// 			{
	// 				mypersonajes = pe[0];
	// 				mypersonajes.setConversation(27, 0, 0);
	// 				mypersonajes = pe[1];
	// 				mypersonajes.setConversation(27, 1, 0);
	// 			} else
	// 			{
	// 				mypersonajes = pe[0];
	// 				mypersonajes.setConversation(27, 0, 1);
	// 				mypersonajes = pe[1];
	// 				mypersonajes.setConversation(27, 1, 1);
	// 			}

	// 		break;

	// 		case 32:
	// 			if (prevWhichScreen == 31)// viene de la carcel
	// 			{
	// 				xAlfred = 321.0f * factX;
	// 				yAlfred = 125.0f * factY;
	// 				myoverlay.setAnimExtra(11, 7, 9, 33, 72, 0, 0);
	// 				AlfredActivity.myConversaciones.mytalkingAnims[31].myPersonajes[0]
	// 						.setConversation(31, 0, 3);
	// 				// mypersonajes = pe[0];
	// 				// mypersonajes.setConversation(31, 0, 3);
	// 			}

	// 		break;

	// 		case 38:
	// 			if (AlfredActivity
	// 					.readExtraVariable(AlfredActivity.ROBA_PELO_PRINCESA) == 0)
	// 			{
	// 				xAlfred = 230.0f * factX;
	// 				yAlfred = 283.0f * factY;
	// 				myoverlay.setAnimExtra(12, 6, 8, 113, 103, -32, 15);
	// 			}
	// 		break;
	// 		case 30:
	// 			myAnims[0].activated = false;
	// 			myAnims[4].activated = false;

	// 			if ((readExtraVariable(PUERTA_SECRETA_ABIERTA) == 1)
	// 					&& (AlfredActivity.readExtraVariable(
	// 							AlfredActivity.ROBA_PELO_PRINCESA) != 2))
	// 				updateConexion(38, 1);

	// 			if (AlfredActivity
	// 					.readExtraVariable(AlfredActivity.ROBA_PELO_PRINCESA) == 1)
	// 			{

	// 				setExtraVariables(AlfredActivity.ROBA_PELO_PRINCESA, 2);
	// 				myAnims[0].activated = true;
	// 				finalObject = 0;
	// 				actionToDo = 0;
	// 				myMovingAlfredThread.stateOfTalking = 0;
	// 				gotoPos(391, 344, 1);
	// 				whichAction = objetos.HABLAR;// .h
	// 				afterMovingAlfred = ALFRED_TALKING;
	// 				posibilityOfFakeAnswer = false;
	// 				varCheckRealTime = -1;
	// 			}
	// 		break;

	// 		case 4:
	// 			if (readExtraVariable(ELECTROCUTACION) == 0)
	// 			{
	// 				setExtraVariables(AlfredActivity.ELECTROCUTACION, 1);
	// 				hideObject(7);
	// 			}
	// 			if (readExtraVariable(AlfredActivity.CABLES_PUESTOS) == 3)
	// 				showObject(8);
	// 			else
	// 				hideObject(8);
	// 		case 8:
	// 		case 14:
	// 			if (AlfredActivity.readExtraVariable(
	// 					AlfredActivity.VENDEDOR_DEJA_DE_JODER) == 1)
	// 			{
	// 				paths.caminos.cams[0].w = (int) (599.0f * AlfredActivity.factX);
	// 				AlfredActivity.updateConexion(16, 1);
	// 			}
	// 		case 16:
	// 		case 19:

	// 			if ((readExtraVariable(PUESTA_SALSA_PICANTE_EN_MENU) == 1)
	// 					&& (readExtraVariable(JEFE_ENCARCELADO) == 0)
	// 					&& (Math.random() < 0.25f))
	// 			{
	// 				// muestra periodico, quita al jefe, etc...
	// 				setExtraVariables(AlfredActivity.JEFE_ENCARCELADO, 1);
	// 				AlfredActivity.addObject(13, 0, false);// anula de que se pueda
	// 														// clickar al jefe
	// 				AlfredActivity.desactivaAnims(13, 0);
	// 				AlfredActivity.desactivaAnims(13, 1);

	// 				myConversaciones.mytalkingAnims[12].myPersonajes[0]
	// 						.setConversation(12, 0, 1);// .currentConversationEach[aux.which]=aux.n;//.setConversation(whichScreen,
	// 													// aux.which, aux.n);

	// 				String x = "extrascreens/"
	// 						+ extraScreens[(1 * myLanguages.length)
	// 								+ whichLanguageInt]
	// 						+ ".png";

	// 				try
	// 				{
	// 					periodico = getBitmapFromAsset(x);
	// 				} catch (IOException e)
	// 				{
	// 				}

	// 				Config cx = periodico.getConfig();
	// 				periodico = periodico.copy(cx, true);

	// 				pintaPeriodicoDandoVueltas = true;

	// 			}

	// 		break;
	// 		case 9:
	// 			// personajes pe[] =
	// 			// AlfredActivity.myConversaciones.mytalkingAnims[whichScreen].myPersonajes;
	// 			mypersonajes = pe[0];

	// 			if (myListUsingObjects.contains(10) == false)
	// 				mypersonajes.setConversation(9, 0, 0);
	// 			else
	// 				mypersonajes.setConversation(9, 0, 3);
	// 		break;
	// 		case 20:
	// 			// personajes pe[] =
	// 			// AlfredActivity.myConversaciones.mytalkingAnims[whichScreen].myPersonajes;
	// 			mypersonajes = pe[0];
	// 			if (myListUsingObjects.contains(75) == true)
	// 				mypersonajes.setConversation(20, 0, 2);
	// 			else
	// 			{
	// 				if (myListUsingObjects.contains(59) == true)// &&
	// 															// (myListUsingObjects.contains(59)==true))
	// 					mypersonajes.setConversation(20, 0, 1);
	// 				else
	// 					mypersonajes.setConversation(20, 0, 0);
	// 			}
	// 		break;
	// 		case 28:
	// 			if (AlfredActivity.readExtraVariable(
	// 					AlfredActivity.CROCODILLO_ENCENDIDO) == 1)
	// 			{
	// 				hideObject(0);
	// 				showObject(1);
	// 				showObject(2);
	// 				showObject(3);
	// 				showObject(4);
	// 				myoverlay.isFadedOut = false;
	// 			} else
	// 			{
	// 				hideObject(1);
	// 				hideObject(2);
	// 				hideObject(3);
	// 				hideObject(4);
	// 				myoverlay.isFadedOut = true;
	// 			}
	// 		break;

	// 		}

	// 		if (whichScreen == 55)
	// 		{
	// 			if (prevWhichScreen == 44)
	// 				myMovingAlfredThread.alfredFrame = 0;
	// 			else
	// 				myMovingAlfredThread.alfredFrame = 17;
	// 		} else
	// 		{
	// 			switch (dirAlfred)
	// 			{
	// 			case 0:
	// 				myMovingAlfredThread.alfredFrame = 0;
	// 			break;
	// 			case 1:
	// 				myMovingAlfredThread.alfredFrame = 9;
	// 			break;
	// 			case 2:
	// 				myMovingAlfredThread.alfredFrame = 18;
	// 			break;
	// 			case 3:
	// 				myMovingAlfredThread.alfredFrame = 23;
	// 			break;
	// 			}

	// 		}

	// 		BufferedReader brfx;
	// 		String word;
	// 		List<String> wordListfx = new ArrayList<String>();
	// 		wordListfx.clear();
	// 		String name = "fx/screen" + String.valueOf(whichScreen) + ".txt";

	// 		try
	// 		{
	// 			brfx = new BufferedReader(
	// 					new InputStreamReader(assetManager.open(name)));
	// 			while ((word = brfx.readLine()) != null)
	// 				wordListfx.add(word);
	// 		} catch (IOException e)
	// 		{
	// 		}

	// 		int size = wordListfx.size();
	// 		filelistFX = new String[size];

	// 		for (int i = 0; i < size; i++)
	// 		{
	// 			filelistFX[i] = new String();
	// 			filelistFX[i] = wordListfx.get(i) + ".mp3";
	// 		}

	// 		if (filelistFX.length > 0)
	// 		{
	// 			int off = (int) (10000.0f + (Math.random() * 5000.0f));
	// 			timeFXtoPlay = SystemClock.uptimeMillis() + off;
	// 		}

	// 		boolean loop;
	// 		if (whichScreen != 48)
	// 			loop = true;
	// 		else
	// 			loop = false;

	// 		playTrack(whichScreen, loop);

	// 		fundidoTime = false;

	// 		timeStartScreen = SystemClock.uptimeMillis();

	// 		screenReady = true;
}

Common::Error PelrockEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Pelrock
