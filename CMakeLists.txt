cmake_minimum_required(VERSION 2.8)

# VitaSDK defines
if( NOT DEFINED CMAKE_TOOLCHAIN_FILE )
  if( DEFINED ENV{VITASDK} )
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

# Project start
set(VITA_APPNAME OpenJazz)
set(VITA_TITLEID "OPENJAZZ1")
set(VITA_VERSION "01.00")

project(${VITA_APPNAME})
include("${VITASDK}/share/vita.cmake" REQUIRED)

# Flags and includes
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -DUSE_MODPLUG -DDATAPATH=\\\"ux0:data/jazz/\\\"")

include_directories(
  ${VITASDK}/arm-vita-eabi/include/SDL
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/vitashaderlibs/vita2d_fbo/includes/
  ${CMAKE_SOURCE_DIR}/vitashaderlibs/vita-shader-collection/includes/
)

link_directories(
  ${CMAKE_SOURCE_DIR}/vitashaderlibs/vita2d_fbo/lib/
  ${CMAKE_SOURCE_DIR}/vitashaderlibs/vita-shader-collection/lib/
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Builds
add_executable(${VITA_APPNAME}
  src/game/clientgame.cpp
  src/game/game.cpp
  src/game/gamemode.cpp
  src/game/localgame.cpp
  src/game/servergame.cpp
  src/io/gfx/anim.cpp
  src/io/gfx/font.cpp
  src/io/gfx/paletteeffects.cpp
  src/io/gfx/sprite.cpp
  src/io/gfx/video.cpp
  src/io/controls.cpp
  src/io/file.cpp
  src/io/network.cpp
  src/io/sound.cpp
  src/jj1bonuslevel/jj1bonuslevelplayer/jj1bonuslevelplayer.cpp
  src/jj1bonuslevel/jj1bonuslevel.cpp
  src/jj1level/jj1event/jj1bridge.cpp
  src/jj1level/jj1event/jj1event.cpp
  src/jj1level/jj1event/jj1guardians.cpp
  src/jj1level/jj1event/jj1standardevent.cpp
  src/jj1level/jj1levelplayer/jj1bird.cpp
  src/jj1level/jj1levelplayer/jj1levelplayer.cpp
  src/jj1level/jj1levelplayer/jj1levelplayerframe.cpp
  src/jj1level/jj1bullet.cpp
  src/jj1level/jj1demolevel.cpp
  src/jj1level/jj1level.cpp
  src/jj1level/jj1levelframe.cpp
  src/jj1level/jj1levelload.cpp
  src/jj1planet/jj1planet.cpp
  src/jj1scene/jj1scene.cpp
  src/jj1scene/jj1sceneload.cpp
  src/jj2level/jj2event/jj2event.cpp
  src/jj2level/jj2event/jj2eventframe.cpp
  src/jj2level/jj2levelplayer/jj2levelplayer.cpp
  src/jj2level/jj2levelplayer/jj2levelplayerframe.cpp
  src/jj2level/jj2layer.cpp
  src/jj2level/jj2level.cpp
  src/jj2level/jj2levelframe.cpp
  src/jj2level/jj2levelload.cpp
  src/level/level.cpp
  src/level/movable.cpp
  src/level/levelplayer.cpp
  src/menu/gamemenu.cpp
  src/menu/mainmenu.cpp
  src/menu/menu.cpp
  src/menu/plasma.cpp
  src/menu/setupmenu.cpp
  src/player/player.cpp
  src/main.cpp
  src/setup.cpp
  src/util.cpp
  src/io/gfx/psp2_shader.c
)

target_link_libraries(${VITA_APPNAME}
  SDL
  modplug
  vita2d
  vitashaders
  z
  SceAudio_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  SceHid_stub
  ScePower_stub
  SceSysmodule_stub
  SceTouch_stub
)

file(GLOB_RECURSE ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} data/*)

set(DATA "")

FOREACH(ITEM ${ASSETS})
  list(APPEND DATA "FILE")
  list(APPEND DATA ${ITEM})
  list(APPEND DATA ${ITEM})
ENDFOREACH()

# Create Vita artifacts
vita_create_self(eboot.bin ${VITA_APPNAME} UNSAFE)
vita_create_vpk(${VITA_APPNAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APPNAME}
  #FILE res/icon0.png sce_sys/icon0.png
  #FILE res/bg.png sce_sys/livearea/contents/bg.png
  #FILE res/startup.png sce_sys/livearea/contents/startup.png
  #FILE res/template.xml sce_sys/livearea/contents/template.xml
  ${DATA}
)
