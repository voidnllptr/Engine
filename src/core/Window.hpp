#pragma once

#include <vulkan/vulkan.h>
#include <memory>

struct SDL_Window;

class Window {
public:
    Window(VkInstance instance, SDL_Window* existingWindow);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    SDL_Window* getHandle() const;
    VkSurfaceKHR getSurface() const;
    bool hasSurface() const;

    int getWidth() const;
    int getHeight() const;
    void getSize(int& width, int& height) const;

    bool shouldClose() const;
    bool wasResized() const;
    bool isMinimized() const;
    bool hasFocus() const;

    void resetResizedFlag();
    void setShouldClose(bool close);
    void updateEvents();

private:
    void createVkSurface();

    struct SDLWindowDeleter {
        void operator()(SDL_Window* window) const;
    };

    std::unique_ptr<SDL_Window, SDLWindowDeleter> m_window;
    VkInstance m_vkInstance;
    VkSurfaceKHR m_vkSurface;

    bool m_shouldClose;
    bool m_resized;
    bool m_minimized;
    bool m_hasFocus;
};