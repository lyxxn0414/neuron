package storage

const (
	AGENT_ID_LENGTH = 16
	ID_LENGTH     = 16
	CKP_ID_LENGTH = 16
	CKP_LENGTH = 64
	FUNC_LENGTH = 4
	OTHER_INFO_LENGTH = 128
	//查询硬件状态的周期，单位：s
	QUERY_INFO_PERIOD = 5
	//多久无响应判定板卡失活, 单位：s
	DEAD_PERIOD = 10
	INFO_LENGTH = 5
	IP_LENGTH = 16
	PORT_LENGTH = 8
)

type Info struct {
	Time    [32]byte
	Agent_id [AGENT_ID_LENGTH]byte
	Id      [ID_LENGTH]byte
	IP		[IP_LENGTH]byte
	Port    [PORT_LENGTH]byte
	Hw_info [OTHER_INFO_LENGTH]byte
}

type Checkpoint struct {
	Time       [32]byte
	Agent_id [AGENT_ID_LENGTH]byte
	Id         [ID_LENGTH]byte
	Ckp_id     [CKP_ID_LENGTH]byte
	Other_info [OTHER_INFO_LENGTH]byte
}

type Board_State int

const (
	ALIVE Board_State = iota
	ERROR_UNHANDLED
	ERROR_HANDLED
)

type Error_Code int

const (
	DEAD Error_Code = iota
)

type DataPackage struct {
	func_name [FUNC_LENGTH]byte
	params    [1024]byte
}

type Board_Info struct {
	Agent_id string
	Id string
	Infos []Info
	Alive bool
	IP	string
	Port string
}

type Error_Context struct {
	Err Error_Code
	IP string
	Port string
	Ckps []Checkpoint
}