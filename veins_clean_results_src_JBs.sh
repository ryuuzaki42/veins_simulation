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
# Script: limpa o histórico de resultados de execução do framework Veins
#
# Última atualização: 26/09/2016
#
# Pasta do Veins (altere se a sua for diferente)
cd /media/sda4/prog/veins_simulation/

inputPar=$1

echo -e "\nEste script remove os results da pasta projects"
echo -e "\nIrá apagar os arquivos:"
echo -e "\tprojects/*/results"
echo -e "\tprojects/*/.tkenvrc"
echo -e "\tprojects/*/run.r"
echo -e "\tprojects/*/runExperimentOuptut.r"

echo -e "\nList of files that will be deleted:\n"
ls -r projects/*/results/
ls projects/*/.tkenvrc
ls projects/*/.tkenvlog
ls projects/*/run.r
ls projects/*/runExperimentOuptut.r

echo -en "\nDeseja continuar?\n(y)es - (n)o: "
if [ "$inputPar" != "y" ]; then
    read resposta
else
    resposta=y
fi

if [ "$resposta" = "y" ]; then # Altere projects para o nome da sua pasta de projetos
    rm -r projects/*/results/
    rm projects/*/.tkenvrc
    rm projects/*/.tkenvlog
    rm projects/*/run.r
    rm projects/*/runExperimentOuptut.r
    echo -e "\nOs arquivos foram apagados\n"
else
    echo -e "\nOs arquivos não foram apagados\n"
fi
echo -e "Fim do script\n"
