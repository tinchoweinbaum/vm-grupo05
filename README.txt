Grupo 05 - Martín Weinbaum, Andoni Catalano y Juan Pablo Gueli

La máquina virtual está representada en el programa como un struct con dos vectores: memoria y registros, en este struct se encuentra también la tabla de segmentos.
El programa cuenta con un archivo main.c y dos archivos adicionales de nuestra autoria operations.c y operations.h para el correcto funcionamiento de la maquina virtua.

Trabajo practico hecho para la carrera de Ingenieria informatica de la universidad nacional de Mar del Plata, asignatura de Fundamentos de la Arquitectura de computadoras del segundo cuatrimestre de 2025.

Esta branch tiene agregada la funcionalidad de un vector de instrucciones, para que las instrucciones de salto hagan referencia a número de línea en el código, en lugar de a un espacio de memoria.

Por ejemplo: JMP 12 ahora salta a la linea de código número 12 (en el archivo .asm) en lugar de a mv->mem[12]