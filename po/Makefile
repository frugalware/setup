LANGS=hu

mos:
	for i in $(LANGS); do \
		msgfmt -o /usr/share/locale/`basename $$i .po`/LC_MESSAGES/setup.mo $$i; \
	done
