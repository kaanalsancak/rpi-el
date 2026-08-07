find_package(Threads REQUIRED)

add_executable(thread_memory_demo
    thread_memory_demo.c
)

target_compile_options(thread_memory_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(thread_memory_demo PRIVATE
    Threads::Threads
)