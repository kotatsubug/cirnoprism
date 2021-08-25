#include "ecs.hh"

ECS::~ECS()
{
	for (std::map<uint32_t, std::vector<uint8_t>>::iterator it = _components.begin(); it != _components.end(); ++it)
	{
		size_t typeSize = BaseECSComponent::GetTypeSize(it->first);
		ECSComponentFreeFunction freefn = BaseECSComponent::GetTypeFreeFunction(it->first);

		for (uint32_t i = 0; i < it->second.size(); i += typeSize)
		{
			freefn((BaseECSComponent*)&it->second[i]);
		}
	}

	for(uint32_t i = 0; i < _entities.size(); i++) {
		delete _entities[i];
	}
}

EntityHandle ECS::MakeEntity(BaseECSComponent** entityComponents, const uint32_t* componentIDs, size_t numComponents)
{
	std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>* newEntity =
		new std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>();

	EntityHandle handle = (EntityHandle) newEntity;

	for (uint32_t i = 0; i < numComponents; i++)
	{
		if (!BaseECSComponent::IsTypeValid(componentIDs[i]))
		{
			DEBUG_LOG("ECS", LOG_ERROR, "'%u' is not a valid component type.", componentIDs[i]);
			delete newEntity;
			return NULL_ENTITY_HANDLE;
		}

		_AddComponentInternal(handle, newEntity->second, componentIDs[i], entityComponents[i]);
	}

	newEntity->first = _entities.size();
	_entities.push_back(newEntity);

	for (uint32_t i = 0; i < _listeners.size(); i++)
	{
		bool isValid = true;
		
		if (_listeners[i]->ShouldNotifyOnAllEntityOperations())
		{
			_listeners[i]->OnMakeEntity(handle);
		}
		else
		{
			for (uint32_t j = 0; j < _listeners[i]->GetComponentIDs().size(); j++) {
				bool hasComponent = false;

				for (uint32_t k = 0; k < numComponents; k++)
				{
					if (_listeners[i]->GetComponentIDs()[j] == componentIDs[k])
					{
						hasComponent = true;
						break;
					}
				}

				if (!hasComponent) {
					isValid = false;
					break;
				}
			}

			if(isValid) {
				_listeners[i]->OnMakeEntity(handle);
			}
		}
	}

	return handle;
}

void ECS::RemoveEntity(EntityHandle handle)
{
	std::vector<std::pair<uint32_t, uint32_t> >& entity = _HandleToEntity(handle);

	for(uint32_t i = 0; i < _listeners.size(); i++) {
		const std::vector<uint32_t>& componentIDs = _listeners[i]->GetComponentIDs();
		bool isValid = true;

		if (_listeners[i]->ShouldNotifyOnAllEntityOperations())
		{
			_listeners[i]->OnRemoveEntity(handle);
		}
		else
		{
			for (uint32_t j = 0; j < componentIDs.size(); j++)
			{
				bool hasComponent = false;

				for (uint32_t k = 0; k < entity.size(); k++)
				{
					if (componentIDs[j] == entity[k].first)
					{
						hasComponent = true;
						break;
					}
				}

				if(!hasComponent)
				{
					isValid = false;
					break;
				}
			}

			if(isValid) {
				_listeners[i]->OnRemoveEntity(handle);
			}
		}
	}
	
	for(uint32_t i = 0; i < entity.size(); i++) {
		_DeleteComponent(entity[i].first, entity[i].second);
	}

	uint32_t destIndex = _HandleToEntityIndex(handle);
	uint32_t srcIndex = _entities.size() - 1;
	delete _entities[destIndex];
	_entities[destIndex] = _entities[srcIndex];
	_entities[destIndex]->first = destIndex;
	_entities.pop_back();
}

void ECS::_AddComponentInternal(EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t> >& entity, uint32_t componentID, BaseECSComponent* component)
{
	ECSComponentCreateFunction createfn = BaseECSComponent::GetTypeCreateFunction(componentID);
	std::pair<uint32_t, uint32_t> newPair;
	newPair.first = componentID;
	newPair.second = createfn(_components[componentID], handle, component);
	entity.push_back(newPair);
}

void ECS::_DeleteComponent(uint32_t componentID, uint32_t index)
{
	std::vector<uint8_t>& arr = _components[componentID];
	ECSComponentFreeFunction freefn = BaseECSComponent::GetTypeFreeFunction(componentID);
	size_t typeSize = BaseECSComponent::GetTypeSize(componentID);
	uint32_t srcIndex = arr.size() - typeSize;

	BaseECSComponent* destComponent = (BaseECSComponent*)&arr[index];
	BaseECSComponent* srcComponent = (BaseECSComponent*)&arr[srcIndex];
	freefn(destComponent);

	if(index == srcIndex) {
		arr.resize(srcIndex);
		return;
	}

	memcpy(destComponent, srcComponent, typeSize);

	std::vector<std::pair<uint32_t, uint32_t> >& srcComponents = _HandleToEntity(srcComponent->entity);

	for (uint32_t i = 0; i < srcComponents.size(); i++)
	{
		if (componentID == srcComponents[i].first && srcIndex == srcComponents[i].second) {
			srcComponents[i].second = index;
			break;
		}
	}

	arr.resize(srcIndex);
}

bool ECS::_RemoveComponentInternal(EntityHandle handle, uint32_t componentID)
{
	std::vector<std::pair<uint32_t, uint32_t> >& entityComponents = _HandleToEntity(handle);

	for (uint32_t i = 0; i < entityComponents.size(); i++)
	{
		if (componentID == entityComponents[i].first)
		{
			_DeleteComponent(entityComponents[i].first, entityComponents[i].second);
			uint32_t srcIndex = entityComponents.size()-1;
			uint32_t destIndex = i;
			entityComponents[destIndex] = entityComponents[srcIndex];
			entityComponents.pop_back();
			return true;
		}
	}
	return false;
}

BaseECSComponent* ECS::_GetComponentInternal(std::vector<std::pair<uint32_t, uint32_t> >& entityComponents, std::vector<uint8_t>& arr, uint32_t componentID)
{
	for (uint32_t i = 0; i < entityComponents.size(); i++)
	{
		if (componentID == entityComponents[i].first)
		{
			return (BaseECSComponent*)&arr[entityComponents[i].second];
		}
	}
	return nullptr;
}

void ECS::UpdateSystems(ECSSystemList& systems, float delta)
{
	std::vector<BaseECSComponent*> componentParam;
	std::vector<std::vector<uint8_t>*> componentArrays;

	for (uint32_t i = 0; i < systems.size(); i++)
	{
		const std::vector<uint32_t>& componentTypes = systems[i]->GetComponentTypes();

		if (componentTypes.size() == 1)
		{
			size_t typeSize = BaseECSComponent::GetTypeSize(componentTypes[0]);
			std::vector<uint8_t>& arr = _components[componentTypes[0]];

			for (uint32_t j = 0; j < arr.size(); j += typeSize)
			{
				BaseECSComponent* component = (BaseECSComponent*)&arr[j];
				systems[i]->UpdateComponents(delta, &component);
			}
		}
		else
		{
			_UpdateSystemWithMultipleComponents(i, systems, delta, componentTypes, componentParam, componentArrays);
		}
	}
}

uint32_t ECS::_FindLeastCommonComponent(const std::vector<uint32_t>& componentTypes, const std::vector<uint32_t>& componentFlags)
{
	uint32_t minSize = (uint32_t)(-1);
	uint32_t minIndex = (uint32_t)(-1);

	for (uint32_t i = 0; i < componentTypes.size(); i++)
	{
		if ((componentFlags[i] & BaseECSSystem::FLAG_OPTIONAL) != 0)
		{
			continue;
		}

		size_t typeSize = BaseECSComponent::GetTypeSize(componentTypes[i]);
		uint32_t size = _components[componentTypes[i]].size() / typeSize;

		if (size <= minSize)
		{
			minSize = size;
			minIndex = i;
		}
	}

	return minIndex;
}

void ECS::_UpdateSystemWithMultipleComponents(
	uint32_t index, ECSSystemList& systems, float delta,
	const std::vector<uint32_t>& componentTypes, std::vector<BaseECSComponent*>& componentParam,
	std::vector<std::vector<uint8_t>*>& componentArrays)
{
	const std::vector<uint32_t>& componentFlags = systems[index]->GetComponentFlags();

	componentParam.resize(std::max(componentParam.size(), componentTypes.size()));
	componentArrays.resize(std::max(componentArrays.size(), componentTypes.size()));

	for (uint32_t i = 0; i < componentTypes.size(); i++)
	{
		componentArrays[i] = &_components[componentTypes[i]];
	}

	uint32_t minSizeIndex = _FindLeastCommonComponent(componentTypes, componentFlags);
	size_t typeSize = BaseECSComponent::GetTypeSize(componentTypes[minSizeIndex]);
	std::vector<uint8_t>& arr = *componentArrays[minSizeIndex];

	for (uint32_t i = 0; i < arr.size(); i += typeSize)
	{
		componentParam[minSizeIndex] = (BaseECSComponent*) &arr[i];
		std::vector<std::pair<uint32_t, uint32_t> >& entityComponents = _HandleToEntity(componentParam[minSizeIndex]->entity);

		bool isValid = true;

		for (uint32_t j = 0; j < componentTypes.size(); j++)
		{
			if (j == minSizeIndex)
			{
				continue;
			}

			componentParam[j] = _GetComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);

			if (componentParam[j] == nullptr && (componentFlags[j] & BaseECSSystem::FLAG_OPTIONAL) == 0)
			{
				isValid = false;
				break;
			}
		}

		if (isValid)
		{
			systems[index]->UpdateComponents(delta, &componentParam[0]);
		}
	}
}

