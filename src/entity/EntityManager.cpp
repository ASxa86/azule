#include <age/entity/EntityManager.h>
#include <algorithm>
#include <numeric>

using namespace age::core;
using namespace age::entity;

EntityManager::EntityManager(std::size_t reserve) : reserve{reserve}
{
	this->entities.reserve(this->reserve);
}

EntityManager::~EntityManager()
{
}

Entity EntityManager::create()
{
	Entity e{};

	if(this->destroyed.empty() == true)
	{
		e.id = static_cast<int>(this->entities.size());
		e.manager = this;
		this->entities.push_back(e);
	}
	else
	{
		e = this->destroyed.back();
		e.manager = this;
		this->entities[e.id] = e;
		this->destroyed.pop_back();
	}

	EventQueue::Instance().sendEvent(std::make_unique<EntityEvent>(e, EntityEvent::Type::EntityAdded));

	return e;
}

void EntityManager::destroy(Entity x)
{
	if(x.valid() == true)
	{
		// Notify handlers that the entity will be removed. We do this before
		// changing the entity so that handlers may see the entity's final state.
		EventQueue::Instance().sendEvent(std::make_unique<EntityEvent>(x, EntityEvent::Type::EntityRemoved));

		// Copy the destroyed entity with a valid id and pointer to manager before invalidating it.
		// This makes it possible to recover the entity on creation.
		this->destroyed.push_back(x);

		for(auto& pool : this->pools)
		{
			if(pool.second->test(x.id) == true)
			{
				pool.second->destroy(x.id);
			}
		}

		// Invalidate the entity within the entity pool.
		this->entities[x.id].manager = nullptr;
		this->entities[x.id].id = -1;
	}
}

bool EntityManager::valid(Entity x) const
{
	return x.id < this->entities.size() && this->entities[x.id].id != -1;
}

const std::vector<Entity>& EntityManager::getEntities() const
{
	return this->entities;
}

const std::map<std::type_index, std::unique_ptr<BasePool>>& EntityManager::getPools() const
{
	return this->pools;
}