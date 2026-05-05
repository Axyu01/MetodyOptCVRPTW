#ifndef CROSSOPS_H
#define CROSSOPS_H
#include "Solution.h"
#include <cstdlib>


class CrossOps
{
    public:
        CrossOps();
        virtual ~CrossOps();
        static Solution* OX(Solution* s1,Solution* s2,int size);
        static Solution* CX(Solution* s1,Solution* s2,int size);
        static Solution* PMX(Solution* s1,Solution* s2,int size);
        const static int OX_ID = 0;
        const static int CX_ID = 1;
        const static int PMX_ID = 2;

    protected:

    private:
};

#endif // CROSSOPS_H
