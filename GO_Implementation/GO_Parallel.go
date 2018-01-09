package main

import (
	"os"
	"strconv"
    "os/exec"
	"fmt"
	// "io/ioutil"
	"time"
	"runtime"
	"math/rand"
)
var currentGen [][]bool;
var nextGen [][]bool;
var numProcs = 1 /* set number of processor to use from here */

//EXTRA FUNCTIONS
func min(x,y int)int {
	if x<=y {
		return x
	}
	return y
}
//-----------------------------------------------------------
func MakeBoard(rows,cols int) {
	// Make board of currentGen for MxN matrix
	currentGen,nextGen = make([][]bool,rows+2),make([][]bool,rows+2)
	for i,_ := range currentGen{
		currentGen[i],nextGen[i] = make([]bool ,cols+2),make([]bool ,cols+2)
	}
}


//----------------------FOR DEBUGGING-----------------------
func FillAll() {
	// Fill current Gen with random life
	lenr:= len(currentGen)-1
	lenc:= len(currentGen[0])-1
	for i,_:=range currentGen {
		for j,_:=range currentGen[i] {
			if i==0 || i==lenr || j==0 || j==lenc {
				continue//to let ghost cell remain false
			}
			currentGen[i][j] = (i%3) == 0
		}
	}
}


//-----------------------------------------------------------
func FillBoard() {
	// Fill current Gen with random life
	lenr:= len(currentGen)-1
	lenc:= len(currentGen[0])-1
	for i,_:=range currentGen {
		for j,_:=range currentGen[i] {
			if i==0 || i==lenr || j==0 || j==lenc {
				continue//to let ghost cell remain false
			}
			currentGen[i][j] = rand.Intn(2) != 0
		}
	}
}

//-----------------------------------------------------------
// PARALLELIZING HERE
func NextGameState() {
	done := make(chan bool)
	var end int
	length:= (len(currentGen)-2)
	numProcs = min(numProcs,length)//to maintain order
	iter:=(length/numProcs)

	for i := 1; i <= length; i+=iter {
		end= min((i-1)+iter,length)
	go func(low,hi int){
		for ; low <= hi; low++ {
			for j := 1; j < (len(currentGen[low])-1); j++ {
					nextGen[low][j] = NextCellState(low,j)
				}
			}
		done<-true
		}(i,end)
	}
	for i := 1; i <= length; i+=iter {
		<-done
	}
}

//-----------------------------------------------------------
func NextCellState(row, col int) bool {
	count := 0
	for i := -1; i < 2; i++ {
		for j := -1; j < 2; j++ {
			if i != 0 || j != 0 /*&& row+i != 0 && col+j != 0 && row+i != len(board)-1 && col+j != len(board[1])-1*/{
				if currentGen[row+i][col+j] == true {
					count++
				}
			}
		}
	}
	if currentGen[row][col] == true {
		if count < 2 {
			return false
		} else if count == 2 || count == 3 {
			return true
		} else if count > 3 {
			return false
		}
	} else if currentGen[row][col] == false {
		if count == 3 {
			return true
		}
	}
	return currentGen[row][col]
}

//-----------------------------------------------------------
func BoardToString(board[][] bool) string{
	var theString string
	for _,x := range board{
		for _,item := range x{
				if item != false{
					theString = theString + "*"
				}else{
					theString = theString + " "
				}
		}
			theString = theString+"\n"
	}	
	return theString[0:len(theString)]
}

func check(e error) {
    if e != nil {
        panic(e)
    }
}
func main() {
	runtime.GOMAXPROCS(numProcs)
	//file write
	// f,err:= os.Create("data")
	// check(err)
	// defer f.Close()


	if len(os.Args) < 4 {
		fmt.Println("Usage: go run <gofilename>.go <rows> <cols> <iterations>")
		return
	}
	rows, err := strconv.Atoi(os.Args[1])
	if err != nil {
		fmt.Println(err)
		return
	}
	cols, err := strconv.Atoi(os.Args[2])
	if err != nil {
		fmt.Println(err)
		return
	}
	iters, err := strconv.Atoi(os.Args[3])
	if err != nil {
		fmt.Println(err)
		return
	}

	MakeBoard(rows,cols)
	FillBoard()
	//FillAll()
	// fmt.Println(currentGen)

	cmd := exec.Command("clear")
	cmd.Stdout = os.Stdout
	cmd.Run()
	fmt.Println("NUMPROCS",numProcs)
	fmt.Println("ITERATIONS",iters)
	fmt.Println("PROBLEM SIZE",rows,cols)
	var total time.Duration
	for i := 0; i < iters; i++ {
		//CREATE GENERAIONS	
		t1:=time.Now()	
		NextGameState()
		t2 := time.Now()
		elapsed:= t2.Sub(t1)
		total += elapsed
	
		fmt.Println("elapsed time:",elapsed)
		// fmt.Println(BoardToString(nextGen))
		currentGen,nextGen = nextGen,currentGen
		time.Sleep(100*time.Millisecond)
	}
	fmt.Println("Average case:",total/time.Duration(iters))
}