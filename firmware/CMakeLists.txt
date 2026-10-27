cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Board selection: one manifest -> one generated header -> one binding component.
# Override with: idf.py -DDADA_BOARD=<manifest id> build
# ---------------------------------------------------------------------------
set(DADA_BOARD "jinyichen-esp32-c3-lcd-2.4" CACHE STRING "Board manifest id")
set(DADA_APP "touch-counter" CACHE STRING "Native app id")
option(DADA_ALLOW_UNVERIFIED "Allow local bring-up against declared, unverified capabilities" OFF)
set_property(CACHE DADA_APP PROPERTY STRINGS touch-counter bringup focus-clock snake paint whale flappy)
# Pomodoro length, in minutes. Defaults to the real 25; override for a quick
# end-to-end run: idf.py -DDADA_FOCUS_MINUTES=2 build
set(DADA_FOCUS_MINUTES 25 CACHE STRING "focus-clock session length in minutes")

string(FIND "${DADA_BOARD}" "-" sep)
if(sep LESS 0)
    message(FATAL_ERROR "DADA_BOARD must look like <vendor>-<board-dir>, got '${DADA_BOARD}'")
endif()
string(SUBSTRING "${DADA_BOARD}" 0 ${sep} DADA_VENDOR)
math(EXPR DADA_AFTER "${sep} + 1")
string(SUBSTRING "${DADA_BOARD}" ${DADA_AFTER} -1 DADA_BOARD_DIR)

set(DADA_HW_DIR "${CMAKE_CURRENT_LIST_DIR}/../hardware/${DADA_VENDOR}/${DADA_BOARD_DIR}")
if(NOT EXISTS "${DADA_HW_DIR}/board.json")
    message(FATAL_ERROR "board '${DADA_BOARD}' has no manifest at ${DADA_HW_DIR}/board.json")
endif()

# ---------------------------------------------------------------------------
# Build-time codegen (ADR 0002): the manifest becomes a C header. The device
# never parses JSON, and a malformed manifest fails the build instead of the
# board.
# ---------------------------------------------------------------------------
find_program(DADA_PYTHON NAMES python3 python)
if(NOT DADA_PYTHON)
    message(FATAL_ERROR "python not found; required to generate the board header")
endif()

# Fail capability mismatch before compiling or touching a device.
set(DADA_SELECTION_ARGS)
if(DADA_ALLOW_UNVERIFIED)
    list(APPEND DADA_SELECTION_ARGS --allow-unverified)
endif()
execute_process(
    COMMAND "${DADA_PYTHON}" "${CMAKE_CURRENT_LIST_DIR}/../tools/dada.py"
            check --board "${DADA_BOARD}" --app "${DADA_APP}" ${DADA_SELECTION_ARGS}
    RESULT_VARIABLE DADA_APP_RC
    ERROR_VARIABLE DADA_APP_ERR
)
if(NOT DADA_APP_RC EQUAL 0)
    message(FATAL_ERROR "app/board selection failed:\n${DADA_APP_ERR}")
endif()
set(DADA_APP_DIR "${CMAKE_CURRENT_LIST_DIR}/../apps/${DADA_APP}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${DADA_HW_DIR}/board.json" "${DADA_APP_DIR}/app.json")

set(DADA_BOARD_CONFIG_DIR "${CMAKE_BINARY_DIR}/board-config")
file(MAKE_DIRECTORY "${DADA_BOARD_CONFIG_DIR}")
execute_process(
    COMMAND "${DADA_PYTHON}" "${CMAKE_CURRENT_LIST_DIR}/../tools/gen_board_config.py"
            "${DADA_HW_DIR}/board.json"
            -o "${DADA_BOARD_CONFIG_DIR}/dada_board_config.h"
    RESULT_VARIABLE DADA_GEN_RC
    OUTPUT_VARIABLE DADA_GEN_OUT
    ERROR_VARIABLE DADA_GEN_ERR
)
if(NOT DADA_GEN_RC EQUAL 0)
    message(FATAL_ERROR "board header generation failed:\n${DADA_GEN_ERR}")
endif()
message(STATUS "dada: board=${DADA_BOARD} header=${DADA_BOARD_CONFIG_DIR}/dada_board_config.h")

# Keep the initial integration target explicit; add other boards deliberately.
if(DEFINED IDF_TARGET AND NOT IDF_TARGET STREQUAL "esp32c3")
    message(FATAL_ERROR "This bootstrap supports esp32c3 only.")
endif()
set(IDF_TARGET "esp32c3")

# Components live at the repository root, not inside firmware/.
# contracts/ is exposed as the `dada_contracts` component through a thin wrapper
# in components/dada_contracts (component names follow directory names).
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../components"
    "${CMAKE_CURRENT_LIST_DIR}/../apps/app_support"
    "${DADA_HW_DIR}/board"
    "${DADA_APP_DIR}"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dada VERSION 0.1.0)

if(NOT IDF_VERSION_MAJOR EQUAL 5 OR
   NOT IDF_VERSION_MINOR EQUAL 4 OR
   NOT IDF_VERSION_PATCH EQUAL 2)
    message(FATAL_ERROR "Use the repository baseline ESP-IDF v5.4.2.")
endif()
