add_executable(process_wait_demo
    main.c
)

target_compile_options(process_wait_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
