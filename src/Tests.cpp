#include "Tests.h"
#include "EvoAlg.h"
#include "Solution.h"
#include "Logger.h"
#include "MutationOps.h"
#include "CrossOps.h"
#include "GreadySolver.h"
#include "SAAlg.h"
Tests::Tests()
{
    //ctor
}
Tests::~Tests()
{
    //dtor
}
void Tests::EvoTest(
Problem* problem,
std::string test_id,
int popSize,
int Xp,
int Mp,
int turSize,
int elitesNum,
int MUT_ID,
int CROSS_ID)
{
    //Init Loggers
    string file_name = "out/evo_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M);
    if(test_id != "")
        file_name+="_"+test_id;


    cout<<endl<<"TEST:"<<file_name<<endl;

    Logger bestLogger(file_name+"_best.csv");
    Logger worstLogger(file_name+"_worst.csv");
    Logger avgLogger(file_name+"_avg.csv");

    int budget = problem->J*problem->J*STANDARD_MULTIPLAYER;//*10;
    if(problem->J >= 500)
        budget = problem->J*OVER500_MULTIPLAYER;//*100;

    int loops = budget/popSize;

    EvoAlg* evo = new EvoAlg(problem,popSize);
    evo->Xp = Xp;
    evo->Mp = Mp;
    evo->turSize = turSize;
    evo->elitesNum = elitesNum;
    evo->MUT_ID = MUT_ID;
    evo->CROSS_ID = CROSS_ID;

    //Start Evolution
    evo->Init();
    evo->Eval();

    //Logging
        Solution* best = evo->GetBest();
        Solution* worst = evo->GetWorst();
        int avg = evo->GetAvarage();
        bestLogger.Log(best);
        worstLogger.Log(worst);
        avgLogger.Log(avg);
        delete best;
        delete worst;

    for(int i=0;i<loops;i++)
    {
        //Evo Loop
        evo->Evolve();
        evo->Eval();
        //Debugging
        if(i%100 == 0 && false)
        {
            cout<<endl<<evo->GetAvarage();
            evo->GetBest()->print();
        }
        //Logging
        Solution* best = evo->GetBest();
        Solution* worst = evo->GetWorst();
        int avg = evo->GetAvarage();
        bestLogger.Log(best);
        worstLogger.Log(worst);
        avgLogger.Log(avg);
        delete best;
        delete worst;
    }
    evo->GetBest()->print();
    evo->GetWorst()->print();
    cout<<endl<<evo->GetAvarage()<<endl;
}
void Tests::EvoTest(Problem* problem,int iteration)
{
    //Init Loggers
    string file_name = "out/evo_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M)+"_"+to_string(iteration);

    cout<<endl<<"TEST:"<<file_name<<endl;

    Logger bestLogger(file_name+"_best.csv");
    Logger worstLogger(file_name+"_worst.csv");
    Logger avgLogger(file_name+"_avg.csv");

    int budget = problem->J*problem->J*10;
    if(problem->J >= 100)
        budget = problem->J*problem->J*10;
    else if(problem->J >= 500)
        budget = problem->J*10;

    int popSize = 50;//50,100,500
    int Xp = 70;
    int Mp = 30;
    int turSize = 2;
    int elitesNum = 3;
    int MUT_ID = MutationOps::SWAP_ID;//MutationOps::INVERSE_ID;
    int CROSS_ID = CrossOps::OX_ID;

    //Init Evo alg
    if(problem->J <= 20)
    {
        popSize = 50;
        Xp = 70;
        Mp = 30;
        turSize = 2;
        elitesNum = 3;
        MUT_ID = MutationOps::SWAP_ID;//MutationOps::INVERSE_ID;
        CROSS_ID = CrossOps::OX_ID;
    }
    else if(problem->J <= 100)
    {
        popSize = 50;
        Xp = 70;
        Mp = 30;
        turSize = 2;
        elitesNum = 3;
        MUT_ID = MutationOps::SWAP_ID;//MutationOps::INVERSE_ID;
        CROSS_ID = CrossOps::OX_ID;
    }
    else
    {
        popSize = 50;//50,100,500
        Xp = 70;
        Mp = 30;
        turSize = 2;
        elitesNum = 3;
        MUT_ID = MutationOps::SWAP_ID;//MutationOps::INVERSE_ID;
        CROSS_ID = CrossOps::OX_ID;
    }
    int loops = budget/popSize;

    EvoAlg* evo = new EvoAlg(problem,popSize);
    evo->Xp = Xp;
    evo->Mp = Mp;
    evo->turSize = turSize;
    evo->elitesNum = elitesNum;
    evo->MUT_ID = MUT_ID;
    evo->CROSS_ID = CROSS_ID;

    //Start Evolution
    evo->Init();
    evo->Eval();
    for(int i=0;i<loops;i++)
    {
        //Evo Loop
        evo->Evolve();
        evo->Eval();
        //Debugging
        if(i%100 == 0 && false)
        {
            cout<<endl<<evo->GetAvarage();
            evo->GetBest()->print();
        }
        //Logging
        Solution* best = evo->GetBest();
        Solution* worst = evo->GetWorst();
        int avg = evo->GetAvarage();
        bestLogger.Log(best);
        worstLogger.Log(worst);
        avgLogger.Log(avg);
        delete best;
        delete worst;
    }
    evo->GetBest()->print();
    evo->GetWorst()->print();
    cout<<endl<<evo->GetAvarage()<<endl;
}
void Tests::EvoParamTest()
{
    int popSize = 33;
    int Xp = 75;
    int Mp = 25;
    int CROSS_ID = CrossOps::OX_ID;
    int MUT_ID = MutationOps::SWAP_ID;
    int turSize = 2;
    int elitesNum = 1;
    string test_id = "MUT_ID";//"CROSS_ID";//"Mp";//"Xp";//"popSize";

    Problem* problem = new Problem("problems/tai20_10_0.fsp");
    popSize = 40;//10,20,40
    Xp = 75;//25,50,75
    Mp = 25;//25,50,75
    CROSS_ID = CrossOps::OX_ID;//CrossOps::OX_ID,CrossOps::CX_ID
    MUT_ID = MutationOps::SWAP_ID;//MutationOps::SWAP_ID,MutationOps::INVERSE_ID

    for(int i=0;i<10;i++)
        EvoTest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;

    problem = new Problem("problems/tai100_10_0.fsp");
    popSize = 50;//50,100,200
    Xp = 75;//25,50,75
    Mp = 25;//25,50,75
    CROSS_ID = CrossOps::OX_ID;//CrossOps::OX_ID,CrossOps::CX_ID
    MUT_ID = MutationOps::SWAP_ID;//MutationOps::SWAP_ID,MutationOps::INVERSE_ID

    for(int i=0;i<10;i++)
        EvoTest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;

    problem = new Problem("problems/tai500_20_0.fsp");
    popSize = 250;//250,500,1000
    Xp = 75;//25,50,75
    Mp = 25;//25,50,75
    CROSS_ID = CrossOps::OX_ID;//CrossOps::OX_ID,CrossOps::CX_ID
    MUT_ID = MutationOps::INVERSE_ID;//MutationOps::SWAP_ID,MutationOps::INVERSE_ID

    for(int i=0;i<10;i++)
        EvoTest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;
}
void Tests::EvoTest()
{
    int popSize = 33;
    int Xp = 75;
    int Mp = 25;
    int turSize = 2;
    int elitesNum = 1;
    int MUT_ID = MutationOps::SWAP_ID;
    int CROSS_ID = CrossOps::OX_ID;

    Problem* problem = new Problem("problems/tai4_5_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;


    //20
    popSize = 40;//10,20,40
    Xp = 75;//25,50,75
    Mp = 25;//25,50,75
    CROSS_ID = CrossOps::OX_ID;//CrossOps::OX_ID,CrossOps::CX_ID
    MUT_ID = MutationOps::SWAP_ID;//MutationOps::SWAP_ID,MutationOps::INVERSE_ID


    problem = new Problem("problems/tai20_5_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;
    //return;
    problem = new Problem("problems/tai20_10_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;
    problem = new Problem("problems/tai20_20_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;


    //100
    popSize = 50;//50,100,200
    Xp = 75;//25,50,75
    Mp = 25;//25,50,75
    CROSS_ID = CrossOps::OX_ID;//CrossOps::OX_ID,CrossOps::CX_ID
    MUT_ID = MutationOps::SWAP_ID;//MutationOps::SWAP_ID,MutationOps::INVERSE_ID


    problem = new Problem("problems/tai100_10_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;
    problem = new Problem("problems/tai100_20_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;


    //500
    popSize = 250;
    Xp = 75;//25,50,75
    Mp = 25;//25,50,75
    CROSS_ID = CrossOps::OX_ID;//CrossOps::OX_ID,CrossOps::CX_ID
    MUT_ID = MutationOps::INVERSE_ID;//MutationOps::SWAP_ID,MutationOps::INVERSE_ID


    problem = new Problem("problems/tai500_20_0.fsp");
    for(int i=0;i<10;i++)
        EvoTest(problem,to_string(i),popSize,Xp,Mp,turSize,elitesNum,MUT_ID,CROSS_ID);
    delete problem;
}
void Tests::EvoConfigTest()
{

}
void Tests::GreedyTest(Problem* problem)
{
    //Init Logger
    string file_name = "out/greedy_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M);

    cout<<endl<<"TEST:"<<file_name<<endl;

    Logger logger(file_name+".csv");
    //Solve greedy J times
    for(int j=0;j<problem->J;j++)
    {
        if(problem->J == 500)
            cout<<((float)j/problem->J*100)<<"%"<<endl;
        GreadySolver solver(problem);
        Solution* s = solver.GetSolution(j);
        s->eval = problem->EstimateSolution(s);
        logger.Log(s);
        delete s;
    }
}
void Tests::GreedyTest()
{
    Problem* problem = new Problem("problems/tai4_5_0.fsp");
    GreedyTest(problem);
    delete problem;

    problem = new Problem("problems/tai20_5_0.fsp");
    GreedyTest(problem);
    delete problem;
    return;

    problem = new Problem("problems/tai20_10_0.fsp");
    GreedyTest(problem);
    delete problem;

    problem = new Problem("problems/tai20_20_0.fsp");
    GreedyTest(problem);
    delete problem;

    problem = new Problem("problems/tai100_10_0.fsp");
    GreedyTest(problem);
    delete problem;

    problem = new Problem("problems/tai100_20_0.fsp");
    GreedyTest(problem);
    delete problem;

    problem = new Problem("problems/tai500_20_0.fsp");
    GreedyTest(problem);
    delete problem;
}
void Tests::RandomTest(Problem* problem)
{
    //Init Logger
    string file_name = "out/random_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M);

    cout<<endl<<"TEST:"<<file_name<<endl;

    Logger logger(file_name+".csv");
    //Generate random solutions
    int tests = problem->J*problem->J*STANDARD_MULTIPLAYER*10;
    if(problem->J>=500)
        tests = problem->J*OVER500_MULTIPLAYER*10;
    long long int sum = 0;
    for(int i=0;i<tests;i++)
    {
        Solution* s = new Solution(problem->J);
        s->eval = problem->EstimateSolution(s);
        sum += s->eval;
        logger.Log(s);
        delete s;
    }
    cout <<"Random avg:"<< sum/tests << endl;
}
void Tests::RandomTest()
{
    Problem* problem = new Problem("problems/tai4_5_0.fsp");
    RandomTest(problem);
    delete problem;

    problem = new Problem("problems/tai20_5_0.fsp");
    RandomTest(problem);
    delete problem;

    problem = new Problem("problems/tai20_10_0.fsp");
    RandomTest(problem);
    delete problem;

    problem = new Problem("problems/tai20_20_0.fsp");
    RandomTest(problem);
    delete problem;

    problem = new Problem("problems/tai100_10_0.fsp");
    RandomTest(problem);
    delete problem;

    problem = new Problem("problems/tai100_20_0.fsp");
    RandomTest(problem);
    delete problem;

    problem = new Problem("problems/tai500_20_0.fsp");
    RandomTest(problem);
    delete problem;
}
void Tests::SATest(
Problem* problem,
std::string test_id,
float startTemp,
float coolingFactor,
int tempIterations,
int MUT_ID)
{
    //Init Logger
    string file_name = "out/sa_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M);

    if(test_id != "")
        file_name+="_"+test_id;

    cout<<endl<<"TEST:"<<file_name<<endl;

    int budget = problem->J*problem->J*STANDARD_MULTIPLAYER;//*10;
    if(problem->J>=500)
        budget = problem->J*OVER500_MULTIPLAYER;//*100;

    Logger logger(file_name+".csv");
    SAAlg sa(problem,startTemp);
    sa.coolingFactor = coolingFactor;
    sa.maxIterations = budget;
    sa.tempIterations = tempIterations;

    int best = sa.bestSolution->eval;
    while(sa.Iterate())
    {
        if(best != sa.bestSolution->eval)
        {
            best = sa.bestSolution->eval;
            //cout <<endl<<sa.currentIteration<<"/"<<sa.maxIterations<<" | temp: "<<sa.currentTemp<<" |new best:"<<best<<endl;
        }
        logger.Log(sa.currentSolution);
    }
    sa.bestSolution->print();
}
void Tests::SAParamTest()
{
    float startTemp = 0;//
    float coolingFactor = 0.995;//0.95f
    int tempIterations = 0;
    int MUT_ID = MutationOps::SWAP_ID;
    int budget = 0;
    string test_id = "MUT_ID";

    Problem* problem = new Problem("problems/tai20_10_0.fsp");
    budget = problem->J*problem->J*STANDARD_MULTIPLAYER;
    startTemp = 30000;
    coolingFactor = 0.995;//0.95
    tempIterations = 1;//budget/2100;
    MUT_ID = MutationOps::INVERSE_ID;

    for(int i=0;i<10;i++)
        SATest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    MUT_ID = MutationOps::INVERSE_ID;
    for(int i=0;i<10;i++)
        SATest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;
    problem = new Problem("problems/tai100_10_0.fsp");
    budget = problem->J*problem->J*STANDARD_MULTIPLAYER;
    startTemp = 400000;
    coolingFactor = 0.995;//0.95f
    tempIterations = 47;//budget/2100;
    MUT_ID = MutationOps::INVERSE_ID;

    for(int i=0;i<10;i++)
        SATest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    MUT_ID = MutationOps::INVERSE_ID;
    for(int i=0;i<10;i++)
        SATest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;

    problem = new Problem("problems/tai500_20_0.fsp");
    budget = problem->J*OVER500_MULTIPLAYER;
    startTemp = 7000000;
    coolingFactor = 0.95;
    tempIterations = 23;//budget/2100;
    MUT_ID = MutationOps::SWAP_ID;

    for(int i=0;i<10;i++)
        SATest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    MUT_ID = MutationOps::INVERSE_ID;
    for(int i=0;i<10;i++)
        SATest(problem,test_id +"_"+ to_string(MUT_ID)+"_"+to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;
}
void Tests::SATest(Problem* problem)
{
    //Init Logger
    string file_name = "out/sa_";
    file_name += to_string(problem->J)+"_"+to_string(problem->M);

    cout<<endl<<"TEST:"<<file_name<<endl;

    int budget = problem->J*problem->J*10;

    Logger logger(file_name+".csv");
    float startTemp = problem->J*problem->J;
    SAAlg sa(problem,startTemp);
    sa.coolingFactor = 0.995;
    sa.maxIterations = budget;//problem->J * 1000;
    sa.tempIterations = 25;//problem->J;

    int best = sa.bestSolution->eval;
    while(sa.Iterate())
    {
        if(best != sa.bestSolution->eval)
        {
            best = sa.bestSolution->eval;
            cout <<endl<<sa.currentIteration<<"/"<<sa.maxIterations<<" | temp: "<<sa.currentTemp<<" |new best:"<<best<<endl;
        }
        logger.Log(sa.currentSolution);
    }
    sa.bestSolution->print();
}
void Tests::SATest()
{
    float startTemp = 100;
    float coolingFactor = 0.95;
    int tempIterations = 1;
    int MUT_ID = MutationOps::SWAP_ID;
    Problem* problem = new Problem("problems/tai4_5_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;

    //20
    startTemp = 30000;
    coolingFactor = 0.995;//0.95
    tempIterations = 1;//budget/2100;
    MUT_ID = MutationOps::INVERSE_ID;

    problem = new Problem("problems/tai20_5_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;
    //return;

    problem = new Problem("problems/tai20_10_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;

    problem = new Problem("problems/tai20_20_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;

    //100
    startTemp = 400000;
    coolingFactor = 0.995;//0.95f
    tempIterations = 47;//budget/2100;
    MUT_ID = MutationOps::INVERSE_ID;

    problem = new Problem("problems/tai100_10_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;

    problem = new Problem("problems/tai100_20_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;

    //500
    startTemp = 7000000;
    coolingFactor = 0.95;
    tempIterations = 23;//budget/2100;
    MUT_ID = MutationOps::SWAP_ID;

    problem = new Problem("problems/tai500_20_0.fsp");
    startTemp = problem->J*problem->J*problem->M*problem->M;
    for(int i=0;i<10;i++)
        SATest(problem,to_string(i),startTemp,coolingFactor,tempIterations,MUT_ID);
    delete problem;
}
