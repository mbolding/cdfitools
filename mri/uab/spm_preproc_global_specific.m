% Written by Rishi Deshpande, March 2010
% This script runs a basic SPM preprocessing algorithm
% to use, change the dicomdir variable to identify the dicoms of interest.  

clc;
dicomdir = '/home/labadmin/scratchpad/test_subject'; %Use the directory path where all the functional dicoms reside

cd(dicomdir);


dicomlist=dir('*.dcm');		%Get a list of dicom files in that directory
dicomlist=char(dicomlist.name);
clc;

%Load SPM defaults

spm_defaults;
global defaults;

%% Anonimization and Conversion to img+hdr formats


display('Step 1: Anonimizing and Converting from Dicom to NII+HDR formats');
tic;
for list = 1:size(dicomlist,1)
    curfile=dicomlist(list,1:findstr(dicomlist(list,:),'.dcm')+3);
    dicomanon(curfile,curfile,'keep',{'PatientID'});		%First anonymize each dicom file
    c=spm_dicom_headers(curfile);
    k(list).files=spm_dicom_convert(c,'all','flat','img');		%Convert the anonymized files to .img/.hdr
    if mod(list,50) ==0
        x=toc;
        timeleft = ceil((size(dicomlist,1)*x)/list -x);		%Don't worry about this part...
        display(sprintf('...converting \t%d of \t%d. Time remaining:\t%d sec...',list,size(dicomlist,1),timeleft));
    end
end


%% Realignment and Reslicing
%All volumes are realigned wrt first time volume

display('Step 2: Realignment');
P=dir('f*.img');
P=char(P.name);
FlagsC = struct('quality',1,'fwhm',5,'rtm',1,'interp',1);   %Flags for realignment
spm_realign(P,FlagsC);

display('Step 3: Reslice');
%Flags for reslicing; Write a mean images don't create 'r' prefix files

FlagsR = struct('mask', 1, 'mean', 1,'interp',4,'which',0, 'wrap', defaults.realign.write.wrap);
spm_reslice(P,FlagsR);


%% Normalization
%Normalize the images to a standard template
%The source image is the mean image created from realign/reslice step.

display('Step 4: Normalization');

template = fullfile(spm('Dir'),'templates', 'EPI.nii');	%Use the EPI.nii template for functional data
srcimage = 'mean*.img';

meanimg = dir(srcimage);
P = char(meanimg.name);
snmat_name = strrep(P, '.img', '_sn.mat');
if exist(snmat_name, 'file')
    disp(['... removing existing file named: ' snmat_name]);
    delete(snmat_name);
end
objmask_name = '';

spm_normalise(template, P, snmat_name, defaults.normalise.estimate.weight, objmask_name,defaults.normalise.estimate);

%% Write Normalized Images

display('Step 6: Writing Normalized Images');

PP = dir('f*.img');
PP = char(PP.name);
spm_write_sn(PP, snmat_name, defaults.normalise.write);

%% Smoothing

display('Step 7: Smoothing ... this is the last step !!!');

PP = dir('wf*.img');
PP = char(PP.name);
PP = cellstr(PP);

for list = 1:length(PP)
    Q = strcat('s', PP{list});
    spm_smooth(PP{list}, Q, defaults.smooth.fwhm);
end
x=ceil(toc);
display(sprintf('\nPreprocessing compled.\n\nTime taken %d sec.\n', x));

