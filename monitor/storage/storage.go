package storage

import (
	"encoding/binary"
	"bytes"
	"fmt"
	"net"
	"unsafe"
	"strconv"
)

func Connect_socket(){
	// 连接到influxdb
	Connect_db()
	// 监听指定端口
	listener, err := net.Listen("tcp", ":8080") // 替换为实际的监听端口
	if err != nil {
		fmt.Println("无法启动服务器:", err)
		return
	}
	defer listener.Close()
	defer Close_db()

	fmt.Println("服务器已启动，等待客户端连接...")

	for {
		// 等待客户端连接
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("接受客户端连接失败:", err)
			return
		}

		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()

	// 接收客户端数据
	// dp := (*DataPackage)(unsafe.Pointer(&buffer))
	// var num = len/ ID_LENGTH
	// temp := make([][ID_LENGTH]byte, num)

	// for i := 0; i < num; i++ {
	// 	copy(temp[i][:], dp.params[i*ID_LENGTH:(i+1)*ID_LENGTH])
	// }

	// for i := 0; i < num; i++ {
	// 	id := string(temp[i][:])
	// 	fmt.Printf("The id is: %s\n", id)
	// }
	for{
		buffer := make([]byte, 1024)
	len, err := conn.Read(buffer)
	if err != nil {
		fmt.Println("接收客户端数据失败:", err)
		return
	}
	fmt.Println("len:",len,",recv:", string(buffer[:len]))

	// 创建一个与结构体具有相同内存布局的临时结构体
	var temp DataPackage

	// 使用unsafe.Pointer将[]byte的指针转换为结构体的指针
	ptr := unsafe.Pointer(&temp)

	// 将字节数组复制到结构体指针中
	copy((*[unsafe.Sizeof(temp)]byte)(ptr)[:], buffer)

	var ret []byte
	fmt.Println("func_name",binary.LittleEndian.Uint16(temp.func_name[:]))
	switch binary.LittleEndian.Uint16(temp.func_name[:]){
		// POST_INFO
		case 1: 
		    fmt.Println("Save info...");
			var num int = len/int(unsafe.Sizeof(Info{}))
			ret = save_info(temp.params[:],num);
			break;
		// POST_CKP
		case 2:
			fmt.Println("Save ckp...");
			var num int = len/int(unsafe.Sizeof(Checkpoint{}))
			ret = save_ckp(temp.params[:],num);
			break;
		// GET_CKP
		case 3: 
		    var num int = (len-ID_LENGTH)/CKP_ID_LENGTH
			ret = get_ckp(temp.params[:],num);
			break;
	}
	// 发送响应给客户端
	_, err = conn.Write(ret)
	if err != nil {
		fmt.Println("发送响应给客户端失败:", err)
		return
	}
	}

}

func get_ckp(b []byte, num int)(v []byte){
	var ids []string
	chip_id := GetString(b[:ID_LENGTH])
	// fmt.Println("The num is:", num)
	// fmt.Println("The chip_id is:", chip_id)
	for i := 0; i < num; i++ {
		str := GetString(b[ID_LENGTH+i*CKP_ID_LENGTH:ID_LENGTH+i*CKP_ID_LENGTH+CKP_ID_LENGTH])
		// fmt.Println("The ID length is:",len(str))
		ids = append(ids,str)
	}
	ret := getCkp(chip_id,ids,num)
	for i := 0; i < num; i++{
		buf := &bytes.Buffer{}
		err := binary.Write(buf, binary.LittleEndian, ret[i])
		if err != nil {
			panic(err)
		}
		v = append(v, buf.Bytes()...)
	}
	return v
}

func save_info(b []byte,num int)(v []byte){
	// fmt.Println("Save info...");
	fmt.Println(num);
	infos := make([]Info, num)
	for i := 0; i < num; i++ {
		// 创建一个与结构体具有相同内存布局的临时结构体
		var temp Info

		// 使用unsafe.Pointer将[]byte的指针转换为结构体的指针
		ptr := unsafe.Pointer(&temp)
	
		// 将字节数组复制到结构体指针中
		copy((*[unsafe.Sizeof(temp)]byte)(ptr)[:], b[i*int(unsafe.Sizeof(Info{})):i*int(unsafe.Sizeof(Info{}))+int(unsafe.Sizeof(Info{}))])
		// fmt.Println("Time is:",string(temp.time[:]));
		infos[i] = temp
	}

	writeInfo(infos,num)
	v = append(v,byte(1))
	return v;
}

func save_ckp(b []byte, num int)(v []byte){
	fmt.Println("Save ckp...");
	ckps := make([]Checkpoint,num)
	for i := 0; i < num; i++ {
		// 创建一个与结构体具有相同内存布局的临时结构体
		var temp Checkpoint

		// 使用unsafe.Pointer将[]byte的指针转换为结构体的指针
		ptr := unsafe.Pointer(&temp)
	
		// 将字节数组复制到结构体指针中
		copy((*[unsafe.Sizeof(temp)]byte)(ptr)[:], b[i*int(unsafe.Sizeof(Checkpoint{})):i*int(unsafe.Sizeof(Checkpoint{}))+int(unsafe.Sizeof(Checkpoint{}))])
		fmt.Println("Time is:",string(temp.Time[:]));
		ckps[i]=temp
	}
	writeCkp(ckps,num)
	res := true
	v = append(v,byte(strconv.FormatBool(res)[0]))
	return v;
}

func my_get(b []byte, num int)([]byte){
	r := make([]byte,num-len(b))
	return append(b,r...)
} 