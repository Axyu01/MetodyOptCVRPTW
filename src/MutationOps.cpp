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
Solution* MutationOps::OptimizeTracks(Problem* problem,Solution* s)
{
    int locations = 0;
    Solution** tracks = new Solution*[problem->MAX_VEHICLES];
    int track_id = 0;
    int track_start = 0;
    for(int i=0;i<problem->MAX_VEHICLES;i++)
        tracks[i] = nullptr;

    int demand = 0;
    for(int g = 0;g<s->size && track_id<problem->MAX_VEHICLES-1;g++)
    {
        int to_location = s->Genome[g]+1;
        if(to_location>problem->SIZE)
        {
            to_location = 0;
        }
        demand += problem->Demand[to_location];
        if(to_location == 0 && track_start == g)
        {
            track_start++;
            continue;
        }
        if(demand>problem->CAPACITY || to_location == 0)
        {
            //Add track
            tracks[track_id] = new Solution(g-track_start);
            for(int l = track_start;l<g;l++)
            {
                tracks[track_id]->Genome[l-track_start] = s->Genome[l];
            }
            track_id++;
            //reset demand
            demand = problem->Demand[to_location];
            //skip
            track_start = g;
            g--;
        }
    }
    //Add last track and prune it from return symbols, all were used beforehand :(
    tracks[track_id] = new Solution(s->size-track_start);
    int detectedSymbols = 0;
    for(int l = track_start;l<s->size;l++)
    {
        if(s->Genome[l]>=problem->SIZE)//symbol detected!!
        {
            detectedSymbols++;
            continue;
        }
        tracks[track_id]->Genome[l-track_start-detectedSymbols] = s->Genome[l];
    }
    if(detectedSymbols >0)
    {
        Solution* temp = tracks[track_id];
        tracks[track_id] = new Solution(tracks[track_id],tracks[track_id]->size-detectedSymbols);
        track_id++;
    }

    //Optimize tracks
    for(int i=0;i<problem->MAX_VEHICLES;i++)
    {
        if(tracks[i] == nullptr)
            break;

        Solution* temp = tracks[i];
        if(tracks[i]->size<=OPT_TRACK_MAX_LOCATION_COUNT)
        {
            tracks[i] = OptimalTrack(problem,tracks[i]);
        }
        else
        {
            Solution* gready = GreadyTrack(problem,tracks[i]);
            problem->EstimateSolution(gready);
            problem->EstimateSolution(tracks[i]);
            if(tracks[i]->eval<gready->eval)
            {
                temp = gready;//will delete gready this way
            }
            else
            {
                tracks[i] = gready;//will save gready if it is better
            }
        }
        delete temp;
    }
    //Put things together back
    track_id = 0;
    track_start =0;
    int RETURN_SYMBOL = problem->SIZE;
    Solution* sOptimized = new Solution(s);
    for(int i=0;i<problem->MAX_VEHICLES;i++)
    {
        if(tracks[i] == nullptr)
            break;
        for(int g =0;g<tracks[i]->size;g++)
        {
            sOptimized->Genome[g+track_start] = tracks[i]->Genome[g];
        }
        if(track_start+tracks[i]->size <sOptimized->size)
            sOptimized->Genome[track_start+tracks[i]->size] = RETURN_SYMBOL++;//return symbol
        track_start += tracks[i]->size + 1;
    }
    //Fill rest with zero's
    int dummy;
    if(track_start == sOptimized->Genome[track_start-1])
        cin>>dummy;
    for(int i=track_start;i<sOptimized->size;i++)
    {
        sOptimized->Genome[i] = i;
    }
    //check for debugging purposes
    bool foundError = false;
    int foundErrorID = -1;
    for(int i =0;i<s->size;i++)
    {
        if(foundError)
                break;
        foundError = true;
        foundErrorID = i;
        for(int j = 0;j<sOptimized->size;j++)
        {
            if(sOptimized->Genome[j] == i)
            {
                foundError = false;
                break;
            }
        }
    }
    if(foundError)
    {
        cout<<endl<<"BROKEN SOLUTION ID:"<<foundErrorID;
        sOptimized->print();
        cout<<endl<<"ORIGINAL SOLUTION:";
        s->print();
        cout<<endl<<"OPTIMIZED TRACKS:";
        for(int t =0;t<problem->MAX_VEHICLES;t++)
        {
            if(tracks[t] == nullptr)
                break;
            tracks[t]->print();
        }
    }
    //Memory management
    for(int i=0;i<problem->MAX_VEHICLES;i++)
    {
        if(tracks[i] == nullptr)
            break;
        delete tracks[i];
    }
    delete[] tracks;


    return sOptimized;
}
Solution* MutationOps::GreadyTrack(Problem* problem,Solution* s)
{
    bool takenLocation[s->size];
    for(int i=0;i<s->size;i++)
        takenLocation[i] = false;

    Solution* best = new Solution(0);
    for(int i=0;i<s->size;i++)
    {
        Solution* curr_best = nullptr;
        int curr_best_index = -1;
        for(int j=0;j<s->size;j++)
        {
            if(takenLocation[j])
                continue;
            Solution* sLocal = new Solution(best,i+1);
            sLocal->Genome[i] = s->Genome[j];
            problem->EstimateSolution(sLocal);
            if(curr_best == nullptr || curr_best->eval>sLocal->eval)
            {
                if(curr_best !=nullptr)
                    delete curr_best;
                curr_best = sLocal;
                curr_best_index = j;
            }
            else
            {
                delete sLocal;
            }
        }
        delete best;
        best = curr_best;
        takenLocation[curr_best_index] = true;
    }
    return best;
}
Solution* MutationOps::OptimalTrack(Problem* problem,Solution* s)
{
    int lSize = s->size;
    int* possibleLocations = new int[lSize];
    for(int i=0;i<lSize;i++)
    {
        possibleLocations[i] = s->Genome[i];
    }
    Solution* best = OptimalTrack(problem,new Solution(0),possibleLocations,lSize);
    delete possibleLocations;
    return best;
}
Solution* MutationOps::OptimalTrack(Problem* problem,Solution* s,int* possibleLocations,int lSize)
{
    if(lSize == 1)
    {
        Solution* best = new Solution(s,s->size+1);
        best->Genome[s->size] = possibleLocations[0];
        problem->EstimateSolution(best);
        return best;
    }

    Solution* best = nullptr;
    for(int g =0;g<lSize;g++)
    {
        Solution* checked = new Solution(s,s->size+1);
        checked->Genome[s->size] = possibleLocations[g];
        int checkedLSize = lSize-1;
        int* checkedPossibleLocations = new int[checkedLSize];
        for(int i=0;i<checkedLSize;i++)
        {
            if(i<g)
               checkedPossibleLocations[i] = possibleLocations[i];
            else
                checkedPossibleLocations[i] = possibleLocations[i+1];
        }
        Solution* sFound = OptimalTrack(problem,checked,checkedPossibleLocations,checkedLSize);
        delete checkedPossibleLocations;
        if(best == nullptr || sFound->eval<best->eval)
        {
            if(best !=nullptr)
                delete best;
            best = sFound;
        }
        else
            delete sFound;
    }
    return best;
}
Solution* MutationOps::Repair(Problem* problem,Solution* s)
{
    return MutationOps::Repair(problem,s,0);
}
Solution* MutationOps::Repair(Problem* problem,Solution* s,int correctGenes)
{
    //int x;
    //cin>>x;
    bool* isGeneRecyclable = FindRecyclableGenes(problem,s);
    Solution* recycledSolution = new Solution(s);
    int recycledGene = 0;
    for(int g=0;g<s->size;g++)
    {
        if(isGeneRecyclable[g] == false)
            recycledSolution->Genome[recycledGene++] = s->Genome[g];
    }
    //cout << endl<<"REPAAAAAAAAAAAAAAAAIR!!!!!!!!!  "<<recycledGene;
    if(recycledGene >=s->size)//The solution is well recycled, return it
    {
        delete[] isGeneRecyclable;
        return recycledSolution;
    }
    int correctedGenes = recycledGene;
    for(int g=0;g<s->size;g++)
    {
        if(isGeneRecyclable[g] == true)
            recycledSolution->Genome[recycledGene++] = s->Genome[g];
    }
    if(correctedGenes == correctGenes)//The solution cannot be improved, return it
    {
        delete[] isGeneRecyclable;
        return recycledSolution;
    }
    delete[] isGeneRecyclable;
    //recycledSolution->print();
    Solution* wellRecycledSolution = Repair(problem,recycledSolution,correctedGenes);//The solution is well recycled, return it
    delete recycledSolution;
    return wellRecycledSolution;
}
bool* MutationOps::FindRecyclableGenes(Problem* problem,Solution* s)
{
    int SIZE = problem->SIZE;
    int CAPACITY = problem->CAPACITY;
    bool* isGeneRecyclable = new bool[s->size];
    for(int i=0;i<s->size;i++)
    {
        isGeneRecyclable[i] = false;
    }

    double estimation = 0;
    int load = 0;
    double time = 0;
    int currentNode;
    int previousNode = 0;

    for(int i = 0; i<s->size; i++)
    {
        currentNode = s->Genome[i]+1;
        int demand = 0;
        if(currentNode > SIZE)
            currentNode = 0;

        if(currentNode == 0)
        {
            load = 0;
            time = 0;
            if(previousNode == 0)
            {
                isGeneRecyclable[i] = true;
                continue;
            }
        }
        else
            demand = problem->Demand[currentNode];

        if(load + demand > CAPACITY)
        {
            estimation += problem->DistanceMatrix[previousNode][0];
            load = 0;
            time =0;
            previousNode = 0;
            i--;
            continue;
        }

        int timeDelta = problem->DistanceMatrix[previousNode][currentNode];

        //check for time window
        if(time + timeDelta> problem->DueDate[currentNode])// if late, add gene to recycle and rollback to previous location
        {
            isGeneRecyclable[i] = true;
            continue;
        }
        estimation += problem->DistanceMatrix[previousNode][currentNode];
        time += timeDelta;
        load+=demand;
        if(time< problem->ReadyTime[currentNode])
        {
                time = problem->ReadyTime[currentNode];
        }
        time += problem->ServiceTime[currentNode];

        previousNode = currentNode;
    }
    estimation += problem->DistanceMatrix[currentNode][0];

    //Mark all last return genes as not recyclable
    for(int g=s->size-1;g>=0;g--)
    {
        if(s->Genome[g]+1 > SIZE)//found return base token
            isGeneRecyclable[g] = false;
        else
            break;
    }

    return isGeneRecyclable;
}
