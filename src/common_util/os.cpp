#include "os.hpp"


#define NIX_DELIM '/'
#define ANTI_NIX_DELIM '\\'

std::filesystem::path pge::make_sys_path(const std::filesystem::path &path)
{
	auto str = path.string();
	constexpr char delim =
#if defined(__linux__) || defined(_unix__)
'/';
#elif
	'\\';
#endif
	constexpr char anti_delim =
#if defined(__linux__) || defined(_unix__)
'\\';
#elif
	'/';
#endif
	for (auto &c : str)
	{
		if (c == anti_delim)
		{
			c = delim;
		}
	}

	return str;
}
