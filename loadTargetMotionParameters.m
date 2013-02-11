% target size
spriteSizeDeg = 1; % sprite size in degrees.
crossThicknessDeg = 0.25;

% spem parameters
averageDegPerSec = 16;
% averageDegPerSec = 4;
amplitudeXdeg = 12; % horizontal spem amplitude

% saccade parameters
saccadePeriod = [118.72 1484 652.96]; % time between saccades
vergencePeriod = 2.0; 
%saccadevLocationsDeg = [-10,-5,0,5,10];

saccadeLocationsDeg = [-10,20];

% saccadeLocationIdxs = [ 4 3 2 5 4 3 1 2 5 1 5 4 2 3 1 3 1 2 4 5 3 5 2 4 3]';
%saccadeLocationIdxs = [ 5 3 2 1 3 2 1 2 3 5 4 2 3 1 3 1 2 4 5 3 5 4 2 3]';

saccadeLocationIdxs = [1 2 1]';

%saccadeLocationIdxs = [ saccadeLocationIdxs ; (3 - saccadeLocationIdxs)];

% saccadeLocationIdxs = [ 3 1 3 5 3 2 3 4 3 2 3 5 3 1 3 4 3 1 3 4 3 2 3 5 ]';
% saccadeLocationIdxs = [ 3 4 5 4 3 2 1 2 3 4 5 4 ]';

numSaccadeLocations = max(size(saccadeLocationIdxs));

% vergence center pt
% pedXdeg = 0.2;
pedXdeg = -0.1;
% vergenceDivisor = 16;
vergenceDivisor = 16;
vergenceVelDivisor = 4;
% vergenceDivisor = 32;

%attention parameters
%attnDimFraction = 20; % units of target intensity to subtract, 8bit, 0-255

attnDimFraction = 0; % units of target intensity to subtract, 8bit, 0-255


attnMinDelay = 2; % minimum period between dimming stims in secs
attnDelayRange = 3; % range of dimming stims in secs
attnDimLength = 0.1; %  how long to dim the target in seconds
attnResponseWindow = 1; % how long to wait for a valid response in seconds
