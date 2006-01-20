LANGS=hu

all: pot update-po

mos:
	for i in $(LANGS); do \
		msgfmt -o /usr/share/locale/`basename $$i .po`/LC_MESSAGES/netconfig.mo $$i; \
	done

pot:
	intltool-update --pot --gettext-package=netconfig

update-po: pot
	for lang in $(LANGS); do \
		if msgmerge $$lang.po netconfig.pot -o $$lang.new.po; then \
			mv -f $$lang.new.po $$lang.po; \
		else \
			echo "msgmerge for $$cat failed!"; \
			rm -f $$lang.new.po; \
		fi; \
	done
