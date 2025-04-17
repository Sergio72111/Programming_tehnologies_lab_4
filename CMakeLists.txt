cmake_minimum_required(VERSION 3.10)

# Название проекта
project(ElectricDevices)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавление исполняемого файла
add_executable(ElectricDevices main.cpp)
