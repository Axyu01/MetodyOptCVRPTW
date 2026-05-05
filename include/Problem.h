#ifndef PROBLEM_H
#define PROBLEM_H

#include "Solution.h"
#include "string"
class Problem
{
    public:
        Problem(std::string,int problemSize);
        virtual ~Problem();
        int EstimateSolution(Solution* s);
        int EstimateSolution(Solution* s,int** &CMXJTable,int jobLimit);
        void EstimateSolution(Solution* s,int** &CMJTable,int xj,int m);
        int CAPACITY;
        int MAX_VEHICLES;
        int SIZE;
        int PREFFERED_GENOME_SIZE;
        std::string NAME;

        double** DistanceMatrix;
        int* ReadyTime;
        int* DueDate;
        int* ServiceTime;
        int* COORD_X;
        int* COORD_Y;
        int* Demand;


    private:
        void Load(std::string path,int problemSize);
};

#endif // PROBLEM_H
