macro(graphsgen_update_3rdparty SUBMODULE)
    if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodule as needed
        message(STATUS "${SUBMODULE} update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive "${SUBMODULE}"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout ${SUBMODULE} submodule")
        endif()
    endif()
endmacro()

# --------------------
# yaml-cpp
# --------------------
graphsgen_update_3rdparty(yaml-cpp)

SET(APPLE_UNIVERSAL_BIN OFF CACHE INTERNAL "" FORCE)
SET(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
SET(MSVC_SHARED_RT ON CACHE INTERNAL "" FORCE)
SET(MSVC_STHREADED_RT OFF CACHE INTERNAL "" FORCE)
SET(YAML_CPP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
SET(YAML_CPP_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
SET(YAML_CPP_BUILD_CONTRIB OFF CACHE INTERNAL "" FORCE)
SET(YAML_CPP_INSTALL OFF CACHE INTERNAL "" FORCE)

set(CMAKE_FOLDER 3rdparty/yaml-cpp)
add_subdirectory(yaml-cpp)
set(CMAKE_FOLDER 3rdparty)

