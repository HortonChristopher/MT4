#include "TouchableObject.h"
#include "MeshCollider.h"
#include "CollisionAttribute.h"

TouchableObject* TouchableObject::Create(Model* model)
{
	
	TouchableObject* instance = new TouchableObject();

	if (instance == nullptr) {
		return nullptr;
	}

	if (!instance->Initialize(model)) {
		delete instance;
		assert(0);
	}

	return instance;
}

bool TouchableObject::Initialize(Model* model)
{
	if (!Object3d::Initialize())
	{
		return false;
	}

	SetModel(model);

	MeshCollider* collider = new MeshCollider;
	SetCollider(collider);
	collider->ConstructTriangles(model);
	collider->SetAttribute(COLLISION_ATTR_LANDSHAPE);

	return true;
}