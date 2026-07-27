add_executable(process_zombie_demo
    main.c
)

target_compile_options(process_zombie_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
