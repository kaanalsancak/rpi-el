add_executable(process_orphan_demo
    main.c
)

target_compile_definitions(process_orphan_demo PRIVATE
    _POSIX_C_SOURCE=200809L
)

target_compile_options(process_orphan_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
