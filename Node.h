#include <iostream>
#include "vector"
#include "cmath"
#include "ctime"
#include "numeric"
#include <fstream>
#include <algorithm>
#include "Globals.h"


using namespace std;

class Node
{
public:
	CircleShape shape;
	int number;
	float inputValue=0;
	float outputValue=0;
	vector<Connection<Node>*> outputConnections;
	int layer = 0;

	Node(int no)
	{
		this->number = no;
		shape = CircleShape(10, 30);
		shape.setOutlineThickness(2);
		shape.setOutlineColor(Color::White);
		shape.setOrigin(shape.getGlobalBounds().width/2, shape.getGlobalBounds().height / 2);
	}
	Node() {}

	void draw(RenderWindow& window)
	{
		window.draw(shape);
	}

	void engage(bool useTangens)
	{
		if (layer != 0) {
			if(!useTangens)
				outputValue = 1. / (pow(M_E, -1. * inputValue) + 1);
			else
				outputValue = (pow(M_E, 2. * inputValue)-1) / (pow(M_E, 2. * inputValue) + 1);
		}

		for (int i = 0; i < outputConnections.size(); i++)
		{
			if (outputConnections[i]->enabled)
			{
				outputConnections[i]->toNode->inputValue += outputConnections[i]->weight * outputValue;
			}
		}
	}

	bool isConnectedTo(Node* node)
	{
		if (node->layer == layer)
			return false;

		if (node->layer < layer)
		{
			for (int i = 0; i < node->outputConnections.size(); i++)
			{
				if (node->outputConnections[i]->toNode->number == number)
					return true;
			}
		}
		else
		{
			for (int i = 0; i < outputConnections.size(); i++)
			{
				if (outputConnections[i]->toNode->number == node->number)
					return true;
			}
		}

		return false;
	}

	Node* clonePointer() {
		Node* n = new Node(number);
		n->layer = layer;
		return n;
	}
};
