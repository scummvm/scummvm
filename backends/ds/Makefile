#SUBDIRS:= `ls | egrep -v '^(CVS|tools)$$'`




export PATH	:=	$(DEVKITARM)/bin:$(PATH)

export portdir = $(CURDIR)/arm9
export srcdir = $(CURDIR)/../..


SUBDIRS := arm7 arm9

all:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i; fi; done;
clean:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i clean; fi; done;

export:
	@for i in $(SUBDIRS); do if test -d $$i; then make -C $$i export; fi; done;

