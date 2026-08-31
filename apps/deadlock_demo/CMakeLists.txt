add_executable(deadlock_demo
    main.c
)

target_link_libraries(deadlock_demo
    PRIVATE
    pthread
)