%define ver      @ELOGBOOK_VERSION@
%define rel      0
%define prefix   /usr

Summary: electronic logbook
Name: elogbook
Version: %{ver}
Release: %{rel}
License: GPL
Group: User Interface/X
Source: %{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

Requires: qt4-common, libqtcore4, libqtgui4, aspell >= 0.60.4, libaspell15 >= 0.60.4

%description
Qt-based electronic logbook

%prep
%setup -q -n %{name}-%{ver} %{rel}

%build
cmake -DCMAKE_INSTALL_PREFIX=%{prefix} .
make -j4

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING INSTALL
%{prefix}/bin/copy-logbook
%{prefix}/bin/synchronize-logbook
%{prefix}/bin/elogbook

%changelog
