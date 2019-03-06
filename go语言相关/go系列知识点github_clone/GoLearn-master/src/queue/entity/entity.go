package main

import (
	"GoLearn/src/queue"
	"fmt"
)

func main()  {
	q := queue.Queue{1}

	q.Push(2)
	q.Push("str")
	q.Push(3)

	fmt.Println(q.Pop())
	fmt.Println(q.Pop())
	fmt.Println(q.IsEmpty())
	fmt.Println(q.Pop())
	fmt.Println(q.IsEmpty())
}
