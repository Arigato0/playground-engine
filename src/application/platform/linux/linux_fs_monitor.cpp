#pragma once

#include <sys/inotify.h>
#include <unistd.h>
#include "linux_fs_monitor.hpp"

pge::LinuxFsMonitor::LinuxFsMonitor()
{
	m_fd = inotify_init1(0);
}

pge::LinuxFsMonitor::~LinuxFsMonitor()
{
	close(m_fd);
}

bool pge::LinuxFsMonitor::add_watch(std::string_view path, FS_EVENTS events, Callback callback)
{
	int status = inotify_add_watch(m_fd, path.data(), events);

    if (status == -1)
    {
        return false;
    }

//	m_watch_events = events;

	return true;
}
