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

	void AddEntity(Entity& entity);
	void DelEntity(unsigned int entityIndex);

private:
	std::map< unsigned int, Entity > entityMap;
};

