add_executable(
    mutex_counter_demo
    mutex_counter_demo.c
)

target_compile_options(
    mutex_counter_demo
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

find_package(Threads REQUIRED)

target_link_libraries(
    mutex_counter_demo
    PRIVATE
    Threads::Threads
)
