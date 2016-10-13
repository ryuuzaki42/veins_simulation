#!/bin/bash
#
# Autor= João Batista Ribeiro
# Bugs, Agradecimentos, Criticas "construtiva"
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
# Última atualização: 13/10/2016
#
if [ $# -lt 3 ]; then # Test at least tree oarametrs
    echo "Error in the parrametrs"
    echo "To run: opp_runall -j2 veins_opp_runall.sh omnet.ini -r 0..1"
    echo " opp_runall - opp_runall command"
    echo " -j\"count of cores\" - number of cores to run"
    echo " veins_opp_runall.sh - this script"
    echo " omnet.ini - ini file of configuration"
    echo " -r runSart..runEnd, like -r 0..1 or -r 0..3,6..9"
else
    iniFile=$1
    #$2 is -r
    runNumber=$3
    fileResult="results/run_"$runNumber".r"
    mkdir results 2> /dev/null # create the results folder if not exists

    dateStart=`date` # Start runing experiment
    echo -e "Experiments staring at: $dateStart" > $fileResult
    echo -e "\nRunning: opp_run -u Cmdenv -n ../../src/veins/ -l ../../src/libveins_simulation.so -f $iniFile -r $runNumber >> $fileResult\n" | tee -a $fileResult

    opp_run -u Cmdenv -n ../../src/veins/ -l ../../src/libveins_simulation.so -f $iniFile -r $runNumber >> $fileResult # runing experiment

    echo -e "\n\nExperiment run $runNumber starts date: $dateStart" | tee -a $fileResult
    echo -e "Experiment run $runNumber ends date: `date`\n" | tee -a $fileResult
fi
