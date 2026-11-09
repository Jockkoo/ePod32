idf_component_register(SRCS "placeholder.c" "inputs.c" "ui_backend.c" "ui.c" "main.c" "button.c" "ui.c" "ui_backend.c"
    INCLUDE_DIRS ".")
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)