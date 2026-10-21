# PS Vita build of the Elasto Mania (2000) source code.
# Requires VitaSDK (https://vitasdk.org) with the default packages (vita2d, zlib,
# libpng, libjpeg-turbo, freetype). Build:
#   mkdir build && cd build && cmake .. && make
# Output: elma.vpk
cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(elma C CXX)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Elasto Mania")
set(VITA_TITLEID  "ELMA00001")
set(VITA_VERSION  "01.10")
# Where the game looks for its data files on the Vita:
set(ELMA_DATA_DIR "ux0:data/elma")

# The code was written for Watcom/MSVC on x86:
#  -fsigned-char : plain 'char' is signed there, unsigned on ARM by default
#  -fpermissive  : 1990s C++ (implicit int, old casts)
set(COMMON_FLAGS "-O2 -fsigned-char -fno-strict-aliasing -w")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=gnu++98 -fpermissive")

# Game translation units (the Windows build minus its platform layer):
file(STRINGS build_units.txt ELMA_UNITS REGEX "^[a-z0-9_]+$")
set(ELMA_SOURCES "")
foreach(unit ${ELMA_UNITS})
  list(APPEND ELMA_SOURCES src/${unit}.cpp)
endforeach()

add_executable(elma
  ${ELMA_SOURCES}
  platform/vita_platform.cpp
  platform/vita_gpu.cpp
  platform/vita_hhigh.cpp
  compat/compat.c
)

target_include_directories(elma PRIVATE compat src platform)
target_compile_definitions(elma PRIVATE PORT_ADATKONYVTAR="${ELMA_DATA_DIR}")

target_link_libraries(elma
  vita2d
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
  SceCtrl_stub
  SceTouch_stub
  SceAudio_stub
  SceCommonDialog_stub
  SceAppUtil_stub
  SceAppMgr_stub
  ScePgf_stub
  ScePvf_stub
  ScePower_stub
  SceLibKernel_stub
  SceKernelThreadMgr_stub
  SceProcessmgr_stub
  freetype
  png
  jpeg
  z
  m
  c
)

vita_create_self(eboot.bin elma)
vita_create_vpk(elma.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
