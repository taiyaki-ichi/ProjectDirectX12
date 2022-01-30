#pragma once
#include<stdexcept>
#include<sstream>

namespace pdx12
{
	inline void throw_exception(char const* fileName, int line, char const* func, char const* str)
	{
		std::stringstream ss{};
		ss << fileName << " , " << line << " , " << func << " : " << str << "\n";
		throw std::runtime_error{ ss.str() };
	}

#define THROW_EXCEPTION(s)	throw_exception(__FILE__,__LINE__,__func__,s);
}