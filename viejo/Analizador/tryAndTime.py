import os
import time
import sys 
from os import listdir
from os.path import isfile, join
import pandas as pd

def separarNombre(file_name):
    aux = file_name[:-4]
    aux_list = aux.split("-")
    nodos = aux_list[0].split("_")
    ejes = aux_list[1].split("_")
    densidad = aux_list[2].split("_")  
    return (nodos[1], ejes[1], densidad[1])

times = int(sys.argv[1])
#thread_count = int(sys.argv[2])
path_str = sys.argv[2]

df = pd.DataFrame(columns=["nodos","ejes","densidad","threads","tiempo"])

onlyfiles = [f for f in listdir(path_str) if isfile(join(path_str, f))]

n,e,d = separarNombre(onlyfiles[0])

with open("log.log",'w') as ff:
    ff.write(str(onlyfiles))
for file_name in onlyfiles:
    n,e,d = separarNombre(file_name)
    for thread_count in [2**i for i in range(8)]:
        for i in range(times):
            print("#"*12)
            print("STARTING RUN #"+str(i)+" of "+file_name+" with "+str(thread_count)+" threads")

            start = time.time()
            command = "./TP1 "+path_str+file_name+" "+str(thread_count)

            os.system(command)
            end = time.time()
            print("time:{0},{1},{2}".format(i, file_name, end - start))
            df2 = pd.DataFrame({"nodos":[n], "ejes":[e],
                                "densidad":[d],"threads":[thread_count] ,"tiempo":[end-start]}) 
  
            df = df.append(df2, ignore_index = True) 
            print("#"*12)

    thread = 1
    for i in range(times):
        print("#"*12)
        print("STARTING RUN #"+str(i)+" of "+file_name+" with "+str(thread)+" threads")

        start = time.time()
        command = "./TP1 "+path_str+file_name+" "+str(thread)
        os.system(command)
        end = time.time()
        print("time:{0},{1},{2}".format(i, file_name, end - start))
        df2 = pd.DataFrame({"nodos":[n], "ejes":[e],
                            "densidad":[d],"threads":[thread], "tiempo":[end-start]}) 

        df = df.append(df2, ignore_index = True) 
        print("#"*12)

    thread = 0
    for i in range(times):
        print("#"*12)
        print("STARTING RUN #"+str(i)+" of "+file_name+" with "+str(thread)+" threads")

        start = time.time()
        command = "./TP1simple "+path_str+file_name
        os.system(command)
        end = time.time()
        print("time:{0},{1},{2}".format(i, file_name, end - start))
        df2 = pd.DataFrame({"nodos":[n], "ejes":[e],
                            "densidad":[d],"threads":[thread], "tiempo":[end-start]}) 

        df = df.append(df2, ignore_index = True) 
        print("#"*12)


df.to_csv("tiempos.csv")
