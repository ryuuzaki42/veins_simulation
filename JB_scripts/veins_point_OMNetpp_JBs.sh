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
# Script: Script to convert SUMO point to OMNeT++ points
#
# last update: 14/04/2017
#
echo -e "\n# Script to convert the SUMO point to OMNeT #\n"

if [ "$#" -eq '6' ]; then
    #$1 -n
    netFile=$2 # .net.xml
    # $3 -x
    xPoint=$4 # x point number
    # $5 -y
    yPoint=$6 # y point number

    echo -e "Getting the boundaries to $netFile\n"
    fileTmp="tmp_" # Create a tmp "net.xml" file
    fileTmp+=$(date +%s | md5sum | head -c 10) # Add some random part
    fileTmp+="net.xml"

    boundaryNetFile=$(netconvert -s "$netFile" -o "$fileTmp" -v | grep "Original boundary" | awk '{print $4}') # Run netconvert to get boundaries

    rm "$fileTmp" # Delete tmpfile

    boundaryXLeft=$(echo "$boundaryNetFile" | cut -d',' -f1) # Get x left
    boundaryYLeft=$(echo "$boundaryNetFile" | cut -d',' -f2) # Get y left
    boundaryXRight=$(echo "$boundaryNetFile" | cut -d',' -f3) # Get x right
    boundaryYRight=$(echo "$boundaryNetFile" | cut -d',' -f4) # Get y right

    echo -e "\nBoundary of $netFile: $boundaryNetFile or ($boundaryXLeft, $boundaryYLeft) to ($boundaryXRight, $boundaryYRight)"

    if [ "$boundaryXLeft" == "0.00" ] && [ "$boundaryYLeft" == "0.00" ]; then # Test if boundaryXLeft and boundaryYLeft are 0.00
        xFinalPos=$(echo "scale=2; ($xPoint)" | bc)
        yFinalPos=$(echo "scale=2; ($boundaryYRight - $yPoint)" | bc)

        echo -e "\nNetwork file: $netFile\nPoints ($xPoint, $yPoint)\nIn the  in the OMNeT++: ($xFinalPos, $yFinalPos)\n"
    else
        echo -e "\n\n\tError: Script not implemented with (x,y) : ($boundaryXLeft $boundaryYLeft) values\n"
    fi
else
    echo -e "Error parameters - Try use: $0 -n file.net.xml -x \"xPoint\" -y \"yPoint\"\n"
fi
