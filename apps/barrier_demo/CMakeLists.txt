add_executable(barrier_demo
    main.c
)

target_link_libraries(barrier_demo
    PRIVATE
    pthread
)
