D = dir ('*.*.mat');
numfiles = size(D,1);
disp('file dates')
for k = 1:numfiles
    temp = textscan(D(k).name,'%f.mat');
    filedate = datestr(temp{1});
    disp([filedate '  -  ' D(k).name])
end