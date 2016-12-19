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
# Script: Parse values of depart time as the function f(x) = integerPart(x/2),
# until depart=1890. For example: f(2)=f(3)=1, f(5)=2
#
# Last update: 17/12/2016
#
fileInput=$1
if [ "$fileInput" == '' ]; then
    echo -e "\nError - need pass the file name\n"
else
    fileOutput=`echo "$fileInput" | rev | cut -d "." -f2- | rev`_parsed3.xml

    echo -e "\nThis script parse value of depart time"
    echo -e "fileInput: $fileInput\nfileOutput: $fileOutput"
    echo "The parse is f(x)= integerPart(x/5), e.g., f(2)=f(3)=1 until depart=1890"
    echo -en "\nWant continue? (y)es - (n)o: "
    read continueOrNot

    if [ "$continueOrNot" == 'y' ]; then
        cp "$fileInput" "$fileOutput"

        inValue=0
                    # (10*60 + 30) * 3 + 1 s
        while [ "$inValue" -lt 1891 ]; do
            outValue=`echo "$inValue/3" | bc`

            echo "Parsing \"$inValue\" to \"$outValue\""
            sed -i 's/depart=\"'$inValue'\"/depart=\"'$outValue'\"/g' "$fileOutput"

            ((inValue++))
        done

        echo -e "\nThe result file as saved as: \"$fileOutput\"\n"
    else
        echo -e "\nJust exiting\n"
    fi
fi
