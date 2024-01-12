#include "OrientationConstraint.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "Vector3.h"
#include "Maths.h"
#include "Quaternion.h"
#include <corecrt_math.h>
using namespace NCL;
using namespace Maths;
using namespace CSC8503;

OrientationConstraint::OrientationConstraint(GameObject* a, GameObject* b)
{
	objectA = a;
	objectB = b;
}

OrientationConstraint::~OrientationConstraint()
{

}
void OrientationConstraint::UpdateConstraint(float dt) {

}
void HingeConstraint::UpdateConstraint(float dt)
{
	Vector3 relativePos = objectA->GetTransform().GetPosition() - objectB->GetTransform().GetPosition();
	relativePos.y = 0;
	Vector3 dir = relativePos.Normalised();

	//Manage rotation
	objectA->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, -Maths::RadiansToDegrees(atan2f(dir.z, dir.x)), 0));
	objectB->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, Maths::RadiansToDegrees(atan2f(-dir.z, dir.x)), 0));
}
void HingeHeightConstraint::UpdateConstraint(float dt)
{
	Vector3 position = object->GetTransform().GetPosition();
	position.y = height;
	object->GetTransform().SetPosition(position);
}