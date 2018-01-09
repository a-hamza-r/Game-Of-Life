# Game-Of-Life
Parallelizing Conway's game of life

## Introduction:
	The Conway’s Game Of Life is a 0-player grid based game. Each cell in the grid is either dead or alive. The game starts by an initial seed (a grid). Every successive generation is calculated from the previous state of the grid according to following rules:
Any live cell with fewer than two live neighbors dies
Any live cell with two or three live neighbors lives
Any live cell with more than three live neighbors dies
Any dead cell with exactly three live neighbors lives

## Objective:
The objective is to parallelize the computation of each successive generation using grid decomposition techniques so that the time of the computation can be reduced.

## Implementations: 
There are two implementations, GOLANG and MPI. They are in separate folders. 

## Results: 
Results are in Test_Results folder.
