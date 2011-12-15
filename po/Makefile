top_srcdir =
PO_DIR=~/git/translations/po
LANGS = $(shell ls $(PO_DIR)/setup)

pos:
	[ -d $(PO_DIR) ]
	for i in $(LANGS); \
	do \
		cp $(PO_DIR)/setup/$$i/setup.po $$i.po; \
		charset=$$(echo $(GLIBC_LANGS)|sed 's/ /\n/g'|grep ^$$i|sed 's/.*,//'); \
		if [ -z "$$charset" ]; then \
			continue; \
		fi; \
		iconv -f UTF-8 -t $$charset < $$i.po > $$i.new.po; \
		sed "s/UTF-8/$$charset/" $$i.new.po > $$i.po; \
		rm $$i.new.po; \
		if ! msgfmt -c --statistics -o /dev/null $$i.po; then \
			echo "$$i.po would break your build!"; \
			exit 1; \
		fi; \
	done

mos:
	for i in $(wildcard *.po); do \
		echo -n "$$i: "; \
		msgfmt -o ../merge/usr/share/locale/`basename $$i .po`/LC_MESSAGES/setup.mo $$i; \
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

distclean:
	rm -f intltool-extract *.pot *.po *.gmo
