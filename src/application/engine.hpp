#pragma once

#include "../game/entity_manager.hpp"
#include "glfw_window.hpp"
#include "error.hpp"
#include "statistics.hpp"
#include "../graphics/renderer_interface.hpp"
#include "../graphics/vulkan/vulkan_manager.hpp"
#include "window.hpp"
#include "../data/asset_manager.hpp"

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

		inline static WINDOW_T		window;
		inline static EntityManager	entity_manager;
		inline static IRenderer	   *renderer;
		inline static Statistics	statistics;
		inline static AssetManager	asset_manager;
		inline static float			time_scale = 1;
	private:
		inline static bool m_initialized = false;
        static void set_graphics_api(GraphicsApi api);
	};
}

