function phase_correct(filenm, outputdir, imageno)

%reconstruct EPI images from 64x64 Varian raw data
%D Twieg November 2004

%[fn,pn]=uigetfile('*fid','Choose FID file');
%filenm = pn(1:length(pn)-5);
%filenm='/home/mark/Desktop/epi/kiwi_retinotopy_20040805_01/data/epidw_004';
[FR,FI,NP,NB,NT,HDR] = load_fid_file(filenm, 1);
HDR
size(FR)
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
 slicestart = ((sliceno-1)*NC+1);
 sliceend = sliceno*NC
%% RE = [AEPI(:,slicestart:sliceend)]; % no: AEPI(:,end/2+slicestart:end/2+sliceend)];
%% RE(:,2:2:end) = RE(end:-1:1,2:2:end);  % reverse every other line of k-space data

AEPI(:,2:2:end) = AEPI(end:-1:1,2:2:end);
RE1(1).aepi = AEPI(:,slicestart:sliceend); 
RE1(2).aepi = AEPI(:,end/4+slicestart:end/4+sliceend);
RE1(3).aepi = AEPI(:,end/2+slicestart:end/2+sliceend);
RE1(4).aepi = AEPI(:,3*end/4+slicestart:3*end/4+sliceend);

% phase for even-numbered columns:
fudge = 0i;  % 25*41i; (?)

for shot=1:4,
  REz=RE1(shot).aepi + fudge;
%
%  Phase correction loop
%
 for nc =  1:64 
    efac = (nc-32)*0.05;
    factor = (efac)*i;
    REz(:,2:2:end) = RE1(shot).aepi(:,2:2:end)*exp(factor);
    %for n=2:2:NC;  
    %  REz(:,n)=(RE(:,n)+fudge)*exp(factor);
    %end
    %figure(1) ;subplot(4,4,nc);imagesc(real(REz));axis off;colormap gray
    xcenter = size(REz,2) / 2;
    rof = ifft(REz(xcenter+1,:));
    ind(nc) = abs(rof(xcenter+1)); % max(abs(rof)); % CHANGED: WAS abs(rof(xcenter-1))
 end;
 val= min(ind)
 for nc = 1:64
    if ind(nc) == val 
        loc = nc
   end;
 end;
 newfc(shot).value = ((loc-32)*0.05)*i
end;

fc = (newfc(1).value+newfc(2).value+newfc(3).value+newfc(4).value)/4;

ae=zeros(NC,NC,nimages,nslices);
%
%  process all images
%
if (nargin < 3)
  imageno = 1;
end;
%for imno=1:1, %nimages-1,
imno = imageno;
    [FR,FI] = load_fid_file(filenm,imno);  %  WAS: imno+1
    AEPI=FR+1i*FI;
size(AEPI)
NC
    for sliceno=1:nslices
        %AE1=AEPI(:,((sliceno-1)*NC+1):sliceno*NC);
        slicestart = ((sliceno-1)*NC/4+1);
        sliceend = sliceno*NC/4;
        AE1 = [AEPI(:,slicestart:sliceend) AEPI(:,end/4+slicestart:end/4+sliceend) AEPI(:,end/2+slicestart:end/2+sliceend) AEPI(:,3*end/4+slicestart:3*end/4+sliceend)];
        AE1(:,2:2:end) = AE1(end:-1:1,2:2:end);  % reverse every other line of k-space data
        % apply phase correction here while every other line was still alternating
        AE1(1:end/4,2:2:end)         = AE1(1:end/4,2:2:end)*exp(newfc(1).value);
        AE1(end/4+1:end/2,2:2:end)   = AE1(end/4+1:end/2,2:2:end)*exp(newfc(2).value);
        AE1(end/2+1:3*end/4,2:2:end) = AE1(end/2+1:3*end/4,2:2:end)*exp(newfc(3).value);
        AE1(3*end/4+1:end,2:2:end)   = AE1(3*end/4+1:end,2:2:end)*exp(newfc(4).value);
        AE = zeros(size(AE1,1), size(AE1,2));
        AE(:,1:4:end) = AE1(:,1:end/4);          % the odd lines of k-space were collected first (shot)
        AE(:,2:4:end) = AE1(:,1+end/4:end/2);    % the even lines of k-space were collected second (shot)
        AE(:,3:4:end) = AE1(:,1+end/2:3*end/4);
        AE(:,4:4:end) = AE1(:,1+3*end/4:end);
   
        %AEz=AE+fudge;
        %for n=2:2:NC;AEz(:,n)=(AE(:,n)+fudge)*exp(newfc);
        %end
        ae(:,:,imno,sliceno)=fftshift(ifft2(fftshift(AE)));
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




