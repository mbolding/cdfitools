srccw = load_nii('srccw.nii');
pwr = freqImg(srccw);
pwr.img(:,:,:,1)=0;
save_nii(pwr,'pwr.nii')
%%
% meanimg = load_nii('srccw.nii',1);
% meanimg.img = mean(srccw.img,4);
% imgmax = max(meanimg.img(:));
% masknii = meanimg;
% masknii.img(meanimg.img < 0.2*imgmax | meanimg.img > 0.8 * imgmax)=0;
% masknii.img(masknii.img>0)=1;
c1srccw = load_nii('c1srccw.nii');
masknii = c1srccw;
masknii.img(masknii.img < 0.3) = 0;
%%
voxels = find(masknii.img);
[vx,vy,vz] = ind2sub(size(masknii.img),voxels);

spectra = zeros(size(voxels,1),size(pwr.img,4));
for k = 1:size(voxels,1)
    spectra(k,:) =  pwr.img(vx(k),vy(k),vz(k),:);
end

% sigs = zeros(size(voxels,1),size(srccw.img,4));
% for k = 1:size(voxels,1)
%     sigs(k,:) =  srccw.img(vx(k),vy(k),vz(k),:);
% end
% 
% phases = zeros(size(voxels,1),size(phs.img,4));
% for k = 1:size(voxels,1)
%     phases(k,:) =  phs.img(vx(k),vy(k),vz(k),:);
% end


%%
partition = masknii;
partition.img = zeros(size(partition.img));

opts = statset('Display','final');
IDX = kmeans(spectra,4,'Distance','correlation','Replicates',5,'Options',opts);

for k = 1:size(voxels,1)
    partition.img(vx(k),vy(k),vz(k)) = IDX(k);
end

view_nii(partition)

%%
partition = masknii;
partition.img = zeros(size(partition.img));

opts = statset('Display','final');
T = clusterdata(spectra,4,'Distance','correlation','Options',opts);

for k = 1:size(voxels,1)
    partition.img(vx(k),vy(k),vz(k)) = IDX(k);
end

view_nii(partition)


% %%
% partition.img = zeros(size(partition.img));
% 
% IDX = kmeans(phases,4,'Distance','correlation','Replicates',5,'Options',opts);
% 
% for k = 1:size(voxels,1)
%     partition.img(vx(k),vy(k),vz(k)) = IDX(k);
% end
% 
% view_nii(partition)
% 
% %%
% partition.img = zeros(size(partition.img));
% 
% IDX = kmeans(sigs,4,'Distance','correlation','Replicates',5 ,'Options',opts);
% 
% for k = 1:size(voxels,1)
%     partition.img(vx(k),vy(k),vz(k)) = IDX(k);
% end
% 
% view_nii(partition)
% 

