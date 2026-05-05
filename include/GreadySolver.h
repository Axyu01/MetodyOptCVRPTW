#ifndef GREADYSOLVER_H
#define GREADYSOLVER_H
#include "Solution.h"
#include "Problem.h"

class GreadySolver
{
    public:
        GreadySolver(Problem* problem);
        virtual ~GreadySolver();
        Solution* GetSolution(int startJ);

    protected:
        Problem* _problem;

    private:
};

#endif // GREADYSOLVER_H
