#include "interception.h"
#include "keys.h"
#include <windows.h>
#include <stdio.h>

//0,1을 쓰는 일반 키들과 다르게 방향키와 윈도우키는 2,3을 사용
typedef enum KeyState
{
	KEY_DOWN = INTERCEPTION_KEY_DOWN,//0
	KEY_UP = INTERCEPTION_KEY_UP,//1
	SPKEY_DOWN = 2,
	SPKEY_UP = 3,
};

typedef enum InputKey
{
	FORWARD_DOWN = 64,
	FORWARD_UP = 128,
	BACKWARD_DOWN = 256,
	BACKWARD_UP = 512,
};


void sendKeyEvent(Keys key, KeyState keyState, bool isSpecial, InterceptionContext context, InterceptionDevice device)
{
	InterceptionKeyStroke stroke = InterceptionKeyStroke();
	stroke.code = key;
	stroke.state = !isSpecial ? keyState : ((int)keyState + 2);//keystate+2 = spkeystate
	interception_send(context, device, (InterceptionStroke*)& stroke, 1);
}

inline KeyState convertToKeyState(InputKey input)
{
	return input == FORWARD_DOWN || input == BACKWARD_DOWN ? KEY_DOWN : KEY_UP;
}

int main()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	InterceptionContext context;
	InterceptionStroke stroke;
	InterceptionDevice device;
	InterceptionDevice keyboardDevice;

	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	context = interception_create_context();


	interception_set_filter(context, interception_is_keyboard,
		INTERCEPTION_FILTER_KEY_ALL);



	while (interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
	{
		if (interception_is_keyboard(device))
		{
			keyboardDevice = device;
			interception_set_filter(context, interception_is_keyboard, NULL);
			interception_set_filter(context, interception_is_mouse,
				FORWARD_DOWN | FORWARD_UP |
				BACKWARD_DOWN | BACKWARD_UP);
			interception_send(context, device, &stroke, 1);
			continue;
		}

		InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)& stroke;
		InputKey inputKey = (InputKey)mstroke.state;
		KeyState sendkeyState = convertToKeyState(inputKey);

		sendKeyEvent(Control, sendkeyState, false, context, keyboardDevice);
		sendKeyEvent(WindowsKey, sendkeyState, true, context, keyboardDevice);

		if (inputKey == FORWARD_DOWN || inputKey == FORWARD_UP)
		{
			sendKeyEvent(Right, sendkeyState, true, context, keyboardDevice);
		}
		else if (inputKey == BACKWARD_DOWN || inputKey == BACKWARD_UP)
		{
			sendKeyEvent(Left, sendkeyState, true, context, keyboardDevice);
		}
	}

	interception_destroy_context(context);

	return 0;
}
