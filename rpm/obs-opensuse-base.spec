# openSUSE basic spec file for Socket Sentry.

Name: 			socketsentry
License: 		GPLv3
Group:          System/GUI/KDE
Summary: 		Network traffic monitor for KDE Plasma
Version:		0.9.0
Release:		1
URL:			http://code.google.com/p/socket-sentry
Source: 		%{name}-%{version}.tar.gz
BuildRoot:		%{_tmppath}/%{name}-%{version}-build
BuildRequires:	cmake >= 2.6, libqt4-devel >= 4.5, libpcap-devel >= 1.0, libkde4-devel >= 4.3
Requires: 		libqt4 >= 4.5.1, kdebase4-runtime >= 4.3.2, libpcap >= 1.0

%description
A KDE Plasma widget that displays real-time traffic information for 
active network connections on your Linux computer. It shows you which
processes are communicating with which hosts, current data transfer 
rates, protocols, and more. You can view all connections or see
summary traffic by host pair and process or program. It supports IPv4
and 6, optional host name lookups with configurable subdomain depth,
and many sorting and filtering options including pcap filter expressions.

%prep  
%setup -n %{name}-%{version}  
   
%build  
%cmake_kde4 -d builddir    
%make_jobs    
   
%install  
cd builddir  
%kde4_makeinstall    
%suse_update_desktop_file -n $RPM_BUILD_ROOT/usr/share/kde4/services/socksent-plasma-engine.desktop
%suse_update_desktop_file -n $RPM_BUILD_ROOT/usr/share/kde4/services/socksent-plasma-widget.desktop
%kde_post_install
kbuildsycoca4
   
%clean  
rm -rf $RPM_BUILD_ROOT  
   
%files
%defattr(-,root,root)
%_bindir/socksent-service
%config /etc/dbus-1/system.d/org.socketsentry.Watcher.conf
%_prefix/share/dbus-1/system-services/org.socketsentry.Watcher.service
%_libdir/libsocketsent-client-common.so
%_bindir/socksent-client
%_kde4_modulesdir/socksent-plasma-engine.so
%_kde4_servicesdir/socksent-plasma-engine.desktop
%_kde4_modulesdir/socksent-plasma-widget.so
%_kde4_servicesdir/socksent-plasma-widget.desktop
%_kde4_iconsdir/hicolor/64x64/apps/./socketsentry_receiving.png
%_kde4_iconsdir/hicolor/64x64/apps/./socketsentry_sendingreceiving.png
%_kde4_iconsdir/hicolor/48x48/apps/./socketsentry.png
%_kde4_iconsdir/hicolor/32x32/apps/./socketsentry.png
%_kde4_iconsdir/hicolor/64x64/apps/./socketsentry_quiet.png
%_kde4_iconsdir/hicolor/64x64/apps/./socketsentry_sending.png
%_kde4_iconsdir/hicolor/64x64/apps/./socketsentry.png
%_kde4_iconsdir/hicolor/scalable/apps/./socketsentry_receiving.svgz
%_kde4_iconsdir/hicolor/scalable/apps/./socketsentry_sendingreceiving.svgz
%_kde4_iconsdir/hicolor/scalable/apps/./socketsentry.svgz
%_kde4_iconsdir/hicolor/scalable/apps/./socketsentry_quiet.svgz
%_kde4_iconsdir/hicolor/scalable/apps/./socketsentry_sending.svgz
