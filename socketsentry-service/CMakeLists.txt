project (SocketSentryService)

# Pull in QT, but not KDE.
find_package (Qt4 4.5 REQUIRED)
set (QT_DONT_USE_QTGUI 1)
set (QT_USE_QTNETWORK 1)
set (QT_USE_QTDBUS 1)
set (QT_USE_QTTEST 1)
include (${QT_USE_FILE})

# Other runtime dependencies.
find_library (PCAP pcap)

include_directories (${QT4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} src)

# QObject-derived class headers (server side)
set (SsService_MOC_HEADERS
	src/PcapThread.h
	src/Watcher.h
	src/WatcherDBusAdaptor.h
	src/PcapManager.h
	src/HostNameResolver.h
)

# QObject-derived class headers (client side)
set (SsClientStub_MOC_HEADERS
	src/WatcherClient.h
	src/WatcherClientConsolePrinter.h
)

# Data objects and other cross-tier units 
Set (SsCommon_SRCS
	src/CommunicationFlow.cpp 
	src/CommunicationFlowData.cpp	
	src/IpEndpointPair.cpp
	src/IpEndpointPairData.cpp
	src/FlowMetrics.cpp
	src/FlowMetricsData.cpp	
	src/FlowStatistics.cpp
	src/FlowStatisticsData.cpp
	src/OsProcess.cpp
	src/OsProcessData.cpp
	src/HostAddressUtils.cpp
)

# Service library sources (no main function)
set (SsService_SRCS 
	src/Watcher.cpp 
	src/WatcherDBusAdaptor.cpp 
	src/PcapThread.cpp
	src/NetworkHistory.cpp
	src/DataLinkPacketDecoder.cpp
	src/EthernetPacketDecoder.cpp
	src/RawPacketDecoder.cpp
	src/CookedPacketDecoder.cpp
	src/InternetProtocolDecoder.cpp
	src/ConnectionProcessCorrelator.cpp
	src/PcapManager.cpp
	src/DateTimeUtils.cpp
	src/HostNameResolver.cpp
	src/Latch.cpp
)

# Client library sources (no main function)
set (SsClientStub_SRCS 
	src/WatcherClient.cpp
	src/WatcherClientConsolePrinter.cpp
)

# Unit tests
set (SsService_TEST_SRCS
	test/NetworkHistoryTest.cpp
	test/EthernetPacketDecoderTest.cpp
	test/CookedPacketDecoderTest.cpp
	test/InternetProtocolDecoderTest.cpp
	test/WatcherTest.cpp
	test/PcapManagerTest.cpp
	test/HostAddressUtilsTest.cpp
	test/HostNameResolverTest.cpp
)

# Create the service static lib.
qt4_wrap_cpp (SsService_MOC_OUTFILES ${SsService_MOC_HEADERS})
set (SsService_ALL_SRCS ${SsService_SRCS} ${SsService_MOC_OUTFILES} ${SsCommon_SRCS})
add_library (socksent-service-common STATIC ${SsService_ALL_SRCS})

# Create the service executable.
set (SS_SERVICE_EXE socksent-service)
add_executable (${SS_SERVICE_EXE} src/SsService.cpp)
target_link_libraries (${SS_SERVICE_EXE} socksent-service-common ${QT_LIBRARIES} ${PCAP})
install (TARGETS ${SS_SERVICE_EXE} DESTINATION bin)

# Install the D-Bus system bus config file.
set (DBUS_SYSTEM_POLICY_DIR /etc/dbus-1/system.d)
if (NOT EXISTS ${DBUS_SYSTEM_POLICY_DIR})
  message (FATAL_ERROR "Couldn't find the D-Bus system policy directory.")
endif (NOT EXISTS ${DBUS_SYSTEM_POLICY_DIR})
install (FILES org.socketsentry.Watcher.conf DESTINATION ${DBUS_SYSTEM_POLICY_DIR})

# Configure and install the D-Bus activation service file.
set (DBUS_SYSTEM_SERVICES_INSTALL_DIR "share/dbus-1/system-services")
if (NOT EXISTS ${CMAKE_INSTALL_PREFIX}/${DBUS_SYSTEM_SERVICES_INSTALL_DIR})
  message (FATAL_ERROR "Couldn't find the D-Bus system services installation directory:" ${CMAKE_INSTALL_PREFIX}/${DBUS_SYSTEM_SERVICES_INSTALL_DIR})
endif (NOT EXISTS ${CMAKE_INSTALL_PREFIX}/${DBUS_SYSTEM_SERVICES_INSTALL_DIR})
set(WATCHER_SERVICE_FILE org.socketsentry.Watcher.service)
configure_file (${WATCHER_SERVICE_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${WATCHER_SERVICE_FILE})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${WATCHER_SERVICE_FILE} DESTINATION ${DBUS_SYSTEM_SERVICES_INSTALL_DIR})

# Create the unit tests.
add_qtestlib_tests ("${SsService_TEST_SRCS}" ${QT_LIBRARIES} ${PCAP} socksent-service-common)

# Create client shared lib.
qt4_wrap_cpp (SsClientStub_MOC_OUTFILES ${SsClientStub_MOC_HEADERS})
set (SsClientStub_ALL_SRCS ${SsClientStub_SRCS} ${SsClientStub_MOC_OUTFILES} ${SsCommon_SRCS})
add_library (socketsent-client-common SHARED ${SsClientStub_ALL_SRCS})
install (TARGETS socketsent-client-common DESTINATION lib)

# Create the client test executable.
add_executable (socksent-client src/SsReceiver.cpp)
target_link_libraries (socksent-client socketsent-client-common ${QT_LIBRARIES})
install (TARGETS socksent-client DESTINATION bin)
