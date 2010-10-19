#!/bin/bash
#set up dirs for experiments
case $1 in
	test )
	echo "setting up test"	
	;;

	exp_cont )
	echo "setting up expansion/contraction"
	# static vs. expansion contraction, AB design, 12s blocks
	mkdir exp_cont
	cd exp_cont && rm *.mov
	cp  ../movies/flow_static_30fps.mov a1.mov
	cp  ../movies/flow_staticr_30fps.mov a2.mov
	cp  ../movies/flow_static_30fps.mov a3.mov
	cp  ../movies/flow_staticr_30fps.mov a4.mov
	cp  ../movies/flow_static_30fps.mov a5.mov
	cp  ../movies/flow_staticr_30fps.mov a6.mov
	cp  ../movies/flow_exp_30fps.mov b1.mov
	cp  ../movies/flow_contract_30fps.mov b2.mov
	cp  ../movies/flow_exp_30fps.mov b3.mov
	cp  ../movies/flow_contract_30fps.mov b4.mov	
	cp  ../movies/flow_exp_30fps.mov b5.mov
	cp  ../movies/flow_contract_30fps.mov b6.mov	
	rm go.m
	echo "PlayMovies('.',96);" > go.m	# 2sec * 96 = 3.2min = 3:12
	;;

	rotation )
	echo "setting up rotation"
	# static vs. rotation, AB design, 12s blocks
	mkdir rotation
	cd rotation && rm *.mov
	cp  ../movies/flow_static_30fps.mov a1.mov
	cp  ../movies/flow_staticr_30fps.mov a2.mov
	cp  ../movies/flow_static_30fps.mov a3.mov
	cp  ../movies/flow_staticr_30fps.mov a4.mov
	cp  ../movies/flow_static_30fps.mov a5.mov
	cp  ../movies/flow_staticr_30fps.mov a6.mov
	cp  ../movies/flow_cw_30fps.mov b1.mov
	cp  ../movies/flow_ccw_30fps.mov b2.mov
	cp  ../movies/flow_cw_30fps.mov b3.mov
	cp  ../movies/flow_ccw_30fps.mov b4.mov	
	cp  ../movies/flow_cw_30fps.mov b5.mov
	cp  ../movies/flow_ccw_30fps.mov b6.mov	
	rm go.m
	echo "PlayMovies('.',96);" > go.m	
	;;

	translation )
	echo "setting up translation"
	# static vs. translation, AB design, 12s blocks
	mkdir translation
	cd translation && rm *.mov
	cp  ../movies/flow_static_30fps.mov a1.mov
	cp  ../movies/flow_staticr_30fps.mov a2.mov
	cp  ../movies/flow_static_30fps.mov a3.mov
	cp  ../movies/flow_staticr_30fps.mov a4.mov
	cp  ../movies/flow_static_30fps.mov a5.mov
	cp  ../movies/flow_staticr_30fps.mov a6.mov
	cp  ../movies/flow_left_30fps.mov b1.mov
	cp  ../movies/flow_right_30fps.mov b2.mov
	cp  ../movies/flow_up_30fps.mov b3.mov
	cp  ../movies/flow_down_30fps.mov b4.mov	
	cp  ../movies/flow_upright_30fps.mov b5.mov
	cp  ../movies/flow_downleft_30fps.mov b6.mov	
	rm go.m
	echo "PlayMovies('.',96);" > go.m	
	;;
	
	slowfast8 )
	echo "setting up slowfast8"
	# static vs. slow vs. fast, ABAC design, 8s blocks
	mkdir slowfast8
	cd slowfast8 && rm *.mov
	cp  ../movies/speed_0_2fps.mov 01.mov
	cp  ../movies/speed_0_2fps.mov 02.mov
	cp  ../movies/speed_0_2fps.mov 03.mov
	cp  ../movies/speed_0_2fps.mov 04.mov
	cp  ../movies/speed_1_30fps.mov 05.mov
	cp  ../movies/speed_0_2fps.mov 06.mov
	cp  ../movies/speed_0_2fps.mov 07.mov
	cp  ../movies/speed_0_2fps.mov 08.mov
	cp  ../movies/speed_0_2fps.mov 09.mov
	cp  ../movies/speed_8_30fps.mov 10.mov
	rm go.m
	echo "PlayMovies('.',60);" > go.m	# 8*4 sec per ABCB * 60/10 repeats = 3.2min = 3:12
	;;
	
	slowfast16 )
	echo "setting up slowfast16"
	# static vs. slow vs. fast, ABAC design, 16s blocks
	mkdir slowfast16
	cd slowfast16 && rm *.mov
	cp  ../movies/speed_0_2fps.mov 01.mov
	cp  ../movies/speed_0_2fps.mov 02.mov
	cp  ../movies/speed_0_2fps.mov 03.mov
	cp  ../movies/speed_0_2fps.mov 04.mov
	cp  ../movies/speed_0_2fps.mov 05.mov
	cp  ../movies/speed_0_2fps.mov 06.mov
	cp  ../movies/speed_0_2fps.mov 07.mov
	cp  ../movies/speed_0_2fps.mov 08.mov
	cp  ../movies/speed_1_30fps.mov 09.mov
	cp  ../movies/speed_1_30fps.mov 10.mov
	cp  ../movies/speed_0_2fps.mov 11.mov
	cp  ../movies/speed_0_2fps.mov 12.mov
	cp  ../movies/speed_0_2fps.mov 13.mov
	cp  ../movies/speed_0_2fps.mov 14.mov
	cp  ../movies/speed_0_2fps.mov 15.mov
	cp  ../movies/speed_0_2fps.mov 16.mov
	cp  ../movies/speed_0_2fps.mov 17.mov
	cp  ../movies/speed_0_2fps.mov 18.mov
	cp  ../movies/speed_8_30fps.mov 19.mov
	cp  ../movies/speed_8_30fps.mov 20.mov
	rm go.m
	echo "PlayMovies('.',60);" > go.m	# 16*4 sec per ABCB * 60/20 repeats = 3.2min = 3:12
	;;	

	all )
	echo "setting them all up"
	$0 exp_cont 
	$0 rotation 
	$0 translation 
	$0 slowfast8 
	$0 slowfast16 
	;;
	
esac
