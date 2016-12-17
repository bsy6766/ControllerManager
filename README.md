# ControllerManager
Simple C++ SDL_Joystick and SDL_Haptic wrapper

## Note
I used SDL GameController and Haptic in SDL2.<br>
To use this code, you need to link SDL2 in to your project.

I only tested on Windows 10 but should work on other suppported OS by SDL2. 

## Background
This is a simple game controller (XBOX 360) wrapper written in C++ for Windows.<br>
The reason why I made this was because I was looking for library that supports crossplatform and forcefeedback(vibration/rumble/haptic). <br>
There were many options such as XInput, SDL, SFML, GLFW, but none of these supported crossplatform and forcefeedback except SDL.

## Features
Detects XBOX 360 controller connection, disconnection and input.
All controllers are tracked with SDL_Joystick's instance ID (number). 
You can set callback functions for connection, disconnection and input or simply check button state, etc to use controller

## Example
ControllerManager is Singleton class. Call getInstance() to get instance. FYI, it uses lazy initialization.<br>
Make sure you **call ControllerManager::DeleteInstance** to terminate and delete instance.<br>
Also look main.cpp for sample run. <br>
Any improvement or new features are welcomed. PR it :)
