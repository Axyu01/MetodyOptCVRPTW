#ifndef MUTATIONOPS_H
#define MUTATIONOPS_H
#include "Solution.h"
#include "Problem.h"

class MutationOps
{
    public:
        MutationOps();
        virtual ~MutationOps();
        static Solution* Swap(Solution* s);
        static Solution* Swap(Solution* s,int numOfSwaps);
        static Solution* Inverse(Solution* s);
        static Solution* OptimizeTracks(Problem* problem,Solution* s);
        static Solution* GreadyTrack(Problem* problem,Solution* s);
        static Solution* OptimalTrack(Problem* problem,Solution* s);
        static Solution* Repair(Problem* problem,Solution* s);
        const static int SWAP_ID = 0;
        const static int INVERSE_ID = 1;

        const static int OPT_TRACK_MAX_LOCATION_COUNT = 5;

    protected:
        static Solution* OptimalTrack(Problem* problem,Solution* checked,int* possibleLocations,int lSize);

    private:
};

#endif // MUTATIONOPS_H
