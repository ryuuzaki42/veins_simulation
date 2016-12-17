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
    echo "Wich value you want grep?"
    echo -e "\n1 - Total message received"
    read grepNumber

    if [ "$grepNumber" == 1 ]; then
        grepValue="Total of message"
    fi

    grepResult=`cat $fileName | grep "$grepValue"`

    echo -e "\n\n$grepResult"
    echo -e "\n\n"
    echo "$grepResult" | cut -d ':' -f4-
fi