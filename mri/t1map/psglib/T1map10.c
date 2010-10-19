/* T1 mapping program modified for Varian PLAN utility 

        T1map2.c

        HP Hetherington
        Brookhaven National Laboratory
        
        
*/      
                
        


#include <standard.h> 
#define HZ_TO_GAUSS 0.002349
#define B1_IR           1500.0
#define B1_SINC3S       244.16 

pulsesequence()
{

/* menu variables */
double  slice_str,pe_length,read_str,power,angle,TR,TIR,slice_coffset;
double  trim_read,trim_slice,freqir,encode_on,read_on,ir_on,sel_crush;
double	nir,stabilize;


/* internal variables */
double  pe_length_eff,GPmin,GPstep;
double  aq_time,area_readout,read_rephase_str,GRR,GRead,freqreadout;
double  slice_rephase_str,area_slice,GSR,GSS,GCR2,grad_factor;
double  time[5],cycle_time,start_cycle[20],relaxation,finish_time;
double  powir,powexcite,sel_length,pe_step,pe_min;
double  slice_offset[128],GCR1,ir_str,irfreqs[128],irpos[128],x;
double  B1_90,B1_excite,extra_TE;
double  rf_max,ir_len,cur_angle,ramp_lenr,ramp_lens;
double	ir_sel_str,ir_factor,GSIR,extrair,irtime,widthir,incrementir;
int     a;




read_str = getval("Read_Str");
slice_str = getval("Slice_Str");
pe_length = getval("PE_length");
ir_str = getval("IR_Grad_Str");
sel_crush = getval("Sel_Crush");
ramp_lenr = getval("ramp_lenr");
ramp_lens = getval("ramp_lens");
extra_TE = getval("extra_TE");
stabilize=getval("stabilize");
/*extrair = getval("extrair");*/

TIR = getval("TIR");
TR = getval("TR");

power=getval("Power");
angle = getval("angle");

trim_slice = getval("Slice_Trim");
trim_read = getval("Read_Trim");

freqreadout = getval("Read_Offset");
freqir = getval("IR_Offset");

encode_on=getval("encode_on");
read_on=getval("read_on");
ir_on=getval("ir_on");

/*rf_max = getval("rf_max"); */
/*ir_len = getval("ir_len");*/
cur_angle = getval("cur_angle");


nir = getval("nir");
ir_sel_str = getval("ir_sel_str");



/* rf length calculation and offset lists #0*/
sel_length = 0.003*(1000.0/(slice_str*thk));

for (a=0;a<ns;a++)
 {
 slice_offset[a] = pss[a]* 10.0 * slice_str;
 
 }


/* IR frequencies */
ir_factor = ns/nir;
ir_len = 0.008;
ir_sel_str = 6400.0/(ir_factor*thk);
printf("\n ir_sel_str =%f",ir_sel_str);



printf("\n ir factor %f",ir_factor);
printf("\n pss[ns-1] %f",pss[(int)(ns-1)]);
printf("\n pss[0] %f",pss[0]);

widthir = pss[(int)(ns-1)]-pss[0]+(thk/10.0);
incrementir = widthir/nir;

printf("\n incrementir = %f ",incrementir);

for (a=0;a<nir;a++)
 {
  irpos[a] = pss[0] + (incrementir*a) + (incrementir/2.0) - (incrementir/(ir_factor*2.0));
  printf("\n ir pos =%f",irpos[a]);
  irfreqs[a] = ir_sel_str * irpos[a] * 10.0;
 }	




GCR2 = sel_crush * HZ_TO_GAUSS;
GSS = slice_str * HZ_TO_GAUSS;
GSIR = ir_sel_str * HZ_TO_GAUSS;

/* phase encoding */
pe_length_eff = (2.0/3.14159)*pe_length;
pe_step = 1.0/(lpe*10.0*pe_length_eff);
pe_min = pe_step*(nv/2);
GPmin =  pe_min * HZ_TO_GAUSS;
GPstep = -pe_step * HZ_TO_GAUSS;

/* rephase for readout */
aq_time = at;
grad_factor = sw/((double)read_str*lro*10.0);
area_readout = (0.001+ (ramp_lenr*0.001/2.0) + (aq_time/2)) * read_str*grad_factor;
read_rephase_str = -1.0*area_readout/pe_length_eff;
GRR = read_rephase_str * HZ_TO_GAUSS * trim_read;
GRead = read_str * HZ_TO_GAUSS*grad_factor;

/* rephase the slice */
area_slice = ((sel_length/2) + (ramp_lens*0.001/2.0))*slice_str;
slice_rephase_str = -1.0*area_slice/pe_length_eff;
GSR = slice_rephase_str * HZ_TO_GAUSS*trim_slice;

/* calculate the rf levels */
B1_90 = (slice_str*thk/1000.0)* B1_SINC3S;
B1_excite = B1_90*(cur_angle/90.0);
powexcite = (B1_excite/1500.0)*4096.0;

powir = 4096.0;


/* calculate the  timing */
if (ir_on==1)
	time[0] = 0.011 + ir_len*0.001 + 0.0001;
else
	time[0] = 0.0001;
	
/* ir */
time[1] =  sel_length + (2*ramp_lens*0.001) + stabilize ;            /* slice select */
time[2] =  extra_TE+ pe_length;                                 /* phase encoding etc. */
time[3] =  aq_time + (2*ramp_lenr*0.001) + stabilize;                 /* acquisition window */
time[4] =  pe_length;                                           /* rewind */

cycle_time = time[1]+time[2]+time[3]+time[4];
  

printf("\nCycle Time = %f",cycle_time); 

irtime = (cycle_time * ir_factor)- (2*ramp_lens*0.001) - ir_len;
extrair = TIR - (nir*ir_factor*cycle_time);
create_offset_list(slice_offset,ns,OBSch,0);

/*
   for (a=0;a<ns;a++)
   printf("\nOffset %d = %f",a,slice_offset[a]); 
*/

GCR1 = ir_str * HZ_TO_GAUSS;

if (ir_on==1)
	start_cycle[0] = 0.011 + TIR;
else
	start_cycle[0] = 0.0001;

finish_time = start_cycle[0] + (ns*cycle_time);
printf("\n Finish Time = %f",finish_time); 
relaxation = TR - finish_time;




/* START THE PULSE SEQUENCE */
assign(v10,zero);
assign(v11,zero);
status(A);
obspower(power);

/* begin phase encode loop */
peloop(seqcon[2],nv,v1,v2);

        delay(relaxation);
	
	
        /* ir */
        obspwrf(powir);
        obsoffset(0.0);
        if (ir_on==1)
          {
           for (a=0;a<nir;a++)
		{	   
	         obsoffset(irfreqs[a]);	 
		 obl_shapedgradient("","","RAMPUP",ramp_lens*0.001,0,0,GSIR,1,WAIT);
	   	 obl_shapedgradient("","","CONST",ir_len,0,0,GSIR,1,NOWAIT);
	     	 shaped_pulse("HYPSEC516",ir_len,v10,rof1,rof2);
	   	 obl_shapedgradient("","","RAMPDN",ramp_lens*0.001,0,0,GSIR,1,WAIT);
           	 zero_all_gradients();
                 delay(irtime);
                }
           }
        
        obl_shapedgradient("","CRUSH","",0.011,0,GCR1,0,1,WAIT);  
        delay(extrair);

        

                        
        msloop(seqcon[1],ns,v5,v6);   /* loop over slices */

        	/* slice selection readout */
                obspwrf(powexcite);
                voffset(0,v6);
                obl_shapedgradient("","","RAMPUP",ramp_lens*0.001,0,0,GSS,1,WAIT);
                obl_shapedgradient("","","CONST",sel_length+stabilize,0,0,GSS,1,NOWAIT);
		delay(stabilize);
                shaped_pulse("SINC3S",sel_length,v11,rof1,rof2);
                obl_shapedgradient("","","RAMPDN",ramp_lens*0.001,0,0,GSS,1,WAIT);
                zero_all_gradients();
                        
                delay(extra_TE);

                /* encode and rephasing */
                if ((read_on==1)&&(encode_on==1))
                                pe3_shapedgradient("SINUSOID",pe_length,GRR,GPmin,GSR,0,GPstep,0,zero,v2,zero);
                if ((read_on!=1)&&(encode_on==1))
                                pe3_shapedgradient("SINUSOID",pe_length,0,GPmin,GSR,0,GPstep,0,zero,v2,zero);
                if ((read_on!=1)&&(encode_on!=1))
                                pe3_shapedgradient("SINUSOID",pe_length,0,0,GSR,0,0,0,zero,zero,zero);
                if ((read_on==1)&&(encode_on!=1))
                                pe3_shapedgradient("SINUSOID",pe_length,GRR,0,GSR,0,0,0,zero,zero,zero);
                                
                                
                                        
                zero_all_gradients();
        
                /* readout */
                obsoffset(freqreadout);
                if (read_on==1)
                        {
                          obl_shapedgradient("RAMPUP","","",ramp_lenr*0.001,GRead,0,0,1,WAIT);
                          obl_shapedgradient("CONST","","",at+stabilize,GRead,0,0,1,NOWAIT);
		          delay(stabilize);
                          acquire(np,1.0/(sw));
                          obl_shapedgradient("RAMPDN","","",ramp_lenr*0.001,GRead,0,0,1,WAIT);
                          zero_all_gradients();
                         }
                         else
                         {
                          delay(0.001);
                          acquire(np,1.0/(sw));
                          } 

                /* rewind and crush */
                if (encode_on==1)
                        {
                         pe3_shapedgradient("SINUSOID",pe_length,0,-GPmin,GCR2,0,-GPstep,0,zero,v2,zero);
                         zero_all_gradients();
                         }
                         else delay(pe_length);
                                                
         endmsloop(seqcon[1],v6);
                
       
                
endpeloop(seqcon[2],v2);

}
