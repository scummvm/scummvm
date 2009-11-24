%rename lib	old_lib
*lib:
%(old_lib) -lSDLmain -lSDL -lGL -lpng -lz -lstdc++ -lc -lm -lpspvfpu -lpspdebug -lpspgu -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpsputility -lpspuser -lpsppower -lpsphprm -lpspsdk -lpsprtc -lpspaudio
