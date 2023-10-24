package FA_module

import(
	"fmt"
	// "encoding/binary"
	"time"
	storage "gitee.com/liyue/storage"  // 使用自定义包
	strategy "gitee.com/liyue/strategy"
	Utils "gitee.com/liyue/Utils"  // 使用自定义包
)

var dir = make(map[[2]string]Utils.Board_State)
var infos = make(map[[2]string]Utils.Board_Info)

func QueryCard(){
	fmt.Println("执行周期性任务")
	board_infos := storage.GetState()
	dir_new := make(map[[2]string]Utils.Board_State)
	for _, b_info := range board_infos{
		id := [2]string{b_info.Agent_id, b_info.Id}
		infos[id] = b_info
		if !b_info.Alive {
			dir_new[id] = dir[id] 
			if(dir[id] == Utils.ALIVE){
				dir_new[id] = Utils.ERROR_UNHANDLED
			}
		}else{
			dir_new[id] = Utils.ALIVE
		}
		fmt.Println(id,dir_new[id])
	}
	dir = dir_new
	for id, state := range dir{
		if(state == Utils.ERROR_UNHANDLED){
			var ctx Utils.Error_Context
			ctx.IP = infos[id].IP
			ctx.Port = infos[id].Port
			ctx.Err  = Utils.DEAD
			//获取对应板卡id的checkpoint
			ctx.Ckps = storage.GetCkpByID(id[0],id[1])
			if(strategy.Handlefault(id,ctx)){
				dir[id] = Utils.ERROR_HANDLED
			}
		}
	}
}

func PeriodicTask(stop chan bool) {
	// 创建一个周期性的定时器，每隔Q秒触发一次
	ticker := time.NewTicker(Utils.QUERY_INFO_PERIOD * time.Second)
	storage.Connect_db()
	defer storage.Close_db()
	for {
		select {
		case <-ticker.C:
			// 在这里执行你的周期性任务
			QueryCard()
		case <-stop:
			// 停止定时器
			ticker.Stop()
			return
		}
	}
}