idf_component_register(SRCS 
        "storage.c" 
         "font_workbench_30num.c"
        "font_terminus_12.c"
        "font_micro_30num.c"
        "font.c"

                    INCLUDE_DIRS "."
                    PRIV_REQUIRES spiffs lvgl
                    )
