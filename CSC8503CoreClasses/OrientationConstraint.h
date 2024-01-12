#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class OrientationConstraint : public Constraint
		{
		public:
			OrientationConstraint(GameObject* a, GameObject* b);
			~OrientationConstraint();

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;

			float distance;
		};
	}
}
namespace NCL
{


	namespace CSC8503
	{
		using namespace Maths;
		class GameObject;

		class HingeConstraint : public Constraint
		{
		public:
			HingeConstraint(GameObject* a, GameObject* b)
			{
				objectA = a;
				objectB = b;
			}

			~HingeConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;
		};
	}
}
namespace NCL
{
	namespace CSC8503
	{
		class GameObject;

		class HingeHeightConstraint : public Constraint
		{
		public:
			HingeHeightConstraint(GameObject* gameObject, const float heightValue) {
				object = gameObject;
				height = heightValue;
			}
			~HingeHeightConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* object;

			float height;
		};
	}
}
