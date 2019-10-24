#Instrucciones para compilar y ejecutar los tests

#Tests de correctitud
Para compilar se debe utilizar el comando "make TP1INV", que
generara el archivo "TP1INV".
No se debe ejecutar directamente, si no que para ejecutar los 
tests se debe utilizar el script de Bash "run_INV_tests", este por
default hace 10000 iteraciones de los grafos en la carpeta instances 
y se asegura que generen correctamente el árbol generador mínimo. 
En el caso de que el test sea exitoso se imprimira "DONE, ALL TESTS PASSED".

#Tests de tiempo
Para compilar se debe utilizar el comando "make TP1T", que
generará el archivo "TP1_testing".
Necesita tener creada la carpeta "generar_grafos".
Primero Correr los scripts python grafoCompleto.py 1000, y python grafoRandom.py 1000.
Para ejecutar los tests se puede ejecutar el comando python3 correTP1.py 1 K , 
pasando como argumento el número de iteraciones y la letra "R" o "K" como segundo 
argumento para especificar si se deben utilizar las instancias de grafos aleatorios
 o completos respectivamente.

