idf_component_register(
    SRCS "focus_clock.c" "clock.c" "render.c"
    INCLUDE_DIRS "."
    REQUIRES dada_contracts
)

if(NOT DADA_FOCUS_MINUTES)
    set(DADA_FOCUS_MINUTES 25)
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    DADA_FOCUS_MINUTES=${DADA_FOCUS_MINUTES})
