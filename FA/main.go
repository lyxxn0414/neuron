package main

import (
	fa "gitee.com/liyue/FA_module" 
)

func main() {
	stop := make(chan bool)
	defer func() { stop <- true }()
	go fa.PeriodicTask(stop)
	for{

	}
}