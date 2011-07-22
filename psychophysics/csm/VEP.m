% function VEP
% A full screen stimulus that alternates between all black and all white
% presented in a square wave pattern.  The frequency of this alternation
% is adjustable.  This program will also sends a analog
% signal (via the DAC) that indicates when the screen
% is white and when it is black.

%% settings
numberOfStims = 4;
whiteDuration = 0.200; % duration of white screen in secs
blackDuration = 0.200; % duration of black screen in secs
whiteColor = [255 255 255];
blackColor = [0 0 0];
testMode = 1; % set to 1 to turn off DAQ and to 0 to turn on

%% DAQ
if testMode == 0
    daq = DaqDeviceIndex; % connect to USB-1208
end

%% open screens
% InitializeMatlabOpenGL([], 0, 1);
screenNumbers=[0];
try
    idx = 0;
    for screenNumber = screenNumbers
        idx = idx +1;
        window(idx) = Screen(screenNumber,'OpenWindow',0);
    end
    numWindows = idx;
    % defining rectangles
    windowRect = Screen(window(1),'Rect');
    SCREEN(window(1),'TextSize',[20]);
    
    %% wait for start
    fprintf('\nPress shift key to start immediately\n');
    Screen('DrawText', window(1), 'Press shift key to start immediately', 10, 10, whiteColor);
    Screen('Flip', window(1));
    KbPressWait;
    
    startTime = GetSecs;
    %% show the stims
    for stimNum = 1:numberOfStims
        % white stim
        for windowNumber = 1:numWindows % fill screen with white
            Screen(window(windowNumber),'FillRect',whiteColor);
        end
        Screen('Flip',window(windowNumber),0,0,0,1);
        WaitSecs(whiteDuration);
        
        
        % black stim
        for windowNumber = 1:numWindows % fill screen with black
            Screen(window(windowNumber),'FillRect',blackColor);
        end
        Screen('Flip',window(windowNumber),0,0,0,1);
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