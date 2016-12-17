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
# Script: Grep result from a file
#
# Last update: 17/12/2016
#
fileName=$1
if [ "$fileName" == '' ]; then
    echo "Error - need pass the file name to grep"
else
    op1="Total of messages received:"
    op2="Total count copy message received:"
    op3="Total average time to receive:"
    op4="Total average copy received:"
    op5="Total average first hops to received:"
    op6="Total average hops to received:"
    op7="counttoDeliveryMsg:"

    op8="Final count messages drop:"
    op9="Final average buffer use:"

    echo -e "\nRSU:"
    echo "    1 - $op1"
    echo "    2 - $op2"
    echo "    3 - $op3"
    echo "    4 - $op4"
    echo "    5 - $op5"
    echo "    6 - $op6"
    echo "    7 - $op7"
    echo "Veh:"
    echo "    8 - $op8"
    echo "    9 - $op9"
    echo "Wich value you want grep?"
    read optionInput

    opTmp="op$optionInput"
    grepValue=`echo ${!opTmp}`

    if [ "$grepValue" == '' ]; then
        echo -e "\nError - option \"$optionInput\" is unknown"
    else
        grepResult=`cat $fileName | grep "$grepValue"`

        echo -e "\n\n$grepResult"

        echo -e "\n\nValues more clean:"
        echo "$grepResult" | rev | cut -d ':' -f1 | rev | cut -d ' ' -f2-
    fi
    echo
fi