#pragma once

#include "fmt.hpp"

#include <ctime>
#include <cstdint>
#include <cstdio>

class Logger
{
public:
	static constexpr uint8_t Disabled = 0x1;
	static constexpr uint8_t Silent   = 0x2;

	static void set_outfile(std::string_view path)
	{
		if (m_file)
		{
			fclose(m_file);
		}

		m_file = fopen(path.data(), "a");
	}

	static void disable_file()
	{
		if (m_file == nullptr)
		{
			return;
		}

		fclose(m_file);

		m_file = nullptr;
	}

	static void set_flags(uint8_t flags)
	{
		m_flags = flags;
	}

	template<typename... A>
    static void info(std::string_view fmt, A&&... a)
    {
		do_log("INFO", stdout, fmt, a...);
    }

	template<typename... A>
	static void warn(std::string_view fmt, A&&... a)
    {
		do_log("WARN", stdout, fmt, a...);
    }

	template<typename... A>
	static void fatal(std::string_view fmt, A&&... a)
    {
		do_log("FATAL", stderr, fmt, a...);
		std::exit(-1);
    }

    template<typename... A>
	static void debug(std::string_view fmt, A&&... a)
    {
#if DEBUG
		do_log("DEBUG", stdout, fmt, a...);
#endif
    }

private:
	static FILE *m_file;
	static uint8_t m_flags;

	template<typename... A>
	static void do_log(std::string_view level, FILE *file, std::string_view fmt, A&&... a)
	{
		if (m_flags & Disabled)
		{
			return;
		}

		std::string message = fmt::format(fmt, std::forward<A>(a)...);

		log(fmt::format("[{}]  {} |{} {}\n", level, time_str(), message), file);
	}

    static void log(std::string_view data, FILE *file)
    {
		if (m_file)
		{
            fwrite(data.data(), 1, data.size(), m_file);
		}

		if (m_flags & Silent)
		{
			return;
		}

		fwrite(data.data(), 1, data.size(), file);
    }

    static std::string time_str()
    {
        time_t tt;
        tm *info;

        static constexpr int MAX_LEN = 18;

        std::string buff;
        buff.resize(MAX_LEN);

        time(&tt);

        info = localtime(&tt);

        strftime(buff.data(), MAX_LEN, "%x %X", info);

        return buff;
    }
};
