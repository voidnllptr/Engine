#include "Window.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>

void Window::SDLWindowDeleter::operator()(SDL_Window* window) const {
    if (window) {
        SDL_DestroyWindow(window);
    }
}

Window::Window(VkInstance instance, SDL_Window* existingWindow)
    : m_window(existingWindow)
    , m_vkInstance(instance)
    , m_vkSurface(VK_NULL_HANDLE)
    , m_shouldClose(false)
    , m_resized(false)
    , m_minimized(false)
    , m_hasFocus(true) {
    createVkSurface();
}

Window::~Window() {

}

SDL_Window* Window::getHandle() const {
    return m_window.get();
}

VkSurfaceKHR Window::getSurface() const {
    return m_vkSurface;
}

bool Window::hasSurface() const {
    return m_vkSurface != VK_NULL_HANDLE;
}

int Window::getWidth() const {
    int w, h;
    SDL_GetWindowSize(m_window.get(), &w, &h);
    return w;
}

int Window::getHeight() const {
    int w, h;
    SDL_GetWindowSize(m_window.get(), &w, &h);
    return h;
}

void Window::getSize(int& width, int& height) const {
    SDL_GetWindowSize(m_window.get(), &width, &height);
}

bool Window::shouldClose() const {
    return m_shouldClose;
}

bool Window::wasResized() const {
    return m_resized;
}

bool Window::isMinimized() const {
    return m_minimized;
}

bool Window::hasFocus() const {
    return m_hasFocus;
}

void Window::resetResizedFlag() {
    m_resized = false;
}

void Window::setShouldClose(bool close) {
    m_shouldClose = close;
}

void Window::createVkSurface() {
    if (m_vkSurface != VK_NULL_HANDLE) {
        return;
    }

    if (!SDL_Vulkan_CreateSurface(m_window.get(), m_vkInstance, nullptr, &m_vkSurface)) {
        throw std::runtime_error("Failed to create Vulkan surface!");
    }
}

void Window::updateEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            m_shouldClose = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            m_resized = true;
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            m_minimized = true;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            m_minimized = false;
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            m_hasFocus = true;
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            m_hasFocus = false;
            break;
        }
    }
}