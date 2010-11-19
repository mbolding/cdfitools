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
    prompt = {'Enter tracker EDF file name (1 to 8 letters or numbers)'};
    dlg_title = 'Create EDF file';
    num_lines= 1;
    def     = {'DEMO'};
%     answer  = inputdlg(prompt,dlg_title,num_lines,def);
    edfFile = 'DATA';
%     edfFile = answer{1};
    
    %% configure eyelink graphics
    resolution = NearestResolution(screenNumber, widthScreen, heightScreen, 60);
    oldResolution = SetResolution(screenNumber, resolution);
    window(1) = Screen('OpenWindow', screenNumber, [0 0 0]);
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
    Eyelink('command', 'calibration_type = H3');
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
    disp('calibrate eyelink now.')
    EyelinkDoTrackerSetup(el);
    %         SetResolution(screenNumber, oldResolution);
    Screen('CloseAll');
else
    fprintf('EyeLink connection not used.\n');
end;
