#include "SAAlg.h"
#include "MutationOps.h"
#include <cmath>
#include <cstdlib>

SAAlg::SAAlg(Problem* problem,float startTemp)
{
    this->problem = problem;
    currentSolution = new Solution(problem->J);
    currentSolution->eval = problem->EstimateSolution(currentSolution);
    bestSolution = new Solution(currentSolution);
    bestSolution->eval = problem->EstimateSolution(bestSolution);
    currentTemp = startTemp;
    currentIteration = 0;
}
SAAlg::~SAAlg()
{
    //dtor
    delete currentSolution;
    delete bestSolution;
}
bool SAAlg::Iterate()
{
    if(currentIteration>= maxIterations)
        return false;
    //Cooling
    if(currentIteration%tempIterations == tempIterations-1)
    {
        currentTemp *= coolingFactor;
    }
    //Create Neighboor
    Solution* neighboorSolution;
    if(MUT_ID == MutationOps::SWAP_ID)
        neighboorSolution = MutationOps::Swap(currentSolution);
    else
        neighboorSolution = MutationOps::Inverse(currentSolution);
    neighboorSolution->eval = problem->EstimateSolution(neighboorSolution);
    //Check for best
    if(neighboorSolution->eval<bestSolution->eval)
    {
        delete bestSolution;
        bestSolution = neighboorSolution;
        delete currentSolution;
        currentSolution = new Solution(bestSolution);
    }
    else if(neighboorSolution->eval<currentSolution->eval)//Always accept better then current
    {
        delete currentSolution;
        currentSolution = neighboorSolution;
    }
    else//Check if alg should accept worse solution
    {
        const int MAX_PROB = RAND_MAX;
        int acceptance_probability = (int)(MAX_PROB * (1/(1+std::exp((neighboorSolution->eval - currentSolution->eval) / currentTemp))));
        if(acceptance_probability>rand()%MAX_PROB)
        {
            delete currentSolution;
            currentSolution = neighboorSolution;
        }
    }
    currentIteration++;
    return true;




}
