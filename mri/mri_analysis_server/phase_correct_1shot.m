function phase_correct(filenm, outputdir, imageno)

%reconstruct EPI images from 64x64 Varian raw data
%D Twieg November 2004

%[fn,pn]=uigetfile('*fid','Choose FID file');
%filenm = pn(1:length(pn)-5);
%filenm='/home/mark/Desktop/epi/kiwi_retinotopy_20040805_01/data/epidw_004';
[FR,FI,NP,NB,NT,HDR] = load_fid_file(filenm, 1);
HDR
size(FR)
size(FI)
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
 RE1=AEPI(:,((sliceno-1)*NC+1):sliceno*NC);
 RE=zeros(NR,NC);
 for ncol=1:2:NC-1
     RE(:,ncol)=RE1(:,ncol);
     RE(:,ncol+1)=RE1((NC:-1:1),ncol+1);
 end
% phase for even-numbered columns:
 REz=RE+25+41i;
%
%  Phase correction loop
%
for nc =  1:16
    efac = (nc-8)*0.1;
    factor = (efac)*i;
    for n=2:2:NC;REz(:,n)=(RE(:,n)+25+41i)*exp(factor);
    end
    %figure(1) ;subplot(4,4,nc);imagesc(real(REz));axis off;colormap gray
xcenter = NC / 2;
rof = fft2(REz(xcenter,:));
ind(nc) = abs(rof(xcenter+1));
end
val= min(ind)
for nc = 1:16
    if ind(nc) == val 
        loc = nc
    end
end
newfc = ((loc-8)*0.1)*i
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
        AE1=AEPI(:,((sliceno-1)*NC+1):sliceno*NC);
        AE=zeros(NC,NC);
        for ncol=1:2:NC-1
            AE(:,ncol)=AE1(:,ncol);
            AE(:,ncol+1)=AE1((NC:-1:1),ncol+1);
        end
   
        AEz=AE+25+41i;
        for n=2:2:NC;AEz(:,n)=(AE(:,n)+25+41i)*exp(newfc);
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




