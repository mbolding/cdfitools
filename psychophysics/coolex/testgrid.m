function testgrid(type)
% testgrid.m

if ~exist('type','var') ; type = 0 ; end;
% Open a window
s=max(Screen('Screens'));
w=Screen('OpenWindow', s);
[wWidth, wHeight]=Screen('WindowSize', w);
Screen('FillRect', w, 0);

% Make up test values
numDemoLines=10;
lineWidths{1}=5;
lineWidths{2}=5;
xPositions=round(linspace(0, wWidth/2, numDemoLines + 2));
xPositions=xPositions(2:end-1);
demoLines{1}=[1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1];
demoLines{2}=[1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1];  
% demoLines{3}=[1 0 1 0 1 0 1 0 1 0 1 0 1 0 1 0];
% demoLines{4}=[1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1];
switch type
    case 0
        myColors{1}=[0 255 0];
        myColors{2}=[255 0 255];
    case 1
        myColors{1}=[255 255 255];
        myColors{2}=[255 255 255];
    otherwise
        myColors{1}=[0 0 0];
        myColors{2}=[0 0 0];
end

stretchFactors{1}=1;
stretchFactors{2}=1;
stretchFactors{3}=1;


%Screen 'LineStipple' reads back the current state.
[isStippleEnabled, stippleStretchFactor, stipplePattern]=Screen('LineStipple',w);
enabledStrings={'No', 'Yes'};
fprintf('Screen ''LineStipple'' reads back the current stippling state for a window.\n');
fprintf('  Reading back the default state, unchanged after opening a new window:\n');
fprintf(['    enabled:          ' enabledStrings{isStippleEnabled+1} '\n']);
fprintf(['    stretch factor:   ' num2str(stippleStretchFactor) '\n']); 
fullPatternImage=repmat('_', 1, 16);
patternImage=fullPatternImage;
patternImage(~stipplePattern)=' ';
fprintf(['    pattern:          ' patternImage '\n']);
fprintf(['    full comparison:  ' fullPatternImage  '\n']);
fprintf('\n');

%Screen 'LineStipple' sets a new state.
newStippleEnabled=1;
newStippleStretchFactor=3;
newStipplePattern=demoLines{2};
fprintf('Screen ''LineStipple'' also sets a new stipple state for a window.\n');
fprintf('  Setting the new stipple state:\n');
Screen('LineStipple',w, newStippleEnabled, newStippleStretchFactor, newStipplePattern);
fprintf(['    enabled:          ' enabledStrings{newStippleEnabled+1} '\n']);
fprintf(['    stretch factor:   ' num2str(newStippleStretchFactor) '\n']); 
fullPatternImage=repmat('_', 1, 16);
patternImage=fullPatternImage;
patternImage(~newStipplePattern)=' ';
fprintf(['    pattern:          ' patternImage '\n']);
fprintf(['    full comparison:  ' fullPatternImage  '\n']);
fprintf('\n');

%Screen 'LineStipple' reads back the new state.
[isStippleEnabled, stippleStretchFactor, stipplePattern]=Screen('LineStipple',w);
fprintf('  Reading back changed state, modified after opening a new window:\n');
fprintf(['    enabled:          ' enabledStrings{isStippleEnabled+1} '\n']);
fprintf(['    stretch factor:   ' num2str(stippleStretchFactor) '\n']); 
fullPatternImage=repmat('_', 1, 16);
patternImage=fullPatternImage;
patternImage(~stipplePattern)=' ';
fprintf(['    pattern:          ' patternImage '\n']);
fprintf(['    full comparison:  ' fullPatternImage  '\n']);
fprintf('\n');

fprintf('  Using Screen ''LineStipple'' and ''DrawLine'' to draw stippled lines... ');
for idx = 1:numDemoLines
    Screen('LineStipple',w, 1);
    Screen('DrawLine', w, myColors{1}, xPositions(idx), 0,  xPositions(idx), wHeight, lineWidths{1});
    Screen('DrawLine', w, myColors{2}, xPositions(idx) + wWidth/2, 0,  xPositions(idx) + wWidth/2, wHeight, lineWidths{1});
end


Screen('DrawText', w, 'Click to end', 0, 0, [255 255 255 ]);
Screen('Flip', w);
fprintf('Done.\n');
fprintf('\n');
[~, ~, buttons] = GetMouse;
while ~buttons(1)
    [~, ~, buttons] = GetMouse;
    WaitSecs(0.2);
end
Screen('CloseAll');
        


