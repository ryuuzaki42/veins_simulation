#!/bin/bash
#
initialFolder=$PWD # Get the initial folder

dropboxFolderD="/media/sda4/prog/Dropbox/z_tmp_cemeai/veins_v001" # Dropbox destination folder
veinsFolderO="/media/sda4/prog/veins_simulation" # Veins origin folder

cd $veinsFolderO

#make clean

cd ../

echo -e "\nThis script create a \"veins_simulation\"(.zip) ($veinsFolderO) in one"
echo -e "Dropbox folder ($dropboxFolderD) with out log files\n"

rm $dropboxFolderD.zip # Delete older veins_v001.zip

echo "Zip the veins_simulation in veins_v001.zip"
zip -r $dropboxFolderD.zip veins_simulation/ -x *.zip\* veins_simulation/.git\* veins_simulation/out\* veins_simulation/src/libveins_simulation.so veins_simulation/projects/*.zip veins_simulation/projects/default_veins/\* veins_simulation/projects/range/\* veins_simulation/projects/*/results\* veins_simulation/projects/*/others\*

cd - $initialFolder > /dev/null # Back to initial folder
