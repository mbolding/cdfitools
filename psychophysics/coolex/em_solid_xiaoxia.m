function runNumber = em_solid(runNumber)
% run an eye movement stimulus using psychophysics toolbox
% See SpriteDemo.
% See also MovieDemo.
% See also EyeLink demo code.
% 02/02/10 msb modifying for spem experiments
% 06/21/10 msb extending for use by DG and MW

%% interactive parameters
fprintf('\n*** %s \nstarting run number %d.\n click user response button to start',mfilename,runNumber);
% wait for user ressponse to confirem button is connected, mouse button 3
%buttons = [0 0 0 0 0 ]; while ~buttons(3) ; [~,~,buttons] = GetMouse; WaitSecs(0.2); end

% choose the screen setup for stereo. dualhead2go is one doublewide screen
% with L and R half sent to different displays that are overlaid. 2 and 3
% screens send L and R views to different displays. 1 screen no sep is for
% testing.
% choice = menu('Screen configuration?','dualhead2go','2 screens','3 screens', '1 screen, no sep','szv');
choice = 1;
switch choice
    case 1
        stereoConfig = 'dh2g';
    case 2
        stereoConfig = '2s';
    case 3
        stereoConfig = '3s';
    case 4
        stereoConfig = '1sns';
    case 5
        stereoConfig = 'szv';
end


% set useEyeLink to false if you are not using the eyelink eye tracker, set
% to true if you are

%useEyeLink=true;

choice = menu('Use EyeLink?','yes','no');
switch choice
    case 1
        useEyeLink = true;
    case 2
        useEyeLink = false;
end

% to true if you want to wait for magnet. will require measurement
% computing USB-1208 DAQ box and send  a TTL pulse to the magnet

%useHardwareTrigger = true;

choice = menu('Trigger Hardware?','yes','no');
switch choice
    case 1
        useHardwareTrigger = true;
    case 2
        useHardwareTrigger = false;
end


close(gcf); % close last menu.

%% hardcoded parameters

visStimType = 'solid';

% DAQ
if useHardwareTrigger
    daq = DaqDeviceIndex; % connect to USB-1208
end
delayAfterMriTrigger = 0; % seconds to wait after MRI trigger

blocks = {};
loadBlockDesignParameters;

% size of target in eyelink dataviewer
outer_diameter = 18;

numberOfSpriteFrames = 1; % The number of animation frames for our sprite

% screen size
widthScreen = 0;
heightScreen = 0;
screenNumber = max(Screen('Screens'));
loadScreenParameters;

%setup target motion
loadTargetMotionParameters;


%% let the user know about the config...
fprintf('\nconfiguration:\n');
fprintf('using %s screens\n',stereoConfig);
if useEyeLink,  fprintf('using eyelink\n'); else fprintf('not using eyelink\n'); end
if useHardwareTrigger,  fprintf('using hardware trigger\n'); else fprintf('not using hardware trigger\n'); end
fprintf('vis stim type %s\n',visStimType);
fprintf('blocks:');
for m = 1:blockRepeats
    for k = 1:length(blocks)
        fprintf(' %s% ',blocks{k});
    end
end
fprintf('\n');
fprintf('%d blocks, block duration: %d seconds, block repeats: %d, total time: %d seconds\n', k, blockDuration,blockRepeats, k*blockDuration*blockRepeats);

%% eyelink init
eyeLinkInit;

try
    %% screen setup
    % define colors
    whiteColor = 255;
    greyColor = 200;
    blackColor = 50;
    filterMode = 0; % opengl filter mode. 0 is nearest neighbor, 1 is bilinear interp, use 0
    globalAlpha = 1; % opengl global alpha
    triWhiteColor = [1 1 1] * whiteColor;
    modulateColor = triWhiteColor; % [r g b] each 0-255, use to change color of target.
    switch stereoConfig
        case {'dh2g'}
            resolution = NearestResolution(screenNumber, widthScreen*2, heightScreen, 60);
            SetResolution(screenNumber, resolution);
    end
    [widthScreen, heightScreen]=Screen('WindowSize', screenNumber);
    % Screen is able to do a lot of configuration and performance checks on
    % open, and will print out a fair amount of detailed information when
    % it does.  These commands supress that checking behavior and just let
    % the demo go straight into action.  See ScreenTest for an example of
    % how to do detailed checking.
    % Open a window and paint the background
    % window is a vector. window(1) and window(2) equal for
    % one screen mode diff for 2 screen mode. window replaced with
    % window(1) or window(2) everywhere. window(1) == window(2) for
    % dualhead2go folded screen
    switch stereoConfig
        case {'dh2g'}
            window(1) = Screen('OpenWindow', screenNumber, blackColor);
 %           window(2) = window(1);
            % if using matrox dualhead2go for stereo,
            % screen is folded over on itself
            % widthScreen not divided by 2 for multiscreen mode.
            widthScreen = widthScreen / 2;
            % sprites are separated by one screen width in 1 screen mode (offset and folded over) and
            % spriteSep is 0 for multiscreen mode (each sprite on own screen)
            spriteSep = widthScreen;
        case {'2s','3s'}
            window(1) = Screen('OpenWindow', screenNumber, blackColor);
            window(2) = Screen('OpenWindow', screenNumber-1, blackColor);
            spriteSep = 0;
        case {'1sns'}
            window(1) = Screen('OpenWindow', screenNumber, blackColor);
            window(2) = window(1);
            spriteSep = 0;
        case {'szv'}
            window(1) = Screen('OpenWindow', screenNumber-1, blackColor);
            window(2) = window(1);
            [widthScreen, heightScreen]=Screen('WindowSize', screenNumber-1);
            spriteSep = 0;
            greyColor = whiteColor;
    end;
    
    %% calculated parameters
    % note: screenWidth in cm is width of view, so for dualhead2go it is
    % width of one display
    pixelWidth = screenWidthInCm / widthScreen;
    degPerPixel = atand(pixelWidth/screenDistInCm);
    % The height and width of the sprites in dots (the sprite is square)
    spriteSize = round(spriteSizeDeg/degPerPixel);
    targThickness = round(crossThicknessDeg/degPerPixel);
    spriteRect = [0 0 spriteSize spriteSize]; % The bounding box for our animated sprite
    % horizontal amplitude of SPEM
    amplitudeX = amplitudeXdeg / degPerPixel;
    Period = 2 * amplitudeXdeg / averageDegPerSec ;
    saccadeLocations = saccadeLocationsDeg / degPerPixel;
    % screen center coordinates
    centerX = widthScreen/2;
    centerY = heightScreen/2;
    pedX = pedXdeg / degPerPixel;
    pedX2 = pedX / 2;
    
    %% make a cross
    targMask = ones(spriteSize);
    maskIndex1 = round(((spriteSize-targThickness)/2));
    maskIndex2 = round(((spriteSize+targThickness)/2));
    targMask(1:maskIndex1,1:maskIndex1)=0;
    targMask(maskIndex2:end,maskIndex2:end)=0;
    targMask(1:maskIndex1,maskIndex2:end)=0;
    targMask(maskIndex2:end,1:maskIndex1)=0;
    %% animation setup
    for idx = 1 : numberOfSpriteFrames
        
        spriteFrame(idx) = Screen('MakeTexture', window(1), targMask .* greyColor); %#ok<AGROW>
        
    end
    %% Init some state and other variables
    spriteFrameIndex = 1; % Which frame of the animation should we show?
    saccadeLocationIdx = 0;
    lastSaccadeTime = 0; 
    lastDimTime = 0;
    dimFlag = 0;
    responseFlag = 0;
    mX = centerX;
    mXr = mX + widthScreen;
    mY = centerY;
    prev_x = mX;
    prev_y = mY;
   
    %% trials
    %     buttons = 0;
    if useEyeLink
        % start recording eye position (preceded by a short pause so that
        % the tracker can finish the mode transition)
        % The paramerters for the 'StartRecording' call controls the
        % file_samples, file_events, link_samples, link_events availability
        Eyelink('Command', 'set_idle_mode');
        WaitSecs(0.05);
        Eyelink('StartRecording', 1, 1, 1, 1);
        % record a few samples before we actually start displaying
        % otherwise you may lose a few msec of data
        WaitSecs(0.1);
    end
    
     %% Send trigger to Scanner
    if useHardwareTrigger        
        % Do we have a USB-1208FS daq?
        if isempty(daq) % No we don't
             fprintf(':Sorry. Couldn''t find a USB-1208 box connected to your computer.\n');
             fprintf('SCAN NOT STARTED!\n');         
             return;
        else % Yes we do
            devices=PsychHID('Devices');
            d=devices(daq(1)); % use only first one if more connected
            fprintf('Found USB-1208 daq Device.');
        end

        % Configuring digital ports for output
        err=DaqDConfigPort(daq(1),1,0); % should be pin 14

        % Make sure the USB-1208FS is "attached". If not give a warning message.
        % We may want to consider giving an error...
        if streq(err.name,'kIOReturnNotAttached')
        fprintf('error message says USB-1208FS is "not attached".\n''If it is attached, we suggest that you quit and restart MATLAB.');
        return;
        end
      
        % using the USB-1208 hardware counter because polling is too
        % slowwwwww and misses triggers     
        pulseLength = 0.5;   % send pulse for 500 millisecond          
        DaqAOut(daq(1),1,1); 
        WaitSecs(pulseLength); % for a particular length, and then
        DaqAOut(daq(1),1,0);  % reset
        
        %start eyemovement, buttons = 0; % When the user clicks the mouse, 'buttons' becomes nonzero.
        Screen('DrawText', window(1), 'Click to start', 0, 0, whiteColor);
        disp('click to start')
        Screen('Flip', window(1));
        WaitSecs(0.1);
        
        buttons = [0 0 1];
        
        while ~buttons(1)
            [~, ~, buttons] = GetMouse;
            WaitSecs(0.1);
        end
        disp('delay for dummy scans')
              
    else
        %         buttons = 0; % When the user clicks the mouse, 'buttons' becomes nonzero.
        Screen('DrawText', window(1), 'Click to start', 0, 0, whiteColor);
        disp('click to start')
        Screen('Flip', window(1));
        
        buttons = [0 0 1];
        
        while ~buttons(1)
            [~, ~, buttons] = GetMouse;
            WaitSecs(0.1);
        end
        disp('delay for dummy scans')
        WaitSecs(delayAfterMriTrigger);
    end
    
    for blockRepeat = 1:blockRepeats
        % Sending a 'TRIALID' message to mark the start of a trial in Data
        % Viewer.  This is different than the start of recording message
        % START that is logged when the trial recording begins. The viewer
        % will not parse any messages, events, or samples, that exist in
        % the data file prior to this message.
        if useEyeLink, Eyelink('Message', 'TRIALID %d', blockRepeat); end;
        for block = blocks
            stimulusType = block{1};
            fprintf('%s \n',stimulusType);
            % This supplies the title at the bottom of the eyetracker display
            if useEyeLink, Eyelink('command', 'record_status_message "BLOCK %s"', stimulusType); end;
            Screen('Flip', window(1));
            % this starts the eyelink analysis data block
            if useEyeLink, Eyelink('Message', 'BLOCKSYNC %s', stimulusType); end;
            
            %% start block fixation for 500ms
            %             startTime = GetSecs;
            %             endTime = 0.5 + startTime;
            %             while GetSecs < endTime
            %                 mX = centerX;
            %                 mXr = mX + widthScreen;
            %                 mY = centerY;
            %                 prev_x = mX;
            %                 prev_y = mY;
            %                 % Draw the sprite at the new location.
            %                 % Screen('DrawTexture', windowPointer, texturePointer ...
            %                 % [,sourceRect] [,destinationRect] [,rotationAngle] [, filterMode] [, globalAlpha] [, modulateColor] [, textureShader] [, specialFlags] [, auxParameters]);
            %                 Screen('DrawTexture', window(1), spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect , mX, mY),0,filterMode);
            %                 Screen('DrawTexture', window(2), spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect , mXr, mY),0,filterMode);
            %                 % Call Screen('Flip') to update the screen.  Note that calling
            %                 % 'Flip' after we have both erased and redrawn the sprite prevents
            %                 % the sprite from flickering.
            %                 Screen('Flip', window(1));
            %             end
            %% stimulus block
            startTime = GetSecs;
            endTime = blockDuration + startTime;
            spriteFrameT = 1;
            while GetSecs < endTime
                spriteFrameT = spriteFrameT + 1;
                if spriteFrameT > numberOfSpriteFrames
                    spriteFrameT = 1;
                end
                spriteFrameIndex = int32(spriteFrameT);
                switch stimulusType
                    case 'spem'
                        mX = centerX + amplitudeX * sin(2*pi*GetSecs/Period);
                        mXr = mX + spriteSep;
                    case 'sacc'
                        if GetSecs > lastSaccadeTime
                            saccadeLocationIdx = 1 + mod(saccadeLocationIdx,numSaccadeLocations);
                            lastSaccadeTime = GetSecs + saccadePeriod(saccadeLocationIdx);
                        end
                        mX = centerX + saccadeLocations(saccadeLocationIdxs(saccadeLocationIdx));
                        mXr = mX + spriteSep;
                    case 'vergtr'
                        mX = centerX  + (amplitudeX/vergenceDivisor) * sin(2*pi*GetSecs/(vergenceVelDivisor*Period)) + pedX2;
                        mXr = -mX + 2*centerX + spriteSep - 2*pedX2;
                    case 'vergst'
                        if GetSecs > lastSaccadeTime
                            saccadeLocationIdx = 1 + mod(saccadeLocationIdx,numSaccadeLocations);
                            lastSaccadeTime = GetSecs + vergencePeriod;
                        end
                        mX = centerX + saccadeLocations(saccadeLocationIdxs(saccadeLocationIdx))/vergenceDivisor + pedX2;
                        mXr = -mX + 2*centerX + spriteSep - 2*pedX2;
                    otherwise % default to fixation
                        mX = centerX;
                        mXr = mX + spriteSep;
                end
                
                %% attention task
                curTime = GetSecs;
                if curTime > lastDimTime && ~dimFlag % make target dim
                    modulateColor = triWhiteColor - [1 1 1] * attnDimFraction;
                    dimFlag = 1;
                    if useEyeLink, Eyelink('Message', 'attn task %d', attnDimFraction); end;
                    fprintf('attn task %d \n', attnDimFraction)
                end
                if curTime > lastDimTime + 1.0 % make taget not dim
                    modulateColor = triWhiteColor;
                    lastDimTime = curTime + attnMinDelay + attnDelayRange * rand - 1.0;
                    dimFlag = 0;
                    if responseFlag
                        responseFlag = 0;
                        attnDimFraction = attnDimFraction - 1;
                        if useEyeLink, Eyelink('Message', 'attn task response'); end;
                        fprintf('responded.\n')
                    else
                        attnDimFraction = attnDimFraction + 1;
                    end
                end
                
                %% Draw the sprite at the new location.
                mY = centerY;
                Screen('DrawTexture', window(1), spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect , mX, mY),0,filterMode,globalAlpha,modulateColor);
    %           Screen('DrawTexture', window(2), spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect , mXr, mY),0,filterMode,globalAlpha,modulateColor);
                % Call Screen('Flip')
                % using multiflip to sync the window flips with dual monitors seems to
                % improve the stereo quality.
                
                Screen('Flip', window(1), 0, 0, 0, 0);
                
                if useEyeLink
                    Eyelink('Message','!V TARGET_POS TARG1 (%d, %d) 1 1',mX,mY);
                    Eyelink('Message','!V FILLBOX 0 0 0 %d %d %d %d',prev_x-(outer_diameter+1)/2,prev_y-(outer_diameter+1)/2,prev_x+(outer_diameter+1)/2,prev_y+(outer_diameter+1)/2);
                    Eyelink('Message','!V FIXPOINT 255 255 255 0 0 0 %d %d %d 4',mX,mY,outer_diameter);
                end;
                prev_x=mX;
                prev_y=mY;
                % end block early with mouse click
                [~,~,buttons] = GetMouse;
                if buttons(2)
                    break;
                end
                if buttons(3) && dimFlag && ~responseFlag
                    responseFlag = 1;
                end
            end
            if useEyeLink, Eyelink('Message', 'BLOCKEND %s', stimulusType); end;
        end
        if useEyeLink
            % Send out necessary integration messages for data analysis
            % Send out interest area information for the trial
            % See "Protocol for EyeLink Data to Viewer Integration-> Interest
            % Area Commands" section of the EyeLink Data Viewer User Manual
            % IMPORTANT! Don't send too many messages in a very short period of
            % time or the EyeLink tracker may not be able to write them all
            % to the EDF file.
            % Consider adding a short delay every few messages.
            % Send messages to report trial condition information
            % Each message may be a pair of trial condition variable and its
            % corresponding value follwing the '!V TRIAL_VAR' token message
            % See "Protocol for EyeLink Data to Viewer Integration-> Trial
            % Message Commands" section of the EyeLink Data Viewer User Manual
            Eyelink('Message', '!V TRIAL_VAR block_rep %d', blockRepeat)
            % Sending a 'TRIAL_RESULT' message to mark the end of a trial in
            % Data Viewer. This is different than the end of recording message
            % END that is logged when the trial recording ends. The viewer will
            % not parse any messages, events, or samples that exist in the data
            % file after this message.
            Eyelink('Message', 'TRIAL_RESULT 0')
        end;
    end
    if useEyeLink
        % adds 100 msec of data to catch final events
        WaitSecs(0.1);
        % stop the recording of eye-movements for the current trial
        Eyelink('StopRecording');
        % End of Experiment; close the file first
        % close graphics window, close data file and shut down tracker
        Eyelink('Command', 'set_idle_mode');
        WaitSecs(0.5);
        Eyelink('CloseFile');
    end;
    
    
    
catch %#ok<CTCH>
    % If there is an error in our try block, let's
    % return the user to the familiar MATLAB prompt.
    ShowCursor;
    Screen('CloseAll');
    psychrethrow(psychlasterror);
    
end

if useEyeLink
    % download eyelink data file
    try
        fprintf('Receiving data file ''%s''\n', edfFile );
        status=Eyelink('ReceiveFile');
        if status > 0
            fprintf('ReceiveFile status %d\n', status);
        end
        if 2==exist(edfFile, 'file')
            fprintf('Data file ''%s'' can be found in ''%s''\n', edfFile, pwd );
        end
    catch %#ok<CTCH>
        fprintf('Problem receiving data file ''%s''\n', edfFile );
    end
    
    
    %close the eye tracker.
    Eyelink('ShutDown');
end;
%% clean up and close all screens
% wait for click
[~, ~, buttons] = GetMouse;
while ~any(buttons)
    [~, ~, buttons] = GetMouse;
    Screen('DrawText', window(1), 'Click to end', 0, 0, whiteColor);
    Screen('Flip', window(1));
end
if useEyeLink
    copyfile([edfFile '.edf'],[num2str(now) '.edf']);
end
fprintf('all done. shutting down...\n');
ShowCursor;
Screen('CloseAll');
