%define VERSION 5.20
%define RELEASE 1

%define _arc  %(getconf LONG_BIT)
%define _is64 %(if [ `getconf LONG_BIT` = "64" ] ; then  printf "64";  fi)

%define _cupsbindir /usr/lib/cups
%define _cupsbindir64 /usr/lib64/cups

%define _prefix	/usr/local
%define _bindir %{_prefix}/bin
%define _libdir /usr/lib%{_is64}
%define _ppddir /usr

%define COM_LIBS libcnnet2 libcnbpcnclapicom2
%define CNLIBS /bjlib2

Summary: IJ Printer Driver Ver.%{VERSION} for Linux
Name: cnijfilter2
Version: %{VERSION}
Release: %{RELEASE}
License: See the LICENSE*.txt file.
Vendor: CANON INC.
Group: Applications/Publishing
Source0: cnijfilter2-source-%{version}-%{release}.tar.gz
BuildRequires: cups-devel
Requires:  cups


%description
IJ Printer Driver for Linux. 
This IJ Printer Driver provides printing functions for Canon Inkjet
printers operating under the CUPS (Common UNIX Printing System) environment.


%prep
%setup -q -n  cnijfilter2-source-%{version}-%{release}


%build
pushd cmdtocanonij2
    ./autogen.sh --prefix=/usr --datadir=%{_prefix}/share LDFLAGS="-L../../com/libs_bin%{_arc}"
	make
popd

pushd cnijbe2
    ./autogen.sh --prefix=/usr --enable-progpath=%{_bindir} 
	make
popd

pushd lgmon3
    ./autogen.sh --prefix=%{_prefix} --enable-libpath=%{_libdir}%{CNLIBS} --enable-progpath=%{_bindir} --datadir=%{_prefix}/share LDFLAGS="-L../../com/libs_bin%{_arc}"
	make
popd

pushd rastertocanonij
    ./autogen.sh --prefix=/usr --enable-progpath=%{_bindir}
	make
popd

pushd tocanonij
    ./autogen.sh --prefix=%{_prefix}
	make
popd

pushd tocnpwg
    ./autogen.sh --prefix=%{_prefix}
	make
popd


%install
mkdir -p ${RPM_BUILD_ROOT}%{_libdir}%{CNLIBS}
mkdir -p ${RPM_BUILD_ROOT}%{_bindir}
mkdir -p ${RPM_BUILD_ROOT}%{_cupsbindir}/filter
mkdir -p ${RPM_BUILD_ROOT}%{_cupsbindir}/backend
mkdir -p ${RPM_BUILD_ROOT}%{_cupsbindir64}/filter
mkdir -p ${RPM_BUILD_ROOT}%{_cupsbindir64}/backend
mkdir -p ${RPM_BUILD_ROOT}%{_ppddir}/share/cups/model

install -c -m 644 com/ini/cnnet.ini  		${RPM_BUILD_ROOT}%{_libdir}%{CNLIBS}
install -c -s -m 755 com/libs_bin%{_arc}/*.so.* 	${RPM_BUILD_ROOT}%{_libdir}

pushd ppd
	install -m 644 *.ppd ${RPM_BUILD_ROOT}%{_ppddir}/share/cups/model
popd

pushd cmdtocanonij2
	make install DESTDIR=${RPM_BUILD_ROOT}
popd

pushd cnijbe2
	make install DESTDIR=${RPM_BUILD_ROOT}
popd

pushd lgmon3
	make install DESTDIR=${RPM_BUILD_ROOT}
popd

pushd rastertocanonij
	make install DESTDIR=${RPM_BUILD_ROOT}
popd

pushd tocanonij
	make install DESTDIR=${RPM_BUILD_ROOT}
popd

pushd tocnpwg
	make install DESTDIR=${RPM_BUILD_ROOT}
popd

install -c -m 755 ${RPM_BUILD_ROOT}%{_cupsbindir}/filter/cmdtocanonij2	${RPM_BUILD_ROOT}%{_cupsbindir64}/filter/cmdtocanonij2
install -c -m 755 ${RPM_BUILD_ROOT}%{_cupsbindir}/filter/rastertocanonij	${RPM_BUILD_ROOT}%{_cupsbindir64}/filter/rastertocanonij
install -c -m 755 ${RPM_BUILD_ROOT}%{_cupsbindir}/backend/cnijbe2	${RPM_BUILD_ROOT}%{_cupsbindir64}/backend/cnijbe2

%clean
rm -rf $RPM_BUILD_ROOT


%post
if [ -x /sbin/ldconfig ]; then
	/sbin/ldconfig
fi

%postun
for LIBS in %{COM_LIBS}
do
	if [ -h %{_libdir}/${LIBS}.so ]; then
		rm -f %{_libdir}/${LIBS}.so
	fi	
done
if [ "$1" = 0 ] ; then
	rmdir -p --ignore-fail-on-non-empty %{_libdir}%{CNLIBS}
fi
if [ -x /sbin/ldconfig ]; then
	/sbin/ldconfig
fi


%files
%defattr(-,root,root)
%{_ppddir}/share/cups/model/canon*.ppd
%{_cupsbindir}/filter/cmdtocanonij2
%{_cupsbindir}/filter/rastertocanonij
%{_cupsbindir}/backend/cnijbe2
%{_cupsbindir64}/filter/cmdtocanonij2
%{_cupsbindir64}/filter/rastertocanonij
%{_cupsbindir64}/backend/cnijbe2
%{_bindir}/tocanonij
%{_bindir}/tocnpwg
%{_bindir}/cnijlgmon3
%{_libdir}/libcnbpcnclapicom2.so*
%{_libdir}/libcnnet2.so*
%attr(644, lp, lp) %{_libdir}%{CNLIBS}/cnnet.ini
%{_prefix}/share/locale/*/LC_MESSAGES/cnijlgmon3.mo
%{_prefix}/share/cnijlgmon3/*
%{_prefix}/share/cmdtocanonij2/*

%doc doc/LICENSE-cnijfilter-%{VERSION}JP.txt
%doc doc/LICENSE-cnijfilter-%{VERSION}EN.txt
%doc doc/LICENSE-cnijfilter-%{VERSION}SC.txt
%doc doc/LICENSE-cnijfilter-%{VERSION}FR.txt

%doc lproptions/lproptions-*.txt

%changeLog
