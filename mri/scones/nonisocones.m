function [numcones,thetasave] = nonisocones(RES,FOV)
kxy = 0;
kz = 1/RES(2);

theta = atan(kxy/kz);
subplot(121)
plot(kxy,kz)
hold on
numcones = 0;
while theta < pi/2
    thetastep = 1/ellipseRadius(FOV(1),FOV(2),theta);
    theta = theta + thetastep;
    [kxy,kz] = ellipseParametric(1/RES(1),1/RES(2),theta);
    plot(kxy,kz)
    numcones = numcones + 1;
    thetasave(numcones) = theta; %#ok<AGROW>
end
hold off
axis equal
theta = thetasave;
subplot(122)
[x1,y1]=pol2cart(theta,ellipseRadius(1/RES(1),1/RES(2),theta));
[x2,y2]=pol2cart(theta,ellipseRadius2(1/RES(1),1/RES(2),theta));
plot(x1,y1,x2,y2)
axis equal

function [x,y] = ellipseParametric(a,b,t)
x = a*cos(t);
y = b*sin(t);

function r = ellipseRadius(a,b,theta)
r = a*b./(sqrt((b*cos(theta)).^2+(a*sin(theta)).^2));

function r = ellipseRadius2(a,b,theta)
r = intlineellipse(a,b,theta);
