function [all_k,g,traj,nint,rolen] = plot_cones(res,fov,len,theta_list)
% function [g,traj,nint,rolen] = mark_cones(res,fov,len,parametric_cone_theta,numcones)
% generates and plots a series of cones based on one parametric cone. the
% parametric cone is generated with findcone().
%
% g - Cone Gradient
% k - Cone k-space trajectory 
% nint - Number of interleaves (only roughly for a range of polar angles)
% rolen - length of the readout waveform
%
% res - Desired Resolution in mm
% fov - Desired Field of View in cm
% len - Desired Length
% parametric_cone_theta - Range of polar angles (e.g. [0 pi/2] for 0 to 90 degrees)
% numcones - number of cones per parametric cone

[g,traj,nint,rolen] = findcone(res,fov,len,[theta_list(1) theta_list(end)]);
theta_list
nint = ceil(nint);
alternate_cone_rho_increment = rotationmat3D(pi/nint,[0 0 1]);

lastx= traj(end, 1);
lasty= traj(end, 2);
lastz= traj(end, 3);
lastxy=sqrt(lastx^2+lasty^2);
kmax_last=sqrt(lastx^2+lasty^2+lastz^2)
theta_mid=atan2(lastz,lastxy);
theta_mid_from_z=atan2(lastxy,lastz);

res_mid = 1/(intlineellipse(1/(res(1)),1/(res(2)),theta_mid)); % calculate res for a given cone angle. this is the radial distance from the center of kspace to an ellipsoid  with total width 1/RES(xy) and total height 1/RES(z) (center to periphery (i.e. kmax) is 1/(2*RES)
kmax_mid=1/(2*res_mid); % calculate kmax for a given cone angle from the res at that cone angle

idx = 0 ;
all_k = [];
for theta = theta_list
    idx = idx+1;
    fprintf('theta = %f\n',theta);
    res_theta = 1/(intlineellipse(1/(res(1)),1/(res(2)),theta)); % calculate res for a given cone angle. this is the radial distance from the center of kspace to an ellipsoid  with total width 1/RES(xy) and total height 1/RES(z) (center to periphery (i.e. kmax) is 1/(2*RES)
    kmax_theta=1/(2*res_theta); % calculate kmax for a given cone angle from the res at that cone angle
%     scale_xy=kmax_theta*sin(theta)/(kmax_mid*sin(theta_mid))
%     scale_z=kmax_theta*cos(theta)/(kmax_mid*cos(theta_mid))
    scale_xy=kmax_theta*cos(theta)/(kmax_last*cos(theta_mid))
    scale_z=kmax_theta*sin(theta)/(kmax_last*sin(theta_mid))
    
    S = [ scale_xy      0           0          ;...
        0               scale_xy    0          ;...
        0               0           scale_z];
    for rho = linspace(0,2*pi,nint)
        M=rotationmat3D(rho,[0 0 1]);
        m = traj * S * M;
        all_k = [all_k ; m];
        plot3(m(end,1),m(end,2),m(end,3),'.');
        drawnow;
    end
    traj = traj * alternate_cone_rho_increment;
end

% [g,k,nint,rolen] = findcone(2,24,1024,[pi/4 pi/2]);
% plot3(k(:,1),k(:,2),k(:,3));
 axis equal
%axis vis3d
