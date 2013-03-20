%define version 1.14.9
%define release 1

Summary: Terminal text editor LE.
Name: le
Version: %{version}
Release: %{release}
License: GNU GPL
Group: Applications/Editors
Source: ftp://ftp.yar.ru/pub/source/le/le-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-buildroot

%description
LE has many block operations with stream and rectangular blocks, can edit
both unix and dos style files (LF/CRLF), is binary clean, has hex mode,
tunable syntax highlighting, tunable color scheme, tunable key map and some
more useful features. It is slightly similar to Norton Editor from DOS.

%prep
%setup

%build
%define __libtoolize :
%configure
make

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(0644 root root 0755)
%doc FEATURES HISTORY NEWS README doc/README.keymap.ru
%attr(0755 root root) %{_bindir}/le
%attr(0755 root root) %{_datadir}/le/help
%attr(0644 root man) %{_mandir}/man*/*
%{_datadir}/le
