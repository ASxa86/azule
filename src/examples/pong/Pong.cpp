#include <examples/pong/Pong.h>

#include <age/audio/AudioEvent.h>
#include <age/audio/AudioSystem.h>
#include <age/core/Engine.h>
#include <age/core/EngineState.h>
#include <age/core/EventQueue.h>
#include <age/core/PimplImpl.h>
#include <age/entity/EntityManager.h>
#include <age/graphics/DrawableSystem.h>
#include <age/graphics/InputComponent.h>
#include <age/graphics/PlayerInputSystem.h>
#include <age/graphics/Window.h>
#include <age/math/TransformComponent.h>
#include <age/physics/BoxCollisionComponent.h>
#include <age/physics/CircleCollisionComponent.h>
#include <age/physics/CollisionEvent.h>
#include <age/physics/KinematicComponent.h>
#include <age/physics/PhysicsSystem.h>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

using namespace age::audio;
using namespace age::core;
using namespace age::entity;
using namespace age::math;
using namespace age::graphics;
using namespace age::physics;
using namespace age::pong;

struct Pong::Impl
{
	Impl()
	{
	}

	std::shared_ptr<Engine> engine{std::make_shared<Engine>()};
	sf::SoundBuffer soundBuffer;
	sf::Sound sound{soundBuffer};
};

Pong::Pong()
{
	auto window = std::make_shared<Window>();
	window->addChild(std::make_shared<DrawableSystem>());
	this->pimpl->engine->addChild(window);
	auto manager = std::make_shared<EntityManager>();
	this->pimpl->engine->addChild(manager);
	this->pimpl->engine->addChild(std::make_shared<PlayerInputSystem>());
	this->pimpl->engine->addChild(std::make_shared<PhysicsSystem>());
	this->pimpl->engine->addChild(std::make_shared<AudioSystem>());

	// Player 1
	auto paddle = manager->create();
	auto rec = std::make_shared<sf::RectangleShape>();
	rec->setSize({1.0, 3.0});
	rec->setFillColor(sf::Color::White);
	rec->setOrigin(rec->getSize().x / 2.0f, rec->getSize().y / 2.0f);
	paddle.addComponent<std::shared_ptr<sf::Drawable>>(rec);
	auto& k = paddle.addComponent<KinematicComponent>();
	k.setBodyType(KinematicComponent::BodyType::Kinematic);
	auto& t = paddle.addComponent<TransformComponent>();
	t.setPosition({5, 10});
	auto& pc = paddle.addComponent<BoxCollisionComponent>();
	pc.setSize({rec->getSize().x, rec->getSize().y});

	auto& input = paddle.addComponent<InputComponent>();
	input.addKeyBinding(sf::Keyboard::Key::Up, [](Entity e, bool isPressed) {
		auto& t = e.getComponent<KinematicComponent>();
		auto v = t.getVelocity();
		v.setY(isPressed == true ? -20.0 : 0.0);
		t.setVelocity(v);
	});

	input.addKeyBinding(sf::Keyboard::Key::Down, [](Entity e, bool isPressed) {
		auto& t = e.getComponent<KinematicComponent>();
		auto v = t.getVelocity();
		v.setY(isPressed == true ? 20.0 : 0.0);
		t.setVelocity(v);
	});

	// Player 2
	auto paddle2 = manager->create();
	auto rec2 = std::make_shared<sf::RectangleShape>();
	rec2->setSize({1.0, 3.0});
	rec2->setFillColor(sf::Color::White);
	rec2->setOrigin(rec2->getSize().x / 2, rec2->getSize().y / 2);
	paddle2.addComponent<std::shared_ptr<sf::Drawable>>(rec2);
	paddle2.addComponent<KinematicComponent>();
	auto& t2 = paddle2.addComponent<TransformComponent>();
	t2.setPosition({35, 10});
	auto& c = paddle2.addComponent<BoxCollisionComponent>();
	c.setSize({rec2->getSize().x, rec2->getSize().y});

	// Ball
	auto ball = manager->create();
	auto circle = std::make_shared<sf::CircleShape>();
	circle->setRadius(0.5f);
	circle->setFillColor(sf::Color::White);
	circle->setOrigin(circle->getRadius(), circle->getRadius());
	ball.addComponent<std::shared_ptr<sf::Drawable>>(circle);
	auto& kb = ball.addComponent<KinematicComponent>();
	kb.setVelocity({5.0, 0.0});
	kb.setBodyType(KinematicComponent::BodyType::Dynamic);
	auto& p = ball.addComponent<TransformComponent>();
	p.setPosition({10, 10});
	auto& cb = ball.addComponent<CircleCollisionComponent>();
	cb.setRadius(circle->getRadius());

	this->pimpl->soundBuffer.loadFromFile("C:/age/data/audio/ball.wav");
	ball.addComponent<sf::Sound>(this->pimpl->soundBuffer);

	EventQueue::Instance().addEventHandler([this, ball, &kb](Event* e) {
		auto evt = dynamic_cast<CollisionEvent*>(e);

		if(evt != nullptr)
		{
			const auto& entities = evt->getEntities();

			for(const auto& entity : entities)
			{
				if(entity == ball)
				{
					EventQueue::Instance().sendEvent(std::make_unique<AudioEvent>(this->pimpl->soundBuffer));
					auto v = kb.getVelocity();
					v.setX(-v.getX());
					kb.setVelocity(v);
				}
			}
		}
	});

	this->pimpl->engine->setEngineState(EngineState::State::Initialize);
}

Pong::~Pong()
{
}

int Pong::run()
{
	while(this->pimpl->engine->getEngineState().getState() < EngineState::State::Exit)
	{
		this->pimpl->engine->frame();
	}

	return 0;
}
