function [widthDegree, heightDegree] = calVisualAngle(side, monitorResolution, screenDimension, viewDistance, objSize)

% side             : Stimulus. 0 = Centered, 1 = One side. 
% monitorResolution: A matrix of monitor resolution in mm. [width, height]
% screenDimension  : A matrix of screen size in mm. [width, height]
% viewDistance     : Subject viewing distance in mm.
% objSize          : Stimulus size in pixels.

% Centered
if side == 0
  widthRadian = atan((objSize / 2) / (viewDistance * monitorResolution(1) / screenDimension(1)));
  heightRadian = atan((objSize / 2) / (viewDistance * monitorResolution(2) / screenDimension(2)));

  widthDegree = widthRadian / pi * 180;
  heightDegree = heightRadian / pi * 180;

  widthDegree = widthDegree * 2;
  heightDegree = heightDegree * 2;
end

% One Side
if side == 1
  widthRadian = atan(objSize / (viewDistance * monitorResolution(1) / screenDimension(1)));
  heightRadian = atan(objSize / (viewDistance * monitorResolution(2) / screenDimension(2)));

  widthDegree = widthRadian / pi * 180;
  heightDegree = heightRadian / pi * 180;
end