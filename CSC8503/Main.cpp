#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

#include "StateMachineMenu.h"
using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

bool istrue;
float timecount =0;
Menu* menu = nullptr;
int main() 
{
	
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720,false);

	if (!w->HasInitialised()) {
		return -1;
	}	

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();
	w->GetTimer().GetTimeDeltaSeconds();
	menu = new Menu();
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) 
	{
		timecount++;
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		
		menu->UpdateMenu(dt);
		if (dt > 0.1f)
		{
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}
		if (timecount <= 2) 
		{
			Debug::Print("CSC 8503 Coursework", Vector2(30, 30));
			Debug::Print("Level 1 - Press 1", Vector2(30, 40));
			Debug::Print("Made by Tianfeng Ma", Vector2(30, 50));
			Debug::Print("Exit - Press ESC", Vector2(30, 60));
			g->UpdateGame(dt);
		}
		if(Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
		{
			
			istrue = !istrue;
			menu->isStart = !menu->isStart;
		}
		if (istrue == true) 
		{
			g->UpdateGame(dt);
			
		}
		
	}
	Window::DestroyGameWindow();
}