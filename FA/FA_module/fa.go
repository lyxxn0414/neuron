package FA_module

import(
	"fmt"
	// "encoding/binary"
	"time"
	storage "gitee.com/liyue/storage"  // 使用自定义包
	strategy "gitee.com/liyue/strategy"
)

var dir = make(map[[2]string]storage.Board_State)
var infos = make(map[[2]string]storage.Board_Info)

func QueryCard(){
	fmt.Println("执行周期性任务")
	// infos := storage.GetInfo(storage.QUERY_INFO_PERIOD)
	// ids := storage.GetID()
	// infos := storage.GetInfo(storage.DEAD_PERIOD, ids)
	board_infos := storage.GetState()
	dir_new := make(map[[2]string]storage.Board_State)
	for _, b_info := range board_infos{
		id := [2]string{b_info.Agent_id, b_info.Id}
		infos[id] = b_info
		if !b_info.Alive {
			dir_new[id] = dir[id] 
			if(dir[id] == storage.ALIVE){
				dir_new[id] = storage.ERROR_UNHANDLED
			}
		}else{
			dir_new[id] = storage.ALIVE
		}
		fmt.Println(id,dir_new[id])
	}
	dir = dir_new
	for id, state := range dir{
		if(state == storage.ERROR_UNHANDLED){
			var ctx storage.Error_Context
			ctx.IP = infos[id].IP
			ctx.Port = infos[id].Port
			ctx.Err  = storage.DEAD
			//获取对应板卡id的checkpoint
			ctx.Ckps = storage.GetCkpByID(id[0],id[1])
			if(strategy.Handlefault(id,ctx)){
				dir[id] = storage.ERROR_HANDLED
			}
		}
	}
}

func PeriodicTask(stop chan bool) {
	// 创建一个周期性的定时器，每隔Q秒触发一次
	ticker := time.NewTicker(storage.QUERY_INFO_PERIOD * time.Second)
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