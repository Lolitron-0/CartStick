#pragma once
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
#include "Brain.h"


using namespace sf;
using namespace std;

float startangle=5;

class FixData
{
public:
	int data;

	static FixData* initFixData(int d)
	{
		FixData* ret = new FixData();
		ret->data = d;
		return ret;
	}
};

class Player
{
public:
	b2BodyDef def;
	b2PolygonShape poly;
	b2Body* cart;
	b2Body* stick;
	b2FixtureDef fixDef;
	b2World* world;

	ConvexShape cartShape, stickShape;
	int secs = 5000;


	Brain brain;
	int inputs = 3, outputs = 1;
	vector<float> vision;
	vector<float> decision;

	Clock lifetime;
	bool alive=true;
	float score;
	float fitness;

	//------------------------------------------------------------------------

	void fitFunc() 
	{
		fitness = lifetime.getElapsedTime().asSeconds();
	}

	//------------------------------------------------------------------------

	void draw(RenderWindow& window)
	{
		window.draw(cartShape);
		window.draw(stickShape);
	}

	//---------------------------------------------------------------------------

	void update(double time,b2Body* ground) 
	{
		if (lifetime.getElapsedTime().asMilliseconds() >= secs)
		{
			secs += 5000;
			float xRnd = 150* (rand() % 2 == 0 ? 1 : -1) * (double(rand()) / RAND_MAX) * time;
			stick->ApplyForce(b2Vec2(xRnd,0),stick->GetWorldCenter(),true);
		}

		cartShape.setPosition(cart->GetPosition().x*SCALE,cart->GetPosition().y*SCALE);
		cartShape.setRotation(cart->GetAngle() * DEG);
		stickShape.setPosition(stick->GetPosition().x*SCALE,stick->GetPosition().y*SCALE);
		stickShape.setRotation(stick->GetAngle()*DEG);

		if (abs(round(stick->GetAngle() * DEG)) == 90 ||
			cartShape.getPosition().x + cartShape.getGlobalBounds().width > 1000 ||
			cartShape.getPosition().x < 100)
			alive = false;

		b2Transform xfA = stick->GetTransform(), xfB = ground->GetTransform();
		bool overlapInner = b2TestOverlap(stick->GetFixtureList()->GetShape(), 0, ground->GetFixtureList()->GetShape(), 0, xfA, xfB);
		if (overlapInner) alive = false;

	}

	//---------------------------------------------------------------

	void look() 
	{
		vision = vector<float>(inputs);

		if (stick->GetAngle() != 0)
			vision[0] = 1. / stick->GetAngle();
		else
			vision[0] = 0;

		if (vision[0] > 1) vision[0] = 1;

		vision[1] = 1./(cartShape.getPosition().x - 100.);
		vision[2] = 1. / (1000. - (cartShape.getPosition().x + cartShape.getGlobalBounds().width));
	}
	
	//------------------------------------------

	void think(double time) 
	{
		decision = brain.feedForward(vision,true);

		if (decision[0] > 0)
			right(time);
		else
			left(time);
	}

	//----------------------------------------------------------------------------

	void reset()
	{
		lifetime.restart();
		alive = true;
		cart->SetTransform(b2Vec2(50, 70),0);
		stick->SetTransform(b2Vec2(51, 70),startangle/DEG);
		secs = 5000;
	}

	//--------------------------------------------------------------------------

	Player* crossover(Player parent) 
	{
		Player* child = new Player(world);
		child->brain = *brain.crossover(parent.brain);
		child->brain.generateNetwork();
		return child;
	}

	//------------------------------------------------------------------------

	void buttonsPressed(double time)
	{
		if (Keyboard::isKeyPressed(Keyboard::D))
			cart->ApplyForce(b2Vec2(10*time,0), cart->GetWorldCenter(), true);
		if (Keyboard::isKeyPressed(Keyboard::A))
			cart->ApplyForce(b2Vec2(-10 * time, 0), cart->GetWorldCenter(), true);
	}

	//-------------------------------------------------------------------------

	void right(double time)
	{
		cart->ApplyForce(b2Vec2(10 * time, 0), cart->GetWorldCenter(), true);
	}

	void left(double time)
	{
		cart->ApplyForce(b2Vec2(-10 * time, 0), cart->GetWorldCenter(), true);
	}

	//---------------------------------------------------------------------------

	Player(b2World* world)
	{
		this->world = world;

		b2Vec2 vertices[5];

		def.type = b2_dynamicBody;
		fixDef.density = 0.1;
		fixDef.friction = 1;

		def.position.Set(50, 70);
		cart = world->CreateBody(&def);
		vertices[0] = b2Vec2(10, 15); vertices[1] = b2Vec2(20, 15); vertices[2] = b2Vec2(20, 22); vertices[3] = b2Vec2(10, 22);
		poly.Set(vertices, 4);
		fixDef.shape = &poly;
		cart->CreateFixture(&fixDef);
		cartShape = ConvexShape(4);
		for (int i = 0; i < 4; i++)
			cartShape.setPoint(i,Vector2f(vertices[i].x*SCALE, vertices[i].y*SCALE));

		fixDef.restitution = 0;
		fixDef.density = 0.15;
		fixDef.friction = 1;
		def.position.Set(51, 70);
		stick = world->CreateBody(&def);
		vertices[0] = b2Vec2(13, -10); vertices[1] = b2Vec2(16, -10); vertices[2] = b2Vec2(16, 10); vertices[3] = b2Vec2(14.5, 12); vertices[4] = b2Vec2(13, 10);
		poly.Set(vertices, 5);
		fixDef.shape = &poly;
		stick->CreateFixture(&fixDef);
		stickShape = ConvexShape(5);
		for (int i = 0; i < 5; i++)
			stickShape.setPoint(i, Vector2f(vertices[i].x*SCALE, vertices[i].y*SCALE));
		stick->SetTransform(def.position, startangle/DEG);

		cart->GetFixtureList()->SetUserData(FixData::initFixData(dataCounter++));
		stick->GetFixtureList()->SetUserData(FixData::initFixData(dataCounter)); dataCounter += 2;


		cartShape.setFillColor(Color::Transparent); cartShape.setOutlineThickness(2); cartShape.setOutlineColor(Color::Green);
		stickShape.setFillColor(Color::Transparent); stickShape.setOutlineThickness(2); stickShape.setOutlineColor(Color::Red);

		brain = Brain(inputs, outputs);
	}

	Player clone()
	{
		Player clone = Player(world);
		clone.brain = brain.clone();
		clone.fitness = fitness;
		clone.brain.generateNetwork();
		return clone;
	}

	Player* clonePointer()
	{
		Player* clone = new Player(world);
		clone->brain = brain.clone();
		clone->fitness = fitness;
		clone->brain.generateNetwork();
		return clone;
	}
};
