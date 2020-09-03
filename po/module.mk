POTFILE := $(srcdir)/po/residualvm/residualvm.pot
POFILES := $(wildcard $(srcdir)/po/*.po)

ENGINE_INPUT_POTFILES := $(sort $(wildcard $(srcdir)/engines/*/POTFILES))
updatepot:
	cat $(srcdir)/po/POTFILES $(ENGINE_INPUT_POTFILES) | \
	xgettext -f - -D $(srcdir) -d residualvm --c++ -k_ -k_s -k_c:1,2c -k_sc:1,2c --add-comments=I18N\
		-kDECLARE_TRANSLATION_ADDITIONAL_CONTEXT:1,2c -o $(POTFILE) \
		--copyright-holder="ResidualVM Team" --package-name=ResidualVM \
		--package-version=$(VERSION) --msgid-bugs-address=https://github.com/residualvm/residualvm/issues -o $(POTFILE)_

	sed -e 's/SOME DESCRIPTIVE TITLE/LANGUAGE translation for ResidualVM/' \
		-e 's/UTF-8/CHARSET/' -e 's/PACKAGE/ResidualVM/' $(POTFILE)_ > $(POTFILE).new

	rm $(POTFILE)_
	$(srcdir)/po/msgcut $(POTFILE).new $(srcdir)/po/scummvm.pot 	# ResidualVM specific


	if test -f $(POTFILE); then \
		sed -f $(srcdir)/po/remove-potcdate.sed < $(POTFILE) > $(POTFILE).1 && \
		sed -f $(srcdir)/po/remove-potcdate.sed < $(POTFILE).new > $(POTFILE).2 && \
		if cmp $(POTFILE).1 $(POTFILE).2 >/dev/null 2>&1; then \
			rm -f $(POTFILE).new; \
		else \
			rm -f $(POTFILE) && \
			mv -f $(POTFILE).new $(POTFILE); \
		fi; \
		rm -f $(POTFILE).1 $(POTFILE).2; \
	else \
		mv -f $(POTFILE).new $(POTFILE); \
	fi;

%.po: $(POTFILE)
	# ResidualVM specific start ->
	$(eval RESIDUALVM_PO = $(dir $@)/residualvm/$(notdir $@))
	if [ -f "$(RESIDUALVM_PO)" ]; then \
		msgmerge $(RESIDUALVM_PO) $(POTFILE) | msgcat --use-first - $@ > $@.new; \
	else \
		cp $@ $@.new; \
	fi
	# ResidualVM specific end <-
	if cmp $@ $@.new >/dev/null 2>&1; then \
		rm -f $@.new; \
	else \
		mv -f $@.new $@; \
	fi;

translations-dat: devtools/create_translations
	devtools/create_translations/create_translations $(POFILES)
	mv translations.dat $(srcdir)/gui/themes/

update-translations: updatepot $(POFILES) translations-dat

update-translations: updatepot $(POFILES)
	@$(foreach file, $(POFILES), echo -n $(notdir $(basename $(file)))": ";msgfmt --statistic $(file);)
	@rm -f messages.mo

.PHONY: updatepot translations-dat update-translations
