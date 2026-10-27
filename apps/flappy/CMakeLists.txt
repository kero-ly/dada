idf_component_register(
    SRCS "flappy_app.c" "flappy.c" "render.c"
    INCLUDE_DIRS "."
    REQUIRES dada_contracts app_support
)
