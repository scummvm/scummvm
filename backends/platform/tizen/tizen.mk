# port files built under eclipse

MODULE := backends/platform/tizen

$(EXECUTABLE): $(OBJS)
	rm -f $@
	arm-linux-gnueabi-ar Tru $@ $(OBJS)
