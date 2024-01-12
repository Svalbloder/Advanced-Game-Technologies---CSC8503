#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "SuperAgent.h"
#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "Assets.h"
#include <fstream>

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() : controller(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse()) {
	world = new GameWorld();
#ifdef USEVULKAN
	renderer = new GameTechVulkanRenderer(*world);
	renderer->Init();
	renderer->InitStructures();
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;

	world->GetMainCamera().SetController(controller);

	controller.MapAxis(0, "Sidestep");
	controller.MapAxis(1, "UpDown");
	controller.MapAxis(2, "Forward");

	controller.MapAxis(3, "XLook");
	controller.MapAxis(4, "YLook");
	
	InitialiseAssets();
	
	
}

void TutorialGame::InitialiseAssets() 
{
	cubeMesh = renderer->LoadMesh("cube.msh");
	sphereMesh = renderer->LoadMesh("sphere.msh");
	charMesh = renderer->LoadMesh("goat.msh");
	enemyMesh = renderer->LoadMesh("Keeper.msh");
	bonusMesh = renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");

	basicTex = renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");
	
	InitCamera();
	InitWorld();
	
	
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) 
{
	UpdateGameTime(dt);
	
	int score = world->GetScore();

	if (this->world->GetScore() >= 3)
	{
		Debug::Print("You have won the game! Hooray", Vector2(5, 45), Debug::YELLOW);
		
	}
	if (lose)
	{
		Debug::Print("You have lose the game! Oooops", Vector2(5, 45), Debug::YELLOW);
		
	}

	Debug::Print("Score " + std::to_string(score), Vector2(50, 10), Debug::WHITE);

	if (!inSelectionMode)
	{
		world->GetMainCamera().UpdateCamera(dt);
	}
	else
	{
		CameraFollow();
		MovePlayerCharacter(dt);
		if (apple->createBullet == true)
		{
			CreateBullet();
			apple->createBullet = false;
		}
		if (apple->IsCatch == true)
		{
			CatchBullet();
			
		}
		Shoot();

		
	}

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera().SetPosition(camPos);
		world->GetMainCamera().SetPitch(angles.x);
		world->GetMainCamera().SetYaw(angles.y);

	}
	if (testStateObject) {
		testStateObject->Update(dt);
	}
	if (police && playerCharacter) {	
		police->UpdatePosition(playerCharacter, dt);
	}
	if (Agent && playerCharacter) 
	{
		Agent->UpdatePosition(playerCharacter,buttom,dt);
	}
	UpdateKeys();
	if (Agent && playerCharacter)
    {
	if (Agent->IsDoneLock == true&&count<3)
	{

		isLock = true;
		AddMazeToWorld();
		count++;
	}
	}
	

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);
		Debug::DrawLine(rayDir,rayDir);
		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}
	DebugObjectMovement();
	SelectObject();
	MoveSelectedObject();
	physics->UseGravity(true);
	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();
	Debug::UpdateRenderables(dt);

}

void TutorialGame::CameraFollow()
{

	Vector3 playerPos = playerCharacter->GetTransform().GetPosition();
	Vector3 cameraOffset(0, 2, 0);
	Vector3 newCameraPos = playerPos + cameraOffset;
	world->GetMainCamera().SetPosition(newCameraPos);
	world->GetMainCamera().SetYaw(yaw);
	world->GetMainCamera().SetPitch(DEFAULT_PITCH);

}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world->GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::DebugObjectMovement() 
{
	if (!inSelectionMode) 
	{
		if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::LEFT)) {
			playerCharacter->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::RIGHT)) {
			playerCharacter->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::UP)) {
			playerCharacter->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::DOWN)) {
			playerCharacter->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

	}
		
	
}

void TutorialGame::InitCamera() {
	world->GetMainCamera().SetNearPlane(0.1f);
	world->GetMainCamera().SetFarPlane(500.0f);
	world->GetMainCamera().SetPitch(-15.0f);
	world->GetMainCamera().SetYaw(315.0f);
	world->GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();

	useGravity = true;
	AddMazeToWorld();
	InitDefaultFloor();
}

GameObject* TutorialGame::AddATMToWorld(const Vector3& position, Vector3 dimensions, float inverseMass)
{
	GameObject* ATM = new GameObject("ATM");

	AABBVolume* volume = new AABBVolume(dimensions);
	ATM->SetBoundingVolume((CollisionVolume*)volume);

	ATM->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	ATM->SetRenderObject(new RenderObject(&ATM->GetTransform(), cubeMesh, basicTex, basicShader));
	ATM->SetPhysicsObject(new PhysicsObject(&ATM->GetTransform(), ATM->GetBoundingVolume()));

	ATM->GetPhysicsObject()->SetInverseMass(inverseMass);

	ATM->GetPhysicsObject()->InitCubeInertia();

	ATM->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
	world->AddGameObject(ATM);

	return ATM;
}

void TutorialGame::AddMazeToWorld() {
	int nodeSize;
	int gridWidth;
	int gridHeight;
	std::ifstream infile(Assets::DATADIR + "TestGrid1.txt");
	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;
	navGrid = new NavigationGrid("TestGrid1.txt");

	GridNode* nodes = new GridNode[gridWidth * gridHeight];
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x)
		{
			if (!isLock) 
			{
				GridNode& n = nodes[(gridWidth * y) + x];
				char type = 0;
				infile >> type;
				n.type = type;
				n.position = Vector3((float)(x * nodeSize), 7, (float)(y * nodeSize));
				if (type == 120)maze.emplace_back(AddCubeToWorld(n.position, { (float)nodeSize / 2,(float)nodeSize / 2,(float)nodeSize / 2 }, 0));
				if (type == 104)AddGateToWorld(Vector3(n.position.x, n.position.y - 2, n.position.z + 4), Vector3(0, 90, 0));
				if (type == 106)AddGateToWorld(Vector3(n.position.x - 4, n.position.y - 2, n.position.z), Vector3(0, 0, 0));
				if (type == 103)maze.emplace_back(AddAgentToWorld(n.position));
				if (type == 105)maze.emplace_back(AddButtomToWorld(n.position, Vector3(3,3,3), 0));
				if (type == 112)maze.emplace_back(AddPlayerToWorld(n.position));
				if (type == 97)maze.emplace_back(AddEnemyToWorld(n.position));
				if (type == 98)maze.emplace_back(AddBonusToWorld(n.position));
				if (type == 99)maze.emplace_back(AddATMToWorld(n.position, { (float)nodeSize / 2,0,(float)nodeSize / 2 }, 0));

			}
			if(isLock)
			{
				GridNode& n = nodes[(gridWidth * y) + x];
				char type = 0;
				infile >> type;
				n.type = type;
				n.position = Vector3((float)(x * nodeSize), 7, (float)(y * nodeSize));
				if (type == 104)AddCubeToWorld(n.position, { (float)nodeSize / 2,(float)nodeSize / 2,(float)nodeSize / 2 }, 0);
				if (type == 106)AddCubeToWorld(n.position, { (float)nodeSize / 2,(float)nodeSize / 2,(float)nodeSize / 2 }, 0);
			}
			
		}
	}
	return;
}

GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();
	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 8.0f;
	float inverseMass = 1.0f;

	playerCharacter = new GameObject("Goat");
	SphereVolume* volume = new SphereVolume(1.0);
	playerCharacter->SetBoundingVolume((CollisionVolume*)volume);

	playerCharacter->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	playerCharacter->SetRenderObject(new RenderObject(&playerCharacter->GetTransform(), enemyMesh, nullptr, basicShader));
	playerCharacter->SetPhysicsObject(new PhysicsObject(&playerCharacter->GetTransform(), playerCharacter->GetBoundingVolume()));

	playerCharacter->GetPhysicsObject()->SetInverseMass(inverseMass);
	playerCharacter->GetPhysicsObject()->InitSphereInertia();
	playerCharacter->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	world->AddGameObject(playerCharacter);

	return playerCharacter;
}

void TutorialGame::MovePlayerCharacter(float dt)
{
	float rotationSpeed = 60.0f;
	Vector3 pyr = playerCharacter->GetTransform().GetOrientation().ToEuler();

	yaw -= (Window::GetMouse()->GetRelativePosition().x);
	Quaternion q = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), yaw);
	playerCharacter->GetTransform().SetOrientation(q);
	if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::A))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() *
			Vector3(-1, 0, 0) * forceMagnitude);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::W))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() * Vector3(0, 0, -1) * forceMagnitude);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::S))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() * Vector3(0, 0, 1) * forceMagnitude /** playerCharacter->GetSpeedMultiplier()*/);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyCodes::D))
	{
		playerCharacter->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() *
			Vector3(1, 0, 0) * forceMagnitude);
	}
}

Apple* TutorialGame::AddBonusToWorld(const Vector3& position) 
{
	apple = new Apple(world);
	SphereVolume* volume = new SphereVolume();
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetScale(Vector3(1, 1, 1)).SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), sphereMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();
	apple->GetRenderObject()->SetColour({ 1,0,1,1 });
	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();
	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, 0, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) 
	{
		inSelectionMode = !inSelectionMode;
		if (!inSelectionMode) 
		{
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);	
			
		}
		else 
		{
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (!inSelectionMode) 
	{
		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	return false;
}

void TutorialGame::MoveSelectedObject() {

	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(1, 1, 1);

	float invCubeMass = 2; // how heavy the middle pieces are
	int numLinks = 0.5;
	float maxDistance = 5; // constraint distance
	float cubeDistance = 5; // distance between links

	Vector3 playerPos = playerCharacter->GetTransform().GetPosition();

	Vector3 startPos = playerPos;


	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);


	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);


	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);

		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;

	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

Apple *TutorialGame::CreateBullet()
{
	Vector3 CameraPos = world->GetMainCamera().GetPosition();
	bullet= new Apple(world);
	AABBVolume* volume = new AABBVolume(Vector3(1, 1, 1));
	bullet->SetBoundingVolume((CollisionVolume*)volume);
	bullet->GetTransform().SetPosition(CameraPos);
	bullet->GetTransform().SetScale(Vector3(1, 1, 1));
	bullet->SetRenderObject(new RenderObject(&bullet->GetTransform(), sphereMesh, nullptr, basicShader));
	bullet->SetPhysicsObject(new PhysicsObject(&bullet->GetTransform(), bullet->GetBoundingVolume()));
	bullet->GetPhysicsObject()->SetInverseMass(1.0f);
	bullet->GetPhysicsObject()->InitCubeInertia();
	bullet->GetRenderObject()->SetColour({ 1,0,1,1 });
	world->AddGameObject(bullet);
	Agent->IsLock = true;
	police->isStolen = true;
	return bullet;
	
}

void TutorialGame::CatchBullet()
{
	Vector3 playerPos = playerCharacter->GetTransform().GetPosition();
	
	Quaternion facingDir = playerCharacter->GetTransform().GetOrientation();
	Vector3 bulletOffset(0,2, -5);
	bulletPos = facingDir * bulletOffset;
	bulletOffset = facingDir * bulletOffset;
	finalBulletPos = bulletPos + playerPos;
	bullet->GetTransform().SetPosition(finalBulletPos);
}

void TutorialGame::Shoot()
{
	/*forceMagnitude += Window::GetMouse()->GetWheelMovement() * 300.0f;*/

	if (!bullet) 
	{
		return;//we haven't selected anything!
	}
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Left)) 
	{
		apple->IsCatch = false;
	   /* bullet->GetPhysicsObject()->AddForce(playerCharacter->GetTransform().GetOrientation() * Vector3(0, 0, -1) * forceMagnitude);*/
			
		
	}
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 1.0f;

	police = new Police("TestGrid1.txt", playerCharacter, position);
	police->SetOrigin(position);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	police->SetBoundingVolume((CollisionVolume*)volume);

	police->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	police->SetRenderObject(new RenderObject(&police->GetTransform(), enemyMesh, nullptr, basicShader));
	police->SetPhysicsObject(new PhysicsObject(&police->GetTransform(), police->GetBoundingVolume()));

	police->GetPhysicsObject()->SetInverseMass(inverseMass);
	police->GetPhysicsObject()->InitSphereInertia();
	police->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	world->AddGameObject(police);

	return police;
}

GameObject* TutorialGame::AddFenceToWorld(const Vector3& position, const Vector3& scale, const Vector3& rotation, const float inverseMass)
{
	GameObject* ramp = new GameObject();

	OBBVolume* volume = new OBBVolume(scale);
	ramp->SetBoundingVolume((CollisionVolume*)volume);
	ramp->GetTransform().SetScale(scale * 2);
	ramp->GetTransform().SetPosition(position);
	ramp->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(rotation.x, rotation.y, rotation.z));

	ramp->SetRenderObject(new RenderObject(&ramp->GetTransform(), cubeMesh, basicTex, basicShader));
	ramp->SetPhysicsObject(new PhysicsObject(&ramp->GetTransform(), ramp->GetBoundingVolume()));

	ramp->GetPhysicsObject()->SetInverseMass(inverseMass);
	ramp->GetPhysicsObject()->InitCubeInertia();

	ramp->GetRenderObject()->SetColour(Vector4(1, 0.9, 0.5, 1));

	world->AddGameObject(ramp);

	return ramp;
}

void TutorialGame::AddGateToWorld(const Vector3& position, const Vector3& rotation)
{
	GameObject* fence1 = AddFenceToWorld(position, Vector3(0.2, 2, 0.3), rotation, 0);
	GameObject* gate = AddFenceToWorld(position + Quaternion::EulerAnglesToQuaternion(rotation.x, rotation.y, rotation.z)
		* Vector3(12, 0, 0), Vector3(8, 1.8, 0.2), rotation, 1);

	PositionConstraint* constraint = new PositionConstraint(fence1, gate, 10);
	HingeConstraint* rot = new HingeConstraint(fence1, gate);
	HingeHeightConstraint* height = new HingeHeightConstraint(gate, gate->GetTransform().GetPosition().y);

	world->AddConstraint(constraint);
	world->AddConstraint(rot);
	world->AddConstraint(height);
}

void TutorialGame::UpdateGameTime(float dt) 
{
	
	Timer -= dt;
	
	Debug::Print("time " + std::to_string(Timer), Vector2(4, 15), Debug::WHITE);
	if (Timer <= 0.0f) 
	{
		lose = true;
	}
}

GameObject* TutorialGame::AddAgentToWorld(const Vector3& position)
{
	float meshSize = 3.0;
	float inverseMass = 1.0f;

    Agent = new SuperAgent("Agent", "TestGrid1.txt", world, playerCharacter, position);
	Agent-> SetOrigin(position);

	SphereVolume* volume = new SphereVolume(1.0);
	Agent->SetBoundingVolume((CollisionVolume*)volume);

	Agent->GetTransform().SetScale(Vector3(meshSize, meshSize, meshSize)).SetPosition(position);

	Agent->SetRenderObject(new RenderObject(&Agent->GetTransform(), enemyMesh, nullptr, basicShader));
	Agent->SetPhysicsObject(new PhysicsObject(&Agent->GetTransform(), Agent->GetBoundingVolume()));

	Agent->GetPhysicsObject()->SetInverseMass(inverseMass);
	Agent->GetPhysicsObject()->InitSphereInertia();
	Agent->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	world->AddGameObject(Agent);

	return Agent;
}

Apple* TutorialGame::AddButtomToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) 
{
    buttom = new Apple(world,"buttom");
	AABBVolume* volume = new AABBVolume(dimensions);
	buttom->SetBoundingVolume((CollisionVolume*)volume);

	buttom->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	buttom->SetRenderObject(new RenderObject(&buttom->GetTransform(), cubeMesh, basicTex, basicShader));
	buttom->SetPhysicsObject(new PhysicsObject(&buttom->GetTransform(), buttom->GetBoundingVolume()));

	buttom->GetPhysicsObject()->SetInverseMass(inverseMass);
	buttom->GetPhysicsObject()->InitCubeInertia();
	buttom->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	world->AddGameObject(buttom);

	return buttom ;
}