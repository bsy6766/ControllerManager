#include <SDL.h>
#include "ControllerManager.h"
#include <iostream>

void onButtonPressed(ControllerID id, IO::XBOX_360::BUTTON button);
void onButtonReleased(ControllerID id, IO::XBOX_360::BUTTON button);
void onAxisMoved(ControllerID id, IO::XBOX_360::AXIS axis, const float value);
void onControllerConnected(ControllerID id);
void OnControllerDisconnected(ControllerID id);

static bool run = true;

int main(int argc, char **argv)
{
	auto cm = ControllerManager::getInstance();

	cm->onButtonPressed = onButtonPressed;
	cm->onButtonReleased = onButtonReleased;
	cm->onAxisMoved = onAxisMoved;
	cm->onControllerConnected = onControllerConnected;
	cm->onControllerDisconnect = OnControllerDisconnected;
	
	while (run)
	{
		cm->update();
	}

	system("pause");
	ControllerManager::deleteInstance();
	return -1;
}

void onButtonPressed(ControllerID id, IO::XBOX_360::BUTTON button)
{
	auto cm = ControllerManager::getInstance();

	if (cm->isButtonPressed(id, IO::XBOX_360::BUTTON::A))
	{
		std::cout << "A is currently Pressed with controller id: " << id << std::endl;
	}

	if (button == IO::XBOX_360::BUTTON::B)
	{
		std::cout << "B Pressed with controller id: " << id << std::endl;
	}

	if (button == IO::XBOX_360::BUTTON::START)
	{
		std::cout << "Breaking loop" << std::endl;
		run = false;
	}
}

void onButtonReleased(ControllerID id, IO::XBOX_360::BUTTON button)
{
	auto cm = ControllerManager::getInstance();

	if (cm->isButtonReleased(id, IO::XBOX_360::BUTTON::A))
	{
		std::cout << "A is currently Released with controller id: " << id << std::endl;
	}

	if (button == IO::XBOX_360::BUTTON::B)
	{
		std::cout << "B Released with controller id: " << id << std::endl;
	}

	if (button == IO::XBOX_360::BUTTON::X)
	{
		std::cout << "BRRRRRRRR" << std::endl;
		cm->playRumble(id, 300, 500);
	}
}

void onAxisMoved(ControllerID id, IO::XBOX_360::AXIS axis, const float value)
{
	if (axis == IO::XBOX_360::AXIS::LT)
	{
		std::cout << "Left Trigger axis moved with value: " << value << std::endl;
	}

	if (axis == IO::XBOX_360::AXIS::L_AXIS_X)
	{
		std::cout << "Left analog stick is moving within X axis with value: " << value << std::endl;
	}

	if (axis == IO::XBOX_360::AXIS::R_AXIS_Y)
	{
		std::cout << "Right analog stick is moving within Y axis with value: " << value << std::endl;
	}
}

void onControllerConnected(ControllerID id)
{
	std::cout << "Controller added with id: " << id << std::endl;
}

void OnControllerDisconnected(ControllerID id)
{
	std::cout << "Controller removed with id: " << id << std::endl;
}
