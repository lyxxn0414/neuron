package Strategy

import(
	"fmt"
	storage "gitee.com/liyue/storage"  // 使用自定义包
	"net"
	"bytes"
	"encoding/binary"
)

func Handlefault(id [2]string, ctx storage.Error_Context)(bool){
		fmt.Println("ID:",id,"Error Code:", ctx.Err)
		for _, ckp := range ctx.Ckps{
			fmt.Println("Agent ID:",storage.GetString(ckp.Agent_id[:]),"Chip_id:",storage.GetString(ckp.Id[:]),"Ckp_id:",storage.GetString(ckp.Ckp_id[:]))
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