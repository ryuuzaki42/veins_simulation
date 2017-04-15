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
# Obs: You need veins_opp_runall_JBr.sh
#
# last update: 14/04/2017
#
if [ "$#" -lt 5 ]; then # Test at least tree parameters
    echo -e "\nError in the parameters"
    echo "To run: veins_opp_runall_easy_JBr.sh -j3 -f omnet.ini -r 0..2"
    echo " veins_opp_runall_easy_JBr.sh - this script"
    echo " -j\"count of cores\" - number of cores to run"
    echo " -f omnet.ini - ini file of configuration"
    echo -e " -r 0..2 or -r runSart..runEnd, like -r 0..1 or -r 0..3,6..9\n"
else
    coreNumber=$1 # Like -j3 with 3 cores
    #$2 is -f
    iniFile=$3
    #$4 is -r
    runNumber=$5

    if [ -e "$iniFile" ]; then
        export TZ=America/Sao_Paulo # Set timezone to America/Sao_Paulo

        fileResultGeneral="results/run_general.r"
        mkdir results 2> /dev/null # create the results folder if not exists

        SimulationStart=$(date) # Start simulation
        SimulationStartSeconds=$(date +%s)
        echo -e "\nSimulation staring at: $SimulationStart\n" | tee "$fileResultGeneral"

        opp_runall "$coreNumber" veins_opp_runall_JBr.sh -f "$iniFile" -r "$runNumber"
        # Example: opp_runall -j3 veins_opp_runall_JBr.sh -f omnet.ini -r 0..2
        # opp_runall command from OMNeT
        # -j3 j(number cores) to be used
        # veins_opp_runall_JBr script to create a line run to opp_runall
        # -f omnet.ini file
        # -r 0..2 run to execute

        SimulationEnd=$(date) # End simulation
        echo -e "Simulation ends at: $SimulationEnd" | tee -a "$fileResultGeneral"

        SimulationEndSeconds=$(date +%s)
        SimulationDiffSeconds=$(echo "($SimulationEndSeconds - $SimulationStartSeconds)" | bc)
        SimulationDiffMin=$(echo "scale=2; $SimulationDiffSeconds/60" | bc)
        SimulationDiffHour=$(echo "scale=2; $SimulationDiffSeconds/(60*60)" | bc)
        echo -e "\nTime spent in simulation: $SimulationDiffSeconds s - or $SimulationDiffMin min - or $SimulationDiffHour h\n" | tee -a "$fileResultGeneral"
    else
        echo -e "\nError: $iniFile not exists\n" | tee "$fileResultGeneral"
    fi
fi
