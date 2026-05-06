#include "Solution.h"
#include <cstdlib>
#include <iostream>

using namespace std;
Solution::Solution(Solution* s)
{
    //Copy init
    Genome = new int[s->size];
    for(int g =0;g<s->size;g++)
    {
        Genome[g] = s->Genome[g];
    }
    this->size = s->size;
    this->eval = s->eval;

}
Solution::Solution(int size)
{
    this->size = size;
    //Random init
    Genome = new int[size];
    for(int g =0;g<size;g++)
    {
        Genome[g] = -1;
    }
    for(int j =0;j<size;j++)//assigning random jobs to genome
    {

        int rand_g = -1;
        while(rand_g == -1)
        {
            int rand_num = rand()%size;
            if(Genome[rand_num]==-1)
            {
                rand_g = rand_num;
            }
        }

        Genome[rand_g] = j;
    }
}
Solution::Solution(Solution*s,int size)
{
    //Copy init
    Genome = new int[size];
    for(int g =0;g<s->size && g<size;g++)
    {
        Genome[g] = s->Genome[g];
    }
    for(int g =s->size;g<size;g++)
    {
        Genome[g] = g;
    }
    this->size = size;
    this->eval = s->eval;
}
Solution::~Solution()
{
    delete[] Genome;
}
Solution::print()
{
     cout <<"\n| ";
    for(int g =0;g<size-1;g++)
    {
        cout << Genome[g] << ":";
    }
    cout <<Genome[size-1];
    cout<<" | size: "<<size<<" | eval: "<<eval<<" |";
}
