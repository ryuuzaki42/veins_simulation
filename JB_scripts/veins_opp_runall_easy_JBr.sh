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
# Script: Create the run a easy line to run in the opp_runall
#
# last update: 20/10/2016
#
if [ $# -lt 5 ]; then # Test at least tree parameters
    echo "Error in the parameters"
    echo "To run: veins_opp_runall_easy_JBr.sh -j3 -f omnet.ini -r 0..2"
    echo " veins_opp_runall_easy_JBr.sh - this script"
    echo " -j\"count of cores\" - number of cores to run"
    echo " -f omnet.ini - ini file of configuration"
    echo " -r 0..2 or -r runSart..runEnd, like -r 0..1 or -r 0..3,6..9"
else
    coreNumber=$1 # Like -j3 with 3 cores
    #$2 is -f
    iniFile=$3
    #$4 is -r
    runNumber=$5

    if [ -e $iniFile ]; then
        export TZ=America/Sao_Paulo

        fileResultGeneral="results/run_general.r"
        mkdir results 2> /dev/null # create the results folder if not exists

        SimulationStart=`date` # Start simulation
        SimulationStartSeconds=`date +%s`
        echo -e "\nSimulation staring at: $SimulationStart\n" | tee $fileResultGeneral

        opp_runall $coreNumber veins_opp_runall_JBr.sh -f $iniFile -r $runNumber

        #opp_runall -j3 veins_opp_runall_JBr.sh -f omnet.ini -r 0..2
        #opp_runall command, -j3 number cores, veins_opp_runall_JBr script to run opp_runall, -f omnet.ini file, -r 0..2 run to execute

        SimulationEnd=`date` # End simulation
        echo -e "Simulation ends at: $SimulationEnd" | tee -a $fileResultGeneral

        SimulationEndSeconds=`date +%s`
        SimulationDiffSeconds=`echo "($SimulationEndSeconds - $SimulationStartSeconds)" | bc`
        SimulationDiffMin=`echo "scale=2; $SimulationDiffSeconds/60" | bc`
        SimulationDiffHour=`echo "scale=2; $SimulationDiffSeconds/(60*60)" | bc`
        echo -e "\nTime spent in simulation: $SimulationDiffSeconds s - or $SimulationDiffMin min - or $SimulationDiffHour h\n" | tee -a $fileResultGeneral
    else
        echo -e "\nError: $iniFile not exists\n" | tee $fileResultGeneral
    fi
fi
