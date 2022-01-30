#include"window.hpp"
#include"device.hpp"
#include<utility>
#include<iostream>

int main()
{
	constexpr std::size_t WINDOW_WIDTH = 500;
	constexpr std::size_t WINDOW_HEIGHT = 500;

	auto hwnd = pdx12::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	try
	{
		auto device = pdx12::create_device();
	}
	catch (std::exception const& e)
	{
		std::cout << e.what() << std::endl;
	}

	while (pdx12::update_window())
	{

	}

	return 0;
}