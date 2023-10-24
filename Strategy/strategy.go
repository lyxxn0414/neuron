package Strategy

import(
	"fmt"
	Utils "gitee.com/liyue/Utils"  // 使用自定义包
	"net"
	"bytes"
	"encoding/binary"
)

func Handlefault(id [2]string, ctx Utils.Error_Context)(bool){
		fmt.Println("ID:",id,"Error Code:", ctx.Err)
		for _, ckp := range ctx.Ckps{
			fmt.Println("Agent ID:",Utils.GetString(ckp.Agent_id[:]),"Chip_id:",Utils.GetString(ckp.Id[:]),"Ckp_id:",Utils.GetString(ckp.Ckp_id[:]))
		}
		fmt.Println(len(ctx.Ckps))
		// 连接到C++服务器
		conn, err := net.Dial("tcp", ctx.IP+":"+ctx.Port)
		if err != nil {
			fmt.Println("无法连接到C++服务器:", err)
			return false
		}
		defer conn.Close()
		// 发送消息到C++服务器
		var v []byte
		for i := 0; i < len(ctx.Ckps); i++{
			buf := &bytes.Buffer{}
			err := binary.Write(buf, binary.LittleEndian, ctx.Ckps[i])
			if err != nil {
				panic(err)
			}
			v = append(v, buf.Bytes()...)
		}
		_, err = conn.Write(v)
		if err != nil {
			fmt.Println("发送消息失败:", err)
			return false
		}

		fmt.Println("已发送消息到C++服务器.")
	return true;
}

func HandleDead(ip string, port string, ids []string)(bool){
	// 连接到C++服务器
	fmt.Println(ip+":"+port)
	conn, err := net.Dial("tcp", ip+":"+port)
	if err != nil {
		fmt.Println("无法连接到C++服务器:", err)
		return false
	}
	defer conn.Close()
	// 发送消息到C++服务器
	// var temp Utils.DataPackage
	// //heartbeat
	// temp.Func_name = intToBytes(0)
	// // 将字符串转换为字节数组
	// bytes := []byte(id)

	// // 创建一个长度为 2 的字节数组
	// byteArray := make([]byte, 2)

	// // 将字符串的长度按照小端编码方式存入字节数组
	// binary.LittleEndian.PutUint16(byteArray, uint16(len(bytes)))

	// fmt.Printf("小端编码字节数组: %v\n", byteArray)

	// v := make([]byte, Utils.ID_LENGTH+Utils.FUNC_LENGTH)
	// v = append(v,temp.Func_name[:]...)
	// v = append(v,byteArray[:]...)
	var temp Utils.ErrorPackage
	binary.LittleEndian.PutUint16(temp.Type[:], uint16(0))
	for i, id := range ids {
		copy(temp.Diff_Heartbeats[i][:], []byte(id))
	}
	copy(temp.Diff_Heartbeats[len(ids)][:], []byte(Utils.END_ID))
	var v []byte
	buf := &bytes.Buffer{}
	err = binary.Write(buf, binary.LittleEndian, temp)
	if err != nil {
		panic(err)
	}
	v = append(v, buf.Bytes()...)
	_, err = conn.Write(v)
	if err != nil {
		fmt.Println("发送消息失败:", err)
		return false
	}

	fmt.Println("已发送消息到C++服务器.")
	return true;
}

func intToBytes(num int) [Utils.FUNC_LENGTH]byte {
	var bytes [Utils.FUNC_LENGTH]byte
	for i := 0; i < Utils.FUNC_LENGTH; i++ {
		bytes[i] = byte(num >> (8 * (Utils.FUNC_LENGTH - i - 1)))
	}
	return bytes
}
