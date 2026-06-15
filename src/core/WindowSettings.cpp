#include "WindowSettings.hpp"

namespace Core {

    WindowSettings::WindowSettings()
        : pos_x(SDL_WINDOWPOS_CENTERED)
        , pos_y(SDL_WINDOWPOS_CENTERED)
    {

        Utils::IniStuff ini("config.ini");

        std::optional<std::string> titleOpt = ini.getValue("window.title");
        title = titleOpt.has_value() && !titleOpt->empty() ? *titleOpt : "VulkanEngine";

        std::optional<int> widthOpt = ini.getInt("window.width");
        width = widthOpt.has_value() && *widthOpt >= 640 && *widthOpt <= 3840 ? *widthOpt : 1280;

        std::optional<int> heightOpt = ini.getInt("window.height");
        height = heightOpt.has_value() && *heightOpt >= 360 && *heightOpt <= 2160 ? *heightOpt : 720;

        std::optional<bool> resizableOpt = ini.getBool("window.resizable");
        resizable = resizableOpt.has_value() ? *resizableOpt : true;

        std::optional<bool> fullscreenOpt = ini.getBool("window.fullscreen");
        fullscreen = fullscreenOpt.has_value() ? *fullscreenOpt : false;
    }
}