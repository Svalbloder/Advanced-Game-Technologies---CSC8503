#include "StateMachineMenu.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"
#include "PhysicsObject.h"

//part of this codes are from www.github.com
Menu::Menu() : GameObject()
{
	
	stateMachine = new StateMachine();

	State* MenuState = new State([&](float dt) -> void
		{
			this->MainMenu();
		}
	);
	State* GameState = new State([&](float dt) -> void
		{

			this->GameStart();
		}
	);

	

	stateMachine->AddState(MenuState);

	stateMachine->AddState(GameState);


	stateMachine->AddTransition(new StateTransition(MenuState, GameState,
		[&]() -> bool
		{
			if (isStart) {
				return true;
			}
			else {
				return false;
			}
		}
	));
	stateMachine->AddTransition(new StateTransition(GameState, MenuState,
		[&]() -> bool
		{
			if (!isStart) {
				return true;
			}
			else {
				return false;
			}
		}
	));


}
void Menu::UpdateMenu(float dt)
{
	stateMachine->Update(dt);
}

void Menu::GameStart()
{
	g->InitWorld();
	

}

void Menu::MainMenu()
{
	//do nothing
}
	
