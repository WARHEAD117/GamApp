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
	Entity* CreateEntityFromXFile(std::string fileName);
	void AddEntity(Entity& entity);
	void DelEntity(unsigned int entityIndex);

private:
	Entity* Creator();
	std::map< unsigned int, Entity > entityMap;
};

typedef CSingleton<EntityManager> ENTITYMANAGER;
