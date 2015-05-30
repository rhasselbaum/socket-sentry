# [![](http://wiki.socket-sentry.googlecode.com/hg/ScreenshotBanner.png)](http://code.google.com/p/socket-sentry/wiki/GuidedTour) It's your 'Net. #

---

# Latest release: [0.9.3](GetSoftware.md) #

Socket Sentry is a KDE Plasma widget that displays real-time network traffic on your Linux computer. It shows you which processes are communicating with which hosts, current data transfer rates, protocols, and more. Socket Sentry combines the best parts of tools like [iftop](http://www.ex-parrot.com/pdw/iftop) and [netstat](http://en.wikipedia.org/wiki/Netstat) in a modern desktop interface that's approachable and easy-to-use.

## Features ##

  * **Monitor one or all** network devices in each widget
  * See **over 10 different stats** and other characteristics of each connection
  * View **all connections** individually or group by host and process/program
  * **Sort by any column** with secondary sorting by transfer rate
  * Quick search box helps you **find specific connections**
  * Optional **hostname lookup** with configurable subdomain depth
  * Customizable **filter rules** using the pcap expression language
  * **IPv6** support
  * Efficient **data sharing** across widgets to minimize resource use
  * Configurable **security** for multiuser PCs

It's the right tool to answer all kinds of questions about the software running on your computer. For example:

  * How many connections is my download manager using? What's the throughput of each?
  * What programs on my computer are phoning home? How often?
  * How much bandwidth does my streaming radio service consume if I leave it open?
  * What programs are the biggest bandwidth hogs?
  * Which video site delivers the best throughput?
  * Does my browser keep talking to the web server after the page is loaded?
  * My program seems frozen. Is it communicating on the network?
  * Is someone connected to my SSH/Samba/FTP/CUPS/Web/Other server right now?
  * The network printer won't print. Is the job being sent to it?

Keep constant tabs on your computer's network activity by placing it on your desktop, or add it as a panel icon to keep it out of the way until you need it. Worried about it slowing down your system? So are we! That's why we've designed it to [minimize its impact](FrequentlyAskedQuestions#How_can_I_reduce_CPU_overhead?.md) on your PC's resources, especially when it's iconified. Learn more by visiting our FrequentlyAskedQuestions page and take the GuidedTour to see some additional screenshots. If it sounds good to you, head over to the GetSoftware page to get started.

Socket Sentry is free software distributed under the [GNU General Public License](http://www.gnu.org/licenses/gpl.html). If you find it useful, please spread the word. We also appreciate your feedback in our [discussion group](http://groups.google.com/group/socket-sentry).

# For developers #

In addition to the graphical widget, Socket Sentry provides a generalized [D-Bus](http://www.freedesktop.org/wiki/Software/dbus) interface atop its data collection service that can be used by other [authorized](FrequentlyAskedQuestions#Security.md) software components to get real-time network traffic updates from the host PC. This module has no dependencies on KDE. It only requires some non-GUI Qt packages, [libpcap](http://www.tcpdump.org/), and of course, D-Bus. If you are interested in integrating with the D-Bus service, take a look at the `socketsentry-service` subproject in the source tree and the `SsReceiver.cpp` file, which shows a simple example of a command line client that connects to the service and dumps the data it receives to the console.
