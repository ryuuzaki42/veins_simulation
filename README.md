**# Readme #**

**Projeto de mestrado - [ICMC USP](http://www.icmc.usp.br/) - João Batista**

**Programas utilizados:**
[**Veins**](http://veins.car2x.org/), [**OMNeT++**](http://omnetpp.org/) e [**SUMO**](http://sumo.dlr.de/)

## Versões dos simuladores
SUMO = 0.25
OMNeT = 4.6
Veins = 4.4

Setar no ~/.bashrc

## veins .bashrc
# Can be different in your PC
export PATH=$PATH:/media/sda4/prog/omnetpp-4.6/bin
export PATH=$PATH:/media/sda4/prog/veins_simulation/
export SUMO_HOME=/media/sda4/prog/sumo-0.25.0
alias veins_folder_cd='cd /media/sda4/prog/veins_simulation/'
export TCL_LIBRARY=/usr/lib64/tcl8.6

# Not change
alias veins_make='veins_folder_cd; make; cd -'
alias opp-exc='veins_make; opp_run -r 0 -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins_simulation.so'
alias opp-exc-res='veins_make; opp_run -r 0 -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins_simulation.so '$1' > veins_run_output.r'
alias opp-exc-all='veins_make; opp_run -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins_simulation.so'
alias opp-exc-all-res='veins_make; opp_run -n ../../src/veins/ -u Cmdenv -l ../../out/gcc-debug/src/libveins_simulation.so '$1' > veins_run_output.r'
