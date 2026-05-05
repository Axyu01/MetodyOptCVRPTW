#include "CrossOps.h"
#include "iostream"
using namespace std;
CrossOps::CrossOps()
{
    //ctor
}

CrossOps::~CrossOps()
{
    //dtor
}
Solution* CrossOps::OX(Solution* s1,Solution* s2,int size)
{
    bool used[size];
    Solution* newSolution = new Solution(size);
    for(int i=0;i<size;i++)
    {
        used[i] = false;
        newSolution->Genome[i] = -1;
    }
    int start = rand()%size;
    int end = rand()%size;

    if(start>end)//swap if needed
    {
        int temp = start;
        start = end;
        end  = start;
    }
    for(int i = start;i<=end;i++)
    {
        newSolution->Genome[i] = s1->Genome[i];
        used[s1->Genome[i]]= true;
    }
    int copyIterator = 0;
    for(int i = 0;i<size;i++)
    {
        if(newSolution->Genome[i] == -1)
        {
            while(used[s2->Genome[copyIterator]])
                copyIterator++;

            newSolution->Genome[i] = s2->Genome[copyIterator];
            used[s2->Genome[copyIterator]] = true;
        }
    }
    return newSolution;
}
Solution* CrossOps::CX(Solution* s1,Solution* s2,int size)
{
    Solution* newSolution = new Solution(size);
    for(int i=0;i<size;i++)
    {
        newSolution->Genome[i] = -1;
    }
    int startG = rand()%size;
    //cout << "\n" << startG <<"\n";
    int startVal = s1->Genome[startG];


    int currentG = startG;
    int currentVal = s2->Genome[startG];
    while(currentVal != startVal)
    {
        newSolution->Genome[currentG] = s1->Genome[currentG];
        for(int g=0;g<size;g++)
        {
            if(currentVal == s1->Genome[g])
            {
                currentG = g;
                currentVal = s2->Genome[currentG];
                break;
            }
        }
    }
    newSolution->Genome[currentG] = s1->Genome[currentG];
    //copy rest
    for(int i = 0;i<size;i++)
    {
        if(newSolution->Genome[i] == -1)
        {
            newSolution->Genome[i] = s2->Genome[i];
        }
    }
    return newSolution;
}
Solution* CrossOps::PMX(Solution* s1,Solution* s2,int size)
{
    bool used[size];
    Solution* newSolution = new Solution(size);
    for(int i=0;i<size;i++)
    {
        used[i] = false;
        newSolution->Genome[i] = -1;
    }
    int start = rand()%size;
    int end = rand()%size;

    if(start>end)//swap if needed
    {
        int temp = start;
        start = end;
        end  = start;
    }
    for(int i = start;i<=end;i++)//crossing
    {
        newSolution->Genome[i] = s1->Genome[i];
        used[s1->Genome[i]]= true;
    }
    //mapping WIP
    for(int g =0;g<size;g++)
    {
        if(newSolution->Genome[g] == -1)
        {
            bool found = false;
            int foundVal = s2->Genome[g];
            while(found == false)
            {
                found = true;

                for(int i = start;i<=end;i++)//crossing
                {
                    if(foundVal == newSolution->Genome[i])
                    {
                        found = false;
                        foundVal = s2->Genome[i];
                        break;
                    }
                }
            }
            newSolution->Genome[g] = foundVal;
        }
    }

    return newSolution;
}
