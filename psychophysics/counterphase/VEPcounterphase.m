% function VEP
% A full screen stimulus that alternates checkerboards
% presented in a square wave pattern.  The frequency of this alternation
% is adjustable.  This program will also sends a analog
% signal (via the DAC) that indicates when the screen flips.
clear all
close all

%% settings
numberOfStims = 240;
whiteDuration = 0.25; % duration of check1 screen in secs
blackDuration = 0.25; % duration of check2 screen in secs
whiteColor = 255;
blackColor = 0;
testMode = 1; % set to 1 to turn off DAQ and to 0 to turn on

expDuration = numberOfStims * (whiteDuration + blackDuration );
disp(numberOfStims)

check_size = 10;
checkrepx = 10;
checkrepy = 10;

checks1 = make_checks(check_size,checkrepy,checkrepx,blackColor,whiteColor);
checks2 = make_checks(check_size,checkrepy,checkrepx,whiteColor,blackColor);
% imshow(checks1);


%% DAQ
if testMode == 0
    daq = DaqDeviceIndex; % connect to USB-1208
end

%% open screens
% InitializeMatlabOpenGL([], 0, 1);
screenNumbers=[1 2];
try
    idx = 0;
    for screenNumber = screenNumbers
        idx = idx +1;
        window(idx) = Screen(screenNumber,'OpenWindow',0);
        checktex1(idx) = Screen('MakeTexture',window(idx),checks1);
        checktex2(idx) = Screen('MakeTexture',window(idx),checks2);
    end
    numWindows = idx;
    % defining rectangles
    windowRect = Screen(window(1),'Rect');
    
    SCREEN(window(1),'TextSize',[20]);
    
    %% wait for start
    fprintf('\nPress shift key to start immediately\n');
%     Screen('DrawText', window(1), 'Press shift key to start immediately', 10, 10, whiteColor);
    Screen('Flip', window(1));
    KbPressWait;
    fprintf('started\n')
%     Screen(window(1),'FillRect',blackColor);
    
    startTime = GetSecs;
    %% show the stims
    for stimNum = 1:numberOfStims
        % white stim
        for windowNumber = 1:numWindows % blit first texture
            Screen('DrawTexture', window(windowNumber),checktex1(windowNumber));
        end
        Screen('Flip',window(windowNumber),0,0,0,1);
        if testMode == 0
            %DaqDOut(daq,1,255) % set line 2 high
            DaqAOut(daq,1,.5); %CA - testing analog outs
        end
        WaitSecs(whiteDuration);
        
        
        % black stim
        for windowNumber = 1:numWindows % blit second texture
            Screen('DrawTexture', window(windowNumber),checktex2(windowNumber));
        end
        Screen('Flip',window(windowNumber),0,0,0,1);
        if testMode == 0
            %DaqDOut(daq,1,0) % set all lines to 0
            DaqAOut(daq,1,0); %CA - testing analog outs
        end
        WaitSecs(blackDuration);
    end
    
    
    endTime = GetSecs;
    
    %% clean up and close down
    save(['VEPsettings' num2str(now) '.mat'])
    Screen('CloseAll')
catch ME
    Screen('CloseAll')
    disp(ME)
end