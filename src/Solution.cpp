#include "Solution.h"
#include <cstdlib>
#include <iostream>
#include <cstdio>

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
    this->penalty = s->penalty;

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
    this->penalty = s->penalty;
}
Solution::~Solution()
{
    delete[] Genome;
}
void Solution::print()
{
    if (size <= 0 || Genome == nullptr)
    {
        cout << "\n| Empty Solution |";
        return;
    }
     cout <<"\n| ";
    for(int g =0;g<size-1;g++)
    {
        cout << Genome[g] << ":";
    }
    cout <<Genome[size-1];
    cout<<" | size: "<<size<<" | eval: "<<eval<<" | penalty: "<<penalty<<" |";
}
void Solution::Insert(int gene,int position)
{
    if (position < 0) position = 0;
    if (position > size) position = size; // Clamp to max bound

    int* oldGenome = Genome;
    size+=1;

    //Copy and insert
    Genome = new int[size];
    for(int g =0;g<size;g++)
    {
        if(g == position)
            Genome[g] = gene;
        else if(g<position)
            Genome[g] = oldGenome[g];
        else
            Genome[g] = oldGenome[g-1];
    }
    this->eval = 0;
    this->penalty = 0;
    delete[] oldGenome;
}
void Solution::Remove(int position)
{
    if (position < 0) position = 0;
    if (position >= size) position = size-1; // Clamp to max bound

    int* oldGenome = Genome;
    size-=1;

    //Copy and delete
    Genome = new int[size];
    for(int g =0;g<size;g++)
    {
        if(g<position)
            Genome[g] = oldGenome[g];
        else
            Genome[g] = oldGenome[g+1];
    }
    this->eval = 0;
    this->penalty = 0;
    delete[] oldGenome;
}
