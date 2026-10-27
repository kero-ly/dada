idf_component_register(
    SRCS "snake_app.c" "snake.c" "render.c" "best_score.c"
    INCLUDE_DIRS "."
    REQUIRES dada_contracts app_support
)
