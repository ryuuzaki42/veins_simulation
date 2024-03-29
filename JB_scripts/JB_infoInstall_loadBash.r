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
# Script: Script that load configuration Veins/OMNeT and SUMO needed
#
# Last update: 16/12/2017
#
## Veins add in ~/.bashrc - Path can be different in your PC
#. /media/sda2/prog/git_clone/8_veins_simulation/JB_scripts/JB_infoInstall_loadBash.r # copy and uncomment (don't remove the . (dot))

## Simulators version
#   SUMO = 0.25.0
#   OMNeT = 4.6
#   Veins = 4.4

## Veins run by ~/.bashrc - Can be different in your PC
TCL_LIBRARY="/usr/lib64/tcl8.6/"
SUMO_HOME="/opt/sumo-0.25.0/"
omnetFolder="/opt/omnetpp-4.6/bin/"
veinsFolder="/media/sda2/prog/git_clone/8_veins_simulation/"

## No need of change in your PC
export PATH=$PATH:$TCL_LIBRARY
export PATH=$PATH:$SUMO_HOME
export PATH=$PATH:"$SUMO_HOME/bin/"
export PATH=$PATH:$omnetFolder

export PATH=$PATH:$veinsFolder
export PATH=$PATH:"$veinsFolder/JB_scripts/"
export veinsFolder
alias veins_folder_cd='cd $veinsFolder'

veins_RunExperiment() {
    dateStart=$(date) # Start running experiments
    echo -e "\\nExperiments staring at: $dateStart"

    # get initial variables
    iniFile=$1
    runExperimentI=$2
    runExperimentF=$3

    # cd veins folder, make and go back
    veins_folder_cd
    echo -e "\\nMake the files in the folder \"$(pwd)\"\\n"
    make
    cd - || exit

    if [ "$runExperimentI" == '' ]; then # test the input, if is '' will run -r 0, if is valid, run -r I..F
        runExperimentI='0'
        runExperimentF='0'
    fi

    echo -e "\\nRunning: opp_run -r $runExperimentI..$runExperimentF -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins_simulation.so $iniFile\\n"
    opp_run -r "$runExperimentI..$runExperimentF" -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins_simulation.so "$iniFile"

    echo -e "\\n\\nExperiment starts date: $dateStart"
    echo -e "Experiment ends date: $(date)\\n" # End of running experiment
}
alias opp-exc=veins_RunExperiment
alias opp-exc-res="veins_RunExperiment > results/ExperimentOutput.r"
