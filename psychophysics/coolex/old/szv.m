function szv()
%% runs an experiment for SEMS
% no arguments yet. :)
% 28 june 10, msb, created

%% ==settings==
%% interactive parameters
% mode: use eyelink and DAC or simulate?
choice = menu('mode:','expt','test screen','test logic');
switch choice
    case 1 % expt
        useDAQ = true;
        useEyeLink = true;
        useScreen = true;
    case 2 % test screen;
        useDAQ = false;
        useEyeLink = false;
        useScreen = true;
    case 3 % test
        useDAQ = false;
        useEyeLink = false;
        useScreen = false;
end

close(gcf) % close last menu

%% hard coded parameters
% display physical parameters
% size of display in mm
screenSizeMm = [400,300];
% distance of display in mm
dispDistMm = 600;
% actual size of display in pixels is found by psychtoolbox call below
% target size
spriteSizeDeg = 2; % sprite size in degrees.

%% trial parameters
eyeMot.orients = {'fronto','verge'};
eyeMot.amp = [5 , 3]; % deg for fronto and MA for verge
eyeMot.ctr = [0 , 5]; % deg for fronto and MA for verge
eyeMot.types = {'antistep','track'};
eyeMot.subtypes = {'normal','oddball'}; % straight task or oddball.
eyeMot.frqs = [0.18 , 0.32 , 0.56];
eyeMot.numTrialReps = 10;

%% build trials
idxTrial = 0;
fprintf('trials\n');
for idxOrient = 1:length(eyeMot.orients)
    for idxType = 1:length(eyeMot.types)
        for idxSubType = 1:length(eyeMot.subtypes)
            for idxFreqs = 1:length(eyeMot.frqs)
                idxTrial = idxTrial + 1;
                trials(idxTrial).orient = eyeMot.orients{idxOrient}; %#ok<AGROW>
                trials(idxTrial).type = eyeMot.types{idxType}; %#ok<AGROW>
                trials(idxTrial).reps = eyeMot.numTrialReps; %#ok<AGROW>
                trials(idxTrial).subtype = eyeMot.subtypes{idxSubType}; %#ok<AGROW>
                trials(idxTrial).frq = eyeMot.frqs(idxFreqs); %#ok<AGROW>
%                 fprintf('%d %s %s %s %.2f Hz\n',trials(idxTrial).reps,trials(idxTrial).orient,...
%                     trials(idxTrial).type,trials(idxTrial).subtype,trials(idxTrial).frq);                
            end
        end
    end
end

trials = trials(randperm(length(trials)));
fprintf('permuted trials\n')
% for idxTrial = 1:length(trials) 
%                     fprintf('%d %s %s %s %.2f Hz\n',trials(idxTrial).reps,trials(idxTrial).orient,...
%                     trials(idxTrial).type,trials(idxTrial).subtype,trials(idxTrial).frq);  
% end
save(['trials.' num2str(now) '.mat'], 'trials'); % save a copy for the analysis step
%% ==setup and initialization==

% init eyelink
if useEyeLink
    % size of target in eyelink dataviewer
    eyelinkTargSize = 18;
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
    prompt = {'Enter tracker EDF file name (1 to 8 letters or numbers)'};
    dlg_title = 'Create EDF file';
    num_lines= 1;
    def     = {'DEMO'};
    answer  = inputdlg(prompt,dlg_title,num_lines,def);
    %edfFile= 'DEMO.EDF'
    edfFile = answer{1};
else
    fprintf('EyeLink connection not used.\n');
end;

% init usb-1208fs
if useDAQ
    daq = DaqDeviceIndex; % connect to USB-1208
    % test output
    DaqAOut(daq,1,1) % turn on LED
    for k = 0:0.01:(4*pi) % move LED around a little.
        position=0.5-0.5*cos(k);
        DaqAOut(daq,0,position);
        WaitSecs(0.005);
    end
    DaqAOut(daq,0,0); % put LED back home
    DaqAOut(daq,1,0); % turn off LED
end

% init psychtoolbox
try % in case something bombs while psychtoolbox controls the display.
    %% screen setup
    if useScreen
        % define colors
        whiteColor = 255;
        %     greyColor = 128;
        blackColor = 0;
        % Choose screens
        screenNumber = max(Screen('Screens'));
        [widthScreenPx, heightScreenPx]=Screen('WindowSize', screenNumber);
        % Open a window and paint the background
        window(1) = Screen('OpenWindow', screenNumber, blackColor);
        %% calculated screen parameters
        pixelWidthMm = screenSizeMm(1) / widthScreenPx;
        pixelHeightMm = screenSizeMm(1) / heightScreenPx;
        horizDegPerPixel = atand(pixelWidthMm/dispDistMm);
        vertDegPerPixel = atand(pixelHeightMm/dispDistMm);
    end
    % wait for mouse click
    buttons = 0; % When the user clicks the mouse, 'buttons' becomes nonzero.
    fprintf('click mouse to start\n')
    while ~any(buttons)
        if useScreen
            Screen('DrawText', window(1), 'Click to start', 0, 0, whiteColor);
            Screen('Flip', window(1));
        end
        [~, ~, buttons] = GetMouse;
        
    end
    %% ==experiment==
    
    
    %% run trial
    for idxTrial = 1:length(trials)
        % do current trial
        fprintf('%d %s %s %s %.2f Hz\n',trials(idxTrial).reps,trials(idxTrial).orient,...
            trials(idxTrial).type,trials(idxTrial).subtype,trials(idxTrial).frq);

        
        
        %% end trial
    end;
%% ==shut down and clean up==
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
    catch EYELINKFAIL
        disp(EYELINKFAIL)
        fprintf('Problem receiving data file ''%s''\n', edfFile );
    end   
    %close the eye tracker.
    Eyelink('ShutDown');
end;

% shut down psychtoolbox
% clean up and close all screens
% wait for click
[~, ~, buttons] = GetMouse;
while ~any(buttons)
    [~, ~, buttons] = GetMouse;
    if useScreen
        Screen('DrawText', window(1), 'Click to end', 0, 0, whiteColor);
        Screen('Flip', window(1));
    end
end
fprintf('all done. shutting down...\n');
if useScreen
    ShowCursor;
    Screen('CloseAll');
end
%% ==catch error in expt==
catch EPICFAIL
    % If there is an error in our try block, let's
    % return the user to the familiar and comforting MATLAB prompt.
    disp(EPICFAIL)
    ShowCursor;
    Screen('CloseAll');
    psychrethrow(psychlasterror);
end
return % expt is done.


%% ==functions==

%% ==notes==
% http://tesla.cdfi.uab.edu/mediawiki/index.php/Proposal
%
% General stimulus characteristics:
% * They are small white targets on a black backgrounds.
% * The fronto-parallel plane tasks will be presented on an approximately 40 cm wide by 30 cm high CRT screen at approximately 57 cm viewing distance.
% * The vergence target will be a white LED with a holographic diffuser. A small black dot will be inscribed on the diffuser. The LED target will be mounted on an x-y plotter carriage enabling it to move in space.
%
% Tasks:
% * Anti-steps
% ** Antivergence steps over a range of 5±3 MA
% ** Antisaccades with amplitude of 5 degrees and unpredictable direction.
% * Steps, oddball
% ** An oddball task consisting of a predictable pattern of visually guided vergence steps (5±3 MA) with pseudorandom syncopations (0.1 probability).
% ** An oddball task consisting of predictable pattern of visually guided saccades (±5 degrees) with pseudorandom syncopations (0.1 probability).
% * Tracking
% ** Triangular vergence tracking in depth of 5±3 MA at 0.18; 0.32; 0.56 Hz.
% ** Triangular smooth pursuit of ±5 degrees at 0.18; 0.32; 0.56 Hz.
% * Tracking, oddball
% ** Predictive vergence tracking in depth with random change in targ motion (0.1 prob. per sweep).
% ** Predictive smooth pursuit with random change in target motion (0.1 prob. per sweep).

