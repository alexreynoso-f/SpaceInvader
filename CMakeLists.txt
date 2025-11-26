cmake_minimum_required(VERSION 3.16)
project(Galaga)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 🔗 Conectar vcpkg
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Alejandro/.vcpkg-clion/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")

# 📌 Ruta donde vcpkg instala SFML (necesario para CONFIG)
set(SFML_DIR "C:/Users/Alejandro/.vcpkg-clion/vcpkg/installed/x64-mingw-dynamic/share/sfml")

# 🔍 Buscar SFML moderno
find_package(SFML CONFIG REQUIRED COMPONENTS Graphics Window System Audio Network)

# 🗂️ Carpeta include/
include_directories(include)

# 🔍 Buscar todos los .cpp dentro de src/
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# 🏗️ Ejecutable
add_executable(Galaga
        main.cpp
        ${SRC_FILES}
        src/Player.cpp
        include/Player.h
        src/Bullet.cpp
        include/Bullet.h
        src/Enemy.cpp
        include/Enemy.h
        src/menu.cpp
        include/menu.h
        src/Formation.cpp
        include/Formation.h
        src/Shield.cpp
        include/Shield.h
        include/Shield.h
        src/Game.cpp
        include/Game.h
)

# 🎵 Ruta para acceder a assets en runtime (NO COMPILA, solo referencia)
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
message(STATUS "Carpeta de assets: ${ASSETS_DIR}")

# 🔗 SFML moderno (targets correctas)
target_link_libraries(Galaga PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
        SFML::Network
)
add_custom_command(TARGET Galaga PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Galaga>/assets
)
