#pragma once
#include <iostream>
#include "vector"
#include "cmath"
#include "ctime"
#include "numeric"
#include <fstream>
#include <algorithm>
#include "Globals.h"
#include "Connection.h"
#include "Node.h"
#include "ConnectionHistory.h"


using namespace std;

class Brain
{
public:
	vector<Connection<Node>*> genes;
	vector<Node*> nodes;

	int inputs;
	int outputs;
	int layers = 2;
	int nextNode = 0;
	int biasNode;

	vector<Node*> network;

	Brain(int in, int out)
	{
		inputs = in;
		outputs = out;

		for (int i = 0; i < inputs; i++)
		{
			nodes.push_back( new Node(i));
			nextNode++;
			nodes[i]->layer = 0;
		}

		for (int i = 0; i < outputs; i++)
		{
			nodes.push_back(new Node(nextNode++));
			nodes[i+inputs]->layer = 1;
		}

		nodes.push_back(new Node(nextNode));
		biasNode = nextNode++;
		nodes[biasNode]->layer = 0;

	}

	Brain(int in, int out, bool crossover)
	{
		inputs = in;
		outputs = out;
	}

	Brain() {}

	Node* getNode(int nodeNum)
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i]->number == nodeNum)
				return nodes[i];
		}
		return NULL;
	}

	void connectNodes()
	{
		for (int i = 0; i < nodes.size(); i++)
		{
			nodes[i]->outputConnections.resize(0);
			nodes[i]->outputConnections.shrink_to_fit();
		}

		for (int i = 0; i < genes.size(); i++)
		{
			genes[i]->fromNode->outputConnections.push_back(genes[i]);
		}
	}

	/// <summary>
	/// </summary>
	/// <param name="inputValues">- Vector of inputs</param>
	/// <param name="useTangens">- If false using sigmoid</param>
	/// <returns>Vector of outputs through the network</returns>
	vector<float> feedForward(vector<float> inputValues, bool useTangens)
	{
		for (int i = 0; i < inputs; i++)
		{
			nodes[i]->outputValue = inputValues[i];
		}
		nodes[biasNode]->outputValue = 1;

		for (int i = 0; i < network.size(); i++)
			network[i]->engage(useTangens);

		vector<float> outs(outputs);
		for (int i = 0; i < outputs; i++) {
			outs[i] = nodes[inputs + i]->outputValue;
		}

		for (int i = 0; i < nodes.size(); i++)
			nodes[i]->inputValue = 0;

		return outs;
	}

	void draw(RenderWindow& window)
	{
		for (int i = 0; i< nodes.size(); i++)
		{
			nodes[i]->shape.setPosition(200+100*nodes[i]->layer,50+(nodes[i]->number)*30);
			nodes[i]->draw(window);
		}

		for (int i = 0; i < genes.size(); i++)
		{
			genes[i]->draw(window);
		}
	}

	void generateNetwork()
	{
		connectNodes();
		network = vector<Node*>();

		for (int i = 0; i < layers;i++)
		{
			for (int j = 0; j < nodes.size(); j++)
			{
				if (nodes[i]->layer == i)
					network.push_back(nodes[j]);
			}
		}
	}

	void addNode(vector<ConnectionHistory> &innovationHistory)
	{
		if (genes.size() == 0)
		{
			addConnection(innovationHistory);
			return;
		}
		int randomConnection;
		do {
			randomConnection = rand() % genes.size();
		} while (genes[randomConnection]->fromNode->number == nodes[biasNode]->number && genes.size() != 1 && !allGenesFromBias());

		genes[randomConnection]->enabled = false;

		int newNodeNo = nextNode++;
		nodes.push_back(new Node(newNodeNo));

		//from prev to new
		int connectionInnovationNumber = getInnovationNumber(innovationHistory,*genes[randomConnection]->fromNode,*getNode(newNodeNo));
		genes.push_back(new Connection<Node>(genes[randomConnection]->fromNode,getNode(newNodeNo),1,connectionInnovationNumber));
		
		//from new to next
		connectionInnovationNumber = getInnovationNumber(innovationHistory,*getNode(newNodeNo),*genes[randomConnection]->toNode);
		genes.push_back(new Connection<Node>(getNode(newNodeNo), genes[randomConnection]->toNode, genes[randomConnection]->weight, connectionInnovationNumber));
		getNode(newNodeNo)->layer = genes[randomConnection]->fromNode->layer + 1;

		//from bias to new
		connectionInnovationNumber = getInnovationNumber(innovationHistory, *nodes[biasNode], *getNode(newNodeNo));
		genes.push_back(new Connection<Node>(nodes[biasNode],getNode(newNodeNo),0,connectionInnovationNumber));


		if(getNode(newNodeNo)->layer==genes[randomConnection]->toNode->layer)
		{
			for (int i = 0; i < nodes.size() - 1; i++)
			{
				if (nodes[i]->layer >= getNode(newNodeNo)->layer)
				{
					nodes[i]->layer++;
				}
			}
			layers++;
		}

		connectNodes();

	}

	void addConnection(vector<ConnectionHistory> &inovationHistory)
	{
		if (fullyConnected())
			return;

		int randomNode1,randomNode2;
		do {
			randomNode1 = rand() % nodes.size();
			randomNode2 = rand() % nodes.size();
		} while (randomConnectionsAreBad(randomNode1, randomNode2));

		int temp;

		if (nodes[randomNode1]->layer > nodes[randomNode2]->layer)
		{
			temp = randomNode1;
			randomNode1 = randomNode2;
			randomNode2 = temp;
		}

		int connectionInnovationNumber = getInnovationNumber(inovationHistory,*nodes[randomNode1],*nodes[randomNode2]);

		genes.push_back(new Connection<Node>(nodes[randomNode1],nodes[randomNode2], double(rand()) / RAND_MAX * (rand() % 2 == 0 ? -1 : 1),connectionInnovationNumber));
		connectNodes();
	}

	bool allGenesFromBias()
	{
		for (int i = 0; i < genes.size(); i++)
		{
			if (genes[i]->fromNode->number != nodes[biasNode]->number)
				return false;
		}
		return true;
	}

	bool randomConnectionsAreBad(int r1, int r2)
	{
		if (nodes[r1]->layer == nodes[r2]->layer) return true;
		if (nodes[r1]->isConnectedTo(nodes[r2])) return true;

		return false;
	}

	int getInnovationNumber(vector<ConnectionHistory> &innovationHistory, Node from, Node to)
	{
		bool isNew = true;
		int connectionInnovationNumber = nextConnectionNo;
		for (int i = 0; i < innovationHistory.size(); i++)
		{
			if (innovationHistory[i].match(this, from, to))
			{
				isNew = false;
				connectionInnovationNumber = innovationHistory[i].innovationNumber;
				break;
			}
		}

		if (isNew)
		{
			vector<int> innoNumbers;
			for (int i = 0; i < genes.size(); i++)
				innoNumbers.push_back(genes[i]->innovation);

			innovationHistory.push_back(ConnectionHistory(from.number,to.number,connectionInnovationNumber,innoNumbers));
			nextConnectionNo++;
		}

		return connectionInnovationNumber;
	}

	bool fullyConnected()
	{
		int maxConnections = 0;
		vector<int> nodesInLayers(layers);

		for (int i = 0; i < nodes.size(); i++)
			nodesInLayers[nodes[i]->layer]++;

		for (int i = 0; i < layers - 1; i++)
		{
			int nodesInFront = 0;
			for (int j = i + 1; j < layers; j++)
				nodesInFront += nodesInLayers[j];

			maxConnections += nodesInFront * nodesInLayers[i];
		}

		return maxConnections == genes.size();
	}

	void mutate(vector<ConnectionHistory> &innovationHistory)
	{
		if (genes.size() == 0)
			addConnection(innovationHistory);

		float rnd = double(rand()) / RAND_MAX;
		if (rnd < mutationChance)
			for (int i = 0; i < genes.size(); i++)
				genes[i]->mutateWeight();

		rnd = double(rand()) / RAND_MAX;
		if (rnd < mutationAddConnectionChance)
			addConnection(innovationHistory);

		rnd = double(rand()) / RAND_MAX;
		if (rnd < mutationAddNodeChance)
			addNode(innovationHistory);
	}

	Brain* crossover(Brain parent2)
	{
		Brain* child= new Brain(inputs,outputs,true);

		child->genes.clear();
		child->genes.shrink_to_fit();
		child->nodes.clear();
		child->nodes.shrink_to_fit();
		child->layers = layers;
		child->nextNode = nextNode;
		child->biasNode = biasNode;

		vector<Connection<Node>> childGenes;
		vector<bool> isEnabled;

		for (int i = 0; i < genes.size(); i++)
		{
			bool setEnabled=true;

			int parentGene = matchingGene(parent2, genes[i]->innovation);

			if (parentGene != -1)
			{
				if (!genes[i]->enabled || !parent2.genes[parentGene]->enabled)
				{
					if(double(rand()) / RAND_MAX<disablingGeneChance)
						setEnabled = false;
				}

				if (double(rand()) / RAND_MAX < 0.5)
					childGenes.push_back(*genes[i]);
				else
					childGenes.push_back(*parent2.genes[i]);
			}
			else
			{
				childGenes.push_back(*genes[i]);
				setEnabled = genes[i]->enabled;
			}

			isEnabled.push_back(setEnabled);
		}

		for (int i = 0; i < nodes.size(); i++)
			child->nodes.push_back(nodes[i]->clonePointer());
	
		for (int i = 0; i < childGenes.size(); i++)
		{
			child->genes.push_back(childGenes[i].clonePointer(child->getNode(childGenes[i].fromNode->number), child->getNode(childGenes[i].toNode->number)));
			child->genes[i]->enabled = isEnabled[i];
		}

		child->connectNodes();

		return child;
	}

	int matchingGene(Brain parent2,int innovationNumber)
	{
		for (int i = 0; i < parent2.genes.size(); i++)
		{
			if (parent2.genes[i]->innovation == innovationNumber)
				return i;
		}

		return -1;
	}

	Brain clone() {

		Brain clone(inputs, outputs, true);

		for (int i = 0; i < nodes.size(); i++) {
			clone.nodes.push_back(nodes[i]->clonePointer());
		}


		for (int i = 0; i < genes.size(); i++) {
			clone.genes.push_back(genes[i]->clonePointer(clone.getNode(genes[i]->fromNode->number), clone.getNode(genes[i]->toNode->number)));
		}

		clone.layers = layers;
		clone.nextNode = nextNode;
		clone.biasNode = biasNode;
		clone.connectNodes();

		return clone;
	}

};