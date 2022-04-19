%rename lib	old_lib
*lib:
-lz -lstdc++ --start-group -lc %:if-exists(libpthreadglue.a%s) %:if-exists(libpthread.a%s) %:if-exists(libcglue.a%s) --end-group -lm -lpspprof -lpspvfpu -lpspdebug -lpspgu -lpspge -lpspdisplay -lpspctrl -lpspsdk -lpsputility -lpspuser -lpsppower -lpsphprm  -lpsprtc -lpspaudio -lpspaudiocodec -lpspkernel -lpspnet_inet %(old_lib)
