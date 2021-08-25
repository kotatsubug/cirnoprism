#pragma once

#include "ecs_component.hh"
#include "ecs_system.hh"

#include "../common.hh"

#include <map>

#define NULL_COPY_AND_ASSIGN(T) \
	T(const T& other) {(void)other;} \
	void operator=(const T& other) { (void)other; }

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

	NULL_COPY_AND_ASSIGN(ECS);
public:
	ECS() {}
	~ECS();

	// ECSListener methods
	inline void AddListener(ECSListener* listener) {
		_listeners.push_back(listener);
	}

	// Entity methods
	EntityHandle MakeEntity(BaseECSComponent** components, const uint32_t* componentIDs, size_t numComponents);

	void RemoveEntity(EntityHandle handle);

// Template methods for entity constructors
	template<class ...COMPONENT_CLASSES>
	EntityHandle MakeEntity(COMPONENT_CLASSES&... args)
	{
		BaseECSComponent* components[] = { &args... };
		uint32_t componentIDs[] = { COMPONENT_CLASSES::ID... };
		return MakeEntity(components, componentIDs, sizeof...(COMPONENT_CLASSES));
	}

/*	template<class A>
	EntityHandle MakeEntity(A& c1)
	{
		BaseECSComponent* components[] = { &c1 };
		uint32_t componentIDs[] = {A::ID};
		return MakeEntity(components, componentIDs, 1);
	}

	template<class A, class B>
	EntityHandle MakeEntity(A& c1, B& c2)
	{
		BaseECSComponent* components[] = { &c1, &c2 };
		uint32_t componentIDs[] = {A::ID, B::ID};
		return makeEntity(components, componentIDs, 2);
	}

	template<class A, class B, class C>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3 };
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID};
		return MakeEntity(components, componentIDs, 3);
	}

	template<class A, class B, class C, class D>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4 };
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID};
		return MakeEntity(components, componentIDs, 4);
	}

	template<class A, class B, class C, class D, class E>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4, &c5 };
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID, E::ID};
		return MakeEntity(components, componentIDs, 5);
	}

	template<class A, class B, class C, class D, class E, class F>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4, &c5, &c6};
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID, E::ID, F::ID};
		return MakeEntity(components, componentIDs, 6);
	}

	template<class A, class B, class C, class D, class E, class F, class G>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7};
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID, E::ID, F::ID, G::ID};
		return MakeEntity(components, componentIDs, 7);
	}

	template<class A, class B, class C, class D, class E, class F, class G, class H>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7, H& c8)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8};
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID, E::ID, F::ID, G::ID, H::ID};
		return MakeEntity(components, componentIDs, 8);
	}

	template<class A, class B, class C, class D, class E, class F, class G, class H, class I>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7, H& c8, I& c9)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8, &c9 };
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID, E::ID, F::ID, G::ID, H::ID, I::ID};
		return MakeEntity(components, componentIDs, 9);
	}

	template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
	EntityHandle MakeEntity(A& c1, B& c2, C& c3, D& c4, E& c5, F& c6, G& c7, H& c8, I& c9, J& c10)
	{
		BaseECSComponent* components[] = { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8, &c9, &c10 };
		uint32_t componentIDs[] = {A::ID, B::ID, C::ID, D::ID, E::ID, F::ID, G::ID, H::ID, I::ID, J::ID };
		return MakeEntity(components, componentIDs, 10);
	}*/
// End template methods for entity constructors

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
