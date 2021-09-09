#pragma once

#include "ecs_component.hh"

class BaseECSSystem
{
private:
	std::vector<uint32_t> _componentTypes;
	std::vector<uint32_t> _componentFlags;
protected:
	void AddComponentType(uint32_t componentType, uint32_t componentFlag = 0)
	{
		_componentTypes.push_back(componentType);
		_componentFlags.push_back(componentFlag);
	}
public:
	enum
	{
		FLAG_OPTIONAL = 1,
	};

	// Empty constructor
	BaseECSSystem()
	{
	}

	// Update components by default is empty. Override to implement functionality into system.
	virtual void UpdateComponents(float delta, BaseECSComponent** components)
	{
	}

	const std::vector<uint32_t>& GetComponentTypes()
	{
		return _componentTypes;
	}

	const std::vector<uint32_t>& GetComponentFlags()
	{
		return _componentFlags;
	}

	bool IsValid();
};

class ECSSystemList
{
private:
	std::vector<BaseECSSystem*> _systems;
public:
	inline bool AddSystem(BaseECSSystem& system)
	{
		if (!system.IsValid())
		{
			return false;
		}

		_systems.push_back(&system);
		return true;
	}

	inline size_t size()
	{
		return _systems.size();
	}

	inline BaseECSSystem* operator[](uint32_t index)
	{
		return _systems[index];
	}

	bool RemoveSystem(BaseECSSystem& system);
};
