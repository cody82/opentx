include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(generaledit_NAMES
  generaledit
  generalsetup
  trainer
  hardware
)

set(generaledit_SRCS
  calibration.cpp
)

set(generaledit_HDRS
  calibration.h
)

set(generaledit_UIS
)

foreach(name ${generaledit_NAMES})
  set(generaledit_SRCS ${generaledit_SRCS} ${name}.cpp)
  set(generaledit_HDRS ${generaledit_HDRS} ${name}.h)
  set(generaledit_UIS  ${generaledit_UIS}  ${name}.ui)
endforeach()
 
qt5_wrap_ui(generaledit_SRCS ${generaledit_UIS})
qt5_wrap_cpp(generaledit_SRCS ${generaledit_HDRS})

add_library(generaledit ${generaledit_SRCS})
qt5_use_modules(generaledit Widgets Xml Multimedia)