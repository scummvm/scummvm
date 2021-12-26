%rename lib	old_lib
*lib:
-lz -lstdc++ -lc %:if-exists(libcglue.a%s) -lm -lpspprof -lpspvfpu -lpspdebug -lpspgu -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpsputility -lpspuser -lpsppower -lpsphprm  -lpsprtc -lpspaudio -lpspaudiocodec -lpspkernel -lpspnet_inet %(old_lib)
