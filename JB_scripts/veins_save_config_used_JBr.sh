#!/bin/bash

countIniFile=`ls *.ini | wc -l`

if [ "$countIniFile" == '' ]; then
    echo -e  "\nError: the \"omnet.ini\" not found\n"
elif [ $countIniFile -gt 1 ]; then
    echo -e "\nError: the has more than one \"omnet.ini\" file\n"
else # $countIniFile == 1
    # grep some values from omnet.ini file
    iniFileGrepAdd=`cat *.ini | grep -E "\*.\*.appl.|sim|rsu.*.mobility|txPower|pMax|^$|launchConfig"`
    #echo -e "##\n$iniFileGrepAdd\n##"

    # remove some value from grep previous
    iniFileGrepRm=`echo "$iniFileGrepAdd" | grep -v -E "^#|raffic|folder|cout|mobility.z"`
    #echo -e "##\n$iniFileGrepRm\n##"

    # replace multiple empty line with one empty line
    iniFileSedEmptyLine=`echo "$iniFileGrepRm" | sed '/^$/N;/^\n$/D'`
    #echo -e "##\n$iniFileSedEmptyLine\n##"

    echo -e "$iniFileSedEmptyLine\n"
fi
