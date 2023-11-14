#pragma once

#include <type_traits>

namespace util
{
	template<class T>
	concept IsIterable = requires(T t)
	{
		t.begin();
		t.end();
	};

    template<class T>
    concept IsFunction = requires(T)
    {
        std::is_function_v<T>;
    };
}
