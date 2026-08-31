
add_executable(livelock_demo
    main.c
)

target_link_libraries(livelock_demo
    PRIVATE
    pthread
)
