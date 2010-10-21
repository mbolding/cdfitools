function writeProcpar()

%% open file for writing
fp = fopen('procpar',w);

%% write out a minimal procpar file for reading by shim code
fprintf(fp,'sfrq 1 1 1000000000 0 0 2 1 11 1 64 \n');
fprintf(fp,'1 400.5391732  \n');
fprintf(fp,'0  \n');
fprintf(fp,'theta 1 1 1000000 -1000000 0 2 1 8 1 64 \n');
fprintf(fp,'1 0  \n');
fprintf(fp,'0  \n');
fprintf(fp,'psi 1 1 1000000 -1000000 0 2 1 8 1 64 \n');
fprintf(fp,'1 0  \n');
fprintf(fp,'0  \n');
fprintf(fp,'phi 1 1 1000000 -1000000 0 2 1 8 1 64 \n');
fprintf(fp,'1 0  \n');
fprintf(fp,'0  \n');
fprintf(fp,'pss0 1 1 9.99999984307e+17 -9.99999984307e+17 0 2 1 0 1 64 \n');
fprintf(fp,'1 0  \n');
fprintf(fp,'0  \n');
fprintf(fp,'lpe 1 1 100 2 0 2 1 0 1 64 \n'); % cm
fprintf(fp,'1 19.2  \n');
fprintf(fp,'0  \n');
fprintf(fp,'nv 1 1 1000000 0 1 2 1 9 1 64 \n');
fprintf(fp,'1 1  \n');
fprintf(fp,'0  \n');
fprintf(fp,'ns 1 1 1000000 0 1 2 1 9 1 64 \n');
fprintf(fp,'1 1  \n');
fprintf(fp,'0 \n');
fprintf(fp,'thk 1 1 1000000 -1000000 0 2 1 0 1 64 \n');
fprintf(fp,'1 2  \n');
fprintf(fp,'0  \n');
fprintf(fp,'gap 1 1 9.99999984307e+17 -9.99999984307e+17 0 2 1 0 1 64 \n');
fprintf(fp,'1 0  \n');
fprintf(fp,'0 \n');
fprintf(fp,'b0delay 1 1 9.99999984307e+17 -9.99999984307e+17 0 2 1 0 1 64 \n');
fprintf(fp,'1 0.001  \n');
fprintf(fp,'0  \n');
fprintf(fp,'b0times 1 1 9.99999984307e+17 -9.99999984307e+17 0 2 1 0 1 64 \n');
fprintf(fp,'1 5  \n');
fprintf(fp,'0  \n');
fprintf(fp,'np 7 1 10000000 32 2 2 1 11 1 64 \n');
fprintf(fp,'1 8192  \n');
fprintf(fp,'0  \n');
fprintf(fp,'rcvrs 4 2 4 0 0 2 1 1 1 64 \n');
fprintf(fp,'1 "y" \n');
fprintf(fp,'2 "n" "y"  \n');
fprintf(fp,'seqcon 2 2 0 0 0 2 1 9 1 64 \n');
fprintf(fp,'1 "ccsnn" \n');
fprintf(fp,'0  \n');

%% clean up
fclose(fp);