#include "ecs_component.hh"

std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>>* BaseECSComponent::_componentTypes;

uint32_t BaseECSComponent::RegisterComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, size_t size)
{
	if (_componentTypes == nullptr)
	{
		_componentTypes = new std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>>();
	}

	uint32_t componentID = _componentTypes->size();
	_componentTypes->push_back(std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>(createfn, freefn, size));

	return componentID;
}
