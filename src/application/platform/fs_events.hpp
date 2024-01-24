#pragma once

namespace pge
{
	enum FS_EVENTS
	{
#if defined(linux)
		FSE_MODIFY = 0x00000002,
		FSE_ACCESS = 0x00000001,
		FSE_CREATE = 0x00000100,
		FSE_DELETE = 0x00000200,
		FSE_ONE_SHOT = 0x80000000,
#endif
	};
}