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
# Script: Run SUMO (sumo ou sumo-gui) as the framework Veins need
#
# last update: 14/04/2017
#
echo -e "\npython sumo-launchd.py -vv -c (\"sumo-gui\"|\"sumo\")"
echo -e "\nWant run sumo-gui or sumo?"
echo -en "(y)es to sumo-gui - (n)o to sumo: "
read -r sumoRun

cd "$veinsFolder" || exit # veinsFolder (change in your ~/.bashrc)
if [ "$sumoRun" = 'y' ]; then
    echo -e "\nRunning sumo-gui\n"
    python sumo-launchd.py -vv -c sumo-gui
else
    echo -e "\nRunning sumo\n"
    python sumo-launchd.py -vv -c sumo
fi
