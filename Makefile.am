SUBDIRS = doc lib src misc m4
EXTRA_DIST = FEATURES HISTORY le.spec
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

REL_DIR=/home/lav/www/download/le
release:
	make distcheck
	mv $(PACKAGE)-$(VERSION).tar.gz $(REL_DIR)
	cd $(REL_DIR) && \
	    (addbz $(PACKAGE)-$(VERSION).tar.gz; \
	    gpg -ba $(PACKAGE)-$(VERSION).tar.gz; \
	    gpg -ba $(PACKAGE)-$(VERSION).tar.bz2; \
	    gpg -ba $(PACKAGE)-$(VERSION).tar.xz; \
	    md5sum $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION).tar.bz2 $(PACKAGE)-$(VERSION).tar.xz > $(PACKAGE)-$(VERSION).md5sum)
