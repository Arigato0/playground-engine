#pragma once

#if defined(linux)
	#include "linux/linux_fs_monitor.hpp"
#endif

namespace pge
{
#if defined(linux)
	using FsMonitor = pge::LinuxFsMonitor;
#else
#error file monitor not implemented for this platform
#endif
}
