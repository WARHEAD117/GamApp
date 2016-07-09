#pragma once  
#include "CommonUtil\\GlobalHeader.h"

class Entity;

class EntityManager
{
public:
	EntityManager();
	~EntityManager();

public:
	void OnBeginFrame();
	void OnFrame();
	void OnEndFrame();

	Entity* CreateEntity();

	template<typename CLASSTYPE>
	CLASSTYPE* CreateEntityFromXFile(std::string fileName);

	void AddEntity(Entity& entity);
	void DelEntity(unsigned int entityIndex);

private:
	Entity* Creator();
	std::map< unsigned int, Entity* > entityMap;
};

template<typename CLASSTYPE>
inline CLASSTYPE* EntityManager::CreateEntityFromXFile(std::string fileName)
{
	CLASSTYPE* newEntity = new CLASSTYPE(); //Creator();
	newEntity->SetMeshFileName(fileName);
	AddEntity(*newEntity);
	return newEntity;
}

typedef CSingleton<EntityManager> ENTITYMANAGER;
