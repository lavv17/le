Summary: This is terminal text editor: LE.
Name: le
Version: 1.5.6
Release: 1
Copyright: GPL
Group: Applications/Editors
Source: ftp://ftp.yars.free.net/pub/software/unix/util/texteditors/le-%{PACKAGE_VERSION}.tar.gz
#Vendor: Alexander V. Lukyanov <lav@yars.free.net>
#Packager: Peter Soos <sp@osb.hu>
BuildRoot: /var/tmp/le-root

%changelog

* Mon Nov 08 1999 Peter Soos <sp@osb.hu>
- Moved to version 1.5.5

* Thu Jul 01 1999 Peter Soos <sp@osb.hu>
- Moved to version 1.5.2

* Thu May 13 1999 Peter Soos <sp@osb.hu>
- Moved to version 1.5.1
- Corrected the file and directory attributes to rebuild the package
  under RedHat Linux 6.0

* Tue Feb 23 1999 Peter Soos <sp@osb.hu>
- Moved to version 1.5.0

* Fri Dec 25 1998 Peter Soos <sp@osb.hu>
- Corrected the file and directory attributes
- Recompiled under RedHat Linux 5.2

* Mon Jun 22 1998 Peter Soos <sp@osb.hu>
- Using %attr

* Thu Mar 18 1998 Peter Soos <sp@osb.hu>
- moved to 1.4.2

* Fri Dec 12 1997 Peter Soos <sp@osb.hu>
- moved to 1.4.1 from 1.4.0

* Sun Dec 7 1997 Peter Soos <sp@osb.hu>
- Recompiled under RedHat Linux 5.0
- Added BuildRoot

%description
LE has many block operations with stream and rectangular blocks, can edit
both unix and dos style files (LF/CRLF), is binary clean, has hex mode,
tunable syntax highlighting, tunable color scheme, tunable key map.

%prep
%setup

%build
./configure --prefix=/usr --with-regex
make "CFLAGS=$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
make "prefix=$RPM_BUILD_ROOT/usr" install
strip $RPM_BUILD_ROOT/usr/bin/le

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0644, root, root, 0755)
%doc FEATURES HISTORY NEWS README 
%attr(0755, root, root) /usr/bin/le
/usr/man/man1/le.1
/usr/share/le
