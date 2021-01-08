INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_LORAPHY loraphy)

FIND_PATH(
    LORAPHY_INCLUDE_DIRS
    NAMES loraphy/api.h
    HINTS $ENV{LORAPHY_DIR}/include
        ${PC_LORAPHY_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    LORAPHY_LIBRARIES
    NAMES gnuradio-loraphy
    HINTS $ENV{LORAPHY_DIR}/lib
        ${PC_LORAPHY_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LORAPHY DEFAULT_MSG LORAPHY_LIBRARIES LORAPHY_INCLUDE_DIRS)
MARK_AS_ADVANCED(LORAPHY_LIBRARIES LORAPHY_INCLUDE_DIRS)

