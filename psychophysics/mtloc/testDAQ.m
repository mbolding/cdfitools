if ~exist('daqID','var')
    daqID=DaqDeviceIndex([],0);
end
while(~KbCheck)
    pause(0.2)
    Dlines=DaqDIn(daqID,2);
    if bitand(1,uint8(Dlines(1)))
        disp('one')
    else
        disp('  zero')
    end
end 

