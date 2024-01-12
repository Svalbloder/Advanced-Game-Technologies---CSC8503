#pragma once
#include "GameObject.h"
#include "NavigationGrid.h"
#include <GameWorld.h>
#include "BehaviourSequence.h"

using namespace std;

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class SuperAgent : public GameObject {
		public:
			SuperAgent(const std::string& objectName,std::string filePath, GameWorld* world, GameObject* goat, Vector3 position);
			~SuperAgent() { delete grid; }

			virtual void UpdatePosition(GameObject* goat, GameObject* buttom,float dt);

			void SetOrigin(Vector3 position) { origin = position; }

			virtual void OnCollisionBegin(GameObject* otherObject);
			bool GoatCaught = false;
			bool IsLock = false;
			bool IsDoneLock = false;

		protected:

			NavigationGrid* grid;
			vector<Vector3> nodes;

			NavigationPath path;


			float speed = 400.0f;

			Vector3 origin;

			Vector3 targetPosition;

			GameObject* scaryGoat = nullptr;
			GameObject* AlarmBullom = nullptr;
			bool stateSwitch;

			BehaviourSequence* rootSequence;

			Vector3 nextDest;

			void drawPath();

			GameWorld* world;

		};
	}
}
