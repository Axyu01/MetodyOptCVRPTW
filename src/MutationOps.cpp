#include "MutationOps.h"
#include <cstdlib>
#include <iostream>
using namespace std;
MutationOps::MutationOps()
{
    //ctor
}

MutationOps::~MutationOps()
{
    //dtor
}
Solution* MutationOps::Swap(Solution* s,int numOfSwaps)
{
    Solution* s_out = new Solution(s);
    int size = s->size;
    for(int i=0;i<numOfSwaps;i++)
    {
        int g1=-1;
        int g2=-1;

        while(g1 == g2)
        {
            g1 = rand()%size;
            g2 = rand()%size;
        }

        int temp = s_out->Genome[g1];
        s_out->Genome[g1] = s_out->Genome[g2];
        s_out->Genome[g2] = temp;
    }

    return s_out;
}
Solution* MutationOps::Swap(Solution* s)
{
    Swap(s,1);
}
Solution* MutationOps::Inverse(Solution* s)
{
    int size = s->size;
    int g1=-1;
    int g2=-1;

    while(g1 == g2)
    {
        g1 = rand()%size;
        g2 = rand()%size;
    }

    if(g1>g2)//swap
    {
        int temp = g1;
        g1= g2;
        g2 = temp;
    }

    Solution* s_out = new Solution(s);
    for(int i = g1;i<=g2;i++)
    {
        s_out->Genome[i] = s->Genome[g2-(i-g1)];
    }
    s_out->Genome[g2] = s->Genome[g1];

    return s_out;
}
