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
# Last update: 01/02/2017
#
if [ "$1" != "noHead" ]; then
    echo -e "\nThis script grep values from a result from a file"
else
    shift
fi

fileName=$1
if [ "$fileName" == '' ]; then
    echo -e "\nError - need pass the file name to grep\n"
else
    # Rsu
    op1="Total of messages received:"
    op2="Total count copy message received:"
    op3="Total average time to receive:"
    op4="Total average copy received:"
    op5="Total average first hops to received:"
    op6="Total average hops to received:"
    op7="counttoDeliveryMsg:"
    op8="counttoDeliveryMsgUnicID:"

    # Veh
    op9="msgUseOnlyDeliveryBufferGeneral total:"
    op10="msgUseOnlyDeliveryBufferGeneral avg:"
    op11="Final count messages drop:"
    op12="Final average buffer use:"
    op13="Final buffer use total general:"
    op14="Final count packets messages send:"
    op15="count SummaryVectorSend:"
    op16="Epidemic count RequestMessageVectorSend:"
    op17="Epidemic \(full\) is the sum of those 3:"

    countOption=17
    optionInput=$2

    ((countOption++))
    if [ "$optionInput" == '' ]; then
        i=1
        while [ "$i" -lt "$countOption" ]; do
            if [ "$i" -eq '1' ]; then
                echo -e "\n###   Rsu"
            elif [ "$i" -eq '9' ]; then
                echo -e "\n###   Veh"
            fi

            opTmp="op$i"
            echo -e "    $i - ${!opTmp}"
            ((i++))
        done
        echo -e "\n###   General"
        echo "    88 - All above"
        echo "    99 - You insert the value"

        echo -en "\nWich value you want grep: "
        read optionInput
    fi

    if [ "$optionInput" == "88" ]; then
        i=1
        while [ "$i" -lt "$countOption" ]; do
            $0 noHead $fileName $i
            ((i++))
        done

        exit 0
    elif [ "$optionInput" == "99" ]; then
        echo -en "\nInsert the value to be used in the grep: "
        read grepValue
    else
        opTmp="op$optionInput"
        grepValue=`echo "${!opTmp}"`
    fi

    if [ "$grepValue" == '' ]; then
        echo -e "\n## Error - option \"$optionInput\" is unknown"
    else
        echo -e "\n## Grep the value: $optionInput - \"${!opTmp}\""
        grepResult=`cat $fileName | grep -E "$grepValue|## Working in the folder.*run_0" | sed 's/## Working in the folder.*run_0//g'`

        echo "$grepResult"

        echo -e "\n## Values more clean:"
        echo "$grepResult" | rev | cut -d ':' -f1 | rev | cut -d ' ' -f2-

        if [ $optionInput -eq "1" ]; then
            echo -e "\n## Values more clean2:"
            echo "$grepResult" | rev | cut -d ':' -f1 | rev | cut -d ' ' -f6- | sed 's/ % received//g'
        fi
    fi
    echo
fi
