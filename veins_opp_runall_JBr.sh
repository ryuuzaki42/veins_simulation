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
# Script: Create the run line to run in the opp_runall
#
# last update: 20/10/2016
#
if [ $# -lt 4 ]; then # Test at least tree parameters
    echo "Error in the parameters"
    echo "To run: opp_runall -j3 veins_opp_runall_JBr.sh -f omnet.ini -r 0..2"
    echo " opp_runall - opp_runall command"
    echo " -j\"count of cores\" - number of cores to run"
    echo " veins_opp_runall_JBr.sh - this script"
    echo " -f omnet.ini - ini file of configuration"
    echo " -r 0..2 or -r runSart..runEnd, like -r 0..1 or -r 0..3,6..9"
else
    #$1 is -f
    iniFile=$2
    #$3 is -r
    runNumber=$4

    if [ -e $iniFile ]; then
        export TZ=America/Sao_Paulo

        fileResult="results/run_"$runNumber".r"
        mkdir results 2> /dev/null # create the results folder if not exists

        dateStart=`date` # Start running experiment
        dateStartSeconds=`date +%s`
        echo -e "\nExperiment run $runNumber staring at: $dateStart" | tee $fileResult

        echo -e "\nRunning: opp_run -u Cmdenv -n ../../src/veins/ -l ../../src/libveins_simulation.so -f $iniFile -r $runNumber >> $fileResult\n" | tee -a $fileResult
        opp_run -u Cmdenv -n ../../src/veins/ -l ../../src/libveins_simulation.so -f $iniFile -r $runNumber >> $fileResult # running experiment

        dateEnd=`date` # End running experiment
        echo -e "\nExperiment run $runNumber starts at: $dateStart" | tee -a $fileResult
        echo -e "Experiment run $runNumber ends at: $dateEnd" | tee -a $fileResult

        dateEndSeconds=`date +%s`
        dateDiffSeconds=`echo "($dateEndSeconds - $dateStartSeconds)" | bc`
        dateDiffMin=`echo "scale=2; $dateDiffSeconds/60" | bc`
        dateDiffHour=`echo "scale=2; $dateDiffSeconds/(60*60)" | bc`
        echo -e "Time spent in run $runNumber: $dateDiffSeconds s - or $dateDiffMin min - or $dateDiffHour h\n" | tee -a $fileResult
    else
        echo -e "\nError: $iniFile not exists\n" | tee $fileResult
    fi
fi
