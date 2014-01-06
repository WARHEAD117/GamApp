#include "Entity.h"

unsigned int Entity::baseFastIndex = 0;
Entity::Entity() :
	fastIndex(++baseFastIndex)
{
}


Entity::~Entity()
{
}

void Entity::OnBeginFrame()
{

}

void Entity::OnFrame()
{

}

void Entity::OnEndFrame()
{

}
