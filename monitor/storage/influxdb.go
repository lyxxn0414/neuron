package storage

import (
	"context"
	"fmt"
	"time"
	"strconv"
	"encoding/binary"
	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
)

var client influxdb2.Client

// You can generate a Token from the "Tokens Tab" in the UI
const token = "2fSh86dOM4y3sXImYFJ6P-yT2on9d22A8f4VfABQbVMhV4IfS4LjZyhdZq4HLdbFcLDccpaLZqZBbhRF_2EmCQ=="
const bucket = "neuron-bucket"
const org = "neuron-org"
const ip = "127.0.0.1"

//Store the URL of your InfluxDB instance
const url = "http://localhost:8086"

//connect url
func Connect_db() {
	// Create new client with default option for server url authenticate by token
	client = influxdb2.NewClient(url, token)
}

//close client
func Close_db() {
	client.Close()
}

//insert
func writePonits() {
	// Get non-blocking write client
	writeAPI := client.WriteAPI(org, bucket)
	p := influxdb2.NewPoint("system", //
		map[string]string{
			"ip":       ip,
			"hostName": "nodekafka"}, //
		map[string]interface{}{
			"temperature": 50.0,
			"diskfree":    "300G",
			"disktotal":   "400G"},
		time.Now())

	// Create point using full params constructor
	p1 := influxdb2.NewPointWithMeasurement("system").
		AddTag("ip", ip).
		AddTag("hostName", "nodekafka").
		AddField("temperature", 38.0).
		AddField("diskfree", "300G").
		AddField("disktotal", "400G").
		SetTime(time.Now())

	//write point asynchronously
	writeAPI.WritePoint(p)
	writeAPI.WritePoint(p1)
	// Flush writes
	writeAPI.Flush()

}

func writeInfo(infos []Info, num int){
	// Get non-blocking write client
	writeAPI := client.WriteAPI(org, bucket)

	for i := 0; i < num; i++ {
		t, err :=  strconv.ParseInt(GetString(infos[i].Time[:]), 10, 64)
		if err != nil {
			fmt.Println(err)
		}
			// Create point using full params constructor
		p1 := influxdb2.NewPointWithMeasurement("Info").
		AddTag("id", GetString(infos[i].Id[:])).
		AddTag("agent_id",GetString(infos[i].Agent_id[:])).
		AddTag("ip", GetString(infos[i].IP[:])).
		AddTag("port", GetString(infos[i].Port[:])).
		AddField("hw_info", GetString(infos[i].Hw_info[:])).
		SetTime(time.Unix(t, 0))
		//write point asynchronously
		writeAPI.WritePoint(p1)
		fmt.Println(GetString(infos[i].IP[:]), GetString(infos[i].Port[:]));
	}
		// Flush writes
		writeAPI.Flush()
}

func GetInfo(second int, ids []string)(infos []Info){
	queryAPI := client.QueryAPI(org)
	for _, id := range ids{
		result, err := queryAPI.Query(context.Background(),
		fmt.Sprintf(`from(bucket: "%v")
		|> range(start: -%vs)
		|> filter(fn: (r) =>
	  	r._measurement == "Info"  and
		r.id == "%v")`, bucket, second, id))
		var temp Info
		copy(temp.Id[:], []byte(id))
		if err == nil {
		// Iterate over query response
			for result.Next() {
				fmt.Printf("field:%v value: %v time:%s\n", result.Record().Field(), result.Record().Value(), result.Record().Time())
				value, _ := result.Record().Value().(string);
				if result.Record().Field()=="hw_info"{
    				copy(temp.Hw_info[:], []byte(value))
					binary.LittleEndian.PutUint32(temp.Time[:], uint32(result.Record().Time().Unix()))
				}
			}
			// Check for an error
			if result.Err() != nil {
				fmt.Printf("query parsing error: %s\n", result.Err().Error())
			}
		} else {
			panic(err)
		}
		infos = append(infos, temp)
	}
	return infos
}

func GetInfoById(agent_id string, id string)(infos []Info){
	queryAPI := client.QueryAPI(org)
	result, err := queryAPI.Query(context.Background(),
		fmt.Sprintf(`from(bucket: "%v")
		|> range(start: -%vs)
		|> filter(fn: (r) =>
	  	r._measurement == "Info" and
		r.agent_id == "%v" and
		r.id == "%v")`, bucket, DEAD_PERIOD, agent_id, id))
		if err == nil {
		// Iterate over query response
			for result.Next() {
				var temp Info
				copy(temp.Agent_id[:], []byte(agent_id))
				copy(temp.Id[:], []byte(id))
				// fmt.Printf("field:%v value: %v time:%s\n", result.Record().Field(), result.Record().Value(), result.Record().Time())
				value, _ := result.Record().Value().(string);
				if result.Record().Field()=="hw_info"{
    				copy(temp.Hw_info[:], []byte(value))
					binary.LittleEndian.PutUint32(temp.Time[:], uint32(result.Record().Time().Unix()))
					// fmt.Println(uint32(result.Record().Time().Unix()))
				}
				infos = append(infos,temp)
			}
			// Check for an error
			if result.Err() != nil {
				fmt.Printf("query parsing error: %s\n", result.Err().Error())
			}
		} else {
			panic(err)
		}
		return infos
}

func getCkp(chip_id string, ids []string, num int)(ckps []Checkpoint){
	// Get query client
	queryAPI := client.QueryAPI(org)
	fmt.Println("The chip_id:", chip_id)
	for i := 0; i < num; i++{
		result, err := queryAPI.Query(context.Background(),
		fmt.Sprintf(`from(bucket: "%v")
		|> range(start: -1h)
		|> filter(fn: (r) =>
	  	r._measurement == "Checkpoint" and
		r.ckp_id == "%v" and
		r.chip_id == "%v")`, bucket, ids[i], chip_id))

		// fmt.Sprintf(`from(bucket: "%v")
		// |> range(start: -1h)
		// |> filter(fn: (r) =>
	  	// r._measurement == "Checkpoint" and
		// r.ckp_id == %v)`, bucket, ids[i]))

		if err == nil {
			var temp Checkpoint
			copy(temp.Ckp_id[:], []byte(ids[i]))
			copy(temp.Id[:], []byte(chip_id))
		// Iterate over query response
			for result.Next() {
				fmt.Printf("field:%v value: %v time:%s\n", result.Record().Field(), result.Record().Value(), result.Record().Time())
				value, _ := result.Record().Value().(string);
				if result.Record().Field()=="other_info"{
    				copy(temp.Other_info[:], []byte(value))
					binary.LittleEndian.PutUint32(temp.Time[:], uint32(result.Record().Time().Unix()))
					// fmt.Println(uint32(result.Record().Time().Unix()))
				}
			}
			// Check for an error
			if result.Err() != nil {
				fmt.Printf("query parsing error: %s\n", result.Err().Error())
			}
			ckps = append(ckps, temp)
		} else {
			panic(err)
		}
	}
	return ckps
	// Get QueryTableResult
}

func GetCkpByID(agent_id string, chip_id string)(ckps []Checkpoint){
	queryAPI := client.QueryAPI(org)
	// 构建查询语句
	fmt.Println("Agent_id:",agent_id,"Chip_id:",chip_id)
	query := fmt.Sprintf(`from(bucket: "%v")
		|> range(start: -100d)
		|> filter(fn: (r) => r._measurement == "Checkpoint" and r.agent_id == "%v" and r.chip_id == "%v")
		|> group(columns: ["agent_id", "chip_id", "ckp_id"])`, bucket, agent_id, chip_id)
	// 执行查询
	result, err := queryAPI.Query(context.Background(), query)
	if err == nil {
		for result.Next() {
			record := result.Record()
			ckp_id := record.ValueByKey("ckp_id").(string)
			fmt.Printf("ckp_id: %v\n", ckp_id)
			// Access data
			var temp Checkpoint
			copy(temp.Ckp_id[:], []byte(ckp_id))
			copy(temp.Id[:], []byte(chip_id))
			copy(temp.Agent_id[:], []byte(agent_id))
			copy(temp.Other_info[:],[]byte(record.Value().(string)))
			binary.LittleEndian.PutUint32(temp.Time[:], uint32(record.Time().Unix()))
			ckps = append(ckps,temp)
		}
    }
	return ckps;
}

func writeCkp(ckps []Checkpoint, num int){
	// Get non-blocking write client
	writeAPI := client.WriteAPI(org, bucket)

	for i := 0; i < num; i++ {
		t, err :=  strconv.ParseInt(GetString(ckps[i].Time[:]), 10, 64)
		if err != nil {
			fmt.Println(err)
		}
			// Create point using full params constructor
		p1 := influxdb2.NewPointWithMeasurement("Checkpoint").
		AddTag("agent_id", GetString(ckps[i].Agent_id[:])).
		AddTag("chip_id", GetString(ckps[i].Id[:])).
		AddTag("ckp_id", GetString(ckps[i].Ckp_id[:])).
		AddField("other_info", GetString(ckps[i].Other_info[:])).
		SetTime(time.Unix(t, 0))

		//write point asynchronously
		writeAPI.WritePoint(p1)
		fmt.Println("Time is:   ",time.Unix(t, 0));
	}
		// Flush writes
		writeAPI.Flush()
}

func GetString(b []byte)(s string){
	i :=0
	for(b[i]!= byte(0)){
		i++
	}
	return string(b[:i])
}

func GetState()(bi []Board_Info){
	// 创建查询API
	queryAPI := client.QueryAPI(org)

	// 构建查询语句
	query := fmt.Sprintf(`from(bucket:"%v")
	|> range(start: -100d)
	|> filter(fn: (r) => r._measurement == "Info" and exists(r.agent_id) and exists(r.id))
	|> group(columns: ["agent_id", "id"])
	|> sort(columns: ["_time"], desc: true)
	|> limit(n: 1)
	|> keep(columns: ["id", "agent_id", "ip", "port"])`, bucket)

	// 执行查询
	result, err := queryAPI.Query(context.Background(), query)
	if err != nil {
		fmt.Println("查询错误:", err.Error())
		return
	}

	// 处理查询结果
	var res []Board_Info
	for result.Next() {
		record := result.Record()
		agentID := record.ValueByKey("agent_id").(string)
		id := record.ValueByKey("id").(string)
		ip := record.ValueByKey("ip").(string)
		port := record.ValueByKey("port").(string)

		var temp Board_Info
		temp.Agent_id = agentID
		temp.Id = id
		temp.Infos = GetInfoById(agentID, id)
		temp.IP = ip
		temp.Port = port
		if len(temp.Infos) > 0 {
			temp.Alive = true
		} else {
			temp.Alive = false
		}
		res = append(res, temp)
	}

	return res
}

// //main()
// func main() {
// 	connect()
// 	defer close()
// 	// writePonits()
// 	queryPoints()
// }