find_package(Threads REQUIRED)

add_executable(thread_create_demo
    main.c
)

target_compile_definitions(thread_create_demo PRIVATE
    _GNU_SOURCE
)

target_compile_options(thread_create_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(thread_create_demo PRIVATE
    Threads::Threads
)
