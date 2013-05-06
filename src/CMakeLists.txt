set(SRC
        gps_com.cpp
        main.cpp
)
add_executable(gpsivan ${SRC})
target_link_libraries(gpsivan ${GPSD_LIBRARY})
