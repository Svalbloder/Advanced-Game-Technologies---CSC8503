#include "GameObject.h"
#include "NavigationGrid.h"
#include "TutorialGame.h"
#include <GameWorld.h>


using namespace std;

namespace NCL
{
	namespace CSC8503
	{
		class StateMachine;
		class Menu: public GameObject {
		public:
			
			Menu();
			~Menu() {delete stateMachine; }
			bool isStart =false;
			TutorialGame* g = new TutorialGame();
			void UpdateMenu(float dt);
		protected:
			
			StateMachine* stateMachine;

			std::string filename;

			bool stateSwitch;

			void MainMenu();

			void GameStart();
			
			
		};
	}
}
