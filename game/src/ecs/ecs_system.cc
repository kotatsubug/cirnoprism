#include "ecs_system.hh"

/// Checks if all components of the ECS system are valid.
/// For instance, there must be at least one non-optional component.
bool BaseECSSystem::IsValid()
{
	for (uint32_t i = 0;  i < _componentFlags.size(); i++)
	{
		if ((_componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) == 0)
		{
			return true;
		}
	}
	return false;
}

bool ECSSystemList::RemoveSystem(BaseECSSystem& system)
{
	for (uint32_t i = 0; i < _systems.size(); i++)
	{
		if (&system == _systems[i])
		{
			_systems.erase(_systems.begin() + i);
			return true;
		}
	}
	return false;
}

