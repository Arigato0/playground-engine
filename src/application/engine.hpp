#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "../game/entity_manager.hpp"
#include "../game/ecs.hpp"
#include "glfw_window.hpp"
#include "error.hpp"
#include "statistics.hpp"
#include "../graphics/vulkan/vulkan_manager.hpp"
#include "window.hpp"

namespace pge
{
	class IRenderer;

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

		inline static WINDOW_T			window;
		inline static EntityManager		entity_manager;
		inline static IRenderer		   *renderer;
		inline static Statistics		statistics;
		inline static float				time_scale = 1;
	private:

        static void set_graphics_api(GraphicsApi api);
	};
}

