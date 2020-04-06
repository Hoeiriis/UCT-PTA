cmake_minimum_required(VERSION 3.12...3.13)
project(UCT_PTA 
        VERSION 0.0.1 
        LANGUAGES "CXX")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_VERBOSE_MAKEFILE ON)


set( CMAKE_POSITION_INDEPENDENT_CODE ON )
############################################
# Cmake commands only enabled when developing 
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  message("Setting local variables")
  set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
  set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/build")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
  set(EXTERNAL_INSTALL_LOCATION "${PROJECT_ROOT}/ExternalLibs")
  set(PREFIX_LOCATION "${PROJECT_ROOT}/.prefixes")

  message(STATUS "Prefix=${PREFIX_LOCATION}")
endif()

#############################################
# MACROS
macro(Set_Multiple_PUBLIC_HEADER TARGET)
  set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER "${current_public_headers};${ARGN}")  
endmacro()

#### DEPENDENCIES ####
# include(FetchContent)
# FetchContent_Declare(
#   MCTS
#   GIT_REPOSITORY https://github.com/Hoeiriis/MCTS.git
#   GIT_TAG        origin/master
# )
# FetchContent_MakeAvailable(MCTS)

include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/external)

ExternalProject_add(MCTS
        GIT_REPOSITORY https://github.com/Hoeiriis/MCTS.git
        GIT_TAG master
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
        )

        
#install(TARGETS ./MCTS-prefix DESTINATION "/")
externalProject_Add_StepTargets(MCTS install)
# we can now include external libraries
include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

find_package(MCTS REQUIRED)
if(MCTS_FOUND)
    set_target_properties(
        MCTS::Environments
        MCTS::Core
        PROPERTIES ENABLE_EXPORTS TRUE
                   IMPORTED_GLOBAL TRUE)
else()
    message(STATUS "MCTS NOT FOUND")
endif()


#### LIBRARY TARGETS ####
add_library(UCT_PTA)
add_subdirectory(src/Environments)
add_subdirectory(src/Core)

add_library(UCT_PTA::Environments ALIAS Environments_uctpta)
add_library(UCT_PTA::Core ALIAS Core_uctpta)

target_link_libraries(UCT_PTA
    PUBLIC
        UCT_PTA::Environments
        UCT_PTA::Core
    PRIVATE
        MCTS::MCTS
        environments_uctpta_objLib
        common_uctpta_objLib
        )


target_compile_features(UCT_PTA PRIVATE cxx_auto_type)
target_compile_options(UCT_PTA PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)
        
##########################################################
# Installation

include(GNUInstallDirs)
install(
    TARGETS
        UCT_PTA
        Environments_uctpta
        environments_uctpta_objLib
        Core_uctpta
        common_uctpta_objLib
        defaultpolicy_uctpta_objLib
        EXPORT UCT_PTATargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT UCT_PTATargets
    FILE UCT_PTATargets.cmake
    NAMESPACE UCT_PTA::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UCT_PTA)

#Create a ConfigVersion.cmake file for helping to find version
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/UCT_PTAConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/UCT_PTATargets.cmake.in
    UCT_PTAConfig.cmake
    @ONLY)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/UCT_PTAConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/UCT_PTAConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UCT_PTA
)

#################################################
# Exporting package to be found with find_package() by other CMake projects 
export(EXPORT UCT_PTATargets
      NAMESPACE UCT_PTA::
      FILE ${CMAKE_CURRENT_BINARY_DIR}/UCT_PTATargets.cmake)
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE UCT_PTA)
