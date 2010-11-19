function visangles

fprintf('\n*** %s \ncalc vis angles for Demets expts\n\n',mfilename)

%% settings 10 Nov 10
% dist in cm unless otherwise noted
% eyeToMirror = 6.5;
% mirrorToScreen = 51.0;
% screenWidth = 26.5;
% screenHeight = 19;
% retinotopyDiameter = screenHeight;

%% settings 19 Nov 10
% dist in cm unless otherwise noted
eyeToMirror = 10.0;

mirrorToScreen = 50;
screenWidthLoc = 28;
screenHeightLoc = 18.5;

screenWidthExp = 26.5;
screenHeightExp = 8;

retinotopyDiameter = screenHeightLoc;

%% calculate
screenDistLoc = eyeToMirror + mirrorToScreen;
screenRatioLoc = screenHeightLoc / screenWidthLoc; % shouldnt this be 0.75??
screenFOVDegLoc = atand(screenWidthLoc/screenDistLoc);
retinotopyDiameterDeg = atand(retinotopyDiameter/screenDistLoc);

screenDistExp = eyeToMirror + mirrorToScreen;
screenRatioExp = screenHeightExp / screenWidthExp; 
screenFOVDegExp = atand(screenWidthExp/screenDistExp);

%% show results
fprintf('localizer screen ratio = %0.2f \n',screenRatioLoc)
fprintf('localizer screen distance = %0.2f cm\n',screenDistLoc)
fprintf('localizer screen FOV = %0.2f degrees (2 * %f)\n',screenFOVDegLoc,screenFOVDegLoc/2)
fprintf('retinotopy diameter = %0.2f degrees (2 * %f)\n',retinotopyDiameterDeg,retinotopyDiameterDeg/2)
fprintf('\n')
fprintf('expt screen ratio = %0.2f \n',screenRatioExp)
fprintf('expt screen distance = %0.2f cm\n',screenDistExp)
fprintf('expt screen FOV = %0.2f degrees (2 * %f)\n',screenFOVDegExp,screenFOVDegExp/2)



