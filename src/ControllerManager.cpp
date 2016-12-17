#include "ControllerManager.h"
#include <iostream>
#include <string>

using namespace std;

ControllerManager* ControllerManager::instance = nullptr;

Controller::Controller(SDL_GameController* controller, 
	SDL_Haptic* haptic, 
	const std::string& name, 
	const ControllerID id, 
	const int buttonCount,
	const int axisCount) 
	: controller(controller),
	haptic(haptic),
	name(name), 
	id(id), 
	buttonCount(buttonCount), 
	axisCount(axisCount),
	hapticEnabled(false),
	hapticModifier(1.0f),
	AXIS_MAX(10000), 
	AXIS_MIN(-10000)
{
	// Reset axis value to 0
	this->axisValueMap[IO::XBOX_360::AXIS::L_AXIS_X] = 0;
	this->axisValueMap[IO::XBOX_360::AXIS::L_AXIS_Y] = 0;
	this->axisValueMap[IO::XBOX_360::AXIS::R_AXIS_X] = 0;
	this->axisValueMap[IO::XBOX_360::AXIS::R_AXIS_Y] = 0;
	this->axisValueMap[IO::XBOX_360::AXIS::LT] = 0;
	this->axisValueMap[IO::XBOX_360::AXIS::RT] = 0;

	// Reset button state to false
	this->buttonStateMap[IO::XBOX_360::BUTTON::A] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::B] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::X] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::Y] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::BACK] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::START] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::L3] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::R3] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::LS] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::RS] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::DPAD_UP] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::DPAD_DOWN] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::DPAD_LEFT] = false;
	this->buttonStateMap[IO::XBOX_360::BUTTON::DPAD_RIGHT] = false;

	// Check if can rumble
	if (SDL_HapticRumbleSupported(haptic))
	{
		if (SDL_HapticRumbleInit(haptic))
		{
			hapticEnabled = true;
		}
		else
		{
			hapticEnabled = false;
		}
	}
}

Controller::~Controller()
{
	if (this->haptic != nullptr)
	{
		SDL_HapticClose(this->haptic);
	}

	if (this->controller != nullptr)
	{
		SDL_GameControllerClose(controller);
	}
}

void Controller::updateButtonState(IO::XBOX_360::BUTTON button, bool state)
{
	if (hasButton(button))
	{
		this->buttonStateMap[button] = state;
	}
}

void Controller::updateAxisValue(IO::XBOX_360::AXIS axis, float value)
{
	if (hasAxis(axis))
	{
		this->axisValueMap[axis] = value;
	}
}

const float Controller::getAxisValue(ControllerID rawValue, const float modifier)
{
	float value = 0;

	if (rawValue < AXIS_MIN)
	{
		value = static_cast<float>(rawValue) / SDL_AXIS_MIN_ABS_VALUE;
	}
	else if (rawValue > AXIS_MAX)
	{
		value = static_cast<float>(rawValue) / SDL_AXIS_MAX_ABS_VALUE;
	}
	else
	{
		value = 0;
	}

	value *= (modifier);

	return value;
}

void Controller::playRumble(const float strength, const Uint32 length)
{
	SDL_HapticRumbleStop(this->haptic);
	SDL_HapticRumblePlay(this->haptic, strength * hapticModifier, length);
}

const bool Controller::hasButton(IO::XBOX_360::BUTTON button)
{
	auto find_it = this->buttonStateMap.find(button);
	return find_it != this->buttonStateMap.end();
}

const bool Controller::hasAxis(IO::XBOX_360::AXIS axis)
{
	auto find_it = this->axisValueMap.find(axis);
	return find_it != this->axisValueMap.end();
}

ControllerManager::ControllerManager()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) < 0)
	{
		cout << "SDL_INIT_GAMECONTROLLER or SDL_INIT_HAPTIC could not initialize!SDL Error : " << SDL_GetError() << endl;
		active = false;
	}
	else
	{
		cout << "SDL is initilized" << endl;
		active = true;
	}
}

ControllerManager::~ControllerManager()
{
}

ControllerManager* ControllerManager::getInstance()
{
	if (instance == nullptr)
	{
		instance = new ControllerManager();
	}

	return instance;
}

void ControllerManager::deleteInstance()
{
	//check if instance is alive pointer
	if (instance != nullptr)
	{
		delete instance;
	}
}

void ControllerManager::update()
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_CONTROLLERDEVICEADDED:
		{
			addController(e.cdevice);
		}
		break;
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			removeController(e.cdevice);
		}
		break;
		case SDL_CONTROLLERBUTTONDOWN:
		{
			buttonPressed(e.cdevice.which, e.cbutton);
		}
		break;
		case SDL_CONTROLLERBUTTONUP:
		{
			buttonReleased(e.cdevice.which, e.cbutton);
		}
		break;
		case SDL_CONTROLLERAXISMOTION:
		{
			axisMoved(e.cdevice.which, e.caxis);
		}
		break;
		default:
			break;
		}
	}
}

void ControllerManager::addController(const SDL_ControllerDeviceEvent event)
{
	if (SDL_IsGameController(event.which)) {
		// Get as controller
		SDL_GameController* newController = SDL_GameControllerOpen(event.which);

		if (newController != nullptr) {
			// Convert to joystick
			SDL_Joystick *joy = SDL_GameControllerGetJoystick(newController);
			// Get instance id
			const ControllerID instanceID = SDL_JoystickInstanceID(joy);

			// Check duplication
			auto find_it = this->controllers.find(instanceID);
			if (find_it == this->controllers.end())
			{
				// Get number of button and axis on controller
				const int buttonCount = SDL_JoystickNumButtons(joy);
				const int axisCount = SDL_JoystickNumAxes(joy);
				// Get name
				std::string name = std::string(SDL_JoystickName(joy));

				SDL_Haptic* newHaptic = nullptr;

				bool hasHaptic = (SDL_JoystickIsHaptic(joy) == 1);
				if (hasHaptic)
				{
					newHaptic = SDL_HapticOpenFromJoystick(joy);
				}
				else
				{
					newHaptic = nullptr;
				}

				this->controllers[instanceID] = new Controller(newController, newHaptic, name, instanceID, buttonCount, axisCount);

				if (this->onControllerConnected)
				{
					onControllerConnected(instanceID);
				}
			}
			else
			{
				// Controller with same id already exists
				return;
			}
		}
		else
		{
			// New controller is invalid
			return;
		}
	}
}

void ControllerManager::removeController(const SDL_ControllerDeviceEvent event)
{
	auto find_it = this->controllers.find(event.which);

	if (find_it != this->controllers.end())
	{
		if (this->onControllerDisconnect)
		{
			onControllerDisconnect(event.which);
		}

		delete find_it->second;
	}

	this->controllers[event.which] = nullptr;
}

void ControllerManager::buttonPressed(ControllerID id, const SDL_ControllerButtonEvent event)
{
	if (event.state == SDL_PRESSED)
	{
		IO::XBOX_360::BUTTON buttonEnum = static_cast<IO::XBOX_360::BUTTON>(event.button);
		
		auto find_it = this->controllers.find(id);
		if (find_it != this->controllers.end())
		{
			(find_it->second)->updateButtonState(buttonEnum, true);
		}

		if (this->onButtonPressed)
		{
			this->onButtonPressed(id, buttonEnum);
		}
	}
}

void ControllerManager::buttonReleased(ControllerID id, const SDL_ControllerButtonEvent event)
{
	if (event.state == SDL_RELEASED)
	{
		IO::XBOX_360::BUTTON buttonEnum = static_cast<IO::XBOX_360::BUTTON>(event.button);

		auto find_it = this->controllers.find(id);
		if (find_it != this->controllers.end())
		{
			(find_it->second)->updateButtonState(buttonEnum, false);
		}

		if (this->onButtonPressed)
		{
			this->onButtonReleased(id, buttonEnum);
		}
	}
}

void ControllerManager::axisMoved(ControllerID id, const SDL_ControllerAxisEvent event)
{
	Controller* controller = findController(id);
	if (controller == nullptr) { return; }

	IO::XBOX_360::AXIS axis = static_cast<IO::XBOX_360::AXIS>(event.axis);
	ControllerID value = event.value;

	// Modifier determine whether axis is x or y. x = 1.0, y = -1.0
	float modifier = 0;
	float newValue = 0;

	if (axis == IO::XBOX_360::AXIS::L_AXIS_X || axis == IO::XBOX_360::AXIS::R_AXIS_X)
	{
		modifier = 1.0f;
		newValue = controller->getAxisValue(value, modifier);
	}
	else if (axis == IO::XBOX_360::AXIS::L_AXIS_Y || axis == IO::XBOX_360::AXIS::R_AXIS_Y)
	{
		modifier = -1.0f;
		newValue = controller->getAxisValue(value, modifier);
	}

	if (axis == IO::XBOX_360::AXIS::LT || axis == IO::XBOX_360::AXIS::RT)
	{
		newValue = controller->getAxisValue(value);
	}

	controller->updateAxisValue(axis, newValue);

	if (newValue != 0)
	{
		if (onAxisMoved)
		{
			onAxisMoved(id, axis, newValue);
		}
	}
}

const Sint16 ControllerManager::getMinAxisValue(ControllerID id)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		return controller->AXIS_MIN;
	}
	else
	{
		return 0;
	}
}

void ControllerManager::setMinAxisValue(ControllerID id, Sint16 value)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		controller->AXIS_MIN = value;
	}
}

const Sint16 ControllerManager::getMaxAxisValue(ControllerID id)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		return controller->AXIS_MAX;
	}
	else
	{
		return 0;
	}
}

void ControllerManager::setMaxAxisValue(ControllerID id, Sint16 value)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		controller->AXIS_MAX = value;
	}
}

Controller* ControllerManager::findController(ControllerID id)
{
	auto find_it = this->controllers.find(id);
	if (find_it != this->controllers.end())
	{
		return (find_it->second);
	}
	else
	{
		return nullptr;
	}
}

const float ControllerManager::getHapticModifier(ControllerID id)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		return controller->hapticModifier;
	}

	return 0.0f;
}

void ControllerManager::setHapticModifier(ControllerID id, float modifier)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		if (modifier < 0)
		{
			modifier = 0;
		}

		if (modifier > 2.0f)
		{
			modifier = 2.0f;
		}

		controller->hapticModifier = modifier;
	}
}

void ControllerManager::playRumble(ControllerID id, float strength, Uint32 length)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		if (strength < 0)
		{
			strength = 0;
		}

		if (length < 0)
		{
			length = 0;
		}
		controller->playRumble(strength, length);
	}
}

const bool ControllerManager::isButtonPressed(ControllerID id, IO::XBOX_360::BUTTON button)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		if (controller->hasButton(button))
		{
			return controller->buttonStateMap[button] == true;
		}
	}

	return false;
}

const bool ControllerManager::isButtonReleased(ControllerID id, IO::XBOX_360::BUTTON button)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		if (controller->hasButton(button))
		{
			return controller->buttonStateMap[button] == false;
		}
	}

	return false;
}

const bool ControllerManager::isAxisMoved(ControllerID id, IO::XBOX_360::AXIS axis)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		if (controller->hasAxis(axis))
		{
			return controller->axisValueMap[axis] != 0;
		}
	}

	return false;
}

const bool ControllerManager::hasHaptic(ControllerID id)
{
	Controller* controller = findController(id);
	if (controller != nullptr)
	{
		return (controller->haptic != nullptr);
	}
	else
	{
		return false;
	}
}