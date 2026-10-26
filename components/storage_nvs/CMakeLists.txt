idf_component_register(
    SRCS "storage_nvs.c"
    INCLUDE_DIRS "include"
    REQUIRES dada_contracts
    PRIV_REQUIRES nvs_flash
)
