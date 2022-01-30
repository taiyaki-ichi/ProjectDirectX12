#include"window.hpp"
#include<utility>

int main()
{
	constexpr std::size_t WINDOW_WIDTH = 500;
	constexpr std::size_t WINDOW_HEIGHT = 500;

	auto hwnd = pdx12::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	while (pdx12::update_window())
	{

	}

	return 0;
}