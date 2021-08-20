#include <iostream>
#include <SFML/Graphics.hpp>
#include "vector"
#include "cmath"
#include "ctime"
#include "numeric"
#include <fstream>
#include <algorithm>
#include "Globals.h"
#include <random>

using namespace std;
using namespace sf;

template<class Node> class Connection
{
public:
	Node* fromNode;
	Node* toNode;
	float weight=1;
	bool enabled = true;
	int innovation;

	Connection(Node* from, Node* to, float w, int inno)
	{
		fromNode = from;
		toNode = to;
		weight = w;
		innovation = inno;
	}

	void draw(RenderWindow& window)
	{
		Vertex shape[] = {
		Vertex(fromNode->shape.getPosition()),
		Vertex(toNode->shape.getPosition())
		};
		shape[0].color = (weight > 0 ? Color::Red : Color::Blue);
		shape[1].color = (weight > 0 ? Color::Red : Color::Blue);
		window.draw(shape,2,Lines);
	}

	void mutateWeight()
	{
		if (double(rand()) / RAND_MAX < weightMutationChance)
		{
			weight = double(rand()) / RAND_MAX * (rand() % 2 == 0 ? -1 : 1);
		}
		else
		{
			std::random_device rd{};
			std::mt19937 gen{ rd() };
			std::normal_distribution<double> d{ 0,0.2 };
			weight += d(gen);

			if (weight > 1) weight = 1;
			else if (weight < -1) weight = -1;
		}
	}

	Connection<Node>* clonePointer(Node* from, Node*  to) {
		Connection<Node>* c = new Connection<Node>(from, to, weight, innovation);
		c->enabled = enabled;
		return c;
	}
};
