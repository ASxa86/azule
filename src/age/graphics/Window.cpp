#include <age/core/Engine.h>
#include <age/core/EngineState.h>
#include <age/core/PimplImpl.h>
#include <age/core/Timer.h>
#include <age/entity/EntityManager.h>
#include <age/graphics/KeyEvent.h>
#include <age/graphics/Window.h>
#include <age/math/TransformComponent.h>
#include <SFML/Graphics.hpp>
#include <iomanip>
#include <numeric>
#include <sstream>

using namespace age::core;
using namespace age::graphics;
using namespace age::math;

class Window::Impl
{
public:
	Impl(unsigned int width, unsigned int height)
		: settings{0, 0, 0}, window{sf::VideoMode{width, height}, "AGE", sf::Style::Close | sf::Style::Resize, settings}, pixelsPerMeter{32}
	{
		this->window.setVerticalSyncEnabled(false);
		this->window.setFramerateLimit(0);

		this->font.loadFromFile("D:/age/resources/sansation.ttf");
		this->text.setFont(this->font);
		this->text.setPosition({10.0, 30.0});
	}

	sf::ContextSettings settings;
	sf::RenderWindow window;
	Timer timer;

	// Temporary until I find a generic way to handle GUI widgets.
	sf::Text text;
	sf::Font font;
	unsigned int pixelsPerMeter;
};

Window::Window(unsigned int width, unsigned int height) : RenderSystem(), pimpl(width, height)
{
}

Window::~Window()
{
}

unsigned int Window::getWidth() const
{
	return this->pimpl->window.getSize().x;
}

unsigned int Window::getHeight() const
{
	return this->pimpl->window.getSize().y;
}

void Window::setPixelsPerMeter(unsigned int x)
{
	this->pimpl->pixelsPerMeter = x;
}

unsigned int Window::getPixelsPerMeter() const
{
	return this->pimpl->pixelsPerMeter;
}

void Window::pollEvents()
{
	sf::Event e;
	const auto engine = this->getParent<Engine>();

	// This entire event loop needs to occur elsewhere so that event can be handled before this frame().
	while(this->pimpl->window.pollEvent(e) == true)
	{
		switch(e.type)
		{
			case sf::Event::EventType::Closed:
				this->pimpl->window.close();
				engine->setEngineState(EngineState::State::Exit);
				break;
			case sf::Event::EventType::KeyPressed:
				engine->sendEvent(std::make_unique<KeyEvent>(e.key.code, KeyEvent::Type::Pressed));
				break;
			case sf::Event::EventType::KeyReleased:
				engine->sendEvent(std::make_unique<KeyEvent>(e.key.code, KeyEvent::Type::Released));
				break;
			case sf::Event::EventType::Resized:
				// engine->sendEvent(std::make_unique<ResizeEvent>());
				break;
			default:
				break;
		}
	}
}

void Window::frame(std::chrono::microseconds /*x*/)
{
	if(this->pimpl->window.isOpen() == true)
	{
		static double elapsed = 0.0;
		const auto delta = std::chrono::duration_cast<age::core::seconds>(this->pimpl->timer.reset());
		this->pimpl->window.clear();
		elapsed += delta.count();

		const auto entities = this->getEntities();

		for(const auto& entity : entities)
		{
			const auto drawables = entity->getChildren<DrawableComponent>();

			for(const auto& drawable : drawables)
			{
				drawable->draw(this->pimpl->window, this->pimpl->pixelsPerMeter);
			}
		}

		if(elapsed >= 0.5)
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(1) << 1.0 / delta.count();
			this->pimpl->text.setString("FPS: " + ss.str());
			elapsed = 0.0;
		}

		this->pimpl->window.draw(this->pimpl->text);
		this->pimpl->window.display();
	}
}
