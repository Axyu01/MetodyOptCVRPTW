#ifndef MUTATIONOPS_H
#define MUTATIONOPS_H
#include "Solution.h"

class MutationOps
{
    public:
        MutationOps();
        virtual ~MutationOps();
        static Solution* Swap(Solution* s);
        static Solution* Swap(Solution* s,int numOfSwaps);
        static Solution* Inverse(Solution* s);
        const static int SWAP_ID = 0;
        const static int INVERSE_ID = 1;

    protected:

    private:
};

#endif // MUTATIONOPS_H
