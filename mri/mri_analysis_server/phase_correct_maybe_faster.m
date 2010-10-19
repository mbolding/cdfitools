function phase_correct_maybe_faster(filenm, outputdir)
%
%  since load_fid_file always reads the file from the beginning each time, see if putting
%  processing code inside a load_fid read loop will make things faster
%

%reconstruct EPI images from 64x64 Varian raw data
%D Twieg November 2004

%[fn,pn]=uigetfile('*fid','Choose FID file');
%filenm = pn(1:length(pn)-5);
%filenm='/home/mark/Desktop/epi/kiwi_retinotopy_20040805_01/data/epidw_004';
[FR,FI,NP,NB,NT,HDR] = load_fid_file(filenm, 1);
nimages = HDR(1)
AEPI=FR+1i*FI;
 % the reference frame for this slice, to be used for phase correction,
arrsize = size(AEPI,1);
NR = arrsize(1);
NC = size(FR,1)
nslices = size(FR,2) / NC
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
fid = fopen(fullname,'r','ieee-be');
if fid == -1
    str = sprintf('Can not open file %s',fullname);
    error(str);
end

% Read datafileheader
nblocks   = fread(fid,1,'int32');
ntraces   = fread(fid,1,'int32');
np        = fread(fid,1,'int32');
ebytes    = fread(fid,1,'int32');
tbytes    = fread(fid,1,'int32');
bbytes    = fread(fid,1,'int32');
vers_id   = fread(fid,1,'int16');
status    = fread(fid,1,'int16');
nbheaders = fread(fid,1,'int32');


s_data    = bitget(status,1);
s_spec    = bitget(status,2);
s_32      = bitget(status,3);
s_float   = bitget(status,4);
s_complex = bitget(status,5);
s_hyper   = bitget(status,6);

% reset output structures
RE = [];
IM = [];

%if exist('blocks') == 0
%    blocks = 1:nblocks;
%end
%outblocks = max(size(blocks));
%if exist('traces') == 0
%    traces = 1:ntraces;
%end
%outtraces = max(size(traces));

inx = 1;
B = 1;
for b = 1:nimages-1,
    % Read a block header
    scale     = fread(fid,1,'int16');
    bstatus   = fread(fid,1,'int16');
    index     = fread(fid,1,'int16');
    mode      = fread(fid,1,'int16');
    ctcount   = fread(fid,1,'int32');
    lpval     = fread(fid,1,'float32');
    rpval     = fread(fid,1,'float32');
    lvl       = fread(fid,1,'float32');
    tlt       = fread(fid,1,'float32');
    
    T = 1;
    update_B = 0;
    for t = 1:ntraces
        %We have to read data every time in order to increment file pointer
        if s_float == 1
            data = fread(fid,np,'float32');
            str='reading floats';
        elseif s_32 == 1
            data = fread(fid,np,'int32');
            str='reading 32bit';
        else
            data = fread(fid,np,'int16');
            str='reading 16bit';
        end
        
       RE(:,inx) = data(1:2:np);
       IM(:,inx) = data(2:2:np);
       inx = inx + 1;

       [FR,FI] = load_fid_file(filenm,imno+1);
       AEPI=FR+1i*FI;

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



        % keep data if this block & trace was in output list
        if (blocks(B) == b)
            if T <= outtraces
                if (traces(T) == t)
                    sprintf('Reading block %d, trace %d, inx = %d\n',b,T,inx);


                    inx = inx + 1;
                    T = T + 1;
                    update_B = 1;
                end %keep this trace
            end %still within limit of outtraces
        end %keep this block
    end %trace loop
    
    if update_B, B = B + 1; end
    
    if B > outblocks
        break
    end
end  % done reading one block

for imno=1:nimages-1,



end

%fidoutputfile = sprintf('%sphase_corrected_imgs.raw',outputdir)
%fid = fopen(fidoutputfile, 'w'); 
%count  = fwrite(fid,single(ae),'single');
%fclose(fid);

imgout = [];
for sliceno = 1:nslices,
  s = ae(:,:,1,sliceno);
  s(1+NC/2,1+NC/2) = (s(1+NC/2,2+NC/2)+s(1+NC/2,NC/2)+s(NC/2,1+NC/2)+s(2+NC/2,1+NC/2))/4;
  imgout = [imgout s];
end;
%figure(2);
figure;
imagesc(abs(imgout));axis image;colormap gray



