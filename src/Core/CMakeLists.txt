add_library(common_objLib OBJECT)
target_sources(common_objLib
    PRIVATE
        UCT.cpp
        MCTSEntry.cpp)

Set_Multiple_PUBLIC_HEADER(common_objLib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UCT.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MCTSEntry.h)

target_include_directories(common_objLib
    PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(common_objLib 
    PUBLIC
        environments_objLib
        defaultpolicy_objLib)

add_subdirectory(defaultPolicy)

add_library(Core_uctpta STATIC)
target_link_libraries(Core_uctpta
    PUBLIC
        common_objLib
        defaultpolicy_objLib
    PRIVATE
        MCTS::MCTS)
