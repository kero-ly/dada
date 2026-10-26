idf_component_register(
    SRCS "board.c"
    INCLUDE_DIRS "include" "${CMAKE_BINARY_DIR}/board-config"
    REQUIRES dada_contracts display_st7789 touch_cst836u
    PRIV_REQUIRES esp_timer
)
