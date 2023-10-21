module monitor

go 1.21.1

require gitee.com/liyue/storage v0.0.0

require (
	gitee.com/liyue/Utils v0.0.0 // indirect
	gitee.com/liyue/strategy v0.0.0 // indirect
	github.com/deepmap/oapi-codegen v1.8.2 // indirect
	github.com/influxdata/influxdb-client-go/v2 v2.12.3 // indirect
	github.com/influxdata/line-protocol v0.0.0-20200327222509-2487e7298839 // indirect
	github.com/pkg/errors v0.9.1 // indirect
	golang.org/x/net v0.7.0 // indirect
)

replace gitee.com/liyue/storage => /home/liyue/neuron/monitor/storage

replace gitee.com/liyue/strategy => /home/liyue/neuron/Strategy

replace gitee.com/liyue/Utils => /home/liyue/neuron/Utils
