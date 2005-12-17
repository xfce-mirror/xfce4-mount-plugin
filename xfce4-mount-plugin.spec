Summary: 	Mount plugin for the Xfce panel
Name: 		xfce4-mount-plugin
Version: 	0.3.2
Release: 	1
License:	GPL
URL: 		http://xfce-goodies.berlios.de/
Source0: 	%{name}-%{version}.tar.gz
Group: 		User Interface/Desktops
BuildRoot: 	%{_tmppath}/%{name}-root
Requires:	xfce4-panel >= 4.0.0
BuildRequires:	xfce4-panel >= 4.0.0

%description
This plugin allows to mount/unmount file systems.

%prep
%setup -q

%build
%configure
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT mandir=%{_mandir}

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog INSTALL README TODO
%{_libdir}/xfce4/panel-plugins/
%{_datadir}/
