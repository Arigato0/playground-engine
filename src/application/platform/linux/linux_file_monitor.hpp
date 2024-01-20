#pragma once

#include <string_view>
#include "../fs_events.hpp"

namespace pge
{
	// lightweight class to monitor a single file or directory
	class LinuxFileMonitor
	{
	public:
		LinuxFileMonitor();
		~LinuxFileMonitor();

		// sets the watched directory or file to path
		bool set_watch(std::string_view path, FS_EVENTS events);
		// will check to see if the watched path triggered an event
		bool poll(uint32_t timeout);

	private:
		int m_fd;
		FS_EVENTS m_watch_events;
	};
}