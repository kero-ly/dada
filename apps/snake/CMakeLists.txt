idf_component_register(
    SRCS "snake_app.c" "snake.c" "render.c"
    INCLUDE_DIRS "."
    REQUIRES dada_contracts app_support
)
