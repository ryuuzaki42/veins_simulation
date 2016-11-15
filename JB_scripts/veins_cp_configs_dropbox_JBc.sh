#!/bin/bash
#
# Autor= João Batista Ribeiro
# Bugs, Agradecimentos, Críticas "construtivas"
# Mande me um e-mail. Ficarei Grato!
# e-mail: joao42lbatista@gmail.com
#
# Este programa é um software livre; você pode redistribui-lo e/ou
# modifica-lo dentro dos termos da Licença Pública Geral GNU como
# publicada pela Fundação do Software Livre (FSF); na versão 2 da
# Licença, ou (na sua opinião) qualquer versão.
#
# Este programa é distribuído na esperança que possa ser útil,
# mas SEM NENHUMA GARANTIA; sem uma garantia implícita de ADEQUAÇÃO a
# qualquer MERCADO ou APLICAÇÃO EM PARTICULAR.
#
# Veja a Licença Pública Geral GNU para maiores detalhes.
# Você deve ter recebido uma cópia da Licença Pública Geral GNU
# junto com este programa, se não, escreva para a Fundação do Software
#
# Livre(FSF) Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
#
# Script: Cria uma pasta no Dropbox ($dropboxFolderDest) com as
# configurações necessárias (*.ini, *.ned etc) e depois zip essa pasta (veins_v001.zip)
#
# Last update: 15/11/2016
#
initialFolder=$PWD # Get the initial folder

dropboxFolderDest="/media/sda4/prog/Dropbox/z_share_code_JB" # Dropbox destination folder
veinsFolderOri="/media/sda4/prog/veins_simulation" # Veins origin folder

veinsVersion="veins_v01-configs"

echo -e "\nThis script copy the \"veins_simulation\" ($veinsFolderOri) to one folder"
echo -e "in Dropbox ($dropboxFolderDest) and delete sensitive data\n"

cd $dropboxFolderDest
folderCP=$dropboxFolderDest/$veinsVersion

rm -r $veinsVersion $veinsVersion.zip 2> /dev/null # Delete older veins_v001 folder and $veinsVersion.zip  if exists

mkdir $veinsVersion/ # Create a new veins_v001 folder

cp $veinsFolderOri/sumo-launchd.py $folderCP/ # Copy sumo-launchd.py to veins_v001
cp -r $veinsFolderOri/JB_scripts/ $folderCP/ # Copy JB_scripts to veins_v001
cp -r $veinsFolderOri/out/ $folderCP/ # Copy the folder out to veins_v001

mkdir $folderCP/projects # Create a folder projects in the veins_v001
cp -r $veinsFolderOri/projects/0scenarios/ $folderCP/projects/  # Copy 0sceenarios to veins_v001
cp -r $veinsFolderOri/projects/1node/ $folderCP/projects/       # Copy 1node to veins_v001
cp -r $veinsFolderOri/projects/epidemic/ $folderCP/projects/    # Copy epidemic to veins_v001
cp -r $veinsFolderOri/projects/vehDist/ $folderCP/projects/     # Copy vehDist to veins_v001

rm -r $folderCP/projects/vehDist/others/ # Delete the others from vehDist in the veins_v001
rm -r $folderCP/projects/*/results/ # Delete results in the veins_v001

cp -r $veinsFolderOri/src/ $folderCP/ # Copy the src to veins_v001

cd $folderCP/src/ # Move to src veins_v001 folder

delCC=`find . | grep "\.cc"` # Get all .cc files
rm $delCC # Delete the .cc files

delH=`find . | grep "\.h"` # Get all .h files
rm $delH # Delete the .h files

dirEmpty=`find . -type d -empty` # Get all empty directories
rmdir $dirEmpty # Delete the empty directories

cd ../../ # Move up to nivels

echo "Zip the $veinsVersion in $veinsVersion.zip"
zip -r $veinsVersion.zip $veinsVersion/
rm -r $veinsVersion

cd $initialFolder # Back to initial folder
