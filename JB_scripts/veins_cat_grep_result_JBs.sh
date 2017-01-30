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
# Last update: 29/01/2017
#
echo -e "\nThis script grep values from a result from a file"

fileName=$1
if [ "$fileName" == '' ]; then
    echo -e "\nError - need pass the file name to grep\n"
else
    op1="Total of messages received:"
    op2="Total count copy message received:"
    op3="Total average time to receive:"
    op4="Total average copy received:"
    op5="Total average first hops to received:"
    op6="Total average hops to received:"
    op7="counttoDeliveryMsg:"
    op8="counttoDeliveryMsgUnicID:"
    op9="msgUseOnlyDeliveryBufferGeneral total:"
    op10="msgUseOnlyDeliveryBufferGeneral avg:"

    op11="Final count messages drop:"
    op12="Final average buffer use:"
    op13="Final buffer use total general:"


    optionInput=$2
    if [ "$optionInput" == '' ]; then
        echo -e "\nRSU"
        echo "    1 - $op1"
        echo "    2 - $op2"
        echo "    3 - $op3"
        echo "    4 - $op4"
        echo "    5 - $op5"
        echo "    6 - $op6"
        echo "    7 - $op7"
        echo "    8 - $op8"
        echo "    9 - $op9"
        echo "    10 - $op10"

        echo "Veh"
        echo "    11 - $op11"
        echo "    12 - $op12"
        echo "    13 - $op13"
        echo "    99 - You insert the value"
        echo -n "Wich value you want grep: "
        read optionInput
    fi

    if [ "$optionInput" -ne "99" ]; then
        opTmp="op$optionInput"
        grepValue=`echo ${!opTmp}`
    else
        echo -en "\nInsert the value to be used in the grep: "
        read grepValue
    fi

    if [ "$grepValue" == '' ]; then
        echo -e "\nError - option \"$optionInput\" is unknown\n"
    else
        echo -e "\nGrep the value: \"${!opTmp}\""
        grepResult=`cat $fileName | grep -E "$grepValue|## Working in the folder.*run_0" | sed 's/## Working in the folder.*run_0//g'`

        echo "$grepResult"

        echo -e "\nValues more clean:"
        echo "$grepResult" | rev | cut -d ':' -f1 | rev | cut -d ' ' -f2-
    fi
    echo
fi
