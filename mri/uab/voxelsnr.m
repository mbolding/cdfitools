function mysnr = voxelsnr(inputfilename, outputfilename)
% this function loads a .nii file.
% calculates the average and std across time for that file
% from that, calculates snr (avg/std)
% saves to another .nii file, appended with _snr
% requires the nifti toolbox from matlab file exchange
% inputfilename must end in .nii
% by Kristina Visscher, Feb 2010

% load the .nii file
mybrain = load_untouch_nii(inputfilename);

% time varying signal to noise ratio is just mean/standard deviation
mymean = mean(mybrain.img,4);
mystd = std(mybrain.img,[],4);
mysnr = mymean./mystd;

% clean up the figure, get rid of NANs
mynans = find(isnan(mysnr));
mysnr(mynans) = 0;
myinfs = find(isinf(mysnr));
mysnr(myinfs) = 0; 
mybrain.img = mysnr;
if outputfilename
    save_untouch_nii(mybrain, outputfilename);
else
    save_untouch_nii(mybrain, ['snr_' inputfilename]);
end