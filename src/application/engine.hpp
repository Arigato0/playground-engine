#pragma once

#include <vector>

#include "glfw_window.hpp"
#include "error.hpp"
#include "../graphics/vulkan/vulkan_manager.hpp"
#include "window.hpp"

namespace pge
{
	struct AppInfo
	{
		std::string_view title;
		glm::ivec2		 window_size;
        GraphicsApi      graphics_api;
	};

	// TODO write an observable object for the application options
	class Engine
	{
	public:

        Engine(AppInfo info) :
        m_info(info)
        {}

		ErrorCode init();

		ErrorCode run();

		uint32_t get_fps() const
		{
			return m_fps;
		}

		~Engine();

	private:
        AppInfo           m_info;
		WINDOW_T          m_window;
		IGraphicsManager *m_graphics_manager = nullptr;
		double m_delta_time;
		uint32_t m_fps;

        void set_graphics_api(GraphicsApi api);
		void draw_ui();
	};
}

