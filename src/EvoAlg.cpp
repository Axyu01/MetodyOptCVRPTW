#include "EvoAlg.h"
#include "cstdlib"
#include "CrossOps.h"
#include "MutationOps.h"
#include <iostream>
using namespace std;
EvoAlg::EvoAlg(Problem* problem,int popSize)
{
    this->popSize = popSize;
    this->problem = problem;
    //ctor
}
EvoAlg::~EvoAlg()
{
    //dtor
}
void EvoAlg::Init()
{
    population = new Solution*[popSize];
    for(int i=0;i<popSize;i++)
    {
        //Random solution
        population[i] = new Solution(problem->J);
    }
}
void EvoAlg::Eval()
{
    for(int i =0;i<popSize;i++)
    {
        population[i]->eval = problem->EstimateSolution(population[i]);
    }
}
Solution* EvoAlg::Select()
{
    //cout<<"\n tursize"<<turSize;
    //return population[rand()%popSize];
    Solution* contestants[turSize];
    for(int s = 0;s<turSize;s++)
    {
        contestants[s] = population[rand()%popSize];
    }
    Solution* bestSolution = contestants[0];
    for(int s = 1;s<turSize;s++)
    {
        if(bestSolution->eval>contestants[s]->eval)
        {
            bestSolution = contestants[s];
        }
    }
    //bestSolution->print();
    return bestSolution;
}
Solution* EvoAlg::Cross(Solution* P1,Solution* P2)
{
    if(CROSS_ID == CrossOps::OX_ID)
        return CrossOps::OX(P1,P2,P1->size);
    else if(CROSS_ID == CrossOps::CX_ID)
        return CrossOps::CX(P1,P2,P1->size);
    else
        return CrossOps::PMX(P1,P2,P1->size);
}
Solution* EvoAlg::Mutate(Solution* s)
{
    if(MUT_ID == MutationOps::INVERSE_ID)
        return MutationOps::Inverse(s);
    else
        return MutationOps::Swap(s,s->size*Xp/MAX_PROB);
}
Solution* EvoAlg::GetBest()
{
    Solution* bestSolution = population[0];
    for(int s = 1;s<popSize;s++)
    {
        if(bestSolution->eval>population[s]->eval)
        {
            bestSolution = population[s];
        }
    }
    return new Solution(bestSolution);
}
Solution* EvoAlg::GetWorst()
{
    Solution* worstSolution = population[0];
    for(int s = 1;s<popSize;s++)
    {
        if(worstSolution->eval<population[s]->eval)
        {
            worstSolution = population[s];
        }
    }
    return new Solution(worstSolution);
}
int EvoAlg::GetAvarage()
{
    long long int sum = 0;
    for(int s = 0;s<popSize;s++)
    {
        sum += population[s]->eval;
    }
    return sum/popSize;
}
void EvoAlg::Evolve()
{
    Solution** newPopulation = new Solution*[popSize];
    //create new population
    //find elites
    for(int elite = 0;elite<elitesNum;elite++)
    {
        Solution* oldElite = population[elite];
        int newElitePos = elite;
        for(int s = elite+1;s<popSize;s++)
        {
            Solution* contender = population[s];
            if(contender->eval < population[newElitePos]->eval)
            {
                newElitePos = s;
            }
        }
        population[elite] = population[newElitePos];
        population[newElitePos] = oldElite;
    }
    //copy elites
    for(int currentSize = 0;currentSize < popSize;currentSize++)
    {
        newPopulation[currentSize] = new Solution(population[currentSize]);
    }
    //create new solutions
    for(int currentSize = elitesNum;currentSize < popSize;currentSize++)
    {
        Solution* P1 = Select();
        Solution* P2 = Select();

        Solution* O1 = nullptr;
        if(rand()%MAX_PROB < Xp)
        {
            O1 = Cross(P1,P2);
        }
        else
        {
            O1 = new Solution(P1);
        }

        if(rand()%MAX_PROB < Mp)
        {
            Solution* temp = O1;
            O1 = Mutate(O1);
            delete temp;
        }

        newPopulation[currentSize] = O1;
    }

    //clean old population
    for(int s = 0;s < popSize;s++)
    {
        delete population[s];
    }
    delete population;

    //assign new one
    population = newPopulation;
}
