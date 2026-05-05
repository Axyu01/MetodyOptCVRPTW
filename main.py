import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
PATH_FILES = "out"
def stats(data,DEBUG = False):
    best = np.array(data)
    std = np.std(best)
    mean = np.mean(best)
    bestVal = best.min()
    worstVal = best.max()
    if DEBUG is True:
        print(f"std: {std} mean:{mean} bestVal: {bestVal} worstVal: {worstVal}")
    return bestVal,worstVal,std,mean
    #print(df.shape)

def statsFile(alg = "random",jobs = 500,machines = 20,DEBUG = False):
    df = pd.read_csv(f"{PATH_FILES}/{alg}_{jobs}_{machines}.csv", header=None,sep = ";")
    #best = np.array(df)
    #std = np.std(best)
    #mean = np.mean(best)
    #bestVal = best.min()
    bestVal,worstVal,std,mean = stats(np.array(df[0]))
    #print(df.shape)
    if DEBUG == True:
        print(f"{alg} jobs:{jobs} machines:{machines}\n std: {std} mean:{mean} bestVal: {bestVal}  worstVal: {worstVal}")

    return bestVal,worstVal,std,mean

def statsAlgFiles(alg = "sa",jobs = 500,machines = 20,DEBUG = False):
    print(alg)
    print(jobs)
    print(machines)
    best = []
    for index in range(10):
        if alg == "evo":
            print(f"{PATH_FILES}/{alg}_{jobs}_{machines}_{index}"+"_best.csv")
            df = pd.read_csv(f"{PATH_FILES}/{alg}_{jobs}_{machines}_{index}"+"_best.csv", header=None,sep = ";")
        else:
            df = pd.read_csv(f"{PATH_FILES}/{alg}_{jobs}_{machines}_{index}"+".csv", header=None,sep = ";")
        df = np.array(df[0])
        best.append(df.min())
    bestVal,worstVal,std,mean = stats(np.array(best))
    if DEBUG == True:
        print(f"{alg} jobs:{jobs} machines:{machines}\n std: {std} mean:{mean} bestVal: {bestVal}  worstVal: {worstVal}")
    return bestVal,worstVal,std,mean

def statsConfigFiles(alg = "",jobs = 500,machines = 20,test_id="popSize",test_val = 30,DEBUG = False):
    best = []
    for index in range(10):
        if alg == "evo":
            print(f"{PATH_FILES}/{alg}_{jobs}_{machines}_{test_id}_{test_val}_{index}"+"_best.csv")
            df = pd.read_csv(f"{PATH_FILES}/{alg}_{jobs}_{machines}_{test_id}_{test_val}_{index}"+"_best.csv", header=None,sep = ";")
        else:
            df = pd.read_csv(f"{PATH_FILES}/{alg}_{jobs}_{machines}_{test_id}_{test_val}_{index}"+".csv", header=None,sep = ";")
        df = np.array(df[0])
        best.append(df.min())
    bestVal,worstVal,std,mean = stats(np.array(best))
    if DEBUG == True:
        print(f"{alg} jobs:{jobs} machines:{machines}\n std: {std} mean:{mean} bestVal: {bestVal}  worstVal: {worstVal}")
    return bestVal,worstVal,std,mean

def plot(alg = "sa",jobs = 500,machines = 20,index = 0):
    #alg = "sa"
    #jobs = 500
    #machines = 20
    #index = 1
    if alg == "evo":
        files = [
        (f"out/{alg}_{jobs}_{machines}_{index}"+"_avg.csv", "blue", "Avg"),
        (f"out/{alg}_{jobs}_{machines}_{index}"+"_best.csv", "red", "Best"),
        (f"out/{alg}_{jobs}_{machines}_{index}"+"_worst.csv", "green", "Worst"),
        ]
    else:
        files = [
            (f"out/{alg}_{jobs}_{machines}_{index}"+".csv", "blue", "Value"),
        ]

    for file, color, label in files:
        df = pd.read_csv(file, header=None,sep = ";")
        plt.plot(df[0], color=color, label=label)  # x = index

    plt.legend()
    plt.xlabel("Iteration")
    plt.ylabel("Value")
    plt.title(f"{alg} jobs:{jobs} machines:{machines}")
    plt.show()

def create_table():
    output = """Instancja & opt & \multicolumn{4}{c}{Random [10k]} & \multicolumn{4}{c}{Greedy [n]} & \multicolumn{4}{c}{EA [10x]} & \multicolumn{4}{c}{SA [10x]} \\\\
&  & best & worst & avg & std & best & worst & avg & std & best & worst & avg & std & best & worst & avg & std \\\\
\hline"""

    instances = [#(jobs,machines,optimal)
        (4,5,1170),
        (20,5,14033),
        (20,10,20911),
        (20,20,33623),
        (100,10,300566),
        (100,20,367267),
        (500,20,6687476),
        ]

    for instance in instances:
        output +=f"\ntai{instance[0]}-{instance[1]}-0 & {instance[2]} "
        bestVal,worstVal,std,mean = statsFile(alg = "random",jobs = instance[0],machines=instance[1])
        std,mean = round(std),round(mean)
        output +=f"& {bestVal} & {worstVal} & {mean} & {std} "
        bestVal,worstVal,std,mean = statsFile(alg = "greedy",jobs = instance[0],machines=instance[1])
        std,mean = round(std),round(mean)
        output +=f"& {bestVal} & {worstVal} & {mean} & {std} "
        bestVal,worstVal,std,mean = statsAlgFiles(alg = "evo",jobs = instance[0],machines=instance[1])
        std,mean = round(std),round(mean)
        output +=f"& {bestVal} & {worstVal} & {mean} & {std} "
        bestVal,worstVal,std,mean = statsAlgFiles(alg = "sa",jobs = instance[0],machines=instance[1])
        std,mean = round(std),round(mean)
        output +=f"& {bestVal} & {worstVal} & {mean} & {std} "
        output += "\\\\"

    output+= "\n\hline"
    print(output)
def create_table_config(alg = "evo",test_id = "popSize",param_values_easy = [10,20,40],param_values_medium = [50,100,200],param_values_large = [250,500,1000]):
    output = """Instancja & opt & wartość param  & \multicolumn{4}{c}{EA [10x]}\\\\
& & & best & worst & avg & std \\\\
\hline"""

    instances = [#(machines,jobs,optimal)
        #(4,5,1170),
        #(20,5,14033),
        (20,10,20911),
        #(20,20,33623),
        (100,10,300566),
        #(100,20,367267),
        (500,20,6687476),
        ]
    params_vals = [
        param_values_easy,
        param_values_medium,
        param_values_large
           ]

    for i in range(3):
        instance = instances[i]
        params = params_vals[i]
        for param in params:
            output +=f"\ntai{instance[0]}-{instance[1]}-0 & {instance[2]} & {param} "
            bestVal,worstVal,std,mean = statsConfigFiles(alg = alg,jobs = instance[0],machines=instance[1],test_id=test_id,test_val=param)
            std,mean = round(std),round(mean)
            output +=f"& {bestVal} & {worstVal} & {mean} & {std} "
            output += "\\\\"

    output+= "\n\hline"
    print(output)

def findBest(alg = "sa",jobs = 500,machines = 20,index = None):
    path = f"{PATH_FILES}/{alg}_{jobs}_{machines}"
    if index is not None:
        path += f"_{index}"
    if alg == "evo":
         path += "_best"
    path +=".csv"

    df = pd.read_csv(path, header=None,sep = ";")

    vals = np.array(df[0])
    arg = vals.argmin()
    value = vals[arg]
    solution = []
    for g in range(1,jobs+1):
        solution.append(df[g][arg])

    return value,solution

def printBestSolutions():
    print("Best solutions")

    instances = [#(jobs,machines,optimal)
        (4,5,1170),
        (20,5,14033),
        (20,10,20911),
        (20,20,33623),
        (100,10,300566),
        (100,20,367267),
        (500,20,6687476),
        ]

    for instance in instances:
        print(f"\nINSTANCE tai{instance[0]}-{instance[1]}-0 OPTIMAL VAL: {instance[2]} ")
        print("RANDOM")
        value,solution = findBest(alg = "random",jobs = instance[0],machines=instance[1])
        print(f"value:{value}")
        print(f"solution:{solution}")
        print("GREADY")
        value,solution = findBest(alg = "random",jobs = instance[0],machines=instance[1])
        print(f"value:{value}")
        print(f"solution:{solution}")
        for i in range(10):
            print(f"EA {i+1}")
            value,solution = findBest(alg = "evo",jobs = instance[0],machines=instance[1],index=i)
            print(f"value:{value}")
            print(f"solution:{solution}")
        for i in range(10):
            print(f"SA {i+1}")
            value,solution = findBest(alg = "sa",jobs = instance[0],machines=instance[1],index=i)
            print(f"value:{value}")
            print(f"solution:{solution}")
    pass
if __name__ == "__main__":
    #plot(alg = "sa",jobs = 500,machines = 20,index = 0)
    DEBUG = True
    #statsFile(alg = "random",DEBUG=DEBUG)
    #statsFile(alg = "greedy",DEBUG=DEBUG)
    #statsFile(alg = "sa",DEBUG=DEBUG)

    #statsAlgFiles(alg="sa",jobs=20,machines=10,DEBUG=DEBUG)
    #statsAlgFiles(alg="evo",jobs=100,machines=20,DEBUG=DEBUG)
    #statsAlgFiles(alg="evo",jobs=500,machines=20,DEBUG=DEBUG)
    #create_table()
    printBestSolutions()

    #Config Tables
    #create_table_config(alg = "evo",test_id="popSize",param_values_easy = [10,20,40],param_values_medium = [50,100,200],param_values_large = [250,500,1000])
    #create_table_config(alg = "evo",test_id="Xp",param_values_easy = [25,50,75],param_values_medium = [25,50,75],param_values_large = [25,50,75])
    #(alg = "evo",test_id="Mp",param_values_easy = [25,50,75],param_values_medium = [25,50,75],param_values_large = [25,50,75])
    #create_table_config(alg = "evo",test_id="CROSS_ID",param_values_easy = [0,1,2],param_values_medium = [0,1,2],param_values_large = [0,1,2])#0->OX, 1-> CX,2 -> PMX
    #create_table_config(alg = "evo",test_id="MUT_ID",param_values_easy = [0,1],param_values_medium = [0,1],param_values_large = [0,1])#0->SWAP, 1-> INVERSE
    #create_table_config(alg = "sa",test_id="coolingFactor",param_values_easy = ["0.950000","0.995000"],param_values_medium = ["0.950000","0.995000"],param_values_large = ["0.950000","0.995000"])
    #create_table_config(alg = "sa",test_id="tempIterations",param_values_easy = [1,20],param_values_medium = [47,500],param_values_large = [23,250])
    #create_table_config(alg = "sa",test_id="MUT_ID",param_values_easy = [0,1],param_values_medium = [0,1],param_values_large = [0,1])#0->SWAP, 1-> INVERSE