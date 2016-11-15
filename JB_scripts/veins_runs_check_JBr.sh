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
# Script: Check if the all simulation run(s) terminated correct
#
# Last update: 15/11/2016
#
fileRsuName="rsu_Count_Messages_Received.r"
fileRSUCheck=` find .| sort | grep $fileRsuName` # Get the result path files to rsu_Count_Messages_Received.r

if [ "$fileRSUCheck" == '' ]; then
    echo -e "\nError: Not found any file \"$fileRsuName\" in the sub directories\n"
else
    echo -e "\nPrint the simulation runs that terminated with error\n"

    tmpFileRun=`mktemp` # Create tmp files
    tmpFilePath=`mktemp`

    for lineFile in $fileRSUCheck; do # Read the fileRSUCheck line by line and set in lineFile
        if ! cat $lineFile | grep -q "This simulation run terminated correctly"; then # Test if terminated success
            echo "$lineFile" >> $tmpFilePath # Print the link in the tmpFilePath
            runNumber=`echo "$lineFile" | rev | cut -d'/' -f2-3 | rev` # Get the run number
            echo -e "\t$runNumber" >> $tmpFileRun
        fi
    done

    experimentName=`echo $lineFile | rev | cut -d'/' -f4 | rev` # Get experiment name

    sizeResultFile=`ls -l $tmpFilePath | awk '{print $5}'` # Get the size of result file
    if [ "$sizeResultFile" != '0' ]; then # If result file is not empty
        echo -e "Path of run(s) terminated with error:\n"
        cat $tmpFilePath
        echo -e "\n\n$experimentName"
        cat $tmpFileRun
    else
        echo -e "\tAll the simulation run(s) terminated correctly\n"
    fi

    rm $tmpFileRun $tmpFilePath # Delete tmp files
fi
