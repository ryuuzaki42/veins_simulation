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
# Last update: 20/10/2016
#
initialFolder=$PWD # Get the initial folder

dropboxFolderDest="/media/sda4/prog/Dropbox/z_tmp_cemeai/veins_v001" # Dropbox destination folder
veinsFolderOri="/media/sda4/prog/veins_simulation" # Veins origin folder

echo -e "\nThis script copy the \"veins_simulation\" ($veinsFolderOri) to one folder"
echo -e "in Dropbox ($dropboxFolderDest) and delete sensitive data\n"

rm -r $dropboxFolderDest/ # Delete older veins_v001 folder
rm $dropboxFolderDest.zip # Delete older veins_v001.zip

mkdir $dropboxFolderDest/ # Create a new veins_v001 folder

cp $veinsFolderOri/sumo-launchd.py $dropboxFolderDest/ # Copy sumo-launchd.py to veins_v001

cp $veinsFolderOri/*JBs.sh $dropboxFolderDest/ # Copy all *JBs.sh to veins_v001

cp -r $veinsFolderOri/out/ $dropboxFolderDest/ # Copy the folder out to veins_v001

mkdir $dropboxFolderDest/projects # Create a folder projects in the veins_v001

cp -r $veinsFolderOri/projects/0scenarios/ $dropboxFolderDest/projects/  # Copy 0sceenarios to veins_v001
cp -r $veinsFolderOri/projects/1node/ $dropboxFolderDest/projects/       # Copy 1node to veins_v001
cp -r $veinsFolderOri/projects/epidemic/ $dropboxFolderDest/projects/    # Copy epidemic to veins_v001
cp -r $veinsFolderOri/projects/vehDist/ $dropboxFolderDest/projects/     # Copy vehDist to veins_v001

rm -r $dropboxFolderDest/projects/vehDist/others/ # Delete the others from vehDist in the veins_v001

cp -r $veinsFolderOri/src/ $dropboxFolderDest/ # Copy the src to veins_v001

cd $dropboxFolderDest/src/ # Move to src veins_v001 folder

delCC=`find . | grep "\.cc"` # Get all .cc files
rm $delCC # Delete the .cc files

delH=`find . | grep "\.h"` # Get all .h files
rm $delH # Delete the .h files

dirEmpty=`find . -type d -empty` # Get all empty directories
rmdir $dirEmpty # Delete the empty directories

cd ../../ # Move up to nivels

echo "Zip the veins_v001 in veins_v001.zip"
zip -r veins_v001.zip veins_v001/

cd - $initialFolder > /dev/null # Back to initial folder
