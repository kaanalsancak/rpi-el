find_package(Threads REQUIRED)

add_executable(semaphore_demo
    semaphore_demo.c
)

target_compile_options(semaphore_demo PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(semaphore_demo PRIVATE
    Threads::Threads
)
