#!/bin/tcsh -f
# $Header: makeqc_fig.m,Visscher $
# $Author: Kristina Visscher $ $Date: 20100215 $
# This function takes in the following inputs in order:
# anatomical image, motion correction and bold image  
# It outputs an html file including figures involved in quality control checking.

# still to do:  Add ability to do this for multiple bolds at once.
#		Plot movement over time.
#		template matching (overlay mni or talairach template and bold/hires)
#		output numerical overall SNR value.  
#		(maybe make snr figure's threshold dynamic depending on mean snr)
#		script should remove snr file (it's huge!)

set anat = $1
set motion = $2
set bold = $3


################# Signal to noise ################################
./fsl_snr.sh $bold # create tempSnr.nii.gz - voxelwise signal to noise
################# Plot the snr values on the anatomy #############
# make an overlay 3d image of the stat map for SNR
overlay 1 1 $anat -a tempSnr.nii.gz 50 150 temp_snr_overlay.nii # the 6 and 7th inputs indicate the thresholds for the overlay... chosen based on RestFDN data.  May need to be changed for new data.
# make 2d image
slicer temp_snr_overlay.nii -s 1 -x 0.35 sla -x 0.45 slb -x 0.55 slc -x 0.65 sld -y 0.35 sle -y 0.45 slf -y 0.55 slg -y 0.65 slh -z 0.35 sli -z 0.45 slj -z 0.55 slk -z 0.65 sll ; 
# convert to a pretty picture (gif)
convert -colors 100 +append sla slb slc sld sle slf slg slh sli slj slk sll SnrToHires.gif ; 
# remove all unused files
/bin/rm -f sla slb slc sld sle slf slg slh sli slj slk sll;

# Overlay anatomy lines on BOLD
# make 2d image
slicer $bold $anat -s 1 -x 0.35 sla -x 0.45 slb -x 0.55 slc -x 0.65 sld -y 0.35 sle -y 0.45 slf -y 0.55 slg -y 0.65 slh -z 0.35 sli -z 0.45 slj -z 0.55 slk -z 0.65 sll ; 
# convert to a pretty picture (gif)
convert -colors 100 +append sla slb slc sld sle slf slg slh sli slj slk sll HiresToBold.gif ; 
# remove all unused files
/bin/rm -f sla slb slc sld sle slf slg slh sli slj slk sll;

# To be added:  
# Make plots of movement over time

echo "making html file"
############## Make an html file with all the images in it #################
touch index.html
echo "<HTML><TITLE>Preprocessing Report - Subject "$bold"</TITLE><BODY>" > index.html

set DATE = `date`;  
set PWD = "file: '`pwd`'/index.html";
echo "$DATE $PWD"

# Put in the figures for the registration
echo "<hr><CENTER> <H1>Registration Report - $bold </H1><br> <H2>Target atlas - none</H2><br> </CENTER>" >>index.html
echo "<hr><p>Registration of BOLD data (underlying image) to initial_highres (red lines)<br><br>" >>index.html
echo "<a href="example_func2initial_highres.mat"><IMG BORDER=0 SRC="HiresToBold.gif"></a><br>" >>index.html
echo "<hr><p>Registration of highres (underlying image) to SNR (colors)<br><br>" >>index.html
echo "<a href="example_func2bhighres.mat"><IMG BORDER=0 SRC="SnrToHires.gif"></a><br>">>index.html
echo "<HR><FONT SIZE=-1>This report was produced automatically by makeqc_fig.sh $DATE<br> $PWD</FONT><br>" >>index.html

  echo "</BODY></HTML>" >> index.html
exit 0 ;

