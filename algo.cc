#include <iostream>
#include <vector>
#include <cmath>

// Individual
struct individual {
	int groupNumber;  //maybe not needed
	int personalBest; //f_best,i in algo
	int oxygen;       //O_x_i,j in algo
	int direction;    //v_i,j in algo
	int solution;     //x_i,j in algo
	int pbSolution;   //p_i in algo
};

struct group {
	std::vector<individual> grp;
	float QEF;
	float joinProb;
};

int f (int solution) {
	return 0;
}

int main ()
{
	int noIndividuals = 10;
	int noGroups = 2;
	int globalBest = 0;
	int curGen = 0;
	int maxGen = 10;
	int prevSolution = 0;
	float QEFSum = 0.0;
	float sum = 0.0;
	float random;
	
	// Init individual
	int groupNumber;
	int pB;
	int ox;
	int v;
	int X;
	
	// Init groups
	std::vector<individual> grp;
	std::vector<group> groups;
	for (int g = 0; g < noGroups; g++) {
		group G = {grp, 0.0, (float)(1.0/(float)noGroups)};
		groups.push_back(G);
	}
	
	// Init	individuals and groups
	for (int i = 0; i < noIndividuals; i++) 
	{
		groupNumber = (i % noGroups);
		pB = 0; // f(x_{i,j})
		ox = 0; // f(x_{i,j})*abs(x_{i,j})
		v = 0;  // U(-v_max1, v_max1)
		X = 0; // Solution
		individual I = {groupNumber, pB, ox, v, X};
		groups[groupNumber].grp.push_back(I);
	}
	
	// Init global best
	if (noGroups > 0)
		globalBest = groups[0].grp[0].solution;
	
	// Algorithm
	while (curGen < maxGen) // Run for set amount of generations maxGen
	{
		for (uint j = 0; j < groups.size(); j++) 
		{
			for (uint k = 0; k < groups[j].grp.size(); k++)
			{
				while ((groups[j].grp[k].oxygen > 0) || (f(groups[j].grp[k].solution) > groups[j].grp[k].personalBest))   
				{
					if (f(groups[j].grp[k].solution) > groups[j].grp[k].personalBest)									
					{
						groups[j].grp[k].personalBest = f(groups[j].grp[k].solution);
						groups[j].grp[k].pbSolution = groups[j].grp[k].solution;
						if (f(groups[j].grp[k].solution) > f(globalBest))
						{
							globalBest = groups[j].grp[k].solution;
						}
					}
					groups[j].grp[k].direction = groups[j].grp[k].oxygen * /*TODO: uniform U(0,1)*/ 1 * abs(groups[j].grp[k].pbSolution - groups[j].grp[k].solution); // Calculate direction
					groups[j].grp[k].solution += groups[j].grp[k].direction; // Update position
					groups[j].grp[k].oxygen += ((f(groups[j].grp[k].solution) - f(prevSolution)) * abs(prevSolution + groups[j].grp[k].solution)); // Update oxygen
					prevSolution = groups[j].grp[k].solution;
				}
				groups[j].QEF += groups[j].grp[k].oxygen; // Update QEF
			}
			QEFSum = 0.0;
			for (uint l = 0; l < groups.size(); l++)
			{
				QEFSum += groups[l].QEF;
			} 
			groups[j].joinProb = groups[j].QEF/QEFSum;
		}
		
		for (uint j = 0; j < groups.size(); j++) // For each individual...
		{
			if (groups[j].grp.size() == 0) 
			{
				/*groups[j].QEF = 0;
				groups[j].joinProb = 0;*/
				groups.erase(groups.begin() + j);
				j--;
			}
			else 
			{
				for (uint k = 0; k < groups[j].grp.size(); k++) // ...in a group 
				{
					sum = 0.0;
					random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
					for (uint m = 0; m < groups.size(); m++) 
					{
						sum += groups[m].joinProb;
						if (random <= sum) // Redistribute according to probability
						{
							groups[m].grp.push_back(groups[j].grp[k]); // Add individual to new group
							groups[j].grp.erase(grp.begin() + k); // Remove from old group
							break;
						}
					}
				}
			}
		}
		curGen++;
	}

	return 0;
}
