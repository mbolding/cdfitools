%% simulated functional connnectivity experiments

%% make the phantom
clear
res = 64; % resolution
snoisefac = .1; % spacial noise ampplitude
sigfac = 1;

%% init some data structures
phan.img = zeros(res); 
phan.mask = phan.img;
r = 3*res/8;
[X,Y] = meshgrid(-res/2:res/2-1,-res/2:res/2-1);
mydist = sqrt(X.*X + Y.*Y);
phan.mask(mydist < r) = 1;

phan.img = phan.mask * sigfac; % make object and add noise
phan.snoise = rand(size(phan.img))*snoisefac;
phan.img = phan.img +  phan.snoise;

imagesc(phan.img); drawnow;
colormap gray