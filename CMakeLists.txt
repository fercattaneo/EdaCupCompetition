
cmake_minimum_required(VERSION 3.0)

project("EDACUP")

set (CMAKE_CXX_STANDARD 11)

# From "Working with CMake" documentation:
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin" OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # AddressSanitizer (ASan)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # UndefinedBehaviorSanitizer (UBSan)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()

add_executable(main main.cpp MQTTClient2.cpp Players.cpp GameModel.cpp data.cpp Robot.cpp)

# Packages
find_package(raylib REQUIRED)
find_package(mosquitto REQUIRED)
find_package(OpenSSL REQUIRED)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(main PRIVATE ${RAYLIB_INCLUDE_DIRS} ${MOSQUITTO_INCLUDE_DIRS})
    target_link_libraries(main PRIVATE ${RAYLIB_LIBRARIES} ${MOSQUITTO_LIBRARIES} OpenSSL::SSL 
                          OpenSSL::Crypto)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(main PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    target_link_libraries(main PRIVATE ${raylib_LIBRARIES} ${MOSQUITTO_LIBRARIES} OpenSSL::SSL
                          OpenSSL::Crypto)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_include_directories(main PRIVATE ${raylib_INCLUDE_DIRS} ${MOSQUITTO_INCLUDE_DIRS})
    target_link_libraries(main PRIVATE ${raylib_LIBRARIES} ${MOSQUITTO_LIBRARIES} OpenSSL::SSL
                          OpenSSL::Crypto)
endif()

#Test
enable_testing()

add_executable(main_test main_test.cpp data.cpp Robot.cpp)

add_test(NAME test1 COMMAND main_test)

# Packages for test
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(main_test PRIVATE ${RAYLIB_INCLUDE_DIRS} ${MOSQUITTO_INCLUDE_DIRS})
    target_link_libraries(main_test PRIVATE ${RAYLIB_LIBRARIES} ${MOSQUITTO_LIBRARIES} OpenSSL::SSL 
                          OpenSSL::Crypto)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(main_test PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
    target_link_libraries(main_test PRIVATE ${raylib_LIBRARIES} ${MOSQUITTO_LIBRARIES} OpenSSL::SSL
                          OpenSSL::Crypto)
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_include_directories(main_test PRIVATE ${raylib_INCLUDE_DIRS} ${MOSQUITTO_INCLUDE_DIRS})
    target_link_libraries(main_test PRIVATE ${raylib_LIBRARIES} ${MOSQUITTO_LIBRARIES} OpenSSL::SSL
                          OpenSSL::Crypto)
endif()
