idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES board dada_contracts log ${DADA_APP}
)
target_compile_definitions(${COMPONENT_LIB} PRIVATE DADA_APP_ID="${DADA_APP}")
