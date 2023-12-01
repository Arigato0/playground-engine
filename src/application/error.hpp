#pragma once

#include "vulkan/vulkan.h"

#include <array>
#include <variant>
#include <string_view>
#include <cstdint>

namespace pge
{
	enum class ErrorCode : uint16_t
	{
		Ok,
        WindowCouldNotOpen,
        GraphicsSubsystemInitError,
        DrawingError,
		EngineNotInitialized,
	};

    template<class T>
	static inline ErrorCode embed_error(ErrorCode base, T other)
    {
        uint16_t value {};

        value = ((uint8_t)base) | ((uint8_t)other << 8);

        return (ErrorCode)value;
    }

    template<class T>
	T get_embedded_error(ErrorCode error)
    {
        return (T)((uint8_t)error >> 8);
    }

	std::string_view error_message(ErrorCode type);

	template<class T, class E>
	class Result
	{
	public:

		Result(E type) :
			m_value(type)
		{}

		Result(T value) :
			m_value(value)
		{}

		T& get()
		{
			return std::get<T>(m_value);
		}

		T& operator *()
		{
			return get();
		}

		T& operator->()
		{
			return get();
		}

		bool ok()
		{
			return m_value.index() == 0;
		}

		E& error()
		{
			return std::get<E>(m_value);
		}

	private:
		std::variant<T, E> m_value;
	};
}
