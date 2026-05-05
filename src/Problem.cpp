#include "Problem.h"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
Problem::Problem(string path)
{
    Load(path);
}

Problem::~Problem()
{
    for(int m=0;m<M;m++)
    {
        delete[] MJTable[m];
    }
    delete MJTable;
}
int Problem::EstimateSolution(Solution* s,int** &CMXJTable,int jobLimit)
{
    //construct ctable
    if(CMXJTable == nullptr)
    {
        CMXJTable = new int*[M];
        for(int m = 0;m<M;m++)
        {
            CMXJTable[m] = new int[J];
            for(int j = 0;j<jobLimit;j++)
            {
                CMXJTable[m][j] = -1;
            }
            for(int j = jobLimit;j<J;j++)
            {
                CMXJTable[m][j] = 0;
            }
        }
    }
    else
    {
        for(int m = 0;m<M;m++)
        {
            CMXJTable[m][jobLimit-1] = -1;
        }
    }
    //estimate
    int returnedSum = 0;
    for(int g = 0;g<J;g++)
    {
        EstimateSolution(s,CMXJTable,g,M-1);
        returnedSum += CMXJTable[M-1][g];
    }

    return returnedSum;
}

int Problem::EstimateSolution(Solution* s)
{
    int** CMXJTable = nullptr;
    int returnedSum = EstimateSolution(s,CMXJTable,J);

    //Display matrix
    /*for(int m=0;m<M;m++)
    {
        for(int j=0;j<M;j++)
        {
            cout <<CMXJTable[m][j]<<" ";
        }
        cout << endl;
    }*/

    //destroy ctable

    for(int m=0;m<M;m++)
    {
        delete[] CMXJTable[m];
    }
    delete CMXJTable;

    return returnedSum;
}

void Problem::EstimateSolution(Solution* s,int** &CMXJTable,int xj,int m)
{
    if(xj == 0 && m == 0)
    {
        CMXJTable[m][xj] = MJTable[m][s->Genome[xj]];
    }
    else if(xj == 0 && m > 0)
    {
        if(CMXJTable[m-1][xj] == -1)
            EstimateSolution(s,CMXJTable,xj,m-1);

        CMXJTable[m][xj] = CMXJTable[m-1][xj] + MJTable[m][s->Genome[xj]];
    }
    else if(xj > 0 && m == 0)
    {
        if(CMXJTable[m][xj-1] == -1)
            EstimateSolution(s,CMXJTable,xj-1,m);

        CMXJTable[m][xj] =CMXJTable[m][xj-1] + MJTable[m][s->Genome[xj]];
    }
    else if(xj > 0 && m > 0)
    {
        if(CMXJTable[m-1][xj] == -1)
            EstimateSolution(s,CMXJTable,xj,m-1);
        if(CMXJTable[m][xj-1] == -1)
            EstimateSolution(s,CMXJTable,xj-1,m);

        int time1 = CMXJTable[m-1][xj];
        int time2 = CMXJTable[m][xj-1];
        int max = time1;
        if(time2>time1)
            max = time2;
        CMXJTable[m][xj] = max + MJTable[m][s->Genome[xj]];

    }
}
void Problem::Load(string path)
{
    cout << "Loading: " << path << endl;
    ifstream file(path);

    string line;
    bool inLocationSection = false;
    bool inDemandSection = false;
    int nodeIterator = 0;

    getline(file, line);//not important text line

    getline(file, line);//get info
    stringstream ss_info(line);
    ss_info >> J >> M >> INIT_SEED >> LOWER_BOUND >> UPPER_BOUND;

    getline(file, line);//not important text line

    MJTable = new int*[M];

    for(int m = 0;m<M;m++)
    {
        MJTable[m] = new int[J];
        getline(file, line);//get machine time data
        stringstream ss_m(line);
        //cout << endl;
        for(int j = 0;j<J;j++)
        {
            int time;
            ss_m>>time;
            MJTable[m][j] = time;
            //cout << time <<" ";
        }
    }

    file.close();
}
