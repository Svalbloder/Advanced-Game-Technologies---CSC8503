#include "Police.h"
#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"
#include "PhysicsObject.h"
//part of this codes are from www.github.com

Police::Police(string filePath, GameObject* goat, Vector3 initialPosition) : GameObject()
{
	scaryGoat = goat;
	lastKnownGoatPosition = scaryGoat->GetTransform().GetPosition();
	filename = filePath;
	grid = new NavigationGrid(filename);
	grid->FindPath(initialPosition, scaryGoat->GetTransform().GetPosition(), path);
	path.PopWaypoint(destPos);

	stateMachine = new StateMachine();

	State* IdleState = new State([&](float dt) -> void
		{
			this->Idle(dt);
		}
	);
	State* ChaseState = new State([&](float dt) -> void
		{
			
			this->Chase(dt);
		}
	);
	stateMachine->AddState(IdleState);

	stateMachine->AddState(ChaseState);
	
	stateMachine->AddTransition(new StateTransition(IdleState, ChaseState,
		[&]() -> bool
		{
			if (isStolen) {
				return true;
			}
			else {
				return false;
			}
		}
	));
	stateMachine->AddTransition(new StateTransition(ChaseState, IdleState,
		[&]() -> bool
		{
			if (this->counter > 30.0f) {
				return true;
			}
			else {
				return false;
			}
		}
	));

	
}

void Police::UpdatePosition(GameObject* goat, float dt)
{
	scaryGoat = goat;
	currentPosition = GetTransform().GetPosition();
	stateMachine->Update(dt);
}

void Police::OnCollisionBegin(GameObject* otherObject)
{
	if (otherObject->GetName() == "Goat") {
		std::cout << "Caught";
	}
}

void Police::Chase(float dt)
{
	nodes.clear();
	path.Clear();
	grid->FindPath(currentPosition, scaryGoat->GetTransform().GetPosition(), path);
	while (path.PopWaypoint(destPos))
	{
		nodes.push_back(destPos);
	}
	//drawPath();
	if (nodes.size() >= 2) {
		Vector3 direction = nodes[1] - nodes[0];
		direction.Normalise();
		direction.y = 0;
		GetPhysicsObject()->SetForce(direction * speed * dt);
	}

	counter += dt;
}

void Police::Idle(float dt)
{
	//do nothing
}

void Police::drawPath()
{
	for (int i = 0; i < nodes.size() - 1; i++)
	{
		Debug::DrawLine(nodes[i] + Vector3(0, 1, 0), nodes[i + 1] + Vector3(0, 1, 0), Vector4(1, 0, 0, 1));
	}
}