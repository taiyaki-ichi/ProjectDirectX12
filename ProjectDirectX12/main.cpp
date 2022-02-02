#include"window.hpp"
#include"device.hpp"
#include"command_manager.hpp"
#include"swap_chain.hpp"
#include<utility>
#include<iostream>

int main()
{
	constexpr std::size_t WINDOW_WIDTH = 500;
	constexpr std::size_t WINDOW_HEIGHT = 500;

	auto hwnd = pdx12::create_window(L"window", WINDOW_WIDTH, WINDOW_HEIGHT);

	auto device = pdx12::create_device();

	pdx12::command_manager<1> commandManager{};
	commandManager.initialize(device.get());

	auto swapChain = pdx12::create_swap_chain(commandManager.get_queue(), hwnd, DXGI_FORMAT_R8G8B8A8_UNORM, 2);

	while (pdx12::update_window())
	{
		commandManager.reset_list(0);






		commandManager.get_list()->Close();
		commandManager.excute();
		commandManager.signal();

		commandManager.wait(0);
	}

	return 0;
}