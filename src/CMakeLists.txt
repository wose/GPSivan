set(SRC
        encoder.cpp
        glgui.cpp
        gps_com.cpp
        main.cpp
)
add_executable(gpsivan ${SRC})
target_link_libraries(gpsivan ${GPSD_LIBRARY} ${PNG_LIBRARY}
        ${KAZMATH_LIBRARIES} ${EGL_LIBRARY} ${OPENGLES2_LIBRARY} bcm_host m WiringPi)

