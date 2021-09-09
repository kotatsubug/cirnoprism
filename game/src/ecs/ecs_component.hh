#pragma once

#include <vector>
#include <tuple>

#include "common.hh"

struct BaseECSComponent;
typedef void* EntityHandle;
typedef uint32_t (*ECSComponentCreateFunction)(std::vector<uint8_t>& memory, EntityHandle entity, BaseECSComponent* comp);
typedef void (*ECSComponentFreeFunction)(BaseECSComponent* comp);
#define NULL_ENTITY_HANDLE nullptr

struct BaseECSComponent
{
private:
	static std::vector<std::tuple<ECSComponentCreateFunction, ECSComponentFreeFunction, size_t>>* _componentTypes;
public:
	static uint32_t RegisterComponentType(ECSComponentCreateFunction createfn, ECSComponentFreeFunction freefn, size_t size);
	EntityHandle entity = NULL_ENTITY_HANDLE;

	inline static ECSComponentCreateFunction GetTypeCreateFunction(uint32_t id)
	{
		return std::get<0>((*_componentTypes)[id]);
	}

	inline static ECSComponentFreeFunction GetTypeFreeFunction(uint32_t id)
	{
		return std::get<1>((*_componentTypes)[id]);
	}

	inline static size_t GetTypeSize(uint32_t id)
	{
		return std::get<2>((*_componentTypes)[id]);
	}

	inline static bool IsTypeValid(uint32_t id)
	{
		return id < _componentTypes->size();
	}
};

template<typename T>
struct ECSComponent : public BaseECSComponent
{
	static const ECSComponentCreateFunction CREATE_FUNCTION;
	static const ECSComponentFreeFunction FREE_FUNCTION;
	static const uint32_t ID;
	static const size_t SIZE; 
};

template<typename Component>
uint32_t ECSComponentCreate(std::vector<uint8_t>& memory, EntityHandle entity, BaseECSComponent* comp)
{
	uint32_t index = memory.size();
	memory.resize(index + Component::SIZE);
	Component* component = new(&memory[index]) Component(*(Component*)comp);
	component->entity = entity;
	return index;
}

template<typename Component>
void ECSComponentFree(BaseECSComponent* comp)
{
	Component* component = (Component*) comp;
	component->~Component();
}

template<typename T>
const uint32_t ECSComponent<T>::ID(BaseECSComponent::RegisterComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T)));

template<typename T>
const size_t ECSComponent<T>::SIZE(sizeof(T));

template<typename T>
const ECSComponentCreateFunction ECSComponent<T>::CREATE_FUNCTION(ECSComponentCreate<T>);

template<typename T>
const ECSComponentFreeFunction ECSComponent<T>::FREE_FUNCTION(ECSComponentFree<T>);

// TODO: BEGIN EXAMPLE CODE
struct TestComponent : public ECSComponent<TestComponent>
{
	float x;
	float y;
};

