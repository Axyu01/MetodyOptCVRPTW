#include "Problem.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <math.h>
using namespace std;
Problem::Problem(string path,int problemSize)
{
    Load(path,problemSize);
}
Problem::~Problem()
{
    for(int l=0;l<SIZE;l++)
    {
        delete[] DistanceMatrix[l];
    }
    delete DistanceMatrix;
    delete ReadyTime;
    delete DueDate;
    delete ServiceTime;
    delete COORD_X;
    delete COORD_Y;
    delete Demand;
}
int Problem::EstimateSolution(Solution* s)
{
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
        }
        else
            demand = Demand[currentNode];

        if(load + demand > CAPACITY)
        {
            estimation += DistanceMatrix[previousNode][0];
            load = 0;
            time =0;
            i--;
            continue;
        }

        load+=demand;

        estimation += DistanceMatrix[previousNode][currentNode];
        time += DistanceMatrix[previousNode][currentNode];
        //cout <<"Going from: "<< previousNode <<" to: "<<currentNode<<endl;

        //check for time window and add penalty
        if(time < ReadyTime[currentNode])
        {
                //estimation += ReadyTime[currentNode] - time;
        }
        else if(time > DueDate[currentNode])
        {
            //estimation += time - DueDate[currentNode];
        }
        time += ServiceTime[currentNode];

        previousNode = currentNode;
    }
    estimation += DistanceMatrix[currentNode][0];
    //cout <<"Going from: "<< currentNode <<" to: "<<0<<endl;
    s->eval = estimation;
    return estimation;
}
void Problem::Load(string path,int problemSize)
{
    SIZE = problemSize;
    cout << "Loading: " << path << endl;
    ifstream file(path);

    string line;
    int nodeIterator = 0;

    getline(file, line);//Problem name
    NAME = line;
    getline(file, line);//nothing
    getline(file, line);//"VEHICLE"
    getline(file, line);//"NUMBER     CAPACITY"
    getline(file, line);//get basic info
    stringstream ss_info(line);
    ss_info >> MAX_VEHICLES >> CAPACITY;

    PREFFERED_GENOME_SIZE = SIZE+MAX_VEHICLES-1;

    getline(file, line);//nothing
    getline(file, line);//"CUSTOMER"
    getline(file, line);//"CUST NO.  XCOORD.   YCOORD.    DEMAND   READY TIME  DUE DATE   SERVICE   TIME"
    getline(file, line);//nothing

    ReadyTime = new int[problemSize+1];
    DueDate = new int[problemSize+1];
    ServiceTime = new int[problemSize+1];
    COORD_X =  new int[problemSize+1];
    COORD_Y =  new int[problemSize+1];
    Demand = new int[problemSize+1];

    //getting every location info
    for(int l = 0;l<problemSize+1;l++)
    {
        int CUST_NO;
        getline(file, line);//get location data
        stringstream ss_l(line);
        ss_l>>CUST_NO>>COORD_X[l]>>COORD_Y[l]>>Demand[l]>>ReadyTime[l]>>DueDate[l]>>ServiceTime[l];
    }

    //calculate distance matrix
    DistanceMatrix  = new double*[problemSize+1];
    for(int l1 = 0;l1<problemSize+1;l1++)
    {
        DistanceMatrix[l1]  = new double[problemSize+1];
        cout << endl;
        for(int l2 = 0;l2<problemSize+1;l2++)
        {
            DistanceMatrix[l1][l2]=sqrt(pow((COORD_X[l1]-COORD_X[l2]),2)+pow((COORD_Y[l1]-COORD_Y[l2]),2));
        }
    }
    file.close();
}
