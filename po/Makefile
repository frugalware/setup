LANGS=hu de nl

mos:
	for i in $(LANGS); do \
		msgfmt -o ../merge/usr/share/locale/`basename $$i .po`/LC_MESSAGES/setup.mo $$i; \
	done

pot:
	intltool-update --pot --gettext-package=setup

update-po: pot
	for lang in $(LANGS); do \
		if msgmerge $$lang.po setup.pot -o $$lang.new.po; then \
			mv -f $$lang.new.po $$lang.po; \
		else \
			echo "msgmerge for $$cat failed!"; \
			rm -f $$lang.new.po; \
		fi; \
	done
