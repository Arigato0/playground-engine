#pragma once

// a filesystem monitor that can monitor many paths and is much more performant than the lightweight file monitor

#include <vector>
#include <string_view>
#include <application/platform/fs_events.hpp>
#include "events/signal.hpp"

namespace pge
{
	class LinuxFsMonitor
	{
	public:
		using Callback = Signal<void(int)>;

		bool init();
		bool add_watch(std::string_view path, FS_EVENTS events, Callback callback);
		bool remove_watch(std::string_view path);

	private:
		int m_fd;
		std::vector<Callback> m_callbacks;
	};
}