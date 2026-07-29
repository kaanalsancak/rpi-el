add_executable(process_signal_demo
    main.c
)

target_compile_definitions(process_signal_demo PRIVATE
    _POSIX_C_SOURCE=200809L
)

target_compile_options(process_signal_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
