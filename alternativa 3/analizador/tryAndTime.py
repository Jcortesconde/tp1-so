import os
import time
import sys 
from os import listdir
from os.path import isfile, join
import subprocess

def not_good_weight(file):
    with open(file, "r") as info:
        for line in info:
            if(line.find("termino peso:") != -1):
                weight = int(line.split(" ")[2])
                return not(weight == 5094)
    return True

times = 100
onlyfiles = ["../arbol1000.txt"]
for file_name in onlyfiles:
    for thread_count in[5]:# [2**i for i in range(8)]:
        for i in range(times):
            output = ""
            print("#"*12)
            print("STARTING RUN #"+str(i)+" of "+file_name+" with "+str(thread_count)+" threads")

            with open('run.txt','w+') as fout:
                command = "../TP1 "+file_name+" "+str(thread_count)
                out=subprocess.call([command],stdout=fout,shell=True)
                fout.write("\nThis run %d\n" % (i))
                fout.seek(0)
                

            if(not_good_weight("run.txt")):
                print("not good weight, at run",i)
                break
            
