add_executable(starvation_demo
    main.c
)

target_compile_definitions(starvation_demo
    PRIVATE
    _POSIX_C_SOURCE=200809L
)

target_link_libraries(starvation_demo
    PRIVATE
    pthread
)