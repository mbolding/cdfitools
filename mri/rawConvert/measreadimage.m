% short example demonstrating the use of measread
% use e.g. with the MDH data from the simple ICE examples on the IDEA home
% page
% call the function, data and MDH are in matlab format
if exist('MeasRead','file')
    [mdh,data]=MeasRead('meas.out');
else
    load testData2Lphantom
end
%strip the additional ADC at the end
mysize=size(data);
data=data(1:mysize(1),1:mysize(2)-1);
mysize=size(data);
% mdh=mdh(1:mysize(1),1:1);
mdh=mdh(1:mysize(2));
% example showing how to access the data in the mdh structure
% (Maybe the selection of structs was not so smart after all, 
% now we have to convert the elements of the struct into cells before we
% can use them...)

% first we read the wanted attribute into cells
[Y{1:mysize(2)}] = deal(mdh.CurSlice);
% then we convert the cell into a matrix
% Careful: before reusing the generated matrix, it has to be reformated (is
% in vector form right now i.e. <1xmysize(1)>
Slice=cell2mat(Y);
Slices = unique(Slice)+1;

[Y{1:mysize(2)}] = deal(mdh.CurAcqu);
Acqu=cell2mat(Y);
Acqs = unique(Acqu);

[Y{1:mysize(2)}] = deal(mdh.CurLine);
Line=cell2mat(Y);
Linemax = unique(max(Line)+1);

% if (max(M) > 0) 
%     sprintf('result may be wrong, more than one coil used') 
% end
% now we fourier transform the data
% of course in reality we have to sort the data first according to the MDH
% entries, the task which is usually done by the ICE program. Of course in
% matlab this can be done easily with the help of indices, sorting and
% comparing indices (find), but who am I to tell you this ;-)
%(e.g.: imagesc(abs(fft2(data(:,find(M)))))  to reconstruct just one coil)

fprintf('Slices =');disp(Slices)
fprintf('Acqs =' );disp(Acqs)
fprintf('Lines =' );disp(Linemax)
SliceMax=max(Slices);
for idx = Slices
    slicedata = data(:,(Slice==(idx-1) & Acqu==0));  % pick a slice and acq number
    slicedata = (slicedata(1:2:end,:) + 1i*slicedata(2:2:end,:));
    Image(:,:,idx) = slicedata;
    
    fftdata=fftshift(fft2(fftshift(Image(:,:,idx))));
    subplot(2,SliceMax,idx)
    imagesc(abs(fftdata));
    axis image
    subplot(2,SliceMax,idx+SliceMax)
    imagesc(angle(fftdata));
    axis image
    drawnow
end
