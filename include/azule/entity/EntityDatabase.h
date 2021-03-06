#pragma once

#include <azule/core/Object.h>
#include <azule/export.hxx>

namespace azule
{
	class Entity;

	class AZULE_EXPORT EntityDatabase : public azule::Object
	{
	public:
		bool addEntity(std::shared_ptr<azule::Entity> x);

		template <typename T = Entity>
		[[maybe_unused]] T* addEntity()
		{
			static_assert(std::is_base_of<azule::Entity, T>::value == true, "T must derive from azule::Entity.");

			auto child = std::make_shared<T>();
			const auto p = child.get();

			if(this->addEntity(std::move(child)) == true)
			{
				return p;
			}

			return nullptr;
		}

	private:
		using Object::addChild;
	};
}
