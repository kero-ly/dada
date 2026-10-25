idf_component_register(
    SRCS "display_st7789.c"
    INCLUDE_DIRS "include"
    REQUIRES dada_contracts esp_lcd driver
)
