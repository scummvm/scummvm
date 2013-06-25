# port files built under eclipse

MODULE := backends/platform/bada

$(EXECUTABLE): $(OBJS)
	rm -f $@
	arm-linux-gnueabi-ar Tru $@ $(OBJS)
