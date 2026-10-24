cmake_minimum_required(VERSION 3.16)

# Keep the initial integration target explicit; add other boards deliberately.
if(DEFINED IDF_TARGET AND NOT IDF_TARGET STREQUAL "esp32c3")
    message(FATAL_ERROR "This bootstrap supports esp32c3 only.")
endif()
set(IDF_TARGET "esp32c3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(dada VERSION 0.1.0)

if(NOT IDF_VERSION_MAJOR EQUAL 5 OR
   NOT IDF_VERSION_MINOR EQUAL 4 OR
   NOT IDF_VERSION_PATCH EQUAL 2)
    message(FATAL_ERROR "Use the repository baseline ESP-IDF v5.4.2.")
endif()
