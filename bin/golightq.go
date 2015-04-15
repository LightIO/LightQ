package main
// #cgo CFLAGS: -m64 -DDEBUG -std=c11 -Wall -Wextra -O3  -g  -I ../include
// #cgo LDFLAGS: -L . -llightq -L/opt/zeromq/lib -lzmq
// #include "lightq_api.h"
import "C"

func main() {
        
        C.create_topic(C.CString("tcp://127.0.0.1:5500"), C.CString("test_1"), C.CString("lightq_admin"), C.CString("T0p$3cr31"), C.CString("test_admin"), C.CString("T0p$3cr31"), 0)
}