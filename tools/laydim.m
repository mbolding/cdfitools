function [h,w,r] = laydim(x)

h = floor(sqrt(x));
w = floor(x/h);
r = x - w*h;
