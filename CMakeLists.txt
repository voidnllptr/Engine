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

add_library(ImGuiFileDialog STATIC
    external/imguifiledialog/ImGuiFileDialog.cpp
)
target_include_directories(ImGuiFileDialog PUBLIC external/imguifiledialog)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE external/stb)

add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE external/tinyobjloader)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE external/vma)

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
    core/Engine.cpp
    core/SceneObject.cpp
    core/Timer.cpp
    core/Window.cpp
    core/WindowSettings.cpp
    renderer/Camera.cpp
    renderer/Material.cpp
    renderer/Renderer.cpp
    renderer/Scene.cpp
    resources/Mesh.cpp
    resources/Model.cpp
    resources/Shader.cpp
    resources/Texture.cpp
    ui/EditorUI.cpp
    ui/ImGuiLayer.cpp
    utils/File.cpp
    utils/StbImage.cpp
    vulkan/Buffer.cpp
    vulkan/CommandBuffer.cpp
    vulkan/Descriptors.cpp
    vulkan/Device.cpp
    vulkan/FrameBuffer.cpp
    vulkan/Memory.cpp
    vulkan/Pipeline.cpp
    vulkan/RenderPass.cpp
    vulkan/Swapchain.cpp
    vulkan/Sync.cpp
    main.cpp
)

add_executable(Engine ${ENGINE_SOURCES})

target_link_libraries(Engine
    PRIVATE
    Vulkan::Vulkan
    SDL3::SDL3
    imgui
    ImGuiFileDialog
    stb_image
    tinyobjloader
    vma
)

target_include_directories(Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan
    external/glm
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