#### 启动高可用系统

```bash
cd /root/neuron/monitor && go build && ./monitor 
```

另起一个终端

```bash
cd /root/neuron/Agent && make && ./test
```
以上两行脚本部署了 Agent 及高可用服务

另起一个终端（OS测试端），该 test 测试了 checkpoint 推送（test_post_heartbeat）和异常反馈（listen_for_error）两个场景

```bash
cd /root/neuron/OS && make && ./test
```

