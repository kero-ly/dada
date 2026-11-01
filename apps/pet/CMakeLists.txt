idf_component_register(
    SRCS "pet_app.c" "pet.c"
    INCLUDE_DIRS "."
    REQUIRES dada_contracts app_support
)
