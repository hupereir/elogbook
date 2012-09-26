%define ver      @VERSION@
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
./autogen.sh --prefix=%{prefix}
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
make prefix=$RPM_BUILD_ROOT%{prefix} install-strip

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING INSTALL
%{prefix}/bin/copy_logbook
%{prefix}/bin/synchronize_logbook
%{prefix}/bin/elogbook

%changelog
