if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(C9X_NAME_SUFFIX ${VERSION_MAJOR}${VERSION_MINOR})
  set(COMPANION_NAME "companion${C9X_NAME_SUFFIX}")
  set(SIMULATOR_NAME "simulator${C9X_NAME_SUFFIX}")
  set(SIMULATOR_LIB_PATH ${CMAKE_INSTALL_PREFIX}/lib/companion${C9X_NAME_SUFFIX})
else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(COMPANION_NAME "companion")
  set(SIMULATOR_NAME "simulator")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

option(ALLOW_NIGHTLY_BUILDS "Allow nightly builds download / update") # Disabled by default
if(ALLOW_NIGHTLY_BUILDS)
  add_definitions(-DALLOW_NIGHTLY_BUILDS)
endif(ALLOW_NIGHTLY_BUILDS)

option(DEBUG_IMPORT "Turn on debug output for EEPROM import") # Disabled by default
if(DEBUG_IMPORT)
  add_definitions(-DDEBUG_MODEL_IMPORT)
  message(STATUS "EEPROM import debugging enabled")
endif(DEBUG_IMPORT)

option(TABLE_LAYOUT "Use QTableWidget for grid layouts") # Disabled by default
if(TABLE_LAYOUT)
  add_definitions(-DTABLE_LAYOUT)
  message(STATUS "Using QTableWidget")
endif(TABLE_LAYOUT)

if(SDL_FOUND)
  include_directories(${SDL_INCLUDE_DIR})
  add_definitions(-DSIMU_AUDIO)
  add_definitions(-DJOYSTICKS)
endif()

message(STATUS "Looking for QT lupdate ")
find_package(Lupdate)
if(LUPDATE_FOUND)
  message("QT lupdate: " ${LUPDATE_EXECUTABLE})
else()
  message("QT lupdate not found!")
endif()

add_definitions(-DQXT_STATIC)

if(MSVC)
  set(CMAKE_CXX_FLAGS "/EHsc /LD")
  include_directories(${PROJECT_SOURCE_DIR}/winbuild C:/Programs/pthreads/Pre-built.2/include C:/Programs/msinttypes C:/Programs/dirent/include)
  link_directories(C:/Programs/pthreads/Pre-built.2/lib/)
  set(PTHREAD_LIBRARY pthreadVC2.lib)
else()
  set(PTHREAD_LIBRARY pthread)
  add_definitions(-Wall)
  link_directories(/usr/local/lib)
endif()

set(RADIO_SRC_DIRECTORY ${PROJECT_SOURCE_DIR}/radio/src)
set(SIMU_SRC_DIRECTORY ${PROJECT_SOURCE_DIR}/companion/src/simulation)
set(COMPANION_SRC_DIRECTORY ${PROJECT_SOURCE_DIR}/companion/src)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${COMPANION_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/shared
  ${SIMU_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/qxtcommandoptions
)

add_subdirectory(shared)
add_subdirectory(modeledit)
add_subdirectory(generaledit)
add_subdirectory(simulation)
add_subdirectory(qcustomplot)
add_subdirectory(qxtcommandoptions)

set(common_SRCS
  eeprominterface.cpp
  firmwares/th9x/th9xeeprom.cpp # TODO not needed
  firmwares/th9x/th9xinterface.cpp
  firmwares/er9x/er9xeeprom.cpp
  firmwares/er9x/er9xinterface.cpp
  firmwares/gruvin9x/gruvin9xeeprom.cpp
  firmwares/gruvin9x/gruvin9xinterface.cpp
  firmwares/ersky9x/ersky9xeeprom.cpp
  firmwares/ersky9x/ersky9xinterface.cpp
  firmwares/opentx/opentxeeprom.cpp
  firmwares/opentx/open9xStockeeprom.cpp
  firmwares/opentx/open9xGruvin9xeeprom.cpp
  firmwares/opentx/open9xSky9xeeprom.cpp
  firmwares/opentx/opentxinterface.cpp
  file.cpp
  appdata.cpp
  )

qt5_wrap_cpp(common_SRCS)

add_library(common ${common_SRCS})
qt5_use_modules(common Core Xml Widgets)

set(companion_SRCS
  hexinterface.cpp
  firmwareinterface.cpp
  converteeprom.cpp
  # xmlinterface.cpp
  # ${PROJECT_BINARY_DIR}/radio.cxx
  helpers.cpp
  helpers_html.cpp
  mdichild.cpp
  modelslist.cpp
  mountlist.cpp
  apppreferencesdialog.cpp
  fwpreferencesdialog.cpp
  burnconfigdialog.cpp
  multimodelprinter.cpp
  comparedialog.cpp
  contributorsdialog.cpp
  releasenotesdialog.cpp
  releasenotesfirmwaredialog.cpp
  customizesplashdialog.cpp
  radiointerface.cpp
  progresswidget.cpp
  progressdialog.cpp
  process_copy.cpp
  process_flash.cpp
  process_sync.cpp
  flashfirmwaredialog.cpp
  flasheepromdialog.cpp
  printdialog.cpp
  modelprinter.cpp
  fusesdialog.cpp
  logsdialog.cpp
  downloaddialog.cpp
  splashlibrarydialog.cpp
  mainwindow.cpp
  companion.cpp
  radionotfound.cpp
  wizarddata.cpp
  wizarddialog.cpp
  )

set(companion_MOC_HDRS
  apppreferencesdialog.h
  fwpreferencesdialog.h
  burnconfigdialog.h
  comparedialog.h
  printdialog.h
  fusesdialog.h
  logsdialog.h
  contributorsdialog.h
  releasenotesdialog.h
  releasenotesfirmwaredialog.h
  customizesplashdialog.h
  splashlibrarydialog.h
  splashlabel.h
  progresswidget.h
  progressdialog.h
  process_copy.h
  process_flash.h
  process_sync.h
  flashfirmwaredialog.h
  flasheepromdialog.h
  downloaddialog.h
  modelslist.h
  mdichild.h
  mainwindow.h
  radionotfound.h
  helpers.h
  wizarddialog.h
  modelprinter.h
  multimodelprinter.h
  )

set(companion_UIS
  mdichild.ui
  comparedialog.ui
  fusesdialog.ui
  logsdialog.ui
  apppreferencesdialog.ui
  fwpreferencesdialog.ui
  burnconfigdialog.ui
  downloaddialog.ui
  printdialog.ui
  htmldialog.ui
  customizesplashdialog.ui
  splashlibrarydialog.ui
  progresswidget.ui
  progressdialog.ui
  flashfirmwaredialog.ui
  flasheepromdialog.ui
  radionotfound.ui
  )

set(companion_RESOURCES
  companion.qrc
  ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
  )

configure_file(${COMPANION_SRC_DIRECTORY}/version.h.in ${CMAKE_BINARY_DIR}/version.h @ONLY)
configure_file(${COMPANION_SRC_DIRECTORY}/translations.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc @ONLY)
configure_file(${COMPANION_SRC_DIRECTORY}/companion.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/companion.desktop @ONLY)
configure_file(${COMPANION_SRC_DIRECTORY}/simulator.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/simulator.desktop @ONLY)

if(WIN32)
  set(companion_SRCS ${companion_SRCS} icon.rc)
  if(NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS -mwindows)
  endif()
endif()

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

set(LANGUAGES he pl pt ru de fr es it sv cs fi nl)
foreach(language ${LANGUAGES})
  set(companion_TS ${companion_TS} translations/companion_${language}.ts)
endforeach(language)

qt5_wrap_ui(companion_SRCS ${companion_UIS})
qt5_wrap_cpp(companion_SRCS ${companion_MOC_HDRS})
qt5_add_translation(companion_QM ${companion_TS})
qt5_add_resources(companion_RCC ${companion_RESOURCES})
add_custom_target(gen_qrc DEPENDS ${companion_RCC})

add_definitions(-DQT_TRANSLATIONS_DIR="${QT_TRANSLATIONS_DIR}")

if(APPLE)
  add_executable(${COMPANION_NAME} MACOSX_BUNDLE ${companion_SRCS} ${companion_RCC} ${companion_QM})
  # OS X compiler no longer automatically includes /Library/Frameworks in search path
  set(CMAKE_EXE_LINKER_FLAGS -F/Library/Frameworks)
else()
  add_executable(${COMPANION_NAME} WIN32 ${companion_SRCS} ${companion_RCC} ${companion_QM})
endif()

add_dependencies(${COMPANION_NAME} gen_qrc)
qt5_use_modules(${COMPANION_NAME} Core Widgets Network)
target_link_libraries(${COMPANION_NAME} PRIVATE generaledit modeledit simulation common qcustomplot shared ${PTHREAD_LIBRARY} ${SDL_LIBRARY})

############# Standalone simu ###############

set(simu_SRCS
  modeledit/node.cpp
  modeledit/edge.cpp # TODO not needed
  simulator.cpp
  )

set(simu_MOC_HDRS
  modeledit/node.h
  )

qt5_wrap_cpp(simu_SRCS ${simu_MOC_HDRS} )

if(WIN32)
  set(simu_SRCS ${simu_SRCS} icon.rc)
endif()

if(APPLE)
  add_executable(${SIMULATOR_NAME} MACOSX_BUNDLE ${simu_SRCS} ${companion_RCC})
else()
  add_executable(${SIMULATOR_NAME} WIN32 ${simu_SRCS} ${companion_RCC})
endif()

add_dependencies(${SIMULATOR_NAME} gen_qrc)
# TODO not the same link command than companion?
target_link_libraries(${SIMULATOR_NAME} PRIVATE simulation common qxtcommandoptions ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY} ${PTHREAD_LIBRARY} ${SDL_LIBRARY} ${OPENTX_SIMULATOR_LIBS})

############# Packaging ####################

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  install(TARGETS ${COMPANION_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  install(TARGETS ${SIMULATOR_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/companion.desktop DESTINATION share/applications RENAME companion${C9X_NAME_SUFFIX}.desktop)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simulator.desktop DESTINATION share/applications RENAME simulator${C9X_NAME_SUFFIX}.desktop)
  if(${CMAKE_INSTALL_PREFIX} MATCHES "/usr/local")
    install(FILES images/linuxicons/16x16/companion.png DESTINATION /usr/share/icons/hicolor/16x16/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/22x22/companion.png DESTINATION /usr/share/icons/hicolor/22x22/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/24x24/companion.png DESTINATION /usr/share/icons/hicolor/24x24/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/32x32/companion.png DESTINATION /usr/share/icons/hicolor/32x32/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/48x48/companion.png DESTINATION /usr/share/icons/hicolor/48x48/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/128x128/companion.png DESTINATION /usr/share/icons/hicolor/128x128/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/256x256/companion.png DESTINATION /usr/share/icons/hicolor/256x256/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/512x512/companion.png DESTINATION /usr/share/icons/hicolor/512x512/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/scalable/companion.svg DESTINATION /usr/share/icons/hicolor/scalable/apps RENAME companion${C9X_NAME_SUFFIX}.svg)
    install(FILES ../targets/linux/45-companion-taranis.rules DESTINATION /lib/udev/rules.d RENAME 45-companion${C9X_NAME_SUFFIX}-taranis.rules)
  else()
    install(FILES images/linuxicons/16x16/companion.png DESTINATION share/icons/hicolor/16x16/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/22x22/companion.png DESTINATION share/icons/hicolor/22x22/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/24x24/companion.png DESTINATION share/icons/hicolor/24x24/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/32x32/companion.png DESTINATION share/icons/hicolor/32x32/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/48x48/companion.png DESTINATION share/icons/hicolor/48x48/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/128x128/companion.png DESTINATION share/icons/hicolor/128x128/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/256x256/companion.png DESTINATION share/icons/hicolor/256x256/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/512x512/companion.png DESTINATION share/icons/hicolor/512x512/apps RENAME companion${C9X_NAME_SUFFIX}.png)
    install(FILES images/linuxicons/scalable/companion.svg DESTINATION share/icons/hicolor/scalable/apps RENAME companion${C9X_NAME_SUFFIX}.svg)
    install(FILES ../targets/linux/45-companion-taranis.rules DESTINATION lib/udev/rules.d RENAME 45-companion${C9X_NAME_SUFFIX}-taranis.rules)
  endif()
  # Linux specific code
  set(OperatingSystem "Linux")
elseif(WIN32)
  get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
  get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)
  get_filename_component(SDL_DIR ${SDL_LIBRARY} PATH)
  install(FILES ${QT_DLL_DIR}/icudt54.dll ${QT_DLL_DIR}/icuin54.dll ${QT_DLL_DIR}/icuuc54.dll ${QT_DLL_DIR}/Qt5Core.dll ${QT_DLL_DIR}/Qt5Gui.dll ${QT_DLL_DIR}/Qt5Widgets.dll ${QT_DLL_DIR}/Qt5Xml.dll ${QT_DLL_DIR}/Qt5Network.dll ${QT_DLL_DIR}/Qt5PrintSupport.dll ${QT_DLL_DIR}/Qt5Multimedia.dll ${SDL_DIR}/SDL.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
  install(FILES ${QT_DLL_DIR}/icudt54.dll ${QT_DLL_DIR}/icuin54.dll ${QT_DLL_DIR}/icuuc54.dll ${QT_DLL_DIR}/Qt5Cored.dll ${QT_DLL_DIR}/Qt5Guid.dll ${QT_DLL_DIR}/Qt5Widgetsd.dll ${QT_DLL_DIR}/Qt5Xmld.dll ${QT_DLL_DIR}/Qt5Networkd.dll ${QT_DLL_DIR}/Qt5PrintSupportd.dll ${QT_DLL_DIR}/Qt5Multimediad.dll ${SDL_DIR}/SDL.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
endif()

if(WIN32)
  if(MSVC)
    set(COMPANION_NSI_IN_FILE companion-vs.nsi.in)
    set(SIMULATOR_NSI_IN_FILE simulator-vs.nsi.in)
  else()
    set(COMPANION_NSI_IN_FILE companion-msys.nsi.in)
    set(SIMULATOR_NSI_IN_FILE simulator-msys.nsi.in)
  endif()

  set(SYSDIR "$ENV{windir}/system32")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../targets/windows/${COMPANION_NSI_IN_FILE} ${PROJECT_BINARY_DIR}/companion/companion.nsi @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../targets/windows/${SIMULATOR_NSI_IN_FILE} ${PROJECT_BINARY_DIR}/companion/simulator.nsi @ONLY)

  find_program(NSIS_EXE makensis.exe PATHS
    "C:/Program Files/NSIS"
    "C:/Program Files (x86)/NSIS"
    "C:/Programs/NSIS"
    )

  add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/companion/companion-windows.exe
    COMMAND "${NSIS_EXE}" ARGS ${PROJECT_BINARY_DIR}/companion/companion.nsi
    DEPENDS companion simulator opentx-simulators ${PROJECT_BINARY_DIR}/companion/companion.nsi
    COMMENT "Companion Windows NSIS Installer")

  add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/companion/simulator-install.exe
    COMMAND "${NSIS_EXE}" ARGS ${PROJECT_BINARY_DIR}/companion/simulator.nsi
    DEPENDS simulator ${PROJECT_BINARY_DIR}/companion/simulator.nsi
    COMMENT "Simulator Windows NSIS Installer")

  add_custom_target(installer
    DEPENDS ${PROJECT_BINARY_DIR}/companion/companion-windows.exe
    DEPENDS ${PROJECT_BINARY_DIR}/companion/simulator-windows.exe
    SOURCES ${PROJECT_BINARY_DIR}/companion/companion.nsi
    SOURCES ${PROJECT_BINARY_DIR}/companion/simulator.nsi
    )
endif()

if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE iconmac.icns)
  if(CMAKE_GENERATOR STREQUAL Xcode)
    set(WORK_DIR ${CMAKE_BINARY_DIR}/Debug)
  else()
    set(WORK_DIR ${PROJECT_BINARY_DIR})
  endif()

  add_custom_target(addframeworks
    COMMAND mkdir -p companion.app/Contents/Frameworks
    COMMAND mkdir -p companion.app/Contents/Resources
    COMMAND mkdir -p companion.app/Contents/translations
    COMMAND rsync ${PROJECT_SOURCE_DIR}/images/${MACOSX_BUNDLE_ICON_FILE} companion.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE} # Copy the icon files
    COMMAND rsync -r --exclude=.svn ${PROJECT_SOURCE_DIR}/../targets/mac/qt_menu.nib companion.app/Contents/Resources/
    COMMAND rsync ${PROJECT_BINARY_DIR}/*.qm companion.app/Contents/translations
    COMMAND rsync ${PROJECT_BINARY_DIR}/*.qm companion.app/Contents/translations
    COMMAND rsync -r /Library/Frameworks/SDL.framework companion.app/Contents/Frameworks/
    COMMAND ${QT_BINARY_DIR}/macdeployqt companion.app
    COMMAND install_name_tool -change @rpath/SDL.framework/Versions/A/SDL @executable_path/../Frameworks/SDL.framework/Versions/A/SDL companion.app/Contents/MacOS/companion
    WORKING_DIRECTORY ${WORK_DIR}
    DEPENDS ${COMPANION_NAME})

  add_custom_target(makedmg
    COMMAND ${QT_BINARY_DIR}/macdeployqt companion.app -dmg -no-plugins
    WORKING_DIRECTORY ${WORK_DIR}
    DEPENDS addframeworks opentx-simulators-bundle)
endif()

if(LUPDATE_FOUND)
  add_custom_target(translations
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${LUPDATE_EXECUTABLE} ${CMAKE_SOURCE_DIR} -no-obsolete -ts ${companion_TS}
    )
else()
  add_custom_target(translations
    COMMAND echo "Sorry, QT lupdate was not found."
    )
endif()

set(CPACK_PACKAGE_NAME "companion${C9X_NAME_SUFFIX}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Models and settings editor for the OpenTx open source firmware")
string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)

# The file stripping is deliberately disabled, with the stripped file we get
# very poor trace-backs from the users when they report Companion crash
set(CPACK_STRIP_FILES FALSE)

find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
if(DPKG_PROGRAM)
  set(CPACK_GENERATOR "DEB")
  execute_process(
    COMMAND ${DPKG_PROGRAM} --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Bertrand Songis <bsongis-nosp@m-gmail.com>")
  set(CPACK_DEBIAN_PACKAGE_VERSION ${VERSION})
  set(CPACK_DEBIAN_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
  set(CPACK_DEBIAN_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
  set(CPACK_DEBIAN_ARCHITECTURE $(CMAKE_SYSTEM_PROCESSOR))
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
else(DPKG_PROGRAM)
  set(CPACK_GENERATOR "RPM")
  set(CPACK_RPM_PACKAGE_VERSION ${VERSION})
  set(CPACK_RPM_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}-${VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
  SET(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/lib /lib/udev /lib/udev/rules.d")
endif(DPKG_PROGRAM)

include(CPack)
