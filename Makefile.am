SUBDIRS = doc lib src misc m4
EXTRA_DIST = FEATURES HISTORY le.spec
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST += $(top_srcdir)/.version
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

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

# AppImage building rules
linuxdeployqt=linuxdeployqt-continuous-$(build_cpu).AppImage
le_desktop=$(prefix)/share/applications/le.desktop
le_appimage=$(PACKAGE)-$(VERSION)-$(host_cpu).AppImage
CLEANFILES=$(le_appimage)
DISTCLEANFILES=$(linuxdeployqt)

$(linuxdeployqt):
	test -x src/le || $(MAKE) $(AM_MAKEFLAGS) all
	wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/$(linuxdeployqt)
	chmod a+x $(linuxdeployqt)

$(le_appimage): $(linuxdeployqt)
	$(MAKE) $(AM_MAKEFLAGS) install DESTDIR="`readlink -f appdir`"
	unset QTDIR QT_PLUGIN_PATH LD_LIBRARY_PATH \
	&& ./$(linuxdeployqt) ./appdir$(le_desktop) -bundle-non-qt-libs \
	&& ./$(linuxdeployqt) ./appdir$(le_desktop) -appimage
	mv -f le-$(host_cpu).AppImage $(le_appimage)
	-rm -rf appdir

appimage: $(le_appimage)
.PHONY: appimage

clean-local:
	-rm -rf appdir
