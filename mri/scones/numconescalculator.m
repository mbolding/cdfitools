% function numcones = nonisocones(RES,FOV)
clear all
FOV=[24 24]; % [xy z] in millimeters
RES=[1 2]; % [xy z] in millimeters
theta = 0; %initial value of theta, which is the angle of a given cone
thetasave(1) = theta;
idx = 1; % idx=index
len=256; %length of readout in # of samples

figure; 
hold on;
while theta < pi/2
    
    fovtheta(idx) = 1/(intlineellipse(1/(FOV(2)),1/(FOV(1)),theta)); %calculate fov for a given cone angle, which gives you the 1/fov distance to the next cone at the periphery of the ellipsoid so as to not violate nyquist. note: the initial 1/fov value, when starting at theta=0, is FOV(z) (i.e FOV(2)) because it is perpendicular to RES(xy) (i.e. RES(1))
    restheta(idx) = 1/(intlineellipse(1/(RES(1)),1/(RES(2)),theta)); % calculate res for a given cone angle. this is the radial distance from the center of kspace to an ellipsoid  with total width 1/RES(xy) and total height 1/RES(z) (center to periphery (i.e. kmax) is 1/(2*RES)
    kmaxtheta(idx)=1/(2*restheta(idx)); % calculate kmax for a given cone angle from the res at that cone angle
    thetastep = atan(1/fovtheta(idx)/kmaxtheta(idx)); % calculate the angular increment to the next cone such that the k space distance at the periphery will be 1/fov for that cone angle
    %xtemp=restheta(idx)*cos(theta);
    %ytemp=restheta(idx)*sin(theta);
    %plot(xtemp,ytemp,'.')

    idx = idx + 1;
    theta = theta + thetastep;
    thetasave(idx) = theta;
    
end

theta = thetasave(1:end-1); % thetasave is incremented until it exceeds pi/2 therefore the last value should be excluded. Note: above theta was a single value that changed with each iteration of the while loop, but here it is re-defined as an array
x = cos(theta) .* kmaxtheta;
y = sin(theta) .* kmaxtheta;
%hold off
plot(x,y,'.')
%hold on
%plot(x,theta,'.')%plots theta as a function of x
axis equal
hold off
numcones = idx - 1 % the index was incremented until theta exceed pi/2 therefore the last value should be excluded.
all_k = [];
numofranges=3% number of ranges of theta angles within which to calculate a single parametric trajectory for that range that will be scaled to each of the cones within that range
conesperrange=round(numcones/numofranges) % how many cones will be in each range not including the last range
conesinlastrange=numcones-conesperrange*(numofranges-1) % how many cones in the last range so that the final number of cones is numcones
figure(1)
clf% clears figure
hold on
for rangenum=1:(numofranges-1) % iterate through the ranges except for the last range which may not have the same number of cones as all the others
%     ranges(rangenum,1)=(rangenum-1)*conesperrange+1;
%     ranges(rangenum,2)=(rangenum*conesperrange);
    ranges(rangenum,1)=theta((rangenum-1)*conesperrange+1); % the first column will be the first angle in that range. Note this is not the same as the last angle of the previous range, but rather the angle of the next cone after the last cone of the previous range
    ranges(rangenum,2)=theta((rangenum*conesperrange)); % the second column will be the last angle in that range
    %plot_cones(RES,FOV,len,ranges(rangenum,:),conesperrange);
   new_k = plot_cones (RES,FOV,len,theta((rangenum-1)*conesperrange+1:(rangenum*conesperrange)));
   all_k = [all_k ; new_k ];
end
%set angles of last range, which may have more or less cones than the other ranges
ranges(numofranges,1)= theta((numofranges-1)*conesperrange+1);
ranges(numofranges,2)=theta((numofranges-1)*conesperrange+conesinlastrange);
new_k = plot_cones (RES,FOV,len,theta((numofranges-1)*conesperrange+1:(numofranges-1)*conesperrange+conesinlastrange));
all_k = [all_k ; new_k ];
%plot_cones(RES,FOV,len,ranges(numofranges,:),conesinlastrange);
hold off
%determine if nyquist is satisfied at periphery of last cone
residualtheta=pi/2-theta(numcones)
if 2*kmaxtheta(numcones)*sin(residualtheta)>1/FOV(1)
    'warning: the distance between the edges of the last cone is'
    2*kmaxtheta(numcones)*sin(residualtheta)
    'but 1/FOVxy='
    1/FOV(1)
end


all_k = [all_k ; all_k];
all_k(end/2:end,3) = -all_k(end/2:end,3);

LEN=128; %length of readout in # of samples
SMAX=40000; % max slew rate in G/cm/s
GMAX=4; % max gradient in 
PRECISION = 0.1;
TS = 0.000015; %0.000004; % sampling interval
oversample = 1; %INCREASING THIS TO 4 (default) doesn't get you to the right place in kspace for some reason
% 
% for rangenum=1:numofranges
%     
% %THETA = min(THETA,pi/2-1e-5); %if a value of theta is greater than pi/2-1e-5, then replace it with pi/2-1e-5
% %THETA = max(THETA,1e-5); %if a value of theta is less than 1e-5, then replace it with 1e-5
% ranges(rangenum,:) = max(ranges(1,:),1e-5);
% ranges(rangenum,:) = min(ranges(1,:),pi/2-1e-5);
% maxtheta = max(ranges(rangenum,:));
% mintheta = min(ranges(rangenum,:));
% 
% % Choose an appropriate angle in between the two thetas
% THETArange = atan2(RES(1)*sin(maxtheta),RES(2)*cos(mintheta)); %****** double check and make sure this doesn't have to be 1/2*res
% % Find the kmax at that angle
% KMAXrange = intlineellipse(1/(2*RES(1)),1/(2*RES(2)),THETArange);
% % Determine the parametric theta for the chosen angle
% tparam = atan2((KMAXrange*sin(THETArange)*kmax_xy),(KMAXrange*cos(THETArange)*kmax_z));
% 
% 
% [g k nint length] = findcone(RES,FOV,LEN,THETA,PRECISION,TS,oversample,SMAX,GMAX);
    