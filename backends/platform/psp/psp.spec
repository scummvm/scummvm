%rename lib	old_lib
*lib:
%(old_lib) -lz -lstdc++ -lc -lm -lpspvfpu -lpspdebug -lpspgu -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpsputility -lpspuser -lpsppower -lpsphprm  -lpsprtc -lpspaudio
