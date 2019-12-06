#include "../../src/Template/Experiments/IOHprofiler_experimenter.hpp"
#include <algorithm>
#include <functional>

IOHprofiler_random random_generator(1);
static int budget_scale = 100;

std::vector<int> Initialization(int dimension) {
  std::vector<int> x;
  x.reserve(dimension);
  for (int i = 0; i != dimension; ++i) {
      x.push_back((random_generator.IOHprofiler_uniform_rand() * 2));
  }
  return x;
};

std::vector<double> InitializationDouble(int dimension) {
  std::vector<double> x;
  x.reserve(dimension);
  for (int i = 0; i != dimension; ++i) {
      x.push_back((random_generator.IOHprofiler_uniform_rand() * 2));
  }
  return x;
};

std::vector<double> calculateDirection(double ox, std::vector<double> pB, std::vector<double> solution){
    std::vector<double> v;
    v.reserve(solution.size());
    for(int i = 0; i < solution.size(); i++){
        v[i] = ox * random_generator.IOHprofiler_uniform_rand() * std::abs(pB[i] - solution[i]);
    }
    return v;
}

double vectorDistance(std::vector<double> a, std::vector<double> b){
    double result = 0.0;
    for(int i = 0; i < a.size(); i++){
        double distance = a[i] - b[i];
        result += distance * distance;
    }
    return std::sqrt(result);
}

double zeroVectorDistance(std::vector<double> a){
    std::vector<double> zero;
    zero.reserve(a.size());
    std::fill(zero.begin(), zero.end(), 0);
    return vectorDistance(a, zero);
}



// Individual
struct individual {
	double personalBest; //f_best,i in algo
	double oxygen;       //O_x_i,j in algo
	std::vector<double> direction;    //v_i,j in algo
	std::vector<double> solution;     //x_i,j in algo
	std::vector<double> pbSolution;   //p_i in algo
};

struct group {
	std::vector<individual> grp;
	float QEF;
	float joinProb;
};

void penguinAlgorithm (std::shared_ptr<IOHprofiler_problem<double> > problem, std::shared_ptr<IOHprofiler_csv_logger> logger)
{
	int noIndividuals = 10;
	int noGroups = 2;
	std::vector<double> globalBest;
	int curGen = 0;
	int maxGen = 10;
	std::vector<double> prevSolution;
	float QEFSum = 0.0;
	float sum = 0.0;
	float random;

	// Init individual
	double pB;
	double ox;
	std::vector<double> v;
	std::vector<double> X;

	// Init groups
	std::vector<individual> grp;
	std::vector<group> groups;
	for (int g = 0; g < noGroups; g++) {
		group G = {grp, 0.0, (float)(1.0/(float)noGroups)};
		groups.push_back(G);
	}

	// Init	individuals and groups
	for (int i = 0; i < problem->IOHprofiler_get_number_of_variables(); i++)
	{
		v = InitializationDouble(problem->IOHprofiler_get_number_of_variables()); // U(-v_max1, v_max1)
		X = InitializationDouble(problem->IOHprofiler_get_number_of_variables());   //random_generator.IOHprofiler_uniform_rand() * 10 - 5; // x_{i,j}
		        logger->write_line(problem->loggerInfo());
		pB = problem->evaluate(X); // f(x_{i,j})
		ox = pB*zeroVectorDistance(X); // f(x_{i,j})*abs(x_{i,j}) //TODO: abs(x) gaat niet werken hier
		individual I = {pB, ox, v, X};
		groups[i % noGroups].grp.push_back(I);
	}



	// Init global best solution
	if (noGroups > 0)
		globalBest = groups[0].grp[0].solution;

	// Algorithm
	while (curGen < maxGen && !problem->IOHprofiler_hit_optimal()) // Run for set amount of generations maxGen
	{
		for (uint j = 0; j < groups.size(); j++)
		{
			for (uint k = 0; k < groups[j].grp.size(); k++)
			{
                prevSolution = groups[j].grp[k].solution; // make current solution prev solution at the start of evaluation for each individual
				while ((groups[j].grp[k].oxygen > 0) || (problem->evaluate(groups[j].grp[k].solution) > groups[j].grp[k].personalBest)) // zoek zolang oxygen >0 en geen kleinere oplossing
				{
					if (problem->evaluate(groups[j].grp[k].solution) < groups[j].grp[k].personalBest) // < want minimization
					{
						groups[j].grp[k].personalBest = problem->evaluate(groups[j].grp[k].solution);
						groups[j].grp[k].pbSolution = groups[j].grp[k].solution;
						if (problem->evaluate(groups[j].grp[k].solution) < problem->evaluate(globalBest)) // < want minimization
						{
							globalBest = groups[j].grp[k].solution;
						}
					}
					//groups[j].grp[k].direction = groups[j].grp[k].oxygen * /*TODO: uniform U(0,1)*/ (int)(random_generator.IOHprofiler_uniform_rand()) * abs(groups[j].grp[k].pbSolution - groups[j].grp[k].solution); // Calculate direction
					groups[j].grp[k].direction = calculateDirection(groups[j].grp[k].oxygen, groups[j].grp[k].pbSolution, groups[j].grp[k].solution);
					//groups[j].grp[k].solution += groups[j].grp[k].direction; // Update position
					std::transform(groups[j].grp[k].solution.begin(), groups[j].grp[k].solution.end(), groups[j].grp[k].direction.begin(), groups[j].grp[k].solution.begin(), std::plus<int>()); // updat pos
					groups[j].grp[k].oxygen += ((problem->evaluate(prevSolution) - problem->evaluate(groups[j].grp[k].solution)) * vectorDistance(groups[j].grp[k].solution, prevSolution)); // Update oxygen
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
}


int mutation(std::vector<int> &x, double mutation_rate) {
  int result = 0;
  int n = x.size();
  for(int i = 0; i != n; ++i) {
    if(random_generator.IOHprofiler_uniform_rand() < mutation_rate) {
      x[i] = (x[i] + 1) % 2;
      result = 1;
    }
  }
  return result;
}


/// This is an (1+1)_EA with static mutation rate = 1/n.
/// An example for discrete optimization problems, such as PBO suite.
void evolutionary_algorithm(std::shared_ptr<IOHprofiler_problem<int> > problem, std::shared_ptr<IOHprofiler_csv_logger> logger) {
  /// Declaration for variables in the algorithm
  std::vector<int> x;
  std::vector<int> x_star;
  double y;
  double best_value;
  double * mutation_rate = new double(1);
  *mutation_rate = 1.0/problem->IOHprofiler_get_number_of_variables();
  int budget = budget_scale * problem->IOHprofiler_get_number_of_variables() * problem->IOHprofiler_get_number_of_variables();

  std::vector<std::shared_ptr<double>> parameters;
  parameters.push_back(std::shared_ptr<double>(mutation_rate));
  std::vector<std::string> parameters_name;
  parameters_name.push_back("mutation_rate");
  logger->set_parameters(parameters,parameters_name);

  x = Initialization(problem->IOHprofiler_get_number_of_variables());
  x_star = x;
  y = problem->evaluate(x);
  logger->write_line(problem->loggerInfo());
  best_value = y;

  int count = 0;
  while (count <= budget && !problem->IOHprofiler_hit_optimal()) {
    x = x_star;
    if (mutation(x,*mutation_rate)) {
      y = problem->evaluate(x);
      logger->write_line(problem->loggerInfo());
    }
    if (y >= best_value) {
      best_value = y;
      x_star = x;
    }
    count++;
  }
}

/// This is an (1+1)_EA with static mutation rate = 1/n.
/// An example for continuous optimization problems, such as BBOB suite.
void random_search(std::shared_ptr<IOHprofiler_problem<double> > problem, std::shared_ptr<IOHprofiler_csv_logger> logger) {
  /// Declaration for variables in the algorithm
  std::vector<double> x(problem->IOHprofiler_get_number_of_variables());
  double y;

  int count = 0;
  while (count <= 500) {
    for (int i = 0; i != problem->IOHprofiler_get_number_of_variables(); ++i) {
      x[i] = random_generator.IOHprofiler_uniform_rand() * 10 - 5;
    }

    y = problem->evaluate(x);
    logger->write_line(problem->loggerCOCOInfo());
    count++;
  }
}

void _run_experiment() {
  std::string configName = "./configuration.ini";
  /// An example for PBO suite.
  //IOHprofiler_experimenter<int> experimenter(configName,evolutionary_algorithm);

  /// An exmaple for BBOB suite.
  IOHprofiler_experimenter<double> experimenter(configName, penguinAlgorithm);
  experimenter._set_independent_runs(10);
  experimenter._run();
}

int main(){
  _run_experiment();
  return 0;
}
