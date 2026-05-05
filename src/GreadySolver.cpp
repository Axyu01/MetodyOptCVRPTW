#include "GreadySolver.h"
#include "iostream"
using namespace std;


GreadySolver::GreadySolver(Problem* problem)
{
    _problem = problem;
    //ctor
}

GreadySolver::~GreadySolver()
{
    //dtor
}

Solution* GreadySolver::GetSolution(int startJ)
{
    int J = _problem->J;
    int M = _problem->M;

    bool isJobAssigned[J];
    for(int j =0; j<J ;j++)
    {
        isJobAssigned[j] = false;
    }
    isJobAssigned[startJ] = true;


    Solution* s = new Solution(J);
    s->Genome[0] = startJ;

    int** CMXJTable = nullptr;
    for(int g =1; g<J ;g++)
    {
        int bestJ = -1;
        int bestJScore = -1;
        for(int j = 0; j<J ;j++)
        {
            if(isJobAssigned[j])
                continue;

            //Solve for table
            s->Genome[g] = j;
            _problem->EstimateSolution(s,CMXJTable,g+1);
            //cout<<endl<<g<<" "<<j<<endl;

            //Calculate score to desired point
            int score = 0;
            for(int i=0;i<g;i++)
            {
                score +=CMXJTable[M-1][i];
            }

            //Check for best
            if(bestJ == -1 || bestJScore>score)
            {
                bestJ = j;
                bestJScore = score;
            }
        }
        s->Genome[g] = bestJ;
        isJobAssigned[bestJ] = true;
    }
    //destroy ctable
    for(int m=0;m<M;m++)
    {
        delete[] CMXJTable[m];
    }
    delete CMXJTable;

    return s;
}

