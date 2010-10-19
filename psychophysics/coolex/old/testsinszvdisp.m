daq = DaqDeviceIndex; % connect to USB-1208
DaqAOut(daq,1,1) % turn on LED

for k = 0:0.01:60 % move LED around a little.
    position=0.5-0.5*cos(k);
    DaqAOut(daq,0,position);
    WaitSecs(0.005);
end

DaqAOut(daq,0,0); % put LED back home
DaqAOut(daq,1,0); % turn off LED

