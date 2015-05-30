![http://wiki.socket-sentry.googlecode.com/hg/OxygenTheme.png](http://wiki.socket-sentry.googlecode.com/hg/OxygenTheme.png)

Socket Sentry respects the current Plasma theme. Here is a widget displayed using the Oxygen theme. Different font weights indicate the current transfer rates of connections. A light font means connections are idle. A bold font means the current rate is close to its peak. You can click and hover over rows to see more details about them. The _State_ column shows data being sent or received at this moment in time.

---

![http://wiki.socket-sentry.googlecode.com/hg/AirTheme.png](http://wiki.socket-sentry.googlecode.com/hg/AirTheme.png)

Here is a widget displayed with the Air theme. You can use the search box to quickly find connections in the list. And see the tiny red and white "wired lighthouse" on the panel? Socket Sentry can be iconified to a panel to stay out of your way until you need it. Just click the lighthouse to see your current network traffic.

---

![http://wiki.socket-sentry.googlecode.com/hg/LocalSettings.png](http://wiki.socket-sentry.googlecode.com/hg/LocalSettings.png)

In the _local settings_ dialog, you can choose which network device to monitor in the current widget. Choose "any" to watch all devices. You can also choose a grouping for rows in the table. The top choice shows the maximum amount of detail with each connection in its own row. The latter two choices group connections by host and process or program. The subdomain levels setting controls how much detail you'll see when host name lookups are enabled. Setting it to 1, for example, will cause a host named _mail.company.com_ to be shortened to _company.com_. Finally, you can select which traffic statistics and other columns are visible in the table using the selector at the bottom. You can also configure this from the widget itself by right-clicking on it to select columns and dragging headers to rearrange and resize them.

---

![http://wiki.socket-sentry.googlecode.com/hg/GlobalSettings.png](http://wiki.socket-sentry.googlecode.com/hg/GlobalSettings.png)

In the _global settings_ dialog, you can enable name lookups of remote hosts. You can also choose which process should appear in the table when sockets are [shared](FrequentlyAskedQuestions#Why_do_some_connections_show_up_with_the_wrong_process_or_progra.md) among two or more of them. Finally, you can enter a [custom filter expression](FrequentlyAskedQuestions#What_can_I_do_with_the_custom_packet_filter_expression_setting?.md) to constrain the set of connections that can appear in the table using the [pcap](http://www.tcpdump.org.) filter language.