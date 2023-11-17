#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "entity_manager.hpp"
#include "../game/ecs.hpp"
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

		static ErrorCode init(AppInfo info);

		static ErrorCode run();

		static void shutdown();

		static uint32_t get_fps()
		{
			return m_fps;
		}

		static WINDOW_T      window;
		static EntityManager entity_manager;
	private:
		static IGraphicsManager *m_graphics_manager;
		static double			 m_delta_time;
		static uint32_t			 m_fps;

        static void set_graphics_api(GraphicsApi api);
		static void draw_ui();
	};
}

