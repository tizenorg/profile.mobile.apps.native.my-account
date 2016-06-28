Name:       my-account
Summary:    My account application
Version:    1.9.103
Release:    1
Group:      main/devel
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  cmake
BuildRequires:  gettext-devel
BuildRequires:  edje-bin

%description
Accounts ug of setting application.

%prep
%setup -q

%define APPDIR /usr/apps/setting-myaccount-efl
%define BINDIR %{APPDIR}/bin
%define LIBDIR %{APPDIR}/lib/ug
%define RESDIR %{APPDIR}/res
%define SHAREDDIR %{APPDIR}/shared
%define DATADIR %{APPDIR}/shared/trusted

%build
export CFLAGS="${CFLAGS} -fPIC -fvisibility=hidden"

%define PREFIX    "/usr"
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX} \

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp %{_builddir}/%{name}-%{version}/LICENSE.APLv2  %{buildroot}/usr/share/license/%{name}
%make_install

%files
%manifest my-account.manifest
%defattr(-,root,root,-)
/usr/share/license/%{name}
%{LIBDIR}/*
%{RESDIR}/*
%{SHAREDDIR}/*
/usr/share/packages/*.xml

%post
/sbin/ldconfig
