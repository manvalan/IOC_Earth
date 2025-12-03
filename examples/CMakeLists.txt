cmake_minimum_required(VERSION 3.10)

# Esempio base
add_executable(simple_map simple_map.cpp)
target_link_libraries(simple_map PRIVATE ioc_earth)

# Esempio con tracciato GPS
add_executable(gps_track gps_track.cpp)
target_link_libraries(gps_track PRIVATE ioc_earth)

# Esempio con mappa dell'Italia
add_executable(italy_map italy_map.cpp)
target_link_libraries(italy_map PRIVATE ioc_earth)

# Esempio con occultazione asteroidale
add_executable(occultation_map occultation_map.cpp)
target_link_libraries(occultation_map PRIVATE ioc_earth)

# Esempio con export HTML occultazione
add_executable(occultation_html occultation_html.cpp)
target_link_libraries(occultation_html PRIVATE ioc_earth)

# Esempio di utilizzo API per app chiamante
add_executable(api_usage_demo api_usage_demo.cpp)
target_link_libraries(api_usage_demo PRIVATE ioc_earth)

# Esempio con finder chart astronomica
add_executable(finder_chart finder_chart.cpp)
target_link_libraries(finder_chart PRIVATE ioc_earth)

# Installa gli esempi (opzionale)
install(TARGETS simple_map gps_track italy_map occultation_map occultation_html api_usage_demo finder_chart
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/ioc_earth_examples
)
