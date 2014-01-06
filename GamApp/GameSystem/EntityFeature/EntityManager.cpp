#include "EntityManager.h"
#include "Entity.h"

EntityManager::EntityManager()
{
}


EntityManager::~EntityManager()
{
}

void EntityManager::OnBeginFrame()
{
	std::map<unsigned int, Entity>::iterator entityIterator = entityMap.begin();
	for (; entityIterator != entityMap.end(); ++entityIterator)
	{
		entityIterator->second.OnBeginFrame();
	}

}

void EntityManager::OnFrame()
{
	std::map<unsigned int, Entity>::iterator entityIterator = entityMap.begin();
	for (; entityIterator != entityMap.end(); ++entityIterator)
	{
		entityIterator->second.OnFrame();
	}
}

void EntityManager::OnEndFrame()
{
	std::map<unsigned int, Entity>::iterator entityIterator = entityMap.begin();
	for (; entityIterator != entityMap.end(); ++entityIterator)
	{
		entityIterator->second.OnEndFrame();
	}
}

void EntityManager::AddEntity(Entity& entity)
{
	entityMap.insert(std::map<unsigned int, Entity>::value_type(entity.GetIndex(), entity));
}

void EntityManager::DelEntity(unsigned int entityIndex)
{
	if (entityIndex < 0 || entityIndex >= entityMap.size())
		return;
	entityMap.erase(entityIndex);
}
