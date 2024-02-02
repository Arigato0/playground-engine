#pragma once

#if defined(__linux__)
	#include "linux/linux_fs_monitor.hpp"
#endif

namespace pge
{
#if defined(__linux__)
	using FsMonitor = pge::LinuxFsMonitor;
#else
#error file monitor not implemented for this platform
#endif
}
