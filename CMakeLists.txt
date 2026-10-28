cmake_minimum_required(VERSION 3.21)

project(SmartSearchSKSE VERSION 1.0.1.0 LANGUAGES CXX)

if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

# CommonLibSSE-NG 6.7.1 from a local checkout: the colorglass vcpkg registry is stuck on
# 3.7.0, which predates 1.7.99 support (runtime classification + format-5 address library).
# 6.x is GPL-3.0-or-later where 3.7.0 was MIT, hence COPYING replacing LICENSE.
if(DEFINED ENV{CommonLibSSEPath})
    file(TO_CMAKE_PATH "$ENV{CommonLibSSEPath}" CLIB_SOURCE_DIR)
elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibSSE-NG")
    set(CLIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibSSE-NG")
elseif(IS_DIRECTORY "F:/Modding/SkyrimDev/CommonLibSSE-NG-1799")
    set(CLIB_SOURCE_DIR "F:/Modding/SkyrimDev/CommonLibSSE-NG-1799")
endif()

if(NOT CLIB_SOURCE_DIR OR NOT EXISTS "${CLIB_SOURCE_DIR}/cmake/CommonLibSSE.cmake")
    message(FATAL_ERROR
        "CommonLibSSE-NG 6.7.1+ checkout not found. Set the CommonLibSSEPath environment "
        "variable, or place a checkout at extern/CommonLibSSE-NG.")
endif()

message(STATUS "CommonLibSSE-NG: ${CLIB_SOURCE_DIR}")

# NG defaults this ON, which pulls Catch2 and fails configure.
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

# SE+AE runtime dispatch = one DLL for SE, AE and 1.7.99.
# VR off: we don't target it, and it links openvr_api.lib from an uninitialized submodule.
set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)

add_subdirectory("${CLIB_SOURCE_DIR}" CommonLibSSE-NG)

# NG only includes this on its prebuilt path (which return()s early), so a source build
# never defines add_commonlibsse_plugin().
include("${CLIB_SOURCE_DIR}/cmake/CommonLibSSE.cmake")

add_commonlibsse_plugin(${PROJECT_NAME} MINIMUM_SKSE_VERSION 2.0.17
    SOURCES
        src/Config.cpp
        src/Utils.cpp
        src/Localization.cpp
        src/Matcher.cpp
        src/Scaleform.cpp
        src/Hooks.cpp
        src/plugin.cpp
        shared/IntentModel/IntentModel.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME} PRIVATE includes shared)
target_precompile_headers(${PROJECT_NAME} PRIVATE includes/PCH.h)

# INTENTMODEL_DIAG keeps the INTENTPOLARITY debug line live for Smart Search's tuning loop.
# The Live Bridge leaves it undefined so the shared unit stays log-silent there.
target_compile_definitions(${PROJECT_NAME} PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING INTENTMODEL_DIAG)

# Always emit a matching PDB, even on Release, so a user's crash-log offsets can be
# resolved later. /Zi generates full debug info; /DEBUG writes the PDB; /OPT:REF /OPT:ICF
# keep the release binary optimized (debug info and optimization coexist fine).
# The PDB is archived (below), NOT shipped with the mod.
target_compile_options(${PROJECT_NAME} PRIVATE /Zi)
target_link_options(${PROJECT_NAME} PRIVATE /DEBUG /OPT:REF /OPT:ICF)

if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")
    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")
	
    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        # Note: the build no longer copies data/SmartSearch.ini over the deployed one. copy_if_different
        # was reverting a hand-edited LogLevel (e.g. debug) back to the source default on every build.
        # The INI is shipped in the mod archive instead; edit the deployed copy freely and it persists.
        VERBATIM
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()

# Archive the PDB (symbols) for every build into a version-stamped folder that lives in the
# source tree, NOT the deployed mod folder.
set(PDB_ARCHIVE "${CMAKE_SOURCE_DIR}/pdb-archive/${PROJECT_VERSION}")
add_custom_command(
    TARGET "${PROJECT_NAME}"
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${PDB_ARCHIVE}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${PDB_ARCHIVE}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${PDB_ARCHIVE}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    VERBATIM
)
