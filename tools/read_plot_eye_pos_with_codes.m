function read_plot_eye_pos_with_codes(FILENAME, BLOCKLEN, EXPLEN)
% READ_PLOT_EYE_POS_WITH_CODES(FILENAME, BLOCKLEN, EXPLEN)
%
% Example (for a 4 min. experiment with 20s blocks):
%    read_plot_eye_pos_with_codes('20060609_n010', 20, 240);
%
% Open's eye position data saved in Sam's format and plots
% 4 eye traces. 
% - Blue lines drawn every BLOCKLEN seconds, for EXPLEN seconds
% - Green lines drawn each trial end (usually when fix lost)
% - green dots are rewards
% - red dots are blinks
%
% History: June 2006 - J.Grossmann

fd = fopen(FILENAME,'r','l');
M = fread(fd, 'int32');
rows = size(M,1) / 64;
M = reshape(M, 64, rows)';
cols = [2 3 5 6];
titles={'Horizontal Left Eye (+ right)','Vertical Left Eye (+ up)','Horizontal Right Eye (+ right)','Vertical Right Eye (+ up)'};
time = M(:,1) / 200;
codes = M(:,55);
start = find(codes==16);
time = time - time(start(1)) ;%- BLOCKLEN;
rng = 2.5; %graph y range
%endblocks = find(codes==2);
%reward = find(codes==8);

xtick = 0:BLOCKLEN:EXPLEN; % time hashes on the graph

blinks = [];   
for i = 1:4  % find the blinks by finding where eye pos traces are pegged min or max
    trace = M(:,cols(i));
    blinks = union(blinks,find(trace<0.90*min(trace)));
    blinks = union(find(trace>0.90*max(trace)),blinks);
end
blinks = [blinks-1; blinks; blinks+1];
blinks(blinks==0)=1;
blinks(blinks==max(blinks))=max(blinks)-1;
blinks = unique(blinks);

for i=1:4,
    subplot(4,1,i);
    hold off;
    trace = M(:,cols(i));
    trace = trace - median(trace(:)); % baseline correction
    trace(blinks) = NaN;
    plot(time, trace/100,'k');
    hold on
%      EYE{i} = [time';trace'];
    h=plot([0 0],[-rng rng],'b');
    set(h,'LineWidth',1);
%    plot([time(codes==2)';time(codes==2)'],[-10000; 10000],'g'); % trial starts
    h=plot([xtick;xtick],[-rng; rng],'b');  % block boundaries
    set(h,'LineWidth',1);
    plot(time(codes==8),rng/3,'g.');   % rewards
    plot(time(blinks),rng/2,'r.');   % blinks
    set(gca,'ytick',[-rng 0 rng] );
    set(gca,'xtick',xtick);
    title(titles{i});
    axis([time(1) time(end) -rng rng]  );
end;
save /tmp/eye_out.mat
return;
