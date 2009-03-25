%rename lib	old_lib
*lib:
-lg -lm -lstdc++ -lsupc++ -lg -lpsprtc -lpspaudio -lpspuser -lpsputility -lpspdebug -lc -lpspgum -lpspvfpu -lpspgu -lpspctrl -lpspge -lpspdisplay_driver -lpspdisplay -lpsphprm -lpspge_driver -lpspsdk -lpsppower -lpsppower_driver -lpspgum_vfpu -lpspkernel %(old_lib)
