add_library(paste-primary MODULE paste-primary.cpp)
target_link_libraries(paste-primary Fcitx5::Core Fcitx5::Module::XCB)
set_target_properties(paste-primary PROPERTIES PREFIX "")
install(TARGETS paste-primary DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")

fcitx5_translate_desktop_file(paste-primary.conf.in paste-primary.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/paste-primary.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")
