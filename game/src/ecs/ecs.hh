#pragma once

#include "ecs_component.hh"
#include "ecs_system.hh"

#include <map>

class ECSListener
{
private:
	std::vector<uint32_t> _componentIDs;
	bool _notifyOnAllComponentOperations = false;
	bool _notifyOnAllEntityOperations = false;
protected:
	void SetNotificationSettings(bool shouldNotifyOnAllComponentOperations, bool shouldNotifyOnAllEntityOperations)
	{
		_notifyOnAllComponentOperations = shouldNotifyOnAllComponentOperations;
		_notifyOnAllEntityOperations = shouldNotifyOnAllEntityOperations;
	}

	void AddComponentID(uint32_t id)
	{
		_componentIDs.push_back(id);
	}
public:
	virtual void OnMakeEntity(EntityHandle handle)
	{
	}

	virtual void OnRemoveEntity(EntityHandle handle)
	{
	}

	virtual void OnAddComponent(EntityHandle handle, uint32_t id)
	{
	}

	virtual void OnRemoveComponent(EntityHandle handle, uint32_t id)
	{
	}

	const std::vector<uint32_t>& GetComponentIDs()
	{ 
		return _componentIDs;
	}

	inline bool ShouldNotifyOnAllComponentOperations()
	{
		return _notifyOnAllComponentOperations;
	}
	inline bool ShouldNotifyOnAllEntityOperations()
	{
		return _notifyOnAllEntityOperations;
	}
};

class ECS
{
private:
	std::map<uint32_t, std::vector<uint8_t>> _components;
	std::vector<std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>*> _entities;
	std::vector<ECSListener*> _listeners;

	inline std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>* _HandleToRawType(EntityHandle handle)
	{
		return (std::pair<uint32_t, std::vector<std::pair<uint32_t, uint32_t>>>*) handle;
	}

	inline uint32_t _HandleToEntityIndex(EntityHandle handle)
	{
		return _HandleToRawType(handle)->first;
	}

	inline std::vector<std::pair<uint32_t, uint32_t> >& _HandleToEntity(EntityHandle handle)
	{
		return _HandleToRawType(handle)->second;
	}

	void _DeleteComponent(uint32_t componentID, uint32_t index);

	bool _RemoveComponentInternal(EntityHandle handle, uint32_t componentID);

	void _AddComponentInternal(
		EntityHandle handle, std::vector<std::pair<uint32_t, uint32_t>>& entity,
		uint32_t componentID, BaseECSComponent* component);

	BaseECSComponent* _GetComponentInternal(
		std::vector<std::pair<uint32_t, uint32_t> >& entityComponents,
		std::vector<uint8_t>& arr, uint32_t componentID);

	void _UpdateSystemWithMultipleComponents(
		uint32_t index, ECSSystemList& systems, float delta, const std::vector<uint32_t>& componentTypes,
		std::vector<BaseECSComponent*>& componentParam, std::vector<std::vector<uint8_t>*>& componentArrays);

	uint32_t _FindLeastCommonComponent(const std::vector<uint32_t>& componentTypes, const std::vector<uint32_t>& componentFlags);
public:
	ECS() {}
	~ECS();

	// ECSListener methods
	inline void AddListener(ECSListener* listener) {
		_listeners.push_back(listener);
	}

	// Entity methods

	/// Create and return a new entity. This is a more thorough constructor and should generally be
	/// called only internally, but is OK to use if you know what you're doing.
	EntityHandle MakeEntity(BaseECSComponent** components, const uint32_t* componentIDs, size_t numComponents);

	void RemoveEntity(EntityHandle handle);

	/// Create and return a new entity.
	template<class ...COMPONENT_CLASSES>
	EntityHandle MakeEntity(COMPONENT_CLASSES&... args)
	{
		BaseECSComponent* components[] = { &args... };
		uint32_t componentIDs[] = { COMPONENT_CLASSES::ID... };
		return MakeEntity(components, componentIDs, sizeof...(COMPONENT_CLASSES));
	}

	// Component methods

	template<class Component>
	inline void AddComponent(EntityHandle entity, Component* component)
	{
		AddComponentInternal(entity, _HandleToEntity(entity), Component::ID, component);

		for (uint32_t i = 0; i < _listeners.size(); i++)
		{
			const std::vector<uint32_t>& componentIDs = _listeners[i]->GetComponentIDs();

			if (_listeners[i]->ShouldNotifyOnAllComponentOperations())
			{
				_listeners[i]->OnAddComponent(entity, Component::ID);
			}
			else
			{
				for (uint32_t j = 0; j < componentIDs.size(); j++)
				{
					if (componentIDs[j] == Component::ID)
					{
						_listeners[i]->OnAddComponent(entity, Component::ID);
						break;
					}
				}
			}
		}
	}

	template<class Component>
	bool RemoveComponent(EntityHandle entity)
	{
		for (uint32_t i = 0; i < _listeners.size(); i++)
		{
			const std::vector<uint32_t>& componentIDs = _listeners[i]->GetComponentIDs();

			for (uint32_t j = 0; j < componentIDs.size(); j++)
			{
				if (_listeners[i]->ShouldNotifyOnAllComponentOperations())
				{
					_listeners[i]->OnRemoveComponent(entity, Component::ID);
				}
				else
				{
					if (componentIDs[j] == Component::ID)
					{
						_listeners[i]->OnRemoveComponent(entity, Component::ID);
						break;
					}
				}
			}
		}
		return _RemoveComponentInternal(entity, Component::ID);
	}

	template<class Component>
	Component* GetComponent(EntityHandle entity)
	{
		return (Component*) _GetComponentInternal(_HandleToEntity(entity), _components[Component::ID], Component::ID);
	}

	BaseECSComponent* GetComponentByType(EntityHandle entity, uint32_t componentID)
	{
		return _GetComponentInternal(_HandleToEntity(entity), _components[componentID], componentID);
	}

	// System methods
	void UpdateSystems(ECSSystemList& systems, float delta);
	
};
