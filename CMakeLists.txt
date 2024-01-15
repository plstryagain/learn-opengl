cmake_minimum_required(VERSION 3.10.0)
project(learn-opengl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if (WIN32)
    # include_directories($ENV{GLM_ROOT})
    include_directories($ENV{GLFW_ROOT}\\include)
    # message(STATUS "Using GLM: $ENV{GLM_ROOT} ")
    message(STATUS "Using GLFW: $ENV{GLFW_ROOT} ")

elseif (LINUX)

endif (WIN32)


include_directories(third-party/GLAD/include)
include_directories(third-party/stb_image)

add_subdirectory(src)
add_subdirectory(third-party/GLAD/src)
add_subdirectory(third-party/stb_image)


add_executable(
    ${PROJECT_NAME}
    $<TARGET_OBJECTS:main_obj>
    $<TARGET_OBJECTS:glad_obj>
    $<TARGET_OBJECTS:stb_image_obj>
)

if (WIN32)
    # add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    #     COMMAND ${CMAKE_SOURCE_DIR}/shaders/compile_win.bat
    # )
    target_link_directories(${PROJECT_NAME} PUBLIC "$ENV{GLFW_ROOT}/lib-vc2022")
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw3.lib -lopengl32)
elseif (LINUX)
    # add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    #     COMMAND ${CMAKE_SOURCE_DIR}/shaders/compile_linux.sh
    # )
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw GL)
endif ()