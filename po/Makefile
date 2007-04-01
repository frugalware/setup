top_srcdir =
PO_DIR=~/darcs/translations/po
LANGS = $(shell ls $(PO_DIR)/setup)

pos:
	[ -d $(PO_DIR) ]
	for i in $(LANGS); \
	do \
		cp $(PO_DIR)/setup/$$i/setup.po $$i.po; \
	done

mos:
	for i in $(LANGS); do \
		echo -n "$$i: "; \
		msgfmt -c --statistics -o ../merge/usr/share/locale/`basename $$i .po`/LC_MESSAGES/setup.mo $$i; \
	done

pot: intltool-extract
	intltool-update --pot --gettext-package=setup

intltool-extract:
	cp /usr/bin/intltool-extract ./

update-po: pot
	for lang in $(LANGS); do \
		if msgmerge $$lang.po setup.pot -o $$lang.new.po; then \
			mv -f $$lang.new.po $$lang.po; \
		else \
			echo "msgmerge for $$cat failed!"; \
			rm -f $$lang.new.po; \
		fi; \
	done

clean:
	rm -f intltool-extract *.pot *.po
