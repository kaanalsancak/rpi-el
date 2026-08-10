find_package(Threads REQUIRED)

add_executable(condition_variable_demo
    condition_variable_demo.c
)

target_compile_options(condition_variable_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(condition_variable_demo PRIVATE
    Threads::Threads
)
