function phase_correct(filenm, outputdir, imageno)

%reconstruct EPI images from 64x64 Varian raw data
%D Twieg November 2004

%[fn,pn]=uigetfile('*fid','Choose FID file');
%filenm = pn(1:length(pn)-5);
%filenm='/home/mark/Desktop/epi/kiwi_retinotopy_20040805_01/data/epidw_004';
[FR,FI,NP,NB,NT,HDR] = load_fid_file(filenm, 1);
HDR

nimages = HDR(1);
sprintf('nimages %d', nimages)
AEPI=FR+1i*FI;
 % the reference frame for this slice, to be used for phase correction,
arrsize = size(AEPI,1);
NR = arrsize(1);
NC = size(FR,1)
nslices = size(FR,2) / NC;
sprintf('nslices %d', nslices)
sliceno = 1;
 %RE=AEPI(:,((sliceno-1)*NC+1):sliceno*NC);
 slicestart = ((sliceno-1)*NC/2+1);
 sliceend = sliceno*NC/2
 RE = [AEPI(:,slicestart:sliceend)]; % AEPI(:,end/2+slicestart:end/2+sliceend)];
 RE(:,2:2:end) = RE(end:-1:1,2:2:end);  % reverse every other line of k-space data

% phase for even-numbered columns:
fudge = 0i;  % 25*41i; (?)
REz=RE + fudge;
%
%  Phase correction loop
%
for nc =  1:64 
    efac = (nc-32)*0.05;
    factor = (efac)*i;
    for n=2:2:NC/2;  %*
      REz(:,n)=(RE(:,n)+fudge)*exp(factor);
    end
    %figure(1) ;subplot(4,4,nc);imagesc(real(REz));axis off;colormap gray
    xcenter = NC / 2;
    rof = fft2(REz(xcenter,:));
    ind(nc) = abs(rof(xcenter/2+1)); %*
end
val= min(ind)
for nc = 1:64
    if ind(nc) == val 
        loc = nc
    end
end
newfc = ((loc-32)*0.05)*i
ae=zeros(NC,NC,nimages,nslices);
%
%  process all images
%
if (nargin < 3)
  imageno = 1;
end;
%for imno=1:1, %nimages-1,
imno = imageno;
    [FR,FI] = load_fid_file(filenm,imno+1);
    AEPI=FR+1i*FI;
    for sliceno=1:nslices
        %AE1=AEPI(:,((sliceno-1)*NC+1):sliceno*NC);
        slicestart = ((sliceno-1)*NC/2+1);
        sliceend = sliceno*NC/2
        AE1 = [AEPI(:,slicestart:sliceend) AEPI(:,end/2+slicestart:end/2+sliceend)];
        AE1(:,2:2:end) = AE1(end:-1:1,2:2:end);  % reverse every other line of k-space data
        AE = zeros(size(AE1,1), size(AE1,2));
        AE(:,2:2:end) = AE1(:,1:end/2);          % the odd lines of k-space were collected first (shot)
        AE(:,1:2:end) = AE1(:,1+end/2:end);      % the even lines of k-space were collected second (shot)
   
        AEz=AE+fudge;
        for n=2:2:NC;AEz(:,n)=(AE(:,n)+fudge)*exp(newfc);
        end
        ae(:,:,imno,sliceno)=fftshift(ifft2(fftshift(AEz)));
    end
%end

%fidoutputfile = sprintf('%sphase_corrected_imgs.raw',outputdir)
%fid = fopen(fidoutputfile, 'w'); 
%count  = fwrite(fid,single(ae),'single');
%fclose(fid);

imgout = [];
for sliceno = 1:nslices,
  s = ae(:,:,imno,sliceno);
  s(1+NC/2,1+NC/2) = (s(1+NC/2,2+NC/2)+s(1+NC/2,NC/2)+s(NC/2,1+NC/2)+s(2+NC/2,1+NC/2))/4;
  imgout = [imgout s];
end;
%figure(2);

figure;
imagesc(abs(imgout));
axis off;
axis image;
colormap gray;




