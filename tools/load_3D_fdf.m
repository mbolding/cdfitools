function [IM,V,ct] = load_3D_fdf(name)
%----------------------------------------
%function load_fdf modified to load 3D data
%Reads magnitude images in fdf format
%Assumes, that the last entry in header is checksum
%----------------------------------------
%Usage [IM,np,nv] = load_fdf(name,sliceno,imno);
%
%Input:
%name = name of data directory without the .dat or .img extension
%
%Output:
%IM = images, sorted by row
%V  = size in voxels
%ct = number of points read from file
%
%Examples:
%
%----------------------------------------
% Maj Hedehus, Varian, Inc., Oct 2001.
% edited Nov 2003 -- Lulu Edwards (changed program to read in new names of files
% msb April 2007 -- mark bolding changed for 3D
%----------------------------------------

% format input name
dirname = sprintf('%s.img',name);

if (exist(dirname,'dir') == 0)
    dirname = sprintf('%s.dat',name);
    if (exist(dirname,'dir') == 0)
        error('Cannot find data directory');
    end
end

fullname = sprintf('%s%sdata.fdf',dirname,'/');
fid = fopen(fullname,'r','ieee-be');
if fid == -1
    error(['Can not open file ' fullname]);
end

checksum_str = fgets(fid);
disp(checksum_str)
while isempty(findstr('checksum',checksum_str))
    str = checksum_str;
    
    % Get matrix size
    if (~isempty(findstr(str,'matrix')))
        n = findstr(str,'{');
        V = sscanf(str(n+1:length(str)),'%d, %d, %d');
        disp(V)
    end

    % Get binary data type
    if (~isempty(findstr(str,'storage')))
        clist = findstr(str,'"');
        dtype = sscanf(str(clist(1)+1:clist(2)-1),'%s');
    end
    if (~isempty(findstr(str,'bits')))
        n = findstr(str,'=');
        bits = sscanf(str(n+1:length(str)),'%d');
    end
    
    checksum_str = fgets(fid);
    disp(checksum_str)
end

if (strcmp(dtype(1:3),'flo'))
    precision = sprintf('float%d',bits);
elseif (strcmp(dtype(1:3),'int'))
    precision = sprintf('int%d',bits);
end

% Skip past NULL character that separates header and data
v = fread(fid,1,'uchar');
while v ~= 0
    v = fread(fid,1,'uchar');
end


[IM ct] = fread(fid,prod(V),precision);
IM = reshape(IM,V');

fclose(fid);






