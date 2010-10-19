function values = query_procpar(var_name,procpar_vars)
% query_procpar.m
% msb dec 5 2006
% msb Feb 2007
% Searches through results of load_procpar.m for a variable name and returns
% the value. Throws and error if it cannot find the var.
% examples:
% value = query_procpar('thk',procpar_vars);
% flips = query_procpar('fliplist',procpar_vars);
% mark = query_procpar('operator_',procpar_vars);
%%
values = NaN;
for k = 1:size(procpar_vars,2)
    if strcmp(procpar_vars{k}.name,var_name)
        values = procpar_vars{k}.values{1};
        return
    end
end

error([var_name ' was not found by query_procpar.'])