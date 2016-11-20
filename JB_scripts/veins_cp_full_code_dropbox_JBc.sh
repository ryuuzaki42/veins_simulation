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
# no Dropbox (dropboxFolderDest)
#
# Last update: 20/11/2016
#
dropboxFolderDest="/media/sda4/prog/Dropbox/z_share_code_JB" # Dropbox destination folder
veinsFolderOri=$veinsFolder # veinsFolder (change in your ~/.bashrc)
veinsNameFolder=`echo $veinsFolder | rev | cut -d"/" -f1 | rev`
veinsVersion="veins_v01-full"

echo -e "\nThis script create a \"$veinsVersion.zip\" from ($veinsFolderOri/)"
echo -e "in the Dropbox ($dropboxFolderDest/) with out log files\n"

rm $dropboxFolderDest/$veinsVersion.zip 2> /dev/null # Delete older $veinsVersion.zip if exists

cd $veinsFolderOri/../ # Go to one up to veinsFolderOri

echo "Zip the veins_simulation in $veinsVersion.zip without \"projects/*/results\" \"*.tkenvrc\" \"*.zip\" and \".git\""
zip -r $dropboxFolderDest/$veinsVersion.zip $veinsNameFolder/ -x *.zip\* *.tkenvrc\* $veinsNameFolder/.git\* $veinsNameFolder/projects/*/results\* $veinsNameFolder/src/*.so\*

cd $dropboxFolderDest
rm -r $veinsNameFolder $veinsVersion 2> /dev/null # Delete the older veins_simulation and $veinsVersion

unzip $veinsVersion.zip # Unzip/Extract the $veinsVersion.zip in the Dropbox folder
rm $veinsVersion.zip # Delete the last $veinsVersion.zip

mv $veinsNameFolder $veinsVersion # Rename the folder to veinsVersion

zip -r $veinsVersion.zip $veinsVersion # Zip $veinsVersion in $veinsVersion.zip
echo -e "\nThe \"$veinsVersion.zip\" was created in \"$veinsFolderOri\"\n"
rm -r $veinsVersion # Delete de $veinsVersion folder
