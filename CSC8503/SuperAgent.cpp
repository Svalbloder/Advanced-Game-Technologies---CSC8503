#include "SuperAgent.h"
#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourAction.h"
#include "PhysicsObject.h"
#include "TutorialGame.h"
SuperAgent::SuperAgent(const std::string& objectName,string filePath, GameWorld* world, GameObject* goat, Vector3 initialPosition) : GameObject()
{
	float timer = 0;
	grid = new NavigationGrid(filePath);
	scaryGoat = goat;
	/*AlarmBullom = alarm;*/
	this->world = world;
	grid->FindPath(initialPosition, scaryGoat->GetTransform().GetPosition(), path);
	path.PopWaypoint(targetPosition);

	BehaviourAction* Idle = new BehaviourAction("Idle",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				state = Ongoing;
			}
			else if (state == Ongoing) 
			{
				if (IsLock) {
					return Success;
				}
			}
			return state;
		}
	);
	BehaviourAction* Alarm= new BehaviourAction("Alarm",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				GetPhysicsObject()->AddForce(Vector3(0, 10, 0) * 50);
				state = Ongoing;
			}
			else if (state == Ongoing) {
				Vector3 alarmPos = AlarmBullom->GetTransform().GetPosition();
				Vector3 goosePos = this->GetTransform().GetPosition();
				Vector3 direction = alarmPos - goosePos;
				direction.Normalise();
				Ray ray(GetTransform().GetPosition(), direction);
				RayCollision closestCollision;
				if (this->world->Raycast(ray, closestCollision, true, this)) 
				{
					if (closestCollision.node == AlarmBullom) {
						std::cout << "Go Press Alarm!\n";
						nodes.clear();
						path.Clear();
						if (grid->FindPath(GetTransform().GetPosition(), AlarmBullom->GetTransform().GetPosition(), path))
						{
							while (path.PopWaypoint(nextDest)) {
								nodes.push_back(nextDest);
							}
							drawPath();
							if (nodes.size() >= 2) {
								Vector3 direction = nodes[1] - nodes[0];
								direction.Normalise();
								GetPhysicsObject()->SetForce(direction * speed * 2 * dt);
								if (IsDoneLock ==true) 
								{
									return Success;
									IsDoneLock == false;
								}
							}
							
								
							
						}
					}
					else {
						nodes.clear();
						path.Clear();
						grid->FindPath(GetTransform().GetPosition(), Vector3(60, 5, 10), path);
						while (path.PopWaypoint(nextDest)) {
							nodes.push_back(nextDest);
						}
						drawPath();
						if (nodes.size() >= 2) {
							Vector3 direction = nodes[1] - nodes[0];
							direction.Normalise();
							GetPhysicsObject()->SetForce(direction * speed * dt);
						}
					}
				}
				
			}
			return state;
		}
	);
	BehaviourAction* ChaseGoat = new BehaviourAction("ChaseGoat",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Agent Starting Chase\n";
				GetPhysicsObject()->AddForce(Vector3(0, 10, 0) * 50);
				state = Ongoing;
			}
			else if (state == Ongoing) {
				nodes.clear();
				path.Clear();
				if (grid->FindPath(GetTransform().GetPosition(), scaryGoat->GetTransform().GetPosition(), path)) {
					while (path.PopWaypoint(nextDest)) {
						nodes.push_back(nextDest);
					}
					drawPath();
					if (nodes.size() >= 2) {
						Vector3 direction = nodes[1] - nodes[0];
						direction.Normalise();
						GetPhysicsObject()->SetForce(direction * speed * 2 * dt);
					}
				}
				// if lost goat or caught
				if (GoatCaught) {
					std::cout << "Caught!\n";
					return Success;
				}
			}
			return state; 
		}
	);

	BehaviourAction* ReturnHome = new BehaviourAction("ReturnHome",
		[&](float dt, BehaviourState state) -> BehaviourState {
			if (state == Initialise) {
				std::cout << "Goose Returning home!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				nodes.clear();
				path.Clear();
				targetPosition = origin;
				targetPosition.y = 0;
				if (grid->FindPath(GetTransform().GetPosition(), origin, path)) {
					Vector3 pos;
					while (path.PopWaypoint(pos)) {
						nodes.push_back(pos);
					}
					drawPath();
					if (nodes.size() >= 2) {
						Vector3 direction = nodes[1] - nodes[0];
						direction.Normalise();
						GetPhysicsObject()->SetForce(direction * speed * dt);
					}
				}
				if (GetTransform().GetPosition() == origin) {
					std::cout << "Returned Home\n";
					return Success;
				}
				return Ongoing;
			}
			return state;
		}
	);
	BehaviourSequence* sequence1 = new BehaviourSequence("Idle");
	sequence1->AddChild(Idle);

	BehaviourSequence* sequence = new BehaviourSequence("Patrol and Alarm");
	sequence->AddChild(Alarm);
	sequence->AddChild(ChaseGoat);

	BehaviourSequence* selection = new BehaviourSequence("Return and Home");
	//selection->AddChild(BringGoatHome);
	selection->AddChild(ReturnHome);

	rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence1);
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);
	rootSequence->Reset();
}

void SuperAgent::UpdatePosition(GameObject* goat,GameObject* buttom,float dt)
{
	scaryGoat = goat;
	AlarmBullom = buttom;
	BehaviourState state = Ongoing;
	state = rootSequence->Execute(dt);
	if (state == Success) {
		std::cout << "GOOSE SUCCESS\n";
		rootSequence->Reset();
	}
	else if (state == Failure) {
		std::cout << "GOOSE FAILURE\n";
		rootSequence->Reset();
	}
}

void SuperAgent::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "Goat") {
		world->RemoveGameObject(scaryGoat);
		GoatCaught = true;
	}
	if (otherObject->GetName() == "buttom") {
		IsDoneLock = true;
	}
}


void SuperAgent::drawPath()
{
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		Debug::DrawLine(nodes[i] + Vector3(0, 1, 0), nodes[i + 1] + Vector3(0, 1, 0), Vector4(1, 0, 1, 1));
	}
}

