#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <Box2D\Box2D.h>
#include <iostream>
#include "cmath"
#include "ctime"
#include "numeric"
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include "vector"
#include "Globals.h"
#include "Player.h"
#include "Population.h"

using namespace sf;
using namespace std;


class ContactFilter : public b2ContactFilter
{
public:
	bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
	{
		//const b2Filter& filterA = fixtureA->GetFilterData();
		//const b2Filter& filterB = fixtureB->GetFilterData();

		//if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0)
		//{
		//	return filterA.groupIndex > 0;
		//}

		//bool collideA = (filterA.maskBits & filterB.categoryBits) != 0;
		//bool collideB = (filterA.categoryBits & filterB.maskBits) != 0;
		//bool collide = collideA && collideB;
		FixData* a = (FixData*)fixtureA->GetUserData();
		FixData* b = (FixData*)fixtureB->GetUserData();
		if (a->data==b->data-1 || a->data == b->data + 1 || a->data==0 || b->data==0)
			return true;
		return false;
	}
};


int main()
{
	srand(time(0));

	ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Lesson 1. kychka-pc.ru", Style::Default, settings);
	window.setFramerateLimit(60);

	b2Vec2 gravity(0, 15);
	b2World world(gravity);
	world.SetAllowSleeping(true);
	ContactFilter f;
	world.SetContactFilter(&f);

	b2BodyDef groundDef; groundDef.position.Set(0,100);
	b2Body* ground = world.CreateBody(&groundDef);
	b2PolygonShape groundShape; groundShape.SetAsBox(500,10);
	ground->CreateFixture(&groundShape,0.);
	ground->GetFixtureList()->SetUserData(FixData::initFixData(0));
	RectangleShape floor = RectangleShape(Vector2f(500 * SCALE, 100 * SCALE)); 
	floor.setPosition(ground->GetPosition().x*SCALE, ground->GetPosition().y * SCALE-10*SCALE);
	floor.setFillColor(Color::Transparent); floor.setOutlineThickness(1);

	Vertex left[] = {
		Vertex(Vector2f(100,-10)),
		Vertex(Vector2f(100,1000)),
	};

	Vertex right[] = {
		Vertex(Vector2f(1000,-10)),
		Vertex(Vector2f(1000,1000)),
	};

	int generation = 0, iteration = 0;

	Clock clock;
	Clock stopTime;
	double time;

	Population population(100, &world);

	while (window.isOpen())
	{
		time = clock.getElapsedTime().asMicroseconds();
		time /= 800;
		clock.restart();

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		//population.pop[0]->buttonsPressed(time);

		world.Step(timeStep, velocityIterations, positionIterations);

		window.clear();
		window.draw(floor);
		window.draw(left,2,Lines); window.draw(right, 2, Lines);
		if (!population.done())
		{
			population.updateAlive(time,window,ground);
			population.getBestOfGen()->brain.draw(window);
		}
		else
		{
			population.naturalSelection();
			time = 0;
			clock.restart();
		}
		//population.pop[0]->update(time);
		//population.pop[0]->draw(window);
		window.display();

	}

	return 0;
}