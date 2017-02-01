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
# Script: limpa o histórico de resultados de execução do framework Veins
#
# Last update: 01/02/2017
#
cd $veinsFolder # veinsFolder (change in your ~/.bashrc)

echo -e "\nThis script remove/delete the results inside folder projects"
echo -e "\nWill delete:"
echo -e "\tprojects/*/results"
echo -e "\tprojects/*/.tkenvrc"
echo -e "\tprojects/*/run.r"
echo -e "\tprojects/*/runExperimentOuptut.r"

echo -e "\nList of file(s) that will be deleted:\n"

fileDelete=`find projects/*/results/ 2> /dev/null | sort && echo`
fileDelete=$fileDelete`find projects/*/.tkenvrc 2> /dev/null && echo`
fileDelete=$fileDelete`find projects/*/.tkenvlog 2> /dev/null && echo`
fileDelete=$fileDelete`find projects/*/run.r 2> /dev/null && echo`
fileDelete=$fileDelete`find projects/*/runExperimentOuptut.r 2> /dev/null && echo`

echo "$fileDelete"

if [ "$fileDelete" == '' ]; then
    echo "   ### No file to be deleted - already clean ###"
else
    echo -en "\nWant continue? (y)es - (n)o: "
    read continueOrNot

    if [ "$continueOrNot" = 'y' ]; then # Change "projects" for your project folder
        rm -r projects/*/results/ 2> /dev/null
        rm projects/*/.tkenvrc 2> /dev/null
        rm projects/*/.tkenvlog 2> /dev/null
        rm projects/*/run.r 2> /dev/null
        rm projects/*/runExperimentOuptut.r 2> /dev/null
        echo -e "\nThe files was deleted"
    else
        echo -e "\nThe files was not deleted"
    fi
fi
echo -e "\nEnd script\n"
