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
# last update: 17/10/2016
#
if ls -l | grep -q "\.ini"; then
    echo -e "\nGreat is a OMNet project folder (with a ini file)"

    if echo $PWD | grep -q "vehDist"; then
        echo "Is a vehDist experiment"
        experimentName="vehDist"
    elif echo $PWD | grep -q "epidemic"; then
        echo "Is a epidemic experiment"
        experimentName="epidemic"
    else
        echo -e "\n$PWD\n\nThis OMNet is unknown"
        experimentName=''
    fi

    if [ "$experimentName" != '' ]; then
        #cd /media/sda4/prog/simulation_veins/projects/vehDist/others/
        # or
        ## Cluster folder, change de v001 for the veins version
        #cd /mnt/nfs/home/luz.marina/0_jonh/veins_v00$part/projects/vehDist$part/others/

        part=$1
        if [ "$part" == '' ]; then # result_part
            echo -e "\nError, you need pass the result_part value, e.g., $0 \"1\" 1 1\n"
            exit 1
        fi

        numExpI_1to8=$2
        if [ "$numExpI_1to8" == '' ]; then # start experiment number
            echo -e "\nError, you need pass the start experiment number (1 to 8) value, e.g., $0 1 \"1\" 1\n"
            exit 1
        fi

        numExpF_1to8=$3
        if [ "$numExpF_1to8" == '' ]; then # finish experiment number
            echo -e "\nError, you need pass the finish experiment number (1 to 8) value, e.g., $0 1 1 \"1\"\n"
            exit 1
        fi

        numExpF_DSCR=$4
        if [ "$numExpF_DSCR" == '' ]; then # total of experiments, for vehDist are 6, for epidemic are 1
            numExpF_DSCR=6
        fi

        if [ "$numExpF_1to8" -lt "$numExpI_1to8" ]; then
            numExpF_1to8=$numExpI_1to8
        fi

        echo -e "\nNumber of experiments: $numExpI_1to8 to $numExpF_1to8"
        ((numExpF_1to8++))

        echo -e "\n## Script to collect the simulation results in one place ##"

        if [ "$experimentName" == "vehDist" ]; then
            pathFolder="results/vehDist_resultsEnd_$part"
        elif [ "$experimentName" == "epidemic" ]; then
            pathFolder="results/epidemic_resultsEnd_$part"
            numExpF_DSCR=1
            experiment="0099_epidemic"
        fi
        echo -e "\n\t## Results from $pathFolder ##\n"

        rsu0File="rsu\[0\]_Count_Messages_Received.r"
        vehiclesFile="Veh_Messages_Drop.r"

        experimentNumber=1
        continueFlag=1
        while [ $continueFlag == 1 ]; do
            if [ "$experimentName" == "vehDist" ]; then
                case $experimentNumber in
                    1) experiment="0001_chosenByDistance" ;;
                    2) experiment="0012_chosenByDistance_Speed" ;;
                    3) experiment="0013_chosenByDistance_Category" ;;
                    4) experiment="0014_chosenByDistance_RateTimeToSend" ;;
                    5) experiment="0123_chosenByDistance_Speed_Category" ;;
                    6) experiment="1234_chosenByDistance_Speed_Category_RateTimeToSend" ;;
                esac
            fi

            echo -e "## Values from experiment $experiment ##"
            i=$numExpI_1to8
            while [ $i -lt $numExpF_1to8 ]; do
                j=0
                continueFlag2=1

                while [ $continueFlag2 == 1 ]; do
                    echo -e "\n\t\t## Experiment $experiment $i run_$j ##"

                    fileRSU=`ls $pathFolder/$experiment/E"$i"_*/run_$j/$rsu0File`
                    echo -e "\t\t\tFile: $fileRSU\n"

                    cat $pathFolder/$experiment/E"$i"_*/run_$j/$rsu0File | grep -E "Exp: "$i""
                    echo

                    fileVeh=`ls $pathFolder/$experiment/E"$i"_*/run_$j/$vehiclesFile`
                    echo -e "\t\t\tFile: $fileVeh\n"

                    cat $pathFolder/$experiment/E"$i"_*/run_$j/$vehiclesFile | grep -E "Exp: $i"
                    ((j++))

                    if [ -e $pathFolder/$experiment/E$i_*/run_$j/$rsu0File ]; then
                        continueFlag2=0
                    fi
                done
                ((i++))
            done

            echo "experimentNumber $experimentNumber numExpF_DSCR $numExpF_DSCR"
            if [ $experimentNumber == $numExpF_DSCR ]; then
                continueFlag=0
            fi
            ((experimentNumber++))
        done
    fi
else
    echo -e "\nError: ($PWD) is not a OMNet++ project folder.\nPlease go the folder with a ini file"
fi
echo -e "\n\t\t## End of script ##\n"