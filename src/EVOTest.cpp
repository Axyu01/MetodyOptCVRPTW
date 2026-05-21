#include "EVOTest.h"
#include "Solution.h"
#include "Logger.h"
#include "MutationOps.h"
#include "CrossOps.h"

void EVOTest::UniversalTest(Problem* problem,EvoAlg* evo,std::string test_id)
{
    //Init Loggers
    string file_name = "out/evo_";
    file_name += problem->NAME;
    if(test_id != "")
        file_name+="_"+test_id;


    cout<<endl<<"TEST:"<<file_name<<endl;

    Logger bestLogger(file_name+"_best.csv");
    Logger worstLogger(file_name+"_worst.csv");
    Logger avgLogger(file_name+"_avg.csv");

    int budget = problem->SIZE*problem->SIZE*STANDARD_MULTIPLAYER;//*10;
    if(problem->SIZE >= 500)
        budget = problem->SIZE*OVER500_MULTIPLAYER;//*100;

    int loops = budget/evo->popSize;

    //Start Evolution
    evo->Init();
    evo->Eval();

    //Entry Logging
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

