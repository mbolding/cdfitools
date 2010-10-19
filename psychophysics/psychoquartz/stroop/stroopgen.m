function stroopgen(filename)
%% stroopgen.m
% .xml script file generator for my stroop.qtz stimulus

%% settings
numTrials = 88; % total number of trials
congruentPadding = 4; % pad the ends with congruent trials
netTrials = numTrials -  2*congruentPadding; % the body of trials that is left
fracCongruent = 0.7; % fraction of these trials that is congruent
netTrialsCongruent = floor(netTrials * fracCongruent); % number of congruent in the main body
netTrialsIncongruent = netTrials - netTrialsCongruent; % what is left is incongruent
congruentFlags = [ ones(1,netTrialsCongruent) zeros(1,netTrialsIncongruent) ]; % 1 if cong, 0 if incong
congruentFlags = congruentFlags(randperm(netTrials)); % mix them up
congruentFlags = [ ones(1,congruentPadding) congruentFlags ones(1,congruentPadding) ]; % add the cong. trials on each end

colorWords = { ...
    'red'...
    'blue'...
    'green'...
    };

numColorWords = size(colorWords,2);
colorList = 1 + mod(1:numTrials,numColorWords); % deal them out evenly
colorList = colorList(randperm(size(colorList,2))); % mix them up

%% define some boilerplate
entryTags.start =    '<entry><word>';
entryTags.middle =   '</word><color>';
entryTags.end =      '</color></entry>';
entryTags.fixation = '<entry><word>+</word><color>white</color></entry>';

%% open the file
% filename = 'stroop_script.xml';
fid = fopen(filename, 'wt');

%% write the file start
P={ '<?xml version="1.0"?>'         ...
    '<data>'                        ...
    '   <entry>'                    ...
    '       <word>get ready</word>' ...
    '       <color>white</color>'   ...
    '   </entry>'                   ...
    '   <entry>'                    ...
    '       <word>3</word>'         ...
    '       <color>white</color>'   ...
    '   </entry>'                   ...
    '   <entry>'                    ...
    '       <word>2</word>'         ...
    '       <color>white</color>'   ...
    '   </entry>'                   ...
    '   <entry>'                    ...
    '       <word>1</word>'         ... 
    '       <color>white</color>'   ...
    '   </entry>'                   ...
    };
printStrings(P,fid);

%% write the body of the file
for k = 1:numTrials % several congruent trials
    colorIdx = colorList(k);
    if congruentFlags(k) == 1
       outString  = makeCongruentTrial(colorIdx,entryTags,colorWords);
    else
       outString  = makeIncongruentTrial(colorIdx,entryTags,colorWords);
    end
    fprintf(fid, '%s\n', outString);
end


%% write the file end
P={                                ...
    '   <entry>'                   ...
    '      <word>the end.</word>'  ...
    '      <color>white</color>'   ...
    '   </entry>'                  ...
    '   <entry>'                   ...
    '      <word>the end.</word>'  ...
    '      <color>white</color>'   ...
    '   </entry>'                  ...
    '   <entry>'                   ...
    '      <word>the end.</word>'  ...
    '      <color>white</color>'   ...
    '   </entry>'                  ...
    '</data>'                      ...
    };
printStrings(P,fid);




%% clean up, show results
fclose(fid);
type(filename)



%% functions
function outString = makeCongruentTrial(idx,entryTags,colorWords)
outString = sprintf('%s %s %s %s %s \n %s' ,...
    entryTags.start, colorWords{idx}, entryTags.middle,...
    colorWords{idx}, entryTags.end, entryTags.fixation);

function outString = makeIncongruentTrial(idx,entryTags,colorWords)
colorIdxs = 1:size(colorWords,2);
otherIdx = setdiff(colorIdxs,idx);
otherIdx = otherIdx(randperm(size(otherIdx,2)));
otherIdx = otherIdx(1);
outString = sprintf('%s %s %s %s %s \n %s' ,...
    entryTags.start, colorWords{idx}, entryTags.middle,...
    colorWords{otherIdx}, entryTags.end, entryTags.fixation);

function printStrings(P,fid)
for k = 1:size(P,2)
    fprintf(fid, '%s\n', P{k});
end




%% notes
% Stimuli consisted of one of three words (RED, GREEN, BLUE) printed 
% in one of three colors. Trials were either congruent or incongruent.
% For all trials, participants were instructed to name the color of 
% the stimulus and to ignore the word. Participants responded with a 
% button press using their right hand, with a choice of three buttons
% representing the three colors. Prior to scanning, subjects received
% a short training to accustom them to the buttons. Trials were 3 seconds 
% long and consisted of a color word for 1.5 s followed by 1.5 s of a 
% fixation cross (+).  An event-related design was used, with 3 runs 
% of 88 trials each. To increase conflict effects, the first and last 
% 4 trials of every block were congruent. The other 80 trials within 
% each block had 70% congruent trials. Among the congruent trials, 
% the incongruent trials were presented in a pseudorandom fashion, 
% with a minimum of 1 and maximum of 5 consecutive stimuli.  As in 
% the original study by Kern et al. (Kerns, Cohen et al., 2004), no 
% jittering of ISI was performed. Rather, the ?jittering? effect was 
% accomplished by using pseudorandom sequences of trial type (Josephs 
% and Henson, 1999). 
