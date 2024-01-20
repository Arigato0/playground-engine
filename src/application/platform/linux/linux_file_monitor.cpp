#pragma once

#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>

#include "linux_file_monitor.hpp"

pge::LinuxFileMonitor::LinuxFileMonitor()
{
	m_fd = inotify_init1(0);
}

pge::LinuxFileMonitor::~LinuxFileMonitor()
{
	close(m_fd);
}

bool pge::LinuxFileMonitor::set_watch(std::string_view path, pge::FS_EVENTS events)
{
	int status = inotify_add_watch(m_fd, path.data(), events);

    if (status == -1)
    {
        return false;
    }

	m_watch_events = events;

	return true;
}

bool pge::LinuxFileMonitor::poll(uint32_t timeout)
{
	pollfd pfd
	{
		.fd = m_fd,
		.events = POLLIN
	};

	int result = ::poll(&pfd, 1, timeout);

	if (result == 0 || !(pfd.events & POLLIN))
	{
		return false;
	}

	inotify_event event {};

	read(m_fd, (char*)&event, sizeof(inotify_event));



	return event.mask == m_watch_events;
}
