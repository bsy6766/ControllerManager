#ifndef JOYSTICK_MANAGER_H
#define JOYSTICK_MANAGER_H

#include <unordered_map>
#include <functional>
#include <SDL.h>

#define MAX_JOYSTICK 4

class ControllerManager;

typedef Sint16 ControllerID;

namespace IO
{
	namespace XBOX_360
	{
		enum class BUTTON
		{
			A = 0,
			B,
			X,
			Y,
			BACK,
			NONE,
			START,
			L3,
			R3,
			LS,
			RS,
			DPAD_UP,
			DPAD_DOWN,
			DPAD_LEFT,
			DPAD_RIGHT
		};

		enum class AXIS
		{
			L_AXIS_X = 0,
			L_AXIS_Y,
			R_AXIS_X,
			R_AXIS_Y,
			LT,
			RT,
		};
	}
}

class Controller
{
private:
	// Manager class if friend
	friend ControllerManager;

	// Private constructor. User can't make their own controller instance.
	Controller(SDL_GameController* controller, SDL_Haptic* haptic, const std::string& name, const ControllerID id, const int buttonCount, const int axisCount);
	// Private destructor. Only manager can delete instance.
	~Controller();

	// SDL instances holder
	SDL_GameController* controller;
	SDL_Haptic* haptic;

	// Informations
	std::string name;
	ControllerID id;
	int buttonCount;
	int axisCount;
	// Note: SDL supports balls for controller, but we will focus on xbox360 for windows.
	bool hapticEnabled;

	// Haptic modifier. 1.0 by default(100%)
	float hapticModifier;

	// Maximum and minimum value that controller can make
	const float SDL_AXIS_MAX_ABS_VALUE = 32767;
	const float SDL_AXIS_MIN_ABS_VALUE = 32768;

	/**
	*	Most controller's axis aren't 'exactly' precise.
	*	Eventhough axises are idle, they are slightly tilted not centered prefectly which means that
	*	axis value aren't always 0 at idle. (The reason can be old controller or losen axises, etc)
	*	Therefore, we set a minimum and maxium value and determines whether if player really made an
	*	input physically or not by comparing input value with min/max setting.
	*
	*	By default, it's set to -10000, 10000 each
	*/
	Sint16 AXIS_MIN;
	Sint16 AXIS_MAX;

	// Button state
	std::unordered_map<IO::XBOX_360::BUTTON, bool> buttonStateMap;
	// Axis movement state
	std::unordered_map<IO::XBOX_360::AXIS, float> axisValueMap;

	// Update specific button's state
	void updateButtonState(IO::XBOX_360::BUTTON button, bool state);
	void updateAxisValue(IO::XBOX_360::AXIS axis, float value);

	/**
	*	Gets axis value based on each controller setting. 
	*/
	const float getAxisValue(ControllerID rawValue, const float modifier = 1.0f);

	// Play rumble effect
	void playRumble(const float strength, const Uint32 length);

	// Check if has button or axis
	const bool hasButton(IO::XBOX_360::BUTTON button);
	const bool hasAxis(IO::XBOX_360::AXIS axis);
public:
};

/**
*	@class ControllerManager
*
*	@brief Manages multiple controllers connections.
*
*	@note Only supports XBOX 360 controller
*
*	This class manages multiple xbox 360 controllers.
*	Here are list of features supported
*	- Connection detection
*	
*/
class ControllerManager
{
private:
	// Constructor
	ControllerManager();

	// Destructor
	~ControllerManager();

	//Singleton instance
	static ControllerManager* instance;

	// True if sdl is usable.
	bool active;

	// Store controller
	std::unordered_map<ControllerID/*SDL controller id*/, Controller*> controllers;

	/**
	*	Add controller to manager
	*/
	void addController(const SDL_ControllerDeviceEvent event);

	/**
	*	Remove controller from manager
	*/
	void removeController(const SDL_ControllerDeviceEvent event);

	/**
	*	Button pressed
	*	This function is called when SDL detects button press. 
	*	It stores down state for each button pressed.
	*	Also it calls onButtonPressed if has.
	*/
	void buttonPressed(ControllerID id, const SDL_ControllerButtonEvent event);

	/**
	*	Button released
	*	Same mechanism with buttonPressed function but just for released/up state
	*/
	void buttonReleased(ControllerID id, const SDL_ControllerButtonEvent event);

	/**
	*	Axis moved
	*	This function is called when SDL detects axis movement.
	*/
	void axisMoved(ControllerID id, const SDL_ControllerAxisEvent event);

	/**
	*	Finds controller by id.
	*	Nullptr if doesn't exists
	*/
	Controller* findController(ControllerID id);
public:
	// Get instance
	static ControllerManager* getInstance();

	// Destroy instance.
	static void deleteInstance();

	// Prevent copying or assigning instance
	ControllerManager(ControllerManager const&) = delete;
	void operator=(ControllerManager const&) = delete;

	const float SDL_AXIS_MAX_ABS_VALUE = 32767;
	const float SDL_AXIS_MIN_ABS_VALUE = 32768;

	/**
	*	Update manager.
	*	@note Call this whenever you want to update input. Recommended to call every frame or tick.
	*/
	void update();

	// Callback function when button is pressed
	static std::function<void(ControllerID id, IO::XBOX_360::BUTTON button)> onButtonPressed;

	// Callback function when button is released.
	static std::function<void(ControllerID id, IO::XBOX_360::BUTTON button)> onButtonReleased;

	// Callback function when axis moved.
	static std::function<void(ControllerID id, IO::XBOX_360::AXIS axis, const float value)> onAxisMoved;

	// Callback function when controller is connected
	static std::function<void(ControllerID id)> onControllerConnected;

	// Callback function when controller is disconnected
	static std::function<void(ControllerID id)> onControllerDisconnected;

	// Get/Set minimum axis value
	const Sint16 getMinAxisValue(ControllerID id);
	void setMinAxisValue(ControllerID id, Sint16 value);

	// Get/Set maximum axis value
	const Sint16 getMaxAxisValue(ControllerID id);
	void setMaxAxisValue(ControllerID id, Sint16 value);

	// Haptic modifier (Vibration power)
	const float getHapticModifier(ControllerID id);
	void setHapticModifier(ControllerID id, float modifier);

	// Check button state and axis value
	const bool isButtonPressed(ControllerID id, IO::XBOX_360::BUTTON button);
	const bool isButtonReleased(ControllerID id, IO::XBOX_360::BUTTON button);
	const bool isAxisMoved(ControllerID id, IO::XBOX_360::AXIS axis);

	// Check if has haptic
	const bool hasHaptic(ControllerID id);

	// Play rumble.
	void playRumble(ControllerID id, float strength, Uint32 length);
};

#endif