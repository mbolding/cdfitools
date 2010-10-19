function PlayMovies(moviedir,itermax)
%
% PlayMovies(moviedir,itermax)
%
% Pressing ESC exits the script.
%
% By default it will play all movies (*.mov) 
% in the current directory. Optionally you can give it a directory as the first
% argument. In that case it will play all the movies in that directory. The movies
% are played in the order they are returned by 'dir'. Basically alpha sorted. 
% The basic idea is to put all of your movies in one directory then make directories 
% for each experiment and put soft links (ln -s) in the experiment directories. 
% for example in the directory 'test':
% 
%     dlpfc:test mbolding$ ls -l
%     lrwxr-xr-x  1 mbolding  staff  31 Feb 18 16:40 1.mov -> ../movies/flow_static_30fps.mov
%     lrwxr-xr-x  1 mbolding  staff  32 Feb 18 16:40 2.mov -> ../movies/flow_staticr_30fps.mov
%     lrwxr-xr-x  1 mbolding  staff  28 Feb 18 16:40 3.mov -> ../movies/flow_exp_30fps.mov
%     lrwxr-xr-x  1 mbolding  staff  33 Feb 18 16:40 4.mov -> ../movies/flow_contract_30fps.mov
% 
% Then run PlayMovies('test') from the parent directory. Hit the spacebar when 
% the 'ready' prompt appears. The movies are played forever or until the optional
% iteration count (itermax) is reached.
%
% Movies are played one after each other. We try to minimize perceptible
% gaps between end of movie i and start of movie i+1 by asynchronous
% loading: While movie i is played back, we ask Psychtoolbox to load the
% next movie i+1 in the background, so startup time for movie i+1 will be
% minimized. This feature is currently only supported on MacOS-X and it
% doesn't work well with movies that have sound tracks. The reason for this
% is currently unknown, but it leads to frequent lockups of the playback
% engine. How well this feature works depends strongly on the computational
% horsepower of your machine. Slow machines will probably stutter...
%
% This demo needs MacOS-X 10.3.9 or 10.4.x with Quicktime-7 installed!

% History:
% 7/5/06  mk  Wrote it.
% 18 Feb 09 msb Modify for MT localizer experiments.

% Async movie loading is only OS-X for now...
AssertOSX;

if nargin < 1
    moviedir = '.';
end;

if nargin < 2
    itermax = 1e6;
end;

% Switch KbName into unified mode: It will use the names of the OS-X
% platform on all platforms in order to make this script portable:
KbName('UnifyKeyNames');
esc=KbName('ESCAPE');

% choose which movies to play
movieglob = '*.mov'; % play all quicktime movies in dir
olddir = pwd; % save starting directory
cd(moviedir); % jump to directory with the movies
% Return full list of movie files from directory+pattern:
moviefiles=dir(movieglob);

try
    % Child protection
    AssertOpenGL;
    background=[0, 0, 0];

    % Open onscreen window:
    screen=max(Screen('Screens'));
    HideCursor;
    [win, scr_rect] = Screen('OpenWindow', screen, background);
    % fixation sprite
    Screen('BlendFunction',win,'GL_SRC_ALPHA','GL_ONE_MINUS_SRC_ALPHA');    
    spriteColor = [255 255 0 255];
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
    spriteSize = max(size(spriteMask));
    spriteTexture = Screen('MakeTexture', win, spriteImage);
    spriteRect = [-1 -1 2 2] * spriteSize;
    [spriteX,spriteY] =  RectCenter( scr_rect );
    
    % Retrieve duration of a single video refresh interval:
    Screen('GetFlipInterval', win);
    
    % Initial display and sync to timestamp:
    Screen('Flip',win);   
    abortit=0;
    
    % Playbackrate defaults to 1:
    rate=1;
    
    % Load first movie. This is a synchronous (blocking) load:
    iteration = 0;
    moviename=moviefiles(mod(iteration, size(moviefiles,1))+1).name;
    [movie movieduration fps imgw imgh] = Screen('OpenMovie', win, moviename);

    % ready prompt
    tsize=20;
    Screen('TextSize', win, tsize);
    Screen('TextColor',win, [128 128 128]);
    Screen('DrawText', win, 'ready',40, 100);    
    % Flip to show the startup screen:
    Screen('Flip',win);
    pause
    
    % Start playback of movie. This will start
    % the realtime playback clock and playback of audio tracks, if any.
    % Play 'movie', at a playbackrate = 1, with 1.0 == 100% audio volume.
    fprintf('starting...\n');
    Screen('PlayMovie', movie, rate, 0, 1.0);
    prefetched=0;
    newmovie = -1;

    % Endless loop, runs until ESC key pressed:
    while (abortit<2 && iteration < itermax)
        % Show basic info about movie:
        fprintf('ITER=%03i::', iteration);
        fprintf('Movie: %s  : %f seconds duration, %f fps, w x h = %i x %i...\n', moviename, movieduration, fps, imgw, imgh);

        i=0;

        % Get moviename of next file:
        iteration=iteration + 1;
        moviename=moviefiles(mod(iteration, size(moviefiles,1))+1).name;

%         t1 = GetSecs;

        % Playback loop: Fetch video frames and display them...
        while(1)
            i=i+1;
            if (abs(rate)>0)
                % Return next frame in movie, in sync with current playback
                % time and sound.
                % tex either the texture handle or zero if no new frame is
                % ready yet. pts = Presentation timestamp in seconds.
                [tex pts] = Screen('GetMovieImage', win, movie, 1);
                
                % Valid texture returned?
                if tex<0
                    % No. This means that the end of this movie is reached.
                    % We exit the loop and prepare next movie:
                    break;
                end;

                if (tex>0)
                    % Yes. Draw the new texture immediately to screen:
                    Screen('DrawTexture', win, tex);
                    Screen('DrawTexture', win, spriteTexture, spriteRect, ...
                        CenterRectOnPoint(spriteRect, spriteX, spriteY));
                    % Update display:
                    Screen('Flip', win);

                    % Release texture:
                    Screen('Close', tex);
                end;
            end;

            % Check for abortion by user:
            abortit=0;
            [keyIsDown,secs,keyCode]=KbCheck;
            if (keyIsDown==1 && keyCode(esc))
                % Set the abort-demo flag.
                abortit=2;
                break;
            end;

            % We start background loading of the next movie 0.5 seconds
            % after start of playback of the current movie:
            if prefetched==0 && pts > 0.5
                % Initiate background async load operation:
                % We simply set the async flag to 1 and don't query any
                % return values:
                Screen('OpenMovie', win, moviename, 1);
                prefetched=1;
            end;

            % If asynchronous load of next movie has been started already
            % and we are less than 0.5 seconds from the end of the current
            % movie, then we try to finish the async load operation and
            % start playback of the new movie in order to give processing a
            % headstart:
            if prefetched==1 && movieduration - pts < 0.5
                % Less than 0.5 seconds until end of current movie. Try to
                % start playback for next movie:
                
                % Open the movie - this should take zero-time as the movie
                % should have been already prefetched during playback of
                % the current movie. In case loading did not finish, this
                % command will pause Matlabs execution until the movie is
                % really ready.
                [newmovie newmovieduration newfps newimgw newimgh] = ...
                    Screen('OpenMovie', win, moviename);
                
                % Start it:
                Screen('PlayMovie', newmovie, rate, 0, 1.0);
                
                prefetched=2;
            end;
        end;
    
%         telapsed = GetSecs - t1;
%         finalcount = i;
        
        % Done with old movie. Stop its playback:
        Screen('PlayMovie', movie, 0);

        % Close movie object:
        Screen('CloseMovie', movie);
        
        % Ok, now our 'newmovie' becomes the current 'movie':
        movie = newmovie;
        movieduration = newmovieduration;
        fps = newfps;
        imgw = newimgw;
        imgh = newimgh;
        prefetched = 0;
        % As playback of the new movie has been started already, we can
        % simply reenter the playback loop:
    end;
    
    % End of movie playback. Shut down and exit:
    if prefetched == 1
        % A prefetch operation for a movie is still in progress. We need to
        % finalize this cleanly by waiting for the movie to open and then
        % closing it.
%         [newmovie newmovieduration newfps newimgw newimgh] = Screen('OpenMovie', win, moviename);
        newmovie = Screen('OpenMovie', win, moviename);
        prefetched = 2;
    end;
    
    if prefetched == 2
        % Playback of a new prefetched movie has been started. We need to stop and
        % close it:
        Screen('PlayMovie', newmovie, 0);
        Screen('CloseMovie', newmovie);
%         prefetched = 0;
    end;
    
    % Close screens.
    ShowCursor;
    Screen('DrawText', win, 'done',40, 100);    
    % Flip to show the startup screen:
    Screen('Flip',win);
    pause % wait for key before clearing screen.

    Screen('CloseAll');
    % Done.
%     return;
    cd(olddir); % jump back to starting directory
catch %#ok<CTCH>
    % Error handling: Close all windows and movies, release all ressources.
    Screen('CloseAll');
    psychrethrow(psychlasterror);
    cd(olddir)
end;



