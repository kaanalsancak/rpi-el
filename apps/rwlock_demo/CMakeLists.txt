add_executable(rwlock_demo
    main.c
)

target_link_libraries(rwlock_demo
    PRIVATE
    pthread
)

