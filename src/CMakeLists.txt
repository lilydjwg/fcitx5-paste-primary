include_directories(${PROJECT_SOURCE_DIR}/src)

set(FCITX_TSUNDERE_SOURCES
  paste-primary.c
  )
fcitx_add_addon_full(paste-primary DESC
  HEADERS paste-primary.h
  SOURCES ${FCITX_TSUNDERE_SOURCES})
