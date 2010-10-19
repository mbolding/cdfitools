function [c] = stripedcircle(p,b,na)
% makes striped circles for 1D noise expts
% p is pixels per bar
% b is bars [black white]
% na is number of angles

if nargin < 1; p = 10;          end;	% pixels per bar

if nargin < 2; b = [25 25];     end;	% bars per screen [black white]

if nargin < 3; na = 18;         end;	% # angular increments from 0 to 180 deg


%% 
nb = sum(b);                % total bars
np = p*nb;                  % total pixels 

x = linspace(-1,1,np);      % make a circular mask
[X,Y] = meshgrid(x);
mask = uint8( X.^2 + Y.^2 <= 1 );

da = 180/na;
c = zeros(np,np,18,'uint8');
c(:,:,1) = imresize(repmat(1:nb,nb,1),p,'nearest');
for i = 2:na
	c(:,:,i) = imrotate(c(:,:,1),i*da-da/2,'nearest','crop') .* mask;
end
c(:,:,1) = imrotate(c(:,:,1),da/2,'nearest','crop') .* mask;
