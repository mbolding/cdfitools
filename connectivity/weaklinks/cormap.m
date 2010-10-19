function cormap = cormap(pwr,c,masknii,thresh)
% cormap = cormap(pwr,masknii,c)
% correlation map
% pwr is 4d func data file
% c is vector for correlation
% mask nii is a tissue prob map like the ones from spm5 segmentation
%  generated from mean functional data
% thresh is tissue prob threshold for mask generation

masknii.img(masknii.img < thresh) = 0;
%%
voxels = find(masknii.img);
[vx,vy,vz] = ind2sub(size(masknii.img),voxels);

spectra = zeros(size(voxels,1),size(pwr.img,4));
for n = 1:size(voxels,1)
    spectra(n,:) =  pwr.img(vx(n),vy(n),vz(n),:);
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
cormap = masknii;
cormap.img = zeros(size(cormap.img));
RHO = corr(spectra,c);

for n = 1:size(voxels,1)
    cormap.img(vx(n),vy(n),vz(n)) = RHO(n);
end

view_nii(cormap)

% %%
% partition = masknii;
% partition.img = zeros(size(partition.img));
% 
% opts = statset('Display','final');
% T = clusterdata(spectra,4,'Distance','correlation','Options',opts);
% 
% for n = 1:size(voxels,1)
%     partition.img(vx(n),vy(n),vz(n)) = IDX(n);
% end
% 
% view_nii(partition)


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

