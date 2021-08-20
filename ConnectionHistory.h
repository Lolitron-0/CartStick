#pragma once
#include <iostream>
#include "vector"
#include "cmath"
#include "ctime"
#include "numeric"
#include <fstream>
#include <algorithm>
#include "Globals.h"
#include <random>

using namespace std;

class ConnectionHistory
{
public:
	int fromNode;
	int toNode;
	int innovationNumber;

	vector<int> innovationNumbers;

	ConnectionHistory(int from, int to, int inno, vector<int> innovationNos)
	{
		fromNode = from;
		toNode = to;
		innovationNumber = inno;
		innovationNumbers=vector<int>(innovationNos);
	}

	template<class Brain, class Node> bool match(Brain brain, Node from, Node to)
	{
		if (brain->genes.size() == innovationNumbers.size())
		{
			if (from.number == fromNode && to.number == toNode)
			{
				for (int i = 0; i < brain->genes.size(); i++)
				{
					if (!contains(innovationNumbers, brain->genes[i]->innovation))
						return false;
				}

				return true;
			}
		}
		return false;
	}

};