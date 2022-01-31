#pragma once
#include<stdexcept>
#include<sstream>
#include<memory>

namespace pdx12
{
	inline void throw_exception(char const* fileName, int line, char const* func, char const* str)
	{
		std::stringstream ss{};
		ss << fileName << " , " << line << " , " << func << " : " << str << "\n";
		throw std::runtime_error{ ss.str() };
	}

#define THROW_PDX12_EXCEPTION(s)	throw_exception(__FILE__,__LINE__,__func__,s);



	template<typename T>
	struct release_deleter {
		void operator()(T* ptr) {
			ptr->Release();
		}
	};

	template<typename T>
	using release_unique_ptr = std::unique_ptr<T, release_deleter<T>>;

}