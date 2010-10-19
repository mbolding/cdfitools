clear
F=dir('epidw*.hdr');
numF=size(F,1);

for k = 1:numF
    disp(F(k).name)
    nii(k) = load_nii(F(k).name);
end

avnii=nii(1);

for  k = 2:numF
    avnii.img = avnii.img + nii(k).img;
end

avnii.img = avnii.img/numF;
view_nii(avnii);
save_nii(avnii,'avnii.hdr')
