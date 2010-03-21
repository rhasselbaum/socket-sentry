project (SocketSentryPlasmaEngine)

find_package (KDE4 4.3 REQUIRED)
include (KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${SocketSentryService_SOURCE_DIR}/src ${KDE4_INCLUDES})
 
set (SsPlasmaEngine_SRCS 
	src/SocketSentryDataEngine.cpp
)

# Now make sure all files get to the right place
kde4_add_plugin(socksent-plasma-engine ${SsPlasmaEngine_SRCS})
target_link_libraries(socksent-plasma-engine socketsent-client-common ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})
 
install(TARGETS socksent-plasma-engine
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
set (SS_ENGINE_DESKTOP_FILE socksent-plasma-engine.desktop)
configure_file (${SS_ENGINE_DESKTOP_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${SS_ENGINE_DESKTOP_FILE})
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${SS_ENGINE_DESKTOP_FILE} DESTINATION ${SERVICES_INSTALL_DIR})
 
