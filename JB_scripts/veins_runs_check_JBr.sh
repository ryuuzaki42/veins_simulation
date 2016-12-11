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
# Obs: Need print "This simulation run terminated correctly" at
#  the end of our simulation to be checked
#
# Last update: 11/12/2016
#
fileRsuName="rsu_Count_Messages_Received.r"
fileRSUCheck=` find .| sort | grep $fileRsuName` # Get the result path files to rsu_Count_Messages_Received.r

if [ "$fileRSUCheck" == '' ]; then
    echo -e "\nError: Not found any file \"$fileRsuName\" in the sub directories\n"
else
    echo -e "\n    # Check the simulation runs status that terminated (correctly or with error) #\n"

    tmpFileRunCorrectly=`mktemp`
    tmpFilePathCorrectly=`mktemp`

    tmpFileRunError=`mktemp`
    tmpFilePathError=`mktemp`

    for lineFile in $fileRSUCheck; do # Read the fileRSUCheck line by line and set in lineFile
        runNumber=`echo "$lineFile" | rev | cut -d'/' -f2-4 | rev` # Get the run number

        if cat $lineFile | grep -q "This simulation run terminated correctly"; then # Test if terminated success
            echo "$lineFile" >> $tmpFilePathCorrectly
            echo -e "    $runNumber" >> $tmpFileRunCorrectly
        else
            echo "$lineFile" >> $tmpFilePathError
            echo -e "    $runNumber" >> $tmpFileRunError
        fi
    done

    experimentName=`echo $lineFile | rev | cut -d'/' -f5 | rev` # Get experiment name

    sizeResultFile=`ls -l $tmpFilePathCorrectly | awk '{print $5}'` # Get the size of result error file
    if [ "$sizeResultFile" != '0' ]; then # If result file is not empty
        echo -e "\nRun(s) terminated correctly:\n"
        cat $tmpFilePathCorrectly
        echo -e "\n\n$experimentName"
        cat $tmpFileRunCorrectly
    else
        echo -e "\n\n    #    All the simulation run(s) terminated with error    #\n"
    fi

    sizeResultFile=`ls -l $tmpFilePathError | awk '{print $5}'`
    if [ "$sizeResultFile" != '0' ]; then
        echo -e "\nRun(s) terminated with error:\n"
        cat $tmpFilePathError
        echo -e "\n\n$experimentName"
        cat $tmpFileRunError
    else
        echo -e "\n\n    #    All the simulation run(s) terminated correctly    #\n"
    fi

    echo
    rm $tmpFileRunCorrectly $tmpFilePathCorrectly
    rm $tmpFileRunError $tmpFilePathError
fi
