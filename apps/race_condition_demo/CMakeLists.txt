add_executable(
    race_condition_demo
    race_condition_demo.c
)

target_compile_options(
    race_condition_demo
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

find_package(Threads REQUIRED)

target_link_libraries(
    race_condition_demo
    PRIVATE
    Threads::Threads
)
