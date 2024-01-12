#include "GameObject.h"
#include "NavigationGrid.h"

#include <GameWorld.h>


using namespace std;

namespace NCL
{
	namespace CSC8503 
	{
		class StateMachine;
		class Police : public GameObject {
		public:
			Police(std::string filePath, GameObject* goat, Vector3 position);
			~Police() { delete grid; delete stateMachine; }

			virtual void UpdatePosition(GameObject* goat, float dt);

			void SetOrigin(Vector3 position) { origin = position; }

			virtual void OnCollisionBegin(GameObject* otherObject);
			void drawPath();
			void IsStolen();
			bool isStolen;
		protected:

			void Chase(float dt);
			void Idle(float dt);
			
			StateMachine* stateMachine;
			float counter;
			NavigationGrid* grid;
			vector<Vector3> nodes;

			float speed = 300.0f;

			Vector3 origin;

			Vector3 targetPosition;

			NavigationPath path;

			Vector3 currentPosition;

			std::string filename;

			Vector3 lastKnownGoatPosition;

			GameObject* scaryGoat = nullptr;

			bool stateSwitch;

			

			Vector3 destPos;

			bool pathNotFound = true;

			int j = 0;
		};
	}
}

