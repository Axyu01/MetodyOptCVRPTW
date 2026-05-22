#ifndef EVOALG_H
#define EVOALG_H
#include "Solution.h"
#include "Problem.h"
#include "MutationOps.h"
#include "CrossOps.h"
class EvoAlg
{
    const int MAX_PROB = 100;
    public:
        Solution** population;
        int popSize = 100;
        int Xp = 70;
        int Mp = 30;
        //try optimize tracks mutation probablity
        int OPTp = 10;
        int OPT_TRACK_MAX_LOCATION_COUNT = 5;
        //Repair mutations parameters
        int REPAIRp = 10;
        bool REPAIR_TRY_BEFORE = false;
        bool REPAIR_TRY_AFTER = false;
        bool REPAIR_USE_DAMAND = false;
        //Redistribute mutation parameters
        int REDISTp = 20;
        int REDIST_TRIES = 10;

        int turSize = 2;
        int elitesNum = 3;
        int MUT_ID = MutationOps::INVERSE_ID;
        int CROSS_ID = CrossOps::OX_ID;
        Problem* problem;
        EvoAlg(Problem* problem,int popSize);
        virtual ~EvoAlg();
        void Init();
        void Eval();
        void Evolve();
        Solution* GetBest();
        Solution* GetWorst();
        int GetAvarage();

    protected:
        Solution* Select();
        Solution* Cross(Solution* P1,Solution* P2);
        Solution* Mutate(Solution* s);

    private:
};

#endif // EVOALG_H
