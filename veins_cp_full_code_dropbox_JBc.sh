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
# Last update: 20/10/2016
#
initialFolder=$PWD # Get the initial folder

dropboxFolderD="/media/sda4/prog/Dropbox/z_tmp_cemeai/veins_v01" # Dropbox destination folder
veinsFolderO="/media/sda4/prog/veins_simulation" # Veins origin folder

cd $veinsFolderO

make clean # Clean compiled files

cd ../

echo -e "\nThis script create a \"veins_simulation\"(.zip) ($veinsFolderO) in one"
echo -e "Dropbox folder ($dropboxFolderD) with out log files\n"

rm $dropboxFolderD.zip # Delete older veins_v01.zip

echo "Zip the veins_simulation in veins_v001.zip"
zip -r $dropboxFolderD.zip veins_simulation/ -x *.zip\* veins_simulation/.git\* veins_simulation/out\* veins_simulation/projects/*.zip veins_simulation/projects/default_veins/\* veins_simulation/projects/range/\* veins_simulation/projects/*/results\* veins_simulation/projects/*/others\*

cd - $initialFolder > /dev/null # Back to initial folder
