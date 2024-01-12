#include "../NCLCoreClasses/KeyboardMouseController.h"

#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"
#include "NavigationGrid.h"
#include "StateGameObject.h"
#include "Police.h"
#include "SuperAgent.h"

class Apple : public GameObject {
public:
	bool createBullet =false;
	bool IsCatch = false;
	bool IsDoneLock = false;
	Apple(GameWorld* world, std::string name = "") : GameObject(name) {
		this->world = world;
		triggerDelete = true;
	};
	void OnCollisionBegin(GameObject* otherObject) override {
		if (otherObject->GetName() == "Goat") 
		{
			world->RemoveGameObject(this, true);
			createBullet = true;	
			IsCatch = true;
			
		}
		if (otherObject->GetName() == "ATM")
		{
			world->RemoveGameObject(this, true);
			world->UpdateScore();
			
		}
	}
	
	GameWorld* world;
};
namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			void InitWorld();
			void InitialiseAssets();
		protected:
			

			void InitCamera();
			void UpdateKeys();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			void BridgeConstraintTest();
			void MovePlayerCharacter(float dt);
			void AddMazeToWorld();
			void CameraFollow();
			void AddGateToWorld(const Vector3& position, const Vector3& rotation);
			
			void CatchBullet();
			void Shoot();
			GameObject* AddAgentToWorld(const Vector3& position);
			void UpdateGameTime(float dt);
			vector<Vector3> mazeNodes;
			NavigationGrid* navGrid;
			Apple* CreateBullet();
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddFenceToWorld(const Vector3& position, const Vector3& scale, const Vector3& rotation, const float inverseMass);
		    GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddATMToWorld(const Vector3& position, Vector3 dimensions, float inverseMass);
			Apple* AddBonusToWorld(const Vector3& position);
			Apple* AddButtomToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;




#ifdef USEVULKAN
			GameTechVulkanRenderer* renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem* physics;
			GameWorld* world;

			KeyboardMouseController controller;

			bool useGravity;
			bool inSelectionMode;
			bool IschangeView;
			bool IsCatch;
			bool isLock;
			std::vector<GameObject*> maze;

			float		forceMagnitude;
			float count = 0;
			GameObject* selectionObject = nullptr;

			Mesh* capsuleMesh = nullptr;
			Mesh* cubeMesh = nullptr;
			Mesh* sphereMesh = nullptr;

			Texture* basicTex = nullptr;
			Shader* basicShader = nullptr;

			//Coursework Meshes
			Mesh* charMesh = nullptr;
			Mesh* enemyMesh = nullptr;
			Mesh* bonusMesh = nullptr;
			Police* police;
			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			GameObject* playerCharacter = nullptr;
			float Timer = 50;
			SuperAgent* Agent = nullptr;
			Apple* buttom = nullptr;
			Apple* apple = nullptr;
			Apple* bullet = nullptr;
			Vector3 bulletPos;
			Vector3 finalBulletPos;
			Vector3 lockedOffset = Vector3(0, 14, 20);
			
			bool lose = false;
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}
			float	yaw;
			GameObject* objClosest = nullptr;
		};
	}
}

