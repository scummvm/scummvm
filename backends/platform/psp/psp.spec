%rename lib	old_lib
*lib:
-lstdc++ -lpspprof -lpspvfpu -lpspdebug -lpspgu -lpspge -lpspdisplay -lpspctrl -lpsppower -lpsphprm -lpspaudio -lpspaudiocodec %(old_lib)
