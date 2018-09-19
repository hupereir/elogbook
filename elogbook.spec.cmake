Name:       @CPACK_PACKAGE_NAME@
Version:    @CPACK_PACKAGE_VERSION@
Release:    0%{dist}
Vendor: @CPACK_PACKAGE_VENDOR@

License: GPLV2

Summary: electronic logbook
Group: User Interface/X

Source: %{name}-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-buildroot

%description
Qt-based electronic logbook

%prep
%setup -q -n %{name}-%{version} %{release}

%build
%define prefix /usr
cmake -DCMAKE_INSTALL_PREFIX=%{prefix} -DUSE_QT5=1 .
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc COPYING INSTALL
%{prefix}/bin/compress-logbook
%{prefix}/bin/copy-logbook
%{prefix}/bin/elogbook
%{prefix}/bin/synchronize-logbook
%{prefix}/bin/uncompress-logbook

%changelog
