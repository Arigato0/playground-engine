#include <sys/inotify.h>
#include <unistd.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <utility>
#include <application/log.hpp>
#include "linux_fs_monitor.hpp"

#define INITIAL_MONITOR_SIZE 1000

pge::LinuxFsMonitor::LinuxFsMonitor()
{
	m_fd = inotify_init1(IN_NONBLOCK);
	m_monitors.resize(INITIAL_MONITOR_SIZE);
}

pge::LinuxFsMonitor::~LinuxFsMonitor()
{
	close(m_fd);
}

int pge::LinuxFsMonitor::add_watch(std::string_view path, int events, Callback callback)
{
	int wd = inotify_add_watch(m_fd, path.data(), events);

    if (wd == -1)
    {
        return -1;
    }

	if (wd >= m_monitors.size())
	{
		m_monitors.resize(m_monitors.size() * 2);
	}

	m_monitors[wd] =
	{
		events,
		std::move(callback)
	};

	return wd;
}

bool pge::LinuxFsMonitor::remove_watch(int wd) const
{
	return inotify_rm_watch(m_fd, wd) == 0;
}

uint32_t pge::LinuxFsMonitor::poll()
{
	uint32_t bytes_to_read;

	ioctl(m_fd, FIONREAD, &bytes_to_read);

	if (bytes_to_read <= 0)
	{
		return 0;
	}

	if (bytes_to_read >= m_event_buffer.size())
	{
		m_event_buffer.resize(bytes_to_read);
	}

	auto bytes_read = read(m_fd, m_event_buffer.data(), bytes_to_read);
	int bytes_processed = 0;

	while (bytes_processed < bytes_read)
	{
		auto event = (inotify_event*)&m_event_buffer[bytes_processed];
		auto monitor = m_monitors[event->wd];

		if (event->mask & monitor.events)
		{
			monitor.callback(event->mask, std::string_view{event->name, event->len});
		}

		bytes_processed += sizeof(inotify_event) + event->len;
	}

	return bytes_processed / sizeof(inotify_event);
}
