function desmat = frames2ev3(infile,outfile,dummytime,normalize,duration)
% converts output of the 1D noise expt to fsl ev3 format
% use: out = frames2ev3(infile,[outfile],[normalize])
% example: frames2ev3('runA292sec','out.txt',3,1,292)
% infile is saved design data from OneDNoise expt
% outfile is the name of the output file
% desmat is the design as a 3 column matrix
%  column 1 is the onsets, column 2 is the durations, column 3 is the
%  value of the stimulus
% dummytime is used to account for dummy scans (in seconds)
%  positive means that dummy scans were running during stimulus
%  presentation
% normalize scales all values to 1
% duration in seconds of stimulus vector

%% check args
switch nargin
    case 1
        outfile = 'temp.txt';                       % default args
        dummytime = 0;
        normalize = 1;
        duration = 292;
    case 2
        normalize = 1;
        dummytime = 0;
        duration = 292;
    case 3
        normalize = 1;
        duration = 292;
    otherwise
        duration = 292;
end

%% time
load(infile);

% d = tf  - t0; why does it think tf is a trasnfer function???
d = duration;
NumFrames = size(FrameNum,2);
dt = d/NumFrames;
timevector = dt*(1:NumFrames) - dummytime;

%% find onsets and offsets
DFrameNum = FrameNum - circshift(FrameNum,[0 1]);   % find where stimulus changes
onsets = DFrameNum;
onsets(onsets<0) = 0;                               % stim on indexes
offsets = DFrameNum;
offsets(offsets>0) = 0;                             % stim off indexes
offsets = abs(offsets);
onsettimes=timevector(onsets>0);                    % find time of index
offsettimes=timevector(offsets>0);
values=onsets(onsets>0);                            % get value at indexes
durations=offsettimes-onsettimes;
if normalize == 1
    values(values>1) = 1;
end

%% output
desmat = [onsettimes ; durations ; values ]';
dlmwrite(outfile,desmat,'delimiter', '\t');
