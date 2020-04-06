add_library(common_uctpta_objLib OBJECT)
target_sources(common_uctpta_objLib
    PRIVATE
        UCT_PTA.cpp
        MCTSEntry.cpp)

Set_Multiple_PUBLIC_HEADER(common_uctpta_objLib
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UCT_PTA.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MCTSEntry.h)

target_include_directories(common_uctpta_objLib
    PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(common_uctpta_objLib 
    PUBLIC
        environments_uctpta_objLib
        defaultpolicy_uctpta_objLib)

add_subdirectory(defaultPolicy)

add_library(Core_uctpta STATIC)
target_link_libraries(Core_uctpta
    PUBLIC
        common_uctpta_objLib
        defaultpolicy_uctpta_objLib
    PRIVATE
        MCTS::MCTS)
