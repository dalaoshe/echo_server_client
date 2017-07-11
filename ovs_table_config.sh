sh ovs-vsctl --id=@ft create Flow_Table flow_limit=10 overflow_policy=evict groups='"NXM_OF_IP_SRC"' -- set Bridge s2 flow_tables:0=@ft 

sh ovs-vsctl dump-flows s2 -O OpenFlow14
sh ovs-ofctl mod-table s1 0 evict -O OpenFlow14
sh ovs-vsctl -- set Bridge s1 protocols=OpenFlow14
sh ovs-vsctl -- set Bridge s2 protocols=OpenFlow14
sh ovs-vsctl -- set Bridge s3 protocols=OpenFlow14
