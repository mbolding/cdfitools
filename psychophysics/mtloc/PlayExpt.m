function PlayExpt(moviename,itermax,useDT)
%
% PlayExpt(moviename,itermax)
%
% This demo accepts a pattern for a valid moviename, e.g.,
% moviename='*.mpg', then it plays all movies in the current working
% directory whose names match the provided pattern, e.g., the '*.mpg'
% pattern would play all MPEG files in the current directory.
%
% This demo uses automatic asynchronous playback for synchronized playback
% of video and sound. Each movie plays until end, then rewinds and plays
% again from the start. SPACE jumps to the
% next movie in the list until itermax is reached. ESC ends the demo.
%
% This demo needs MacOS-X 10.3.9 or greater with Quicktime-7 installed!
% 
% There is also an option to use DAQtoolbox to blank the stimulus.
% Wire port one, pin one of the USB-1208 to a TTL blanking signal.
% High input (about +5v) blanks and low (gnd) shows. 
% Set useDT to 1 to use the blanking signal. 

% History:
% 10/30/05  mk  Wrote it.
% 19 Feb 09 msb modifying for own fMRI experiments

if nargin < 1
    moviename = '*.mov'; % default all get all movies in dir
end;
moviefiles = dir(moviename);
moviecount = size(moviefiles,1);

if nargin < 2
    itermax = 1; % default play 1 movie.
end;

if nargin < 3
    useDT = 0; % 1 = use; 0 = do not use DAQtoolbox for stimulus blank
end

if useDT == 1
    clear PsychHID;
    daqID=DaqDeviceIndex([],0);
end

% Switch KbName into unified mode: It will use the names of the OS-X
% platform on all platforms in order to make this script portable:
KbName('UnifyKeyNames');

space=KbName('SPACE');
esc=KbName('ESCAPE');
% right=KbName('RightArrow');
% left=KbName('LeftArrow');
% up=KbName('UpArrow');
% down=KbName('DownArrow');
% shift=KbName('RightShift');
bkey=KbName('b');

try
    % Child protection
    AssertOpenGL;
    background=[0,0,0]; % black

    % Open onscreen window:
    screen=max(Screen('Screens'));
    [win, scr_rect] = Screen('OpenWindow', screen);
    disp(['scr_rect:' num2str(scr_rect)])
    
    % fixation sprite
    Screen('BlendFunction',win,'GL_SRC_ALPHA','GL_ONE_MINUS_SRC_ALPHA'); % for transparency   
    spriteColor = [255 255 0 255]; % yellow, opaque, RGBA
    spriteMask = [...
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    1 1 1 1 1 1 1 1 1 1 1 1 1 ;
    1 1 1 1 1 1 1 1 1 1 1 1 1 ;
    1 1 1 1 1 1 1 1 1 1 1 1 1 ;
    1 1 1 1 1 1 1 1 1 1 1 1 1 ;
    1 1 1 1 1 1 1 1 1 1 1 1 1 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ;
    0 0 0 0 1 1 1 1 1 0 0 0 0 ...
    ];
    spriteImage = repmat(spriteMask,[1 1 4]) .* ...
        repmat(reshape(spriteColor,1,1,4),size(spriteMask));
    spriteSize = max(size(spriteMask)); % combine mask and RGBA
    spriteTexture = Screen('MakeTexture', win, spriteImage);
    spriteRect = [-1 -1 2 2] * spriteSize; % double the size
    [spriteX,spriteY] =  RectCenter( scr_rect ); % find the screen center
    

    % Retrieve duration of a single video refresh interval:
    ifi = Screen('GetFlipInterval', win);
    disp(['ifi:' num2str(ifi)])

    
    % Clear screen to background color:
    Screen('FillRect', win, background);
    HideCursor;    
    abortit=0;
    blankscreen=0;
    
    % initial movie load.
    iteration=1;
    fprintf('ITER=%i::', iteration);
    moviename=moviefiles(mod(iteration, moviecount)+1).name;

    % Open movie file and retrieve basic info about movie:
    [movie movieduration fps imgw imgh] = Screen('OpenMovie', win, moviename);
    fprintf('Movie: %s  : %f seconds duration, %f fps, w x h = %i x %i...\n', moviename, movieduration, fps, imgw, imgh);
    
    % ready prompt, add magnet sync code here.
    tsize=20;
    Screen('TextSize', win, tsize);
    Screen('TextColor',win, [128 128 128]);
    Screen('DrawText', win, 'hit a key to start.',10, 10);    
    % Flip to show the ready screen:
    Screen('Flip',win);
    KbWait([],3);
    
    % Initial display and sync to timestamp:
    vbl=Screen('Flip',win);
    disp(['vbl:' num2str(vbl)])

    
    % Playbackrate defaults to 1:
    rate=1;
    
    % Endless loop, runs until ESC key pressed or max iterations:
    while (abortit<2 && iteration < itermax+1)
        
        i=0;
    
        % Seek to start of movie (timeindex 0):
        Screen('SetMovieTimeIndex', movie, 0);
        
        % Start playback of movie. This will start
        % the realtime playback clock and playback of audio tracks, if any.
        % Play 'movie', at a playbackrate = 1, with endless loop=1 and
        % 1.0 == 100% audio volume.
        Screen('PlayMovie', movie, rate, 1, 1.0);
    
        t1 = GetSecs;
        
        % Infinite playback loop: Fetch video frames and display them...
        while(1)
            i=i+1;
            if (abs(rate)>0)
                if blankscreen == 0
                    % Return next frame in movie, in sync with current playback
                    % time and sound.
                    % tex either the texture handle or zero if no new frame is
                    % ready yet. pts = Presentation timestamp in seconds.
                    tex = Screen('GetMovieImage', win, movie, 1, [], [], 1);
                    
                    % Valid texture returned?
                    if tex<=0
                        break;
                    end;
                  
                    % Draw the new texture and fixation immediately to screen:
                    Screen('DrawTexture', win, tex);
                    Screen('DrawTexture', win, spriteTexture, spriteRect, ...
                        CenterRectOnPoint(spriteRect, spriteX, spriteY));
                    
                    % Update display:
                    Screen('Flip', win);
                    
                    % Release texture:
                    Screen('Close', tex);
                else
                    % blank stimulus and leave the fixation.
                    Screen('FillRect', win, background);
                    Screen('DrawTexture', win, spriteTexture, spriteRect, ...
                        CenterRectOnPoint(spriteRect, spriteX, spriteY));
                    % Update display:
                    Screen('Flip', win);
                    blankscreen=0;
                end

            end;
            
            % Check for abort:
            abortit=0;
            [keyIsDown,secs,keyCode]=KbCheck;
            if (keyIsDown==1 && keyCode(esc))
                % Set the abort-demo flag.
                abortit=2;
                break;
            end;
            
            if (keyIsDown==1 && keyCode(space))
                % Exit while-loop: This will load the next movie...
                break;
            end;
            
            if (keyIsDown==1 && keyCode(bkey))
                blankscreen=1;
            end;
            
            if useDT == 1
                % check port 1 line 1 here for stimulus blanking
                Dlines=DaqDIn(daqID,2);
                if bitand(1,uint8(Dlines(1)))
                    blankscreen=1;
                end
            end
        end
    
        telapsed = GetSecs - t1;
        disp(['telapsed:' num2str(telapsed)])

        finalcount=i;
        disp(['finalcount:' num2str(finalcount)])

        Screen('Flip', win);
        while KbCheck; end; % wait for key up.
        
        % Done. Stop playback:
        Screen('PlayMovie', movie, 0);

        % Close movie object:
        Screen('CloseMovie', movie);
        
        iteration=iteration + 1;
        fprintf('ITER=%i::', iteration);
        moviename=moviefiles(mod(iteration, moviecount)+1).name;
        
        % Open movie file and retrieve basic info about movie:
        [movie movieduration fps imgw imgh] = Screen('OpenMovie', win, moviename);
        fprintf('Movie: %s  : %f seconds duration, %f fps, w x h = %i x %i...\n', moviename, movieduration, fps, imgw, imgh);

    end;
    
    
    ShowCursor;
    Screen('DrawText', win, 'done. hit a key to end.',10, 10);    
    % Flip to show the last screen:
    Screen('Flip',win);
    KbWait([],3);
    % Close screens.
    Screen('CloseAll');

    % Done.
catch %#ok<CTCH>
    % Error handling: Close all windows and movies, release all ressources.
    Screen('CloseAll');
    psychrethrow(psychlasterror);
end;
