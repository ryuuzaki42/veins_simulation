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
# Script: Script to collect the simulation result in one place
#
# last update: 10/01/2018
#
echo -e "\\n## Script to collect the simulation results in one place ##"

if find . | grep -q "\\.ini"; then
    echo -e "\\nGreat, looks like it ($PWD) is a OMNeT project folder (with a ini file)"

    noConfig=$1
    if [ "$noConfig" == "noConfig" ]; then
        shift
    fi

    part=$1
    if [ "$part" == '' ]; then # result_part
        echo -e "\\nError, you need pass the result_part value, e.g., $0 \"1\" 1 1\\n"
        exit 1
    fi

    numExpI_1to8=$2
    if [ "$numExpI_1to8" == '' ]; then # start experiment number
        echo -e "\\nError, you need pass the start experiment number (1 to 8) value, e.g., $0 1 \"1\" 1\\n"
        exit 1
    fi

    numExpF_1to8=$3
    if [ "$numExpF_1to8" == '' ]; then # finish experiment number
        echo -e "\\nError, you need pass the finish experiment number (1 to 8) value, e.g., $0 1 1 \"1\"\\n"
        exit 1
    fi

    if [ "$numExpF_1to8" -lt "$numExpI_1to8" ]; then
        numExpF_1to8=$numExpI_1to8
    fi

    if echo "$PWD" | grep -q "mfcv"; then
        pathFolder="results/mfcv_resultsEnd_$part"
        echo -e "\\n    It is a mfcv experiment ($pathFolder)"
    elif echo "$PWD" | grep -q "epidemic"; then
        pathFolder="results/epidemic_resultsEnd_$part"
        echo -e "\\n    It is a epidemic experiment ($pathFolder)"
    else
        echo -e "\\n    The ($PWD) is OMNeT project is unknown for this script\\nexiting"
        exit 1
    fi
    rsuFile="rsu_Count_Messages_Received.r"
    vehiclesFile="Veh_Messages_Drop.r"

    echo -e "\\nNumber of experiments: $numExpI_1to8 to $numExpF_1to8"
    for i in $(seq "$numExpI_1to8" "$numExpF_1to8"); do
        for experiment in $(tree -fdi --noreport "$pathFolder" | grep "E$i.*run"); do
            echo -e "\\n## Working in the folder: $experiment"

            rsuFileLs=$(find "$experiment/$rsuFile")
            vehiclesFileLs=$(find "$experiment/$vehiclesFile")

            if [ "$rsuFileLs" != '' ]; then
                echo -e "\\n            File: $rsuFileLs"
                grep "Exp:" < "$rsuFileLs"

                filesFound+="    $experiment/$rsuFile\\n"
            else
                echo -e "\\nThe experiment: $experiment\\n Don't have the file \"$rsuFileLs\""
                filesNotFound+="    $experiment/$rsuFile\\n"
            fi

            if [ "$vehiclesFileLs" != '' ]; then
                echo -e "\\n            File: $vehiclesFileLs"
                grep "Exp:" < "$vehiclesFileLs"

                filesFound+="    $experiment/$vehiclesFile\\n"
            else
                echo -e "\\nThe experiment: $experiment\\n Don't have the file \"$vehiclesFileLs\""
                filesNotFound+="    $experiment/$vehiclesFile\\n"
            fi
        done
    done

    if [ "$filesFound" != '' ]; then
        echo -e "\\n# Result - Files found:\\n\\n$filesFound"
    else
        echo -e "\\n# No file found"
    fi

    if [ "$filesNotFound" != '' ]; then
        echo -e "\\n# Result - Files not found:\\n\\n$filesNotFound"
    else
        echo -e "\\n# All files found"
    fi
else
    echo -e "\\nError: ($PWD) it is not a OMNeT project folder\\nPlease go to the folder with a ini file\\n"
    exit 1
fi

if [ "$noConfig" != "noConfig" ]; then
    veins_save_config_used_JBr.sh inTheTerminal
fi
echo -e "\\n        ## End of script ##\\n"
