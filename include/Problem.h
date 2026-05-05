#ifndef PROBLEM_H
#define PROBLEM_H

#include "Solution.h"
#include "string"
class Problem
{
    public:
        Problem(std::string);
        virtual ~Problem();
        int EstimateSolution(Solution* s);
        int EstimateSolution(Solution* s,int** &CMXJTable,int jobLimit);
        void EstimateSolution(Solution* s,int** &CMJTable,int xj,int m);
        int M;
        int J;
        int INIT_SEED;
        int UPPER_BOUND;
        int LOWER_BOUND;

    protected:
        int** MJTable;


    private:
        void Load(std::string path);
};

#endif // PROBLEM_H
