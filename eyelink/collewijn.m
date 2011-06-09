% function trials = collewijn(trials)
if ~exist('trials','var')
    [filename,pathname] = uigetfile;
    load([pathname filename])
end
% view trials
numTrials = size(trials,2);
for i=1:numTrials
    
    figure(i)
    numSac_L=size(trials(i).sac_L,1);
    if numSac_L > 0
        a=mean(trials(i).sac_L,1);
        meanDur_L=a(3);
        meanAmp_L=a(8);
        meanVel_L=a(9);
    else
        trials(i).sac_L = [0 0 0 0 0 0 0 0 0];
        meanDur_L=0;
        meanAmp_L=0;
        meanVel_L=0;
    end
    numSac_R=size(trials(i).sac_R,1);
    if numSac_R > 0
        a=mean(trials(i).sac_R,1);
        meanDur_R=a(3);
        meanAmp_R=a(8);
        meanVel_R=a(9);
    else
        trials(i).sac_R = [0 0 0 0 0 0 0 0 0];
        meanDur_R=a(3);
        meanAmp_R=a(8);
        meanVel_R=a(9);
    end
    %% decomp into fast and slow
    
    subplot(2,2,1)
    % figure(6)
%     eye_nan = [0; diff( isnan(trials(i).eye(:,2)) )];
%     eye_nan_start = zeros(size(eye_nan));
%     eye_nan_end = eye_nan_start;
%     eye_nan_start(eye_nan == 1) = 1; 
%     eye_nan_end(eye_nan == -1) = 1;
%     eye_nan_start_idx = find(eye_nan_start);
%     eye_nan_end_idx = find(eye_nan_end);
    
    eye_vel = [trials(i).eye(1,2) ; diff( trials(i).eye(:,2))];
    eye_vel(isnan(eye_vel))=0; 
    
    sacc_on = 1 + trials(i).sac_R(:,1)/(1000/sample_rate); % converts time to index /2 for 500Hz /4 for 250Hz
    sacc_off = 1 + trials(i).sac_R(:,2)/(1000/sample_rate);
    eye_slow = ones(size(eye_vel));
    for sacc_idx = 1:length(sacc_on)
        eye_slow(sacc_on(sacc_idx):sacc_off(sacc_idx)) = 0;
    end
    eye_fast = 1 - eye_slow;
    slow_eye_vel = eye_vel.*eye_slow;
    fast_eye_vel = eye_vel.*eye_fast;
    
    slow_eye_pos = cumsum(slow_eye_vel);
    fast_eye_pos = cumsum(fast_eye_vel);
    eye_pos = cumsum(eye_vel);
   
    
    
    plot(...
        trials(i).eye(:,1),eye_pos,...
        trials(i).eye(:,1),slow_eye_vel,...
        trials(i).eye(:,1),slow_eye_pos,...
        trials(i).eye(:,1),fast_eye_pos,...
        trials(i).target(:,1),trials(i).target(:,2),'k:')
    xlim([0 40000])
    %% plot eyetraces horiz
%     subplot(2,2,1)
%     plot(trials(i).eye(:,1),trials(i).eye(:,2),...
%         'b',trials(i).target(:,1),trials(i).target(:,2),...
%         'k:',trials(i).sac_L(:,1),trials(i).sac_L(:,4),'g*',...
%         trials(i).sac_L(:,2),trials(i).sac_L(:,6),'r*');
%     xlabel('Time (msec)')
%     ylabel('Eye position (pixels)')
%     title('Horizontal Eye Movements');
%     %     legend('Hori Eye Position','Hori Target Position','Start Sac','End Sac');
%     ylim([0 800])
%     ax=axis;
%     if numSac_L>=numSac_R
%         s=['#Sac=' num2str(numSac_L) '; aveDur=' num2str(meanDur_L) ' msec; aveAmp=' num2str(meanAmp_L) ' deg; aveVel=' num2str(meanVel_L) ' deg/sec'];
%     else
%         s=['#Sac=' num2str(numSac_R) '; aveDur=' num2str(meanDur_R) ' msec; aveAmp=' num2str(meanAmp_R) ' deg; aveVel=' num2str(meanVel_R) ' deg/sec'];
%     end
%     %     text(500,ax(4)-20,s);
%     
    %% plot eye traces vertical
    subplot(2,2,2), plot(trials(i).eye(:,1),trials(i).eye(:,3), 'b',trials(i).target(:,1),trials(i).target(:,3),'k:',trials(i).sac_L(:,1),trials(i).sac_L(:,5),'g*',trials(i).sac_L(:,2),trials(i).sac_L(:,7),'r*');
    xlabel('Time (msec)'), ylabel('Eye position (pixels)'), title('Vertical Eye Movements');
    %     legend('Vert Eye Position','Vert Target Position','Start Sac','End Sac');
    ylim([0 600])
    
    %% fourier amplitude
    subplot(2,2,3)
    Fs = sample_rate;
    Fs2 = 120;
    %     T = 1/Fs;
    L = length(trials(i).eye(:,2));
    L2 = length(trials(i).target(:,2));
    L3 = length(slow_eye_pos);
    %     t = trials(i).eye(:,1);
    %     t2 = trials(i).target(:,1);
    %     t = (0:L-1)*T;                % Time vector
%     NFFT = 2^(nextpow2(L)); % Next power of 2 from length of y
%     NFFT2 = 2^(nextpow2(L2)); % Next power of 2 from length of y

%     NFFT = L - 54 ; % 
    NFFT = L2 * ( Fs/Fs2 );
    NFFT2 = L2; % 
    NFFT3 = NFFT;
    
    Y = fft(eye_pos,NFFT)/L;
    Y2 = fft(trials(i).target(:,2),NFFT2)/L2;
    Y3 = fft(slow_eye_pos,NFFT3)/L3;
    
    f = Fs/2*linspace(0,1,NFFT/2+1);
    f2 = Fs2/2*linspace(0,1,NFFT2/2+1);
    f3 = Fs/2*linspace(0,1,NFFT3/2+1);
    
    eye_amp = 2*abs(Y(1:NFFT/2+1));
    targ_amp = 2*abs(Y2(1:NFFT2/2+1));
    slow_eye_amp = 2*abs(Y3(1:NFFT3/2+1));
    
    plot(f,eye_amp,f2,targ_amp,f3,slow_eye_amp)

    title('Single-Sided Amplitude Spectrum of eye(t)')
    xlabel('Frequency (Hz)')
    ylabel('|Y(f)|')
    xlim([0.1 2])
    %     ylim([0 300])
    
    %% gain
    subplot(2,2,4)
    eye_gain = eye_amp(1:length(targ_amp))./targ_amp;
    slow_eye_gain = slow_eye_amp(1:length(targ_amp))./targ_amp;
%     eye_gain(targ_amp < 0) = NaN;
%     slow_eye_gain(targ_amp < 50) = NaN;
    plot(f2,eye_gain,'.',f2,slow_eye_gain,'.')
    ylim([0 1.1]);
    xlim([0.1 2])
    
    %% fourier phase
    %     subplot(2,2,4)
    %     plot(f,rad2deg(unwrap(angle(Y(1:NFFT/2+1)))),'')
    %     hold on
    %     plot(f2,rad2deg(unwrap(angle(Y2(1:NFFT2/2+1)))),'k')
    %     hold off
    %     title('Single-Sided Phase Spectrum of eye(t)')
    %     xlabel('Frequency (Hz)')
    %     ylabel('|Y(f)|')
    %     xlim([0.1 0.8])
    %%
    % PTamp = -108;
    % PTphase = 1.599;
    % PTfreq = 0.00094;
    % PToffset = 399;
    % pseudotarget = PTamp * sin(PTfreq * trials(i).eye(:,1) + PTphase) + PToffset;
    %
    % subplot(2,2,4)
    % comet(trials(i).eye(:,2),pseudotarget)
    %
    % %     ylim([250 559])
    %         pause
    

    
    
end


