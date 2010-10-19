function nii = T1mapRecon(outname)
%% load the data
D = dir('T1map2*.fid');
for k = 1:size(D,1)
    disp(D(k).name(1:end-4))
    [RE,IM,np,nb,HDR] = load_fids(D(k).name(1:end-4));
    procpars = load_procpar(D(k).name(1:end-4));
    pss = query_procpar('pss',procpars);
    spacing = abs(pss(1) - pss(2));
%     gap = query_procpar('gap',procpars);
    lro = query_procpar('lro',procpars);
    lpe = query_procpar('lpe',procpars);
    if pss(end) > pss(1)
        disp('linear')
    else
        disp('reverse linear')
    end
    if k < 10
        save(['T1map2_0' num2str(k)],'RE','IM','np','nb','HDR','pss')
    else
        save(['T1map2_' num2str(k)],'RE','IM','np','nb','HDR','pss')
    end

end
disp([num2str(k) ' fids'])

%% process the data
clear D RE IM np nb HDR procpars k
D2 = dir('T1map2*.mat');
try
    load(D2(1).name)
catch ME
    disp(ME)
end
numVols = size(D2,1);
outvols = zeros(nb,np,HDR,numVols);
myvol = RE+i*IM;
dcoff=mean(myvol(:));
figure(1)
for k = 1:numVols
    load(D2(k).name)
    disp(D2(k).name)
    if pss(end) > pss(1)
        disp('linear')
    else
        disp('reverse linear')
    end
%     outvol = zeros(nb,np,HDR);
    myvol = RE+i*IM;
    for sliceNum = 1:HDR; 
        temp = myvol(:,sliceNum:HDR:end) - dcoff;
        temp(1:4,:)=conj(temp(end:-1:end-3,:));
        if pss(end) > pss(1)
            outvols(:,:,sliceNum,k) = abs(fftshift(ifft2(temp)))'; 
        else
            outvols(:,:,1+HDR-sliceNum,k) = abs(fftshift(ifft2(temp)))';         
        end
    end
    [w,h]=laydim(HDR);
    for n = 1:w
        for m = 1:h
            subplot(w,h,(n-1)*h+m,'align');
            imagesc(outvols(:,:,(n-1)*h+m,k))
            set(gca,'xtick',[],'ytick',[])
            colormap gray
            axis image
        end
    end
    title(num2str(k))
    drawnow
end



%% average the result


avvol = mean(outvols(:,:,:,:),4);
figure(2)

[w,h]=laydim(HDR);
for k = 1:w
    for m = 1:h
        subplot(w,h,(k-1)*h+m,'align');
        imagesc(avvol(:,:,(k-1)*h+m))
        set(gca,'xtick',[],'ytick',[])
        colormap gray
        axis image
    end
end
% save('avvol','avvol')
dims = size(avvol);
voxdim = [lro lpe 1 ] ./ dims;
voxdim(3) = spacing;
voxdim = 10*voxdim;
nii = make_nii( avvol(:,end:-1:1,end:-1:1) , voxdim);
save_nii(nii,outname);
% 
% for k = 1:6
%     for m = 1:4
%         subplot(6,4,(k-1)*4+m,'align');
%         imagesc(abs(myvol(:,:,(k-1)*4+m)))
%         set(gca,'xtick',[],'ytick',[])
%         colormap gray
%         
%     end
% end



