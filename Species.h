#pragma once
#include <iostream>
#include "vector"
#include "cmath"
#include "ctime"
#include "numeric"
#include <fstream>
#include <algorithm>
#include "Globals.h"
#include "Player.h"

class Species
{
public:
	vector<Player*> players;
	Brain rep;
	Player* champ;
	float bestFitness=0;
	float averageFitness=0;
	int staleness = 0;

	float excessCoeff = 1;
	float weightDiffCoeff = 0.5;
	float compatibilityThreshold = 3;

	Species() {}

	Species(Player* player)
	{
		players.push_back(player);
		bestFitness = player->fitness;
		rep = player->brain;
		champ = player->clonePointer();
	}

	bool sameSpecies(Brain brain)
	{
		float compability;
		float excessAndDisjoint = getExcessDisjoint(brain,rep);
		float avgWeightDiff = averageWeightDiff(brain,rep);

		float normalizer=0;
		normalizer = (int)brain.genes.size() - 20;
		if (normalizer < 1)
			normalizer = 1;

		compability = (excessCoeff * excessAndDisjoint / normalizer) + (weightDiffCoeff * avgWeightDiff);

		return compability < compatibilityThreshold;
	}

	void addToSpecies(Player* p)
	{
		players.push_back(p);
	}

	float getExcessDisjoint(Brain brain1, Brain brain2)
	{
		float matching = 0.;
		for (int i = 0; i < brain1.genes.size(); i++)
		{
			for (int j = 0; j < brain2.genes.size(); j++)
			{
				if (brain1.genes[i]->innovation == brain2.genes[j]->innovation)
					matching++;
			}
		}

		return ((brain1.genes.size() + brain2.genes.size()) - (2 * matching));
	}

	float averageWeightDiff(Brain brain1, Brain brain2)
	{
		if (brain1.genes.size() == 0 || brain2.genes.size() == 0)
			return 0;

		float matching = 0.;
		float totalDiff = 0.;
		for (int i = 0; i < brain1.genes.size(); i++)
		{
			for (int j = 0; j < brain2.genes.size(); j++)
			{
				if (brain1.genes[i]->innovation == brain2.genes[j]->innovation) {
					matching++;
					totalDiff += abs(brain1.genes[i]->weight - brain2.genes[j]->weight);
					break;
				}
			}
		}

		if (matching == 0)
			return 100;

		return totalDiff / matching;
	}

	void sortSpecies()
	{
		vector<Player*> temp;
		for (int i = 0; i < players.size(); i++) {
			float max = 0;
			int maxIndex = 0;
			for (int j = 0; j < players.size(); j++) {
				if (players[j]->fitness > max) {
					max = players[j]->fitness;
					maxIndex = j;
				}
			}
			temp.push_back(players[maxIndex]);
			players.erase(players.begin()+maxIndex, players.begin() + maxIndex + 1);
			i--;
		}

		players = vector<Player*>(temp);

		if(players.size() == 0)
		{
			cout << "UwU" << endl;
			staleness = 200;
			return;
		}

		if (players[0]->fitness > bestFitness)
		{
			staleness = 0;
			bestFitness = players[0]->fitness;
			rep = players[0]->brain.clone();
			champ = players[0]->clonePointer();
		}
		else
			staleness++;
	}

	void setAverage()
	{
		float sum = 0;
		for (int i = 0; i < players.size(); i++)
			sum += players[i]->fitness;

		averageFitness = sum / players.size();
	}

	Player* giveMeBaby(vector<ConnectionHistory> &innovationHistory)
	{
		Player* baby;

		if (double(rand()) / RAND_MAX < noCrossoverChance)
			baby = selectPlayer();
		else
		{
			Player* parent1 = selectPlayer();
			Player* parent2 = selectPlayer();

			if (parent1->fitness > parent2->fitness)
				baby = parent1->crossover(*parent2);
			else
				baby = parent2->crossover(*parent1);
		}
		baby->brain.mutate(innovationHistory);
		return baby;
	}

	Player* selectPlayer()
	{
		float fitnessSum = 0.;
		for (int i = 0; i < players.size(); i++)
			fitnessSum += players[i]->fitness;

		float rnd;
		try {
			if ((int)fitnessSum == 0) throw exception();
			 rnd = (rand() % (int)round(fitnessSum)) + double(rand()) / RAND_MAX;
		}
		catch (exception) { rnd = fitnessSum; }
		float runningSum = 0.;

		for (int i = 0; i < players.size(); i++)
		{
			runningSum += players[i]->fitness;
			if (runningSum > rnd)
				return players[i];
		}

		return players[0];
	}

	void cull()
	{
		if (players.size() > 2)
		{
			for (int i = players.size() / 2; i < players.size(); i++)
			{
				players.erase(players.begin()+i, players.begin() + i + 1);
				i--;
			}
		}
	}

	void fitnessSharing()
	{
		for (int i = 0; i < players.size(); i++)
			players[i]->fitness /= players.size();
	}

};