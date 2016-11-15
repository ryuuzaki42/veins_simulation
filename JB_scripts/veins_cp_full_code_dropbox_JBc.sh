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
# Script: Cria um versão zipada do pasta veins_simulation em um pasta
# no Dropbox (dropboxFolderD)
#
# Last update: 15/11/2016
#
echo -e "\nThis script create a \"veins_simulation\"(.zip) ($veinsFolderO) in one"
echo -e "Dropbox folder ($dropboxFolderD) with out log files\n"

initialFolder=$PWD # Get the initial folder
dropboxFolderD="/media/sda4/prog/Dropbox/z_share_code_JB" # Dropbox destination folder
veinsFolderO="/media/sda4/prog/veins_simulation" # Veins origin folder

veinsVersion="veins_v01-full"

rm $dropboxFolderD/$veinsVersion.zip 2> /dev/null # Delete older $veinsVersion.zip if exists

cd $veinsFolderO/../ # Go to one up to veinsFolderO

echo "Zip the veins_simulation in $veinsVersion.zip without \"projects/*/results\" \"*.zip\" and \".git\""
zip -r $dropboxFolderD/$veinsVersion.zip veins_simulation/ -x *.zip\* *.tkenvrc\* veins_simulation/.git\* veins_simulation/projects/*/results\*

cd $dropboxFolderD
rm -r veins_simulation $veinsVersion 2> /dev/null # Delete the older veins_simulation and $veinsVersion

unzip $veinsVersion.zip # Unzip/Extract the $veinsVersion.zip in the Dropbox folder
rm $veinsVersion.zip # Delete the last $veinsVersion.zip

mv veins_simulation $veinsVersion # Rename the folder to veinsVersion
zip -r $veinsVersion.zip $veinsVersion # Zip $veinsVersion in $veinsVersion.zip
rm -r $veinsVersion # Delete de $veinsVersion folder

cd $initialFolder # Back to initial folder

echo -e "\n\n\tEnd script\n"
