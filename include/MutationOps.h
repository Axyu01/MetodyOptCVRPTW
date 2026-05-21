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
        static Solution* Repair(Problem* problem,Solution* s,bool tryBefore,bool tryAfter,bool useDemand);
        const static int SWAP_ID = 0;
        const static int INVERSE_ID = 1;

        const static int OPT_TRACK_MAX_LOCATION_COUNT = 5;

    protected:
        static void DEBUG_CheckSolution(Problem* problem,Solution* checked,Solution* original,Solution** tracks);
        static void DEBUG_PRINT_Tracks(Problem*problem,Solution** tracks);
        static Solution** FindTracks(Problem* problem,Solution* s);
        static Solution* ReconstructSolutionFromTracks(Problem* problem,Solution** tracks);
        static Solution* OptimalTrack(Problem* problem,Solution* checked,int* possibleLocations,int lSize);
        static double* FindRecyclableGenes(Problem* problem,Solution* s);
        static bool* RestructureRecycledGenes(Problem* problem,Solution* s);

    private:
};

#endif // MUTATIONOPS_H
