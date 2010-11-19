
switch runNumber
    case 1
        % blocks, experiment design
        blocks = {'fix','sacc','spem','vergtr','vergst'};
     case 2
        % blocks, experiment design
        blocks = {'fix','vergtr','vergst','sacc','spem'};
     case 3
        % blocks, experiment design
        blocks = {'fix','sacc','vergtr','spem','vergst'};
     case 4
        % blocks, experiment design
        blocks = {'fix','vergst','vergtr','spem','sacc'};
     case 5
        % blocks, experiment design
        blocks = {'fix','spem','vergst','sacc','vergtr'};
     case 6
        % blocks, experiment design
        blocks = {'fix','vergtr','sacc','spem','vergst'};
    otherwise
        blocks = {'vergtr','vergst'};
        
end
runNumber = runNumber + 1;
blockDuration = 20;
blockRepeats = 4;

