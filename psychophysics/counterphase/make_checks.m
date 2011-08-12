function checks = make_checks(checksize,repeatx,repeaty,white_color,black_color)
% make checkerboard for visstim
if ~exist('white_color','var')
    white_color = 255;
end

if ~exist('black_color','var')
    black_color = 0;
end



black_square = zeros(checksize);
white_square = ones(checksize);

white_square(:) = white_color;
black_square(:) = black_color;

a_check = [black_square white_square ; white_square black_square];

checks = repmat(a_check,repeaty,repeatx);