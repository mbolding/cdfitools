function procpar_vars = load_procpar(fid_dir)
% load_procpar.m
% msb dec 5 2006
% loads variables from procpar into two cell arrays of structures.
% each cell contains a name and an array of cells of values.
% Some of the vars are numeric and some are strings,
% so the data structure has to hold arrays of nums 
% or arrays of variable length strings. That is why it is made up
% of arrays of cells.  
% examples:
% procpar_vals = load_procpar('pinecone_01')
% disp(procpar_vars{1})
% disp(procpar_vars{1}.name)
% disp(procpar_vars{1}.values{1})

%% open file for reading, init some vars
fullname = sprintf('%s.fid%cprocpar',fid_dir,'/'); % N.B. windows \ or *nix /
fid = fopen(fullname,'r');
if fid == -1
    error(['Can not open file.' fullname]);
end
var_ctr=0;

%% read in the data and store it in a structure
while 1
    % read the var name and attributes line
    % name subtype basictype max min step Ggroup Dgroup protection active intptr
    var_attributes = textscan(fid,'%s %d %d %f %f %f %d %d %d %d %*[^\n]',1);
    if size(var_attributes{1},1)==0, break, end % if there is not a name then we are done
    var_array_size = textscan(fid,'%d ',1);
    switch var_attributes{3}(1) % basic type of var
        case 0 % undefined
            % ignore
        case 1 % real
            var_ctr = var_ctr+1;
            procpar_vars{var_ctr}.name = var_attributes{1}{1};
            procpar_vars{var_ctr}.values = textscan(fid,'%f ',var_array_size{1}(1));
        case 2 % string
            var_ctr = var_ctr+1;
            procpar_vars{var_ctr}.name = var_attributes{1}{1};
            procpar_vars{var_ctr}.values = textscan(fid,'%q',var_array_size{1}(1));
        otherwise
            % ignore
    end
    procpar_vars{var_ctr}.size = var_array_size{1}(1);
    textscan(fid,'%*[^\n]',1); % eat the last line in the var record
end

%% close the file and clean up
try
    fclose(fid);
catch
    error(['could not close file' fullname]);
end

%% return the structure with the procpar var name value pairs
% disp(['read ' num2str(var_ctr) ' vars from the procpar file.']);
return