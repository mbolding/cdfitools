function OneDprober
% makes and 1D noise probes for the electrophysiology part of the NSE
% project. 


%% DAQ setup for hardware triggering on line A zero of USB-1208FS and
%% analog out indicating vis stim
daq=DaqDeviceIndex([],0);


if isempty(daq)
  % check again because user might have run PsychHID('Devices') and then plugged
  % in the daq.  This should fix that.
  clear PsychHID;
  daq = DaqDeviceIndex([],0);
end

DaqAOut(daq,0,0);
% devices=PsychHID('Devices');


%% key assignments
KbName('UnifyKeyNames');
leftKey = KbName('LeftArrow');      % rotate stim ccw
rightKey = KbName('RightArrow');    % rotate stim cw
upKey = KbName('UpArrow');          % larger stim
downKey = KbName('DownArrow');      % smaller stim
eqKey = KbName('p');                % next movie
dashKey = KbName('o');              % prev movie
qKey = KbName('q');                 % movie 1
wKey = KbName('w');                 % move fix up
eKey = KbName('e');                 % show some parameters
aKey = KbName('a');                 % move fix left
sKey = KbName('s');                 % move fix down
dKey = KbName('d');                 % move fix right
fKey = KbName('f');                 % flash fix
tKey = KbName('t');                 % mouse one shot
mKey = KbName('m');                 % toggle stim location lock
nKey = KbName('n');                 % toggle static dynamic
escapeKey = KbName('ESCAPE');       % quit
spaceKey = KbName('space');         % toggle trigger / freerun

% prevent Matlab from receiving typed characters by calling ListenChar(2);
% at the beginning of you script and calling ListenChar(0); at the end of
% your script.
ListenChar(2); 

%% make movies
framesPerSec = 85;
frameDiv = 4; % to slow down movies
shortMovies = 0;
if shortMovies == 1
    movies = [...
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;...
        1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;...
        1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 ;...
        ];
    numMovies = size(movies,1);
else
    totalSecs = 3;
    numMovies = 7;
    preDelaySecs = 1;
    disp(['pre secs ' num2str(preDelaySecs)])
    preDelayFrames = framesPerSec * preDelaySecs;
    movies = zeros(numMovies,preDelayFrames);
    stimFrames = zeros(numMovies,2^(numMovies-1));
    stimLen = 2.^(cumsum(ones(numMovies,1))-1); % 1,2,4,8,16 ...
%     stimLen(end) = 2 * framesPerSec;
    for k = 1:numMovies
        stimFrames(k,1:stimLen(k)) = 1;
        disp(['stim ' num2str(k) ' ' num2str(stimLen(k)/framesPerSec) ' secs, ' num2str(stimLen(k)) ' frames'])
    end
    postDelaySecs = totalSecs - (preDelaySecs + stimLen(k)/framesPerSec);
    if postDelaySecs < 0; postDelaySecs = 0; end
    disp(['post secs ' num2str(postDelaySecs)])
    postDelayFrames = framesPerSec * postDelaySecs;
    movies = horzcat(movies,stimFrames,zeros(numMovies,postDelayFrames)); %#ok<AGROW>
    numMovies = numMovies + 1;
%     fixMovie = ones(1,size(movies,2));
%     fixMovie(end) = 0;
    movies = vertcat(ones(1,size(movies,2)),movies); %#ok<AGROW>
    stimLen = [0 ; stimLen];
end

movieLength = size(movies,2);
disp(['movie length ' num2str(movieLength/framesPerSec) ' seconds at ' num2str(framesPerSec/frameDiv) ' Hz'])

%%  make maps for sprites 
p = 10; % pixels per bar
b = [5 5]; % number of bars
na = 36; % number of angles
c = stripedcircle(p,b,na); % 1D gradient with circular window
spriteSize = p*sum(b);
numberOfSpriteFrames = na;
fixSize = 7; % fixation point
fixPt = ones(fixSize,fixSize,1,'uint8') * 256;

try
    %%  Screen and Color Setup 

    % Choose a screen
    screenNumber = 1;
    
    % Make a backup copy of original LUT into origLUT.
    origLUT=Screen('ReadNormalizedGammaTable', screenNumber);
    newLUT=origLUT;
    newLUT(1:128,:)= min(origLUT(:));
    newLUT(129:end,:)= max(origLUT(:));
    % Get colors
    backgroundColor = BlackIndex(screenNumber);
%     foregroundColor = WhiteIndex(screenNumber);

    
	% Screen is able to do a lot of configuration and performance checks on
	% open, and will print out a fair amount of detailed information when
	% it does.  These commands supress that checking behavior and just let
    % the demo go straight into action.  See ScreenTest for an example of
    % how to do detailed checking.
	oldVisualDebugLevel = Screen('Preference', 'VisualDebugLevel', 3);
    oldSupressAllWarnings = Screen('Preference', 'SuppressAllWarnings', 1);
    
    % Open a window and paint the background white
    window = Screen('OpenWindow', screenNumber, backgroundColor);

    % Hide the mouse cursor.
    HideCursor;


    %%  Make Textures 
    fixFrame = Screen('MakeTexture', window, fixPt); % fixation point
    
    for i = 1 : numberOfSpriteFrames
        % Create the frames for the animated sprite.  Here the animation
        % consists of noise.        
        temp = c(:,:,i);
        spriteFrame(i) = Screen('MakeTexture', window, temp ); %#ok<AGROW>
    end

    %%  Initial state 
    fixRect = [ 0 0 fixSize fixSize ];
    fixLocX = 30;
    fixLocY = 30;
    spriteRect = [0 0 spriteSize spriteSize]; % The bounding box for our animated sprite
    spriteFrameIndex = 1; % Which frame of the animation should we show?
%     buttons = 0; % When the user clicks the mouse, 'buttons' becomes nonzero.
    spriteScale = 1;
    movieNum = 1;
    movieIdx = 1;
    triggeredMode = 0;
    mouseMove = 0;
    staticMode = 0;
    mX = 100; % initial mouse location
    mY = 100;
    fixOn = 1;
    if exist('lastOneDprober.mat','file') % load some settings from last run if they exist
        load lastOneDprober.mat fixLocX fixLocY mX mY spriteFrameIndex spriteScale
        disp('loading settings from last run')
    end

    %% display loop
    % Exit the demo as soon as the user presses ESC.
    disp('press ESC to exit.');
    while (1)
        % Get the location and click state of the mouse.
        if mouseMove == 1; [mX, mY, buttons] = GetMouse; end
        
        % Draw the sprite at the new location.
        if movies(movieNum,movieIdx) == 1;
            Screen('DrawTexture', window, spriteFrame(floor(spriteFrameIndex)), spriteRect, CenterRectOnPoint(floor(spriteRect*spriteScale), mX, mY),0,0);
        end
        if fixOn == 1;
            Screen('DrawTexture', window, fixFrame, fixRect, CenterRectOnPoint(fixRect,fixLocX,fixLocY));
        end
        % Call Screen('Flip') to update the screen.  Note that calling
        % 'Flip' after we have both erased and redrawn the sprite prevents
        % the sprite from flickering.
        
        Screen('Flip', window);
%         [vblt sost fts miss bp] = Screen('Flip', window);
%         if miss > 0; disp(miss); end
        DaqAOut(daq,0,movies(movieNum,movieIdx)-staticMode/2); % analog for external record of stimulus 
        
        if mod(movieIdx,frameDiv) == 0 && staticMode == 0
            newLUT(2:(end-1),:) = newLUT(randperm(254)+1,:); % scramble all but first and last colors in LUT
            Screen('LoadNormalizedGammaTable', screenNumber, newLUT); 
        end
        movieIdx = movieIdx + 1; % increment movie frame
        if movieIdx > movieLength; 
            movieIdx = 1;
            newLUT(2:(end-1),:) = newLUT(randperm(254)+1,:); % scramble all but first and last colors in LUT for the next movie
            Screen('LoadNormalizedGammaTable', screenNumber, newLUT); 
            %% triggering
            if triggeredMode == 1
%                 fprintf(1,'waiting for TTL trigger.\n'); % at the end of the movie wait for trigger
                daq_data = [254,254];      % edge trig, hi to low
                while (daq_data(1) == 254 && ~KbCheck)
                    daq_data=DaqDIn(daq);
                    
                end
                while (daq_data(1) == 255 && ~KbCheck)
                    daq_data=DaqDIn(daq);
                end
            end
        end
        
        %% keyboard input
        [ keyIsDown, seconds, keyCode ] = KbCheck; % did user press any keys?
        if keyIsDown
            if keyCode(leftKey) % rotate ccw
                spriteFrameIndex = spriteFrameIndex + 0.1;
                if spriteFrameIndex > numberOfSpriteFrames
                    spriteFrameIndex = 1;
                end
            elseif keyCode(rightKey) % rotate cw
                    spriteFrameIndex = spriteFrameIndex - 0.1;
                    if spriteFrameIndex < 1
                        spriteFrameIndex = numberOfSpriteFrames;
                    end
            elseif keyCode(upKey) % bigger
                spriteScale = spriteScale*1.05;
            elseif keyCode(downKey) % smaller
                spriteScale = spriteScale/1.05;
            elseif keyCode(eqKey) % next movie
                movieNum = movieNum + 1;
                if movieNum > numMovies
                    movieNum = 1;
                end
                disp(['stim ' num2str(movieNum) ' ' num2str(stimLen(movieNum)/framesPerSec) ' secs, ' num2str(stimLen(movieNum)) ' frames'])
                pause(0.2)
            elseif keyCode(dashKey) % previous movie
                 movieNum = movieNum - 1;
                if movieNum < 1
                    movieNum = numMovies;
                end       
                disp(['stim ' num2str(movieNum) ' ' num2str(stimLen(movieNum)/framesPerSec) ' secs, ' num2str(stimLen(movieNum)) ' frames'])
                pause(0.2)
            elseif keyCode(qKey) % movie 1
                movieNum=1;
            elseif keyCode(wKey) % fixation up
                fixLocY = fixLocY - 2;
            elseif keyCode(aKey) % fixation left
                fixLocX = fixLocX - 2;
            elseif keyCode(sKey) % fixation down
                fixLocY = fixLocY + 2;
            elseif keyCode(dKey) % fixation right
                fixLocX = fixLocX + 2;
            elseif keyCode(fKey) % fixation
                if fixOn == 1;
                    fixOn = 0; 
                    disp('fix off');
                else
                    fixOn = 1; 
                    disp('fix on');
                end;
                pause(0.5); % wait for key raise
            elseif keyCode(eKey) % disp current stim locations
                fprintf(1,' %d %d %d %d %d %d \n',fixLocX, fixLocY, mX, mY, spriteFrameIndex, spriteScale)
            elseif keyCode(tKey) % stop here for one trigger
                fprintf(1,'waiting for mouse click.\n');
                while ~any(buttons)
                    [x,y,buttons] = GetMouse;
                end
            elseif keyCode(escapeKey) % exit 
                break;
            elseif keyCode(spaceKey) % toggle between free run and triggered mode
                if triggeredMode == 1;
                    triggeredMode = 0; 
                    disp('freerun mode');
                else
                    triggeredMode = 1; 
                    disp('triggered mode');
                end;
                pause(0.5); % wait for key raise
            elseif keyCode(mKey) % toggle between mouse move and locked mode
                if mouseMove == 1;
                    mouseMove = 0; 
                    disp('locked mode');
                else
                    mouseMove = 1; 
                    disp('mouse move mode');
                end;
                pause(0.5); % wait for key raise
            elseif keyCode(nKey) % toggle static mode
                if staticMode == 1;
                    staticMode = 0;
                    disp('flicker mode');
                else
                    staticMode = 1;
                    disp('static mode');
                end;
                pause(0.5); % wait for key raise
            end
        end
    end

    %% Revive the mouse cursor.
    ShowCursor; 

    % Close screen
    Screen('CloseAll');
    
    %% Restore preferences
    Screen('LoadNormalizedGammaTable', screenNumber, origLUT);
    ListenChar(0);
    Screen('Preference', 'VisualDebugLevel', oldVisualDebugLevel);
    Screen('Preference', 'SuppressAllWarnings', oldSupressAllWarnings);
    %% save the state
    save([num2str(now) '.mat'])
    save lastOneDprober.mat fixLocX fixLocY mX mY spriteFrameIndex spriteScale
catch %#ok<CTCH>
    %% If there is an error in our try block, let's
    % return the user to the familiar MATLAB prompt.
    ListenChar(0);
    ShowCursor; 
    if exist('origLUT', 'var')
        Screen('LoadNormalizedGammaTable', screenNumber, origLUT);
    end
    Screen('CloseAll');
    save('error.mat')
    Screen('Preference', 'VisualDebugLevel', oldVisualDebugLevel);
    Screen('Preference', 'SuppressAllWarnings', oldSupressAllWarnings);
    psychrethrow(psychlasterror);
    
end
