cmake_minimum_required(VERSION 3.20)
project(Engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX- /MP)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK from https://vulkan.lunarg.com/")
endif()

message(STATUS "Vulkan found: ${Vulkan_VERSION}")

find_package(SDL3 CONFIG REQUIRED)

find_program(GLSLC_EXECUTABLE glslc)
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found. Please install Vulkan SDK and ensure glslc is in PATH")
endif()

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
)

FetchContent_Declare(
    ImGuiFileDialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG master
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(imgui ImGuiFileDialog glm)

set(SHADER_COMPILED_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders_compiled)
file(MAKE_DIRECTORY ${SHADER_COMPILED_DIR})

function(compile_shader SHADER_SRC SHADER_OUT)
    add_custom_command(
        OUTPUT ${SHADER_OUT}
        DEPENDS ${SHADER_SRC}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SRC} -o ${SHADER_OUT}
        COMMENT "Compiling shader: ${SHADER_SRC} -> ${SHADER_OUT}"
    )
endfunction()

set(VERTEX_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shaders/default.vert)
set(FRAGMENT_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shaders/default.frag)

set(VERTEX_SHADER_OUT ${SHADER_COMPILED_DIR}/default.vert.spv)
set(FRAGMENT_SHADER_OUT ${SHADER_COMPILED_DIR}/default.frag.spv)

compile_shader(${VERTEX_SHADER_SRC} ${VERTEX_SHADER_OUT})
compile_shader(${FRAGMENT_SHADER_SRC} ${FRAGMENT_SHADER_OUT})

set(ENGINE_SOURCES
    src/main.cpp
    src/core/Engine.cpp
    src/core/SceneObject.cpp
    src/core/Timer.cpp
    src/core/Window.cpp
    src/core/WindowSettings.cpp
    src/renderer/Camera.cpp
    src/renderer/Material.cpp
    src/renderer/Renderer.cpp
    src/renderer/Scene.cpp
    src/resources/Mesh.cpp
    src/resources/Model.cpp
    src/resources/Shader.cpp
    src/resources/Texture.cpp
    src/ui/EditorUI.cpp
    src/ui/ImGuiLayer.cpp
    src/utils/File.cpp
    src/utils/StbImage.cpp
    src/vulkan/Buffer.cpp
    src/vulkan/CommandBuffer.cpp
    src/vulkan/Descriptors.cpp
    src/vulkan/Device.cpp
    src/vulkan/FrameBuffer.cpp
    src/vulkan/Memory.cpp
    src/vulkan/Pipeline.cpp
    src/vulkan/RenderPass.cpp
    src/vulkan/Swapchain.cpp
    src/vulkan/Sync.cpp
)

add_executable(Engine ${ENGINE_SOURCES})

target_link_libraries(Engine
    PRIVATE
    Vulkan::Vulkan
    SDL3::SDL3
    imgui
    ImGuiFileDialog
    glm
)

target_include_directories(Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader
    ${CMAKE_CURRENT_SOURCE_DIR}/external/vma
    ${SHADER_COMPILED_DIR}
)

add_custom_target(CompileShaders DEPENDS ${VERTEX_SHADER_OUT} ${FRAGMENT_SHADER_OUT})
add_dependencies(Engine CompileShaders)

if(WIN32)
    set_target_properties(Engine PROPERTIES WIN32_EXECUTABLE OFF)
    target_compile_definitions(Engine PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(Engine PRIVATE dl pthread)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/models)
    add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/models
            $<TARGET_FILE_DIR:Engine>/models
        COMMENT "Copying models to output directory"
    )
else()
    message(STATUS "models directory not found, skipping copy")
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/textures)
    add_custom_command(TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/textures
            $<TARGET_FILE_DIR:Engine>/textures
        COMMENT "Copying textures to output directory"
    )
else()
    message(STATUS "textures directory not found, skipping copy")
endif()