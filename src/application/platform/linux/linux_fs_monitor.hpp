#pragma once

#include <vector>
#include <sys/inotify.h>
#include <string_view>
#include <data/hash_table.hpp>
#include <common_util/misc.hpp>
#include "events/signal.hpp"

namespace pge
{
	class LinuxFsMonitor
	{
	public:
		using Callback = std::function<void(int, std::string_view path)>;

		LinuxFsMonitor();
		~LinuxFsMonitor();

		int add_watch(std::string_view path, int events, Callback callback);
		bool remove_watch(int wd) const;
		uint32_t poll();

	private:
		int m_fd;

		struct Monitors
		{
			int events;
			Callback callback;
		};

		std::vector<Monitors> m_monitors;
		std::string m_event_buffer;
	};
}