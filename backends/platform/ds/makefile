#SUBDIRS:= `ls | egrep -v '^(CVS|tools)$$'`


export PATH	:=	$(DEVKITARM)/bin:$(PATH)

export portdir = $(CURDIR)/arm9
export srcdir = $(CURDIR)/../../..


SUBDIRS := arm7 arm9

all:
	@for i in $(SUBDIRS); do if test -d $$i; then $(MAKE) -C $$i; fi; done;
clean:
	@for i in $(SUBDIRS); do if test -d $$i; then $(MAKE) -C $$i clean; fi; done;
semiclean:
	@for i in $(SUBDIRS); do if test -d $$i; then $(MAKE) -C $$i semiclean; fi; done;

export:
	@for i in $(SUBDIRS); do if test -d $$i; then $(MAKE) -C $$i export; fi; done;

allbuilds:
	$(MAKE) semiclean
	$(MAKE) all       SCUMM_BUILD=a
	$(MAKE) all       SCUMM_BUILD=b
	$(MAKE) all       SCUMM_BUILD=c
	$(MAKE) all       SCUMM_BUILD=d
	$(MAKE) all       SCUMM_BUILD=e
	$(MAKE) all       SCUMM_BUILD=f
	$(MAKE) all       SCUMM_BUILD=g
	$(MAKE) all       SCUMM_BUILD=h
	$(MAKE) all       SCUMM_BUILD=i
	$(MAKE) all       SCUMM_BUILD=k

allbuildssafe:
	$(MAKE) clean     SCUMM_BUILD=a
	$(MAKE) all       SCUMM_BUILD=a
	$(MAKE) all       SCUMM_BUILD=b
	$(MAKE) all       SCUMM_BUILD=c
	$(MAKE) all       SCUMM_BUILD=d
	$(MAKE) all       SCUMM_BUILD=e
	$(MAKE) all       SCUMM_BUILD=f
	$(MAKE) all       SCUMM_BUILD=g
	$(MAKE) all       SCUMM_BUILD=h
	$(MAKE) all       SCUMM_BUILD=i
	$(MAKE) all       SCUMM_BUILD=k
