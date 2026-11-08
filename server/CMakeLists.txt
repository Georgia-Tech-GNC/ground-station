
add_executable(ground_station_server)

# Add source files
target_sources(ground_station_server PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/database_writer.c
    ${CMAKE_CURRENT_LIST_DIR}/src/ring_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/src/web_server.c
    ${CMAKE_CURRENT_LIST_DIR}/src/packet_types.c
)

# Add include directory
target_include_directories(ground_station_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

# Add library sources
target_include_directories(ground_station_server PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib)
target_sources(ground_station_server PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lib/sqlite3.c
)
