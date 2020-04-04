package main

import (
	"os"
	"fmt"
)

func create(time int) []byte {
	var meta []byte;
	for i := 'a'; i <= 'z'; i++ {
		meta = append(meta, byte(i))
	}
	
	for idx := 1; idx <= time; idx++ {
		meta = append(meta, meta...)
	}
	fmt.Printf("len %v\n", len(meta))
	return meta
}

func main() {
	file := "test"
	f, err := os.Create(file)
	if err != nil {
		return
	}

	content := create(20 + 4) // 26 * 16M
	f.Write(content);

}
