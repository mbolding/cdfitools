function plotter(edfFile)
% See SpriteDemo.
% See also MovieDemo.
% See also EyeLink demo code.
% 02/02/10 msb modifying for spem experiments
% 06/21/10 msb extending for use by DG and MW

%% interactive parameters
fprintf('\n***\nstarting.\n');

% choose the screen setup for stereo. dualhead2go is one doublewide screen
% with L and R half sent to different displays that are overlaid. 2 and 3
% screens send L and R views to different displays. 1 screen no sep is for
% testing.
stereoConfig = 'szv';
useHardwareTrigger = false;
% set useEyeLink to false if you are not using the eyelink eye tracker, set
% to true if you are
% choice = menu('Use EyeLink?','yes, please','no, thank you');
choice = 1;
switch choice
    case 1
        useEyeLink = true;
    case 2
        useEyeLink = false;
end
visStimType = 'solid';
blocks = {'vergtr','vergst'};
% close(gcf); % close last menu.

%% hardcoded parameters
% DAQ
daq = DaqDeviceIndex; % connect to USB-1208
delayAfterMriTrigger = 0; % seconds to wait after MRI trigger

% blocks, experiment design
blockDuration = 20;
blockRepeats = 5;

% size of target in eyelink dataviewer
outer_diameter = 18;

% number of animation frames
if strcmp(visStimType,'cyclopean')
    numberOfSpriteFrames = 16; % The number of animation frames for our sprite
else
    numberOfSpriteFrames = 1; % The number of animation frames for our sprite
end

% size of RDS dot in pixels.
dotSize = 4;
dotDensity = 0.2;
% rdsUpdateRate = 0.25; % needs to go evenly into 1

% background masks monocular visibility
bkgSpriteSizeDeg = 25;
% bkgDispDeg = 1; % static disparity. "null" point

% physical screen parameters in cm
screenWidthInCm = 27.2;
screenDistInCm = 42;

% target size
spriteSizeDeg = 2; % sprite size in degrees.
crossThicknessDeg = 0.25;

% spem parameters
% averageDegPerSec = 16.7;
% amplitudeXdeg = 14; % horizontal spem amplitude

% saccade parameters
% saccadePeriod = 0.75; % time between saccades
saccadeLocationsDeg = [-10,-5,0,5,10];
% saccadeLocationIdxs = [ 4 3 2 5 4 3 1 2 5 1 5 4 2 3 1 1 1 2 4 5 3 5 2 4 3]';
% numSaccadeLocations = max(size(saccadeLocationIdxs));

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
fprintf('block duration: %d, block repeats: %d\n',blockDuration,blockRepeats);

%% eyelink init
if useEyeLink
    % Initialization of the connection with the Eyelink Gazetracker.
    % exit program if this fails.
    initializedummy=0;
    if initializedummy~=1
        if Eyelink('initialize') ~= 0
            fprintf('error in connecting to the eye tracker');
            return;
        end
    else
        Eyelink('initializedummy');
    end
    % Added a dialog box to set your own EDF file name before opening
    % experiment graphics. Make sure the entered EDF file name is 1 to 8
    % characters in length and only numbers or letters are allowed.
    %     prompt = {'Enter tracker EDF file name (1 to 8 letters or numbers)'};
    %     dlg_title = 'Create EDF file';
    %     num_lines= 1;
    %     def     = {'DEMO'};
    %     answer  = inputdlg(prompt,dlg_title,num_lines,def);
    %     %edfFile= 'DEMO.EDF'
    %     edfFile = answer{1};
else
    fprintf('EyeLink connection not used.\n');
end;
try
    %% screen setup
    % define colors
    whiteColor = 255;
    greyColor = 128;
    blackColor = 0;
    % Choose screens
    screenNumber = max(Screen('Screens'));
    [widthScreen, heightScreen]=Screen('WindowSize', screenNumber);
    % Screen is able to do a lot of configuration and performance checks on
    % open, and will print out a fair amount of detailed information when
    % it does.  These commands supress that checking behavior and just let
    % the demo go straight into action.  See ScreenTest for an example of
    % how to do detailed checking.
    oldVisualDebugLevel = Screen('Preference', 'VisualDebugLevel', 3);
    oldSupressAllWarnings = Screen('Preference', 'SuppressAllWarnings', 1);
    % Open a window and paint the background
    % window is a vector. window(1) and window(2) equal for
    % one screen mode diff for 2 screen mode. window replaced with
    % window(1) or window(2) everywhere. window(1) == window(2) for
    % dualhead2go folded screen
    switch stereoConfig
        
        case {'szv'}
            window(1) = Screen('OpenWindow', screenNumber, blackColor);
            window(2) = window(1);
            [widthScreen, heightScreen]=Screen('WindowSize', screenNumber);
            %             spriteSep = 0;
            greyColor = whiteColor;
    end;
    
    %% calculated parameters
    % note: screenWidth in cm is width of view, so for dualhead2go it is
    % width of one display
    pixelWidth = screenWidthInCm / widthScreen;
    degPerPixel = atand(pixelWidth/screenDistInCm);
    % The height and width of the sprites in dots (the sprite is square)
    spriteSize = round(spriteSizeDeg/(degPerPixel * dotSize));
    bkgSpriteSize = round(bkgSpriteSizeDeg / (degPerPixel * dotSize));
    crossThickness = round(crossThicknessDeg/(degPerPixel * dotSize));
    %     spriteRect = [0 0 spriteSize spriteSize]; % The bounding box for our animated sprite
    %     bkgSpriteRect = [0 0 bkgSpriteSize bkgSpriteSize]; % The bounding box for our animated background
    % horizontal amplitude of SPEM
    %     amplitudeX = amplitudeXdeg / degPerPixel;
    %     Period = 2 * amplitudeXdeg / averageDegPerSec ;
    saccadeLocations = saccadeLocationsDeg / degPerPixel;
    %     bkgDisp = round(bkgDispDeg / degPerPixel);
    % screen center coordinates
    %     centerX = widthScreen/2;
    %     centerY = heightScreen/2;
    
    %% configure eyelink graphics
    if useEyeLink
        % Provide Eyelink with details about the graphics environment
        % and perform some initializations. The information is returned
        % in a structure that also contains useful defaults
        % and control codes (e.g. tracker state bit and Eyelink key values).
        el=EyelinkInitDefaults(window(1));
        
        [~, vs]=Eyelink('GetTrackerVersion');
        fprintf('Running experiment on a ''%s'' tracker.\n', vs );
        
        % open file to record data to
        i = Eyelink('Openfile', edfFile);
        if i~=0
            fprintf('Cannot create EDF file ''%s'' ', edfFile);
            Eyelink( 'Shutdown');
            return;
        end
        Eyelink('command', 'add_file_preamble_text ''Recorded by EyelinkToolbox demo-experiment''');
        
        %% SET UP eyelink TRACKER CONFIGURATION
        % Setting the proper recording resolution, proper calibration type,
        % as well as the data file content;
        Eyelink('command','screen_pixel_coords = %ld %ld %ld %ld', 0, 0, widthScreen - 1, heightScreen - 1);
        Eyelink('message', 'DISPLAY_COORDS %ld %ld %ld %ld', 0, 0, widthScreen - 1, heightScreen - 1);
        % set calibration type.
        Eyelink('command', 'calibration_type = HV9');
        % set parser (conservative saccade thresholds)
        %     Eyelink('command', 'saccade_velocity_threshold = 35');  % these two
        %     just illustrate that you can change any variable.  change thresholds
        %     for saccade start and end.  we probably want defaults.
        %     Eyelink('command', 'saccade_acceleration_threshold = 9500');
        % set EDF file contents
        Eyelink('command', 'file_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK,MESSAGE,BUTTON');
        Eyelink('command', 'file_sample_data  = LEFT,RIGHT,GAZE,HREF,AREA,GAZERES,STATUS');
        % set link data (used for gaze cursor)
        Eyelink('command', 'link_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK,MESSAGE,BUTTON');
        Eyelink('command', 'link_sample_data  = LEFT,RIGHT,GAZE,GAZERES,AREA,STATUS');
        % allow to use the big button on the eyelink gamepad to accept the
        % calibration/drift correction target
        Eyelink('command', 'button_function 5 "accept_target_fixation"');
        
        % make sure we're still connected.
        if Eyelink('IsConnected')~=1
            return;
        end;
        
        % Calibrate the eye tracker
        % setup the proper calibration foreground and background colors
        % needs to be the same lum as expt, pupil size affects calibration
        el.backgroundcolour = 0;
        el.foregroundcolour = 255;
        EyelinkDoTrackerSetup(el);
    end
    %% make a cross
    crossMask = ones(spriteSize);
    maskIndex1 = floor(((spriteSize-crossThickness)/2));
    maskIndex2 = ceil(((spriteSize+crossThickness)/2));
    crossMask(1:maskIndex1,1:maskIndex1)=0;
    crossMask(maskIndex2:end,maskIndex2:end)=0;
    crossMask(1:maskIndex1,maskIndex2:end)=0;
    crossMask(maskIndex2:end,1:maskIndex1)=0;
    %% animation setup
    for idx = 1 : numberOfSpriteFrames
        % Create the frames for the animated sprite.  Here the animation
        % consists of RDS patch or a cross
        if strcmp(visStimType,'cyclopean')
            spriteSize = floor(spriteSize);
            bkgSpriteSize = floor(bkgSpriteSize);
            % for double dots halve the size and halve the source
            % rect below in the texture draw. leave dest rect alone.
            % round(rand(sizeY,sizeX)-.5+dens)
            spriteFrame(idx) = Screen('MakeTexture', window(1), whiteColor * round(rand(spriteSize)-0.5+dotDensity)); %#ok<AGROW>
            %             bkgFrame(idx)    = Screen('MakeTexture', window(1), whiteColor * round(rand(bkgSpriteSize)-0.5+dotDensity)); %#ok<AGROW>
            saccadeLocations = round(saccadeLocations);
        else % solid target
            spriteFrame(idx) = Screen('MakeTexture', window(1), crossMask .* greyColor); %#ok<AGROW>
        end
    end
    %% Init some state and other variables
    %     spriteFrameIndex = 1; % Which frame of the animation should we show?
    %     saccadeLocationIdx = 0;
    %     lastSaccadeTime = 0;
    %% wait for trigger
    if useHardwareTrigger
        % using the USB-1208 hardware counter because polling is too
        % slowwwwww and misses triggers
        countStart = DaqCIn(daq);
        countEnd = countStart;
        fprintf('Waiting for magnet. start counter value: %d  \n',countStart)
        while countStart == countEnd
            countEnd = DaqCIn(daq);
            % so we do not poll the USB-1208 too fast, was hanging without
            % the delay
            WaitSecs(0.1);
        end
        fprintf('Rcvd trigger. end counter value: %d \n',countEnd)
        WaitSecs(delayAfterMriTrigger);
    else
        buttons = 0; % When the user clicks the mouse, 'buttons' becomes nonzero.
        %         while ~any(buttons)
        %             Screen('DrawText', window(1), 'Click to start', 0, 0, whiteColor);
        %             Screen('Flip', window(1));
        %             [~, ~, buttons] = GetMouse;
        %         end
        Screen('Flip', window(1));
    end
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
    for blockRepeat = 1:blockRepeats
        fprintf('- block:%d ',blockRepeat);
        % Sending a 'TRIALID' message to mark the start of a trial in Data
        % Viewer.  This is different than the start of recording message
        % START that is logged when the trial recording begins. The viewer
        % will not parse any messages, events, or samples, that exist in
        % the data file prior to this message.
        if useEyeLink, Eyelink('Message', 'TRIALID %d', blockRepeat); end;
        
        DaqAOut(daq,1,1); % turn on LED
        for k = 0:0.01:60 % move LED around a little, step
            position=0.5-0.5*cos(k);
            DaqAOut(daq,0,position>0.5);
            WaitSecs(0.005);
        end
        DaqAOut(daq,0,0); % put LED back home
        DaqAOut(daq,1,0); % turn off LED
        WaitSecs(2);
        
        DaqAOut(daq,1,1); % turn on LED
        for k = 0:0.01:60 % move LED around a little, sin
            position=0.5-0.5*cos(k);
            DaqAOut(daq,0,position);
            WaitSecs(0.005);
        end
        DaqAOut(daq,0,0); % put LED back home
        DaqAOut(daq,1,0); % turn off LED
        WaitSecs(2);
        
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
            Eyelink('Message', '!V TRIAL_VAR block_rep %d', blockRepeat);
            % Sending a 'TRIAL_RESULT' message to mark the end of a trial in
            % Data Viewer. This is different than the end of recording message
            % END that is logged when the trial recording ends. The viewer will
            % not parse any messages, events, or samples that exist in the data
            % file after this message.
            Eyelink('Message', 'TRIAL_RESULT 0');
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
    % Restore preferences
    Screen('Preference', 'VisualDebugLevel', oldVisualDebugLevel);
    Screen('Preference', 'SuppressAllWarnings', oldSupressAllWarnings);
    
catch %#ok<CTCH>
    % If there is an error in our try block, let's
    % return the user to the familiar MATLAB prompt.
    ShowCursor;
    Screen('CloseAll');
    Screen('Preference', 'VisualDebugLevel', oldVisualDebugLevel);
    Screen('Preference', 'SuppressAllWarnings', oldSupressAllWarnings);
    psychrethrow(psychlasterror);
    
end

if useEyeLink
    % download eyelink data file
    try
        fprintf('\nReceiving data file ''%s''\n', edfFile );
        status=Eyelink('ReceiveFile');
        if status > 0
            fprintf('ReceiveFile status %d\n', status);
        end
        if 2==exist(edfFile, 'file')
            fprintf('Data file ''%s'' can be found in ''%s''\n', edfFile, pwd );
        end
        fprintf('Data received.\n');
    catch %#ok<CTCH>
        fprintf('Problem receiving data file ''%s''\n', edfFile );
    end   
    %close the eye tracker.
    Eyelink('ShutDown');
end;
%% clean up, save data, and close all screens
[~, ~, buttons] = GetMouse;
Screen('DrawText', window(1), 'Click to end', 0, 0, whiteColor);
Screen('Flip', window(1));
while ~any(buttons) % blank screen at end of run
    [~, ~, buttons] = GetMouse; % wait for click
end
copyfile([edfFile '.edf'],[edfFile '.' num2str(now) '.edf']); % make a time stamped copy of data
fprintf('all done.\n');
ShowCursor;
Screen('CloseAll');
Screen('Preference', 'VisualDebugLevel', oldVisualDebugLevel);
Screen('Preference', 'SuppressAllWarnings', oldSupressAllWarnings);

