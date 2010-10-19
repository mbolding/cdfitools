#!/bin/tcsh -f
# This function uses fsl commands to compute the signal to noise ratio for a bold image
# (that is, the mean divided by the std. deviation)
# takes as input the path to a bold file in .nii format
# outputs a voxelwise snr file as a temp file tempSnr.nii.gz

# Kristina Visscher Feb 16, 2010
# alterd Fen 24 to take an output. so syntax is fsl_snr infile outfile

set input = $1
set output = $2
fslmaths $input -Tstd tempStd
#fslmaths $input -Tmean -div tempStd  $input_snr.nii.gz # this would put it in same directory as orignal bold
fslmaths $input -Tmean -div tempStd $output
rm tempStd.nii.gz
