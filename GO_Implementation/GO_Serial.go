package main

import (
	"os"
	"strconv"
    "os/exec"
	"fmt"
	// "io/ioutil"
	"time"
	"math/rand"
	"runtime"
)

func MakeBoard(rows, cols int) (board [][]bool) {
	board = make([][]bool, rows+2)
	for i, _ := range board {
		board[i] = make([]bool, cols+2)
	}
	return board
}

func BoardToString(board[][] bool) string{
	var theString string
	for i,x := range board{
		for j,item := range x{
			// fmt.Println(i!=0 && i!= (len(board)-1) && j != 0 && j!=(len(x)-1))
			if i!=0 && i!= (len(board)-1) && j != 0 && j!=(len(x)-1){
				if item != false{
					theString = theString + "*"
				}else{
					theString = theString + " "
				}
			}
		}
		if i!=0 && i!= (len(board)-1){
			theString = theString+"\n"
		}
		// fmt.Printf("\n")
	}	
	return theString[0:len(theString)]
}

func StringToBoard(str []byte,board [][]bool) {
	i:= 1
	j :=0+1
	for _,val := range str {
		if val == '\n'{
			i++
			j = 0+1
			continue
		}
		if val =='1'{//change this if neccessary
				board[i][j] = true
			}else{
				board[i][j] = false
			}
		j++
	}
	return
}

func NextCellState(board [][]bool, row, col int) bool {
	count := 0
	for i := -1; i < 2; i++ {
		for j := -1; j < 2; j++ {
			if i != 0 || j != 0 /*&& row+i != 0 && col+j != 0 && row+i != len(board)-1 && col+j != len(board[1])-1*/{
				if board[row+i][col+j] == true {
					count++
				}
			}
		}
	}
	if board[row][col] == true {
		if count < 2 {
			return false
		} else if count == 2 || count == 3 {
			return true
		} else if count > 3 {
			return false
		}
	} else if board[row][col] == false {
		if count == 3 {
			return true
		}
	}
	return board[row][col]
}

func NextGameState(oldBoard [][]bool, newBoard [][]bool) {
	for i := 1; i < (len(oldBoard)-1); i++ {
		for j := 1; j < (len(oldBoard[i])-1); j++ {
					newBoard[i][j] = NextCellState(oldBoard,i,j)
				}		
	}		
}


//-----------------------------------------------------------
func FillBoard(board [][]bool) {
	// Fill current Gen with random life
	lenr:= len(board)-1
	lenc:= len(board[0])-1
	for i,_:=range board {
		for j,_:=range board[i] {
			if i==0 || i==lenr || j==0 || j==lenc {
				continue//to let ghost cell remain false
			}
			board[i][j] = rand.Intn(2) != 0
		}
	}
}


func main() {
	runtime.GOMAXPROCS(1)
	if len(os.Args) < 4 {
		fmt.Println("Usage: go run game_of_life.go <rows> <cols> <iterations>")
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
	oldBoard := MakeBoard(rows, cols)
	FillBoard(oldBoard)
	newBoard := MakeBoard(rows, cols)


	//parallel PART START
	cmd := exec.Command("clear")
	cmd.Stdout = os.Stdout
	cmd.Run()

	fmt.Println("NUMPROCS",1)
	fmt.Println("ITERATIONS",iters)
	fmt.Println("PROBLEM SIZE",rows,cols)
	var total time.Duration	
	for i := 0; i < iters; i++ {
		t1:= time.Now()
		NextGameState(oldBoard, newBoard)
		t2:= time.Now()
		elapsed:= t2.Sub(t1)
		total +=elapsed
		fmt.Println("Time Elapsed:",elapsed)
	
		// fmt.Print("\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  GENERATION #")
		// fmt.Print(i+1)
		// fmt.Println("  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
		// fmt.Print(BoardToString(newBoard))
		oldBoard, newBoard = newBoard, oldBoard
		time.Sleep(time.Second/30)
	}
	fmt.Println("Average case:",total/time.Duration(iters))
}
