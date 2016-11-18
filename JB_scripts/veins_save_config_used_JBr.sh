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
# Script: Save configs from a omnet.ini file
#
# last update: 18/11/2016
#
countIniFile=`ls *.ini | wc -l`

if [ "$countIniFile" == '' ]; then
    echo -e  "\nError: the \"omnet.ini\" not found\n"
elif [ $countIniFile -gt 1 ]; then
    echo -e "\nError: the has more than one \"omnet.ini\" file\n"
else # $countIniFile == 1
    # grep some values from omnet.ini file
    iniFileGrepAdd=`cat *.ini | grep -E "\*.\*.appl.|sim|rsu.*.mobility|txPower|pMax|^$|launchConfig"`
    #echo -e "##\n$iniFileGrepAdd\n##"

    # remove some value from grep previous
    iniFileGrepRm=`echo "$iniFileGrepAdd" | grep -v -E "^#|raffic|folder|cout|mobility.z"`
    #echo -e "##\n$iniFileGrepRm\n##"

    # replace multiple empty line with one empty line
    iniFileSedEmptyLine=`echo "$iniFileGrepRm" | sed '/^$/N;/^\n$/D'`
    #echo -e "##\n$iniFileSedEmptyLine\n##"

    echo -en "\nFile name to save the config: "
    read fileNameSave

    echo -e "\nType some information about this in this config: "
    read newInformationAdd

    echo -e "\n# Config saved #" > $fileNameSave
    echo -e "\nInformation about this config:\n$newInformationAdd" >> $fileNameSave
    echo -e "\n## Config\n$iniFileSedEmptyLine" >> $fileNameSave

    echo -e "\n\tConfig saved in the file $fileNameSave\n"
fi
