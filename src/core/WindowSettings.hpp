#pragma once

#include <iostream>
#include <optional>
#include <SDL3/SDL.h>
#include "../utils/File.hpp"

namespace Core {

	struct WindowSettings {

		std::string title;
		int width;
		int height;
		int pos_x;
		int pos_y;
		bool resizable;
		bool fullscreen;

		WindowSettings();
	}; 
}