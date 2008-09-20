#SUBDIRS:= `ls | egrep -v '^(CVS|tools)$$'`




export PATH	:=	$(DEVKITARM)/bin:$(PATH)

export portdir = $(CURDIR)/arm9
export srcdir = $(CURDIR)/../../..


SUBDIRS := arm7 arm9

all:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i; fi; done;
clean:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i clean; fi; done;
semiclean:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i semiclean; fi; done;

export:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i export; fi; done;

allbuilds:
	make semiclean
	make all       SCUMM_BUILD=a
	make semiclean
	make all       SCUMM_BUILD=b
	make semiclean
	make all       SCUMM_BUILD=c
	make semiclean
	make all       SCUMM_BUILD=d
	make semiclean
	make all       SCUMM_BUILD=e
	make semiclean
	make all       SCUMM_BUILD=f
	make semiclean
	make all       SCUMM_BUILD=g
	make semiclean
	make all       SCUMM_BUILD=h

allbuildssafe:
	make clean     SCUMM_BUILD=a
	make all       SCUMM_BUILD=a
	make clean     SCUMM_BUILD=b
	make all       SCUMM_BUILD=b
	make clean     SCUMM_BUILD=c
	make all       SCUMM_BUILD=c
	make clean     SCUMM_BUILD=d
	make all       SCUMM_BUILD=d
	make clean     SCUMM_BUILD=e
	make all       SCUMM_BUILD=e
	make clean     SCUMM_BUILD=f
	make all       SCUMM_BUILD=f
	make clean     SCUMM_BUILD=g
	make all       SCUMM_BUILD=g
	make clean     SCUMM_BUILD=h
	make all       SCUMM_BUILD=h
