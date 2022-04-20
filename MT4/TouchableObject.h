#pragma once

#include "Object3d.h"

class TouchableObject : public Object3d
{
public:

	static TouchableObject* Create(Model* model = nullptr);

	bool Initialize(Model* model);
};