#include <age/entity/Entity.h>
#include <pong/PaddleAIComponent.h>

using namespace age::entity;
using namespace age::pong;

PaddleAIComponent::PaddleAIComponent() : ball{}
{
}

PaddleAIComponent::~PaddleAIComponent()
{
}

void PaddleAIComponent::setBall(Entity /*x*/)
{
	// this->ball = x;
}

const Entity& PaddleAIComponent::getBall() const
{
	return this->ball;
}