function em
% 
% There are many ways to create animations.  The simplest is to show a
% movie, computing all the frames in advance, as in MovieDemo.  Sprites may
% be a better approach when you want to show a relatively small object
% moving unpredictably.  Here we generate one offscreen window holding the
% sprite image and copy it to the screen for each frame of the animation,
% specifying the screen location by using the destination rect argument of
% Screen 'CopyWindow'.
% 
% See also MovieDemo.
% 
% Thanks to tj <thomasjerde@hotmail.com> for asking how. 
% web http://groups.yahoo.com/group/psychtoolbox/message/1101 ;
%
% 6/20/02 awi  Wrote it as TargetDemo.  
% 6/20/02 dgp  Cosmetic.  Renamed SpriteDemo.
% 8/25/06 rhh  Added noise to the sprite.  Expanded comments.
% 10/14/06 dhb Save and restore altered prefs, more extensive comments for them
% 09/20/09 mk  Improve screenNumber selection as per suggestion of Peter April.
% 02/02/10 msb modifying for spem experiments

%% ------ Parameters ------

% visStimType = 'cyclopean'; % cyclopean or solid
visStimType = 'solid';

if strcmp(visStimType,'cyclopean')
    numberOfSpriteFrames = 16; % The number of animation frames for our sprite
    
else
    numberOfSpriteFrames = 1; % The number of animation frames for our sprite
end
 
bkgSpriteSizeDeg = 25;
bkgDispDeg = 1;

screenWidthInCm = 27.2;
screenDistInCm = 42;
spriteSizeDeg = 1; % sprite size in degrees.
crossThicknessDeg = 0.25;

% spem parameters
averageDegPerSec = 16.7;
amplitudeXdeg = 14; % spem amplitude

% saccade parameters
saccadePeriod = 0.75; % time between saccades
saccadeLocationsDeg = [-10,-5,0,5,10];
saccadeLocationIdxs = [
     4
     3
     2
     5
     4
     3
     1
     2
     5
     1
     5
     4
     2
     3
     1
     1
     1
     2
     4
     5
     3
     5
     2
     4
     3];
numSaccadeLocations = max(size(saccadeLocationIdxs));

blockDuration = 10;
% blocks = {'sacc','spem','vergtr','vergst','fix'};
blocks = {'sacc','spem'};
% blocks = {'vergtr','vergst'};
blockRepeats = 2;
%%
try

    %% ------ Screen and Color Setup ------

    % Choose a screen
    screenNumber = max(Screen('Screens'));
    [widthScreen, heightScreen]=Screen('WindowSize', screenNumber);
    widthScreen = widthScreen / 2; % using dualhead2go for stereo so screen is folded over on itself
    pixelWidth = screenWidthInCm / widthScreen;
    degPerPixel = atand(pixelWidth/screenDistInCm);
    spriteSize = round(spriteSizeDeg/degPerPixel); % The height and width of the sprite in pixels (the sprite is square)
    bkgSpriteSize = round(bkgSpriteSizeDeg / degPerPixel);
    crossThickness = round(crossThicknessDeg/degPerPixel);   
    amplitudeX = amplitudeXdeg / degPerPixel;
    Period = 2 * amplitudeXdeg / averageDegPerSec ;
    saccadeLocations = saccadeLocationsDeg / degPerPixel;
    
    bkgDisp = round(bkgDispDeg / degPerPixel);

    
    % set colors
    whiteColor = 255;
    greyColor = 128;
    blackColor = 0;
%     redColor = [255,0,0];

    
	%% Screen is able to do a lot of configuration and performance checks on
	% open, and will print out a fair amount of detailed information when
	% it does.  These commands supress that checking behavior and just let
    % the demo go straight into action.  See ScreenTest for an example of
    % how to do detailed checking.
	oldVisualDebugLevel = Screen('Preference', 'VisualDebugLevel', 3);
    oldSupressAllWarnings = Screen('Preference', 'SuppressAllWarnings', 1);
    
    % Open a window and paint the background white
    window = Screen('OpenWindow', screenNumber, blackColor);

    % Hide the mouse cursor.
    HideCursor;

    %make a cross
    crossMask = ones(spriteSize);
    maskIndex1 = floor(((spriteSize-crossThickness)/2));
    maskIndex2 = ceil(((spriteSize+crossThickness)/2));
    crossMask(1:maskIndex1,1:maskIndex1)=0;
    crossMask(maskIndex2:end,maskIndex2:end)=0;
    crossMask(1:maskIndex1,maskIndex2:end)=0;
    crossMask(maskIndex2:end,1:maskIndex1)=0;
    
    % ------ Animation Setup ------
    for idx = 1 : numberOfSpriteFrames
        % Create the frames for the animated sprite.  Here the animation
        % consists of noise or a cross
        if strcmp(visStimType,'cyclopean')
            spriteSize = floor(spriteSize);
            bkgSpriteSize = floor(bkgSpriteSize);
            spriteFrame(idx) = Screen('MakeTexture', window, whiteColor * rand(spriteSize)); %#ok<AGROW>
            bkgFrame(idx)    = Screen('MakeTexture', window, whiteColor * rand(bkgSpriteSize)); %#ok<AGROW>
            saccadeLocations = round(saccadeLocations);
        else
            spriteFrame(idx) = Screen('MakeTexture', window, crossMask .* greyColor); %#ok<AGROW>
        end
    end
    % ------ Bookkeeping Variables ------
    
    spriteRect = [0 0 spriteSize spriteSize]; % The bounding box for our animated sprite
    bkgSpriteRect = [0 0 bkgSpriteSize bkgSpriteSize];
    spriteFrameIndex = 1; % Which frame of the animation should we show?
    buttons = 0; % When the user clicks the mouse, 'buttons' becomes nonzero.
    centerX = widthScreen/2; % The x-coordinate of the mouse cursor
    centerY = heightScreen/2; % The y-coordinate of the mouse cursor   
%     mX = centerX; % The x-coordinate of the mouse cursor
%     mY = centerY; % The y-coordinate of the mouse cursor

    saccadeLocationIdx = 0;
    lastSaccadeTime = 0;
    % wait for trigger
    while ~any(buttons)
        Screen('DrawText', window, 'Click to start', 0, 0, greyColor);
        Screen('Flip', window);
        [~, ~, buttons] = GetMouse;
    
    end
    for blockRepeat = 1:blockRepeats
        for block = blocks
            stimulusType = block{1};
            % start block fixation for 500ms
            startTime = GetSecs;
            endTime = 0.5 + startTime;
            while GetSecs < endTime
                mX = centerX;
                mXr = mX + widthScreen;
                mY = centerY;
                % Draw the sprite at the new location.
                Screen('DrawTexture', window, spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect, mX, mY));
                Screen('DrawTexture', window, spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect, mXr, mY));
                % Call Screen('Flip') to update the screen.  Note that calling
                % 'Flip' after we have both erased and redrawn the sprite prevents
                % the sprite from flickering.
                Screen('Flip', window);
            end
            % stimulus block
            startTime = GetSecs;
            endTime = blockDuration + startTime;
            spriteFrameT = 1;
            while GetSecs < endTime
                spriteFrameT = spriteFrameT + 1;
                if spriteFrameT > numberOfSpriteFrames
                    spriteFrameT = 1;
                end
                spriteFrameIndex = int16(floor(spriteFrameT));
                switch stimulusType
                    case 'spem'
                        mX = centerX + amplitudeX * sin(2*pi*GetSecs/Period);
                        mXr = mX + widthScreen;
                    case 'sacc'
                        if GetSecs > lastSaccadeTime
                            saccadeLocationIdx = 1 + mod(saccadeLocationIdx,numSaccadeLocations);
                            lastSaccadeTime = GetSecs + saccadePeriod;
                        end
                        mX = centerX + saccadeLocations(saccadeLocationIdxs(saccadeLocationIdx));
                        mXr = mX + widthScreen;
                    case 'vergtr'
                        mX = centerX + (amplitudeX/5) * sin(2*pi*GetSecs/Period);
                        mXr = -mX + 2*centerX + widthScreen;
                    case 'vergst'
                        if GetSecs > lastSaccadeTime
                            saccadeLocationIdx = 1 + mod(saccadeLocationIdx,numSaccadeLocations);
                            lastSaccadeTime = GetSecs + saccadePeriod;
                        end
                        mX = centerX + saccadeLocations(saccadeLocationIdxs(saccadeLocationIdx))/5;
                        mXr = -mX + 2*centerX + widthScreen;
                    otherwise % default to fixation
                        mX = centerX;
                        mXr = mX + widthScreen;
                end
                mY = centerY;
                % Draw the sprite at the new location.
                if strcmp(visStimType,'cyclopean')
                    Screen('DrawTexture', window, bkgFrame(spriteFrameIndex), bkgSpriteRect, CenterRectOnPoint(bkgSpriteRect, centerX + bkgDisp, centerY));
                    Screen('DrawTexture', window, bkgFrame(spriteFrameIndex), bkgSpriteRect, CenterRectOnPoint(bkgSpriteRect, centerX + widthScreen - bkgDisp, centerY));
                end
                Screen('DrawTexture', window, spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect, mX, mY));
                Screen('DrawTexture', window, spriteFrame(spriteFrameIndex), spriteRect, CenterRectOnPoint(spriteRect, mXr, mY));
                % Call Screen('Flip') to update the screen.  Note that calling
                % 'Flip' after we have both erased and redrawn the sprite prevents
                % the sprite from flickering.
                Screen('Flip', window);
            end
        end
    end
    % Revive the mouse cursor.
    ShowCursor; 
    
    % Close screen
    Screen('CloseAll');
    
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
