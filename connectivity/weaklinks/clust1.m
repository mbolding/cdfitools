function [partition,C] = clust1(nii4d,masknii,thresh,k)
% "functional" clustering
% nii4d is smoothed coregistered functional data
% mask nii is a tissue prob map like the ones from spm5 segmentation
%  generated from mean functional data
% thresh is tissue prob threshold for mask generation

try  
    disp('loading /tmp/pwr.nii')
    pwr = load_nii('/tmp/pwr.nii');
catch
    pwr = freqImg(nii4d);
    pwr.img(:,:,:,1)=0; % remove DC
    save_nii(pwr,'/tmp/pwr.nii')
end
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
partition = masknii;
partition.img = zeros(size(partition.img));

opts = statset('Display','final');
[IDX,C] = kmeans(spectra,k,'Distance','correlation','Replicates',3,'Options',opts);

for n = 1:size(voxels,1)
    partition.img(vx(n),vy(n),vz(n)) = IDX(n);
end

view_nii(partition)

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

