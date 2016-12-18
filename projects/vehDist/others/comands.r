## randomTrips
    http://sumo.dlr.de/wiki/Tools/Trip
    SUMO older tool folder: /media/sda4/prog/sumo-0.25.0/tools
    SUMO tool folder: /opt/sumo-0.25.0/tools/

    python /opt/sumo-0.25.0/tools/randomTrips.py --help
    http://sumo.dlr.de/wiki/FAQ#How_do_I_generate_random_routes.3F

## km^2, grid: (1, 5) (4, 9) (9, 13) (16, 17) (25, 21) (36, 25)

## Generate the grid 1 km^2
    netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o vehDist.net.xml
    # or with no --no-turnarounds
    netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --no-turnarounds --default.speed 85 -o vehDist.net.xml
        # Change --default.lanenumber to 1; added --no-turnarounds and the (max) speed to 85

## Generate the grid 9 km^2
    netgenerate -g --grid.number=13 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o vehDist.net.xml
    # or with no --no-turnarounds
    netgenerate -g --grid.number=13 --grid.length=250 --default.lanenumber=1 --no-turnarounds --default.speed 85 -o vehDist.net.xml
    ## Afer this command run the script
    ./gridConvertOneCharacter.sh 13

## Generate the grid 36 km^2
    netgenerate -g --grid.number=25 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o vehDist.net.xml
    # or with no --no-turnarounds
    netgenerate -g --grid.number=25 --grid.length=250 --default.lanenumber=1 --no-turnarounds --default.speed 85 -o vehDist.net.xml
    ## Afer this command run the script
    ./gridConvertOneCharacter.sh 25

## Generate the trips and the routes (with various distance)
    # Generate 3 M routes
    python /opt/sumo-0.25.0/tools/randomTrips.py -n vehDist.net.xml --min-distance=100000 -b 0 -e 3000 -i 200 -s 1 -r vehDist_tmp.rou.xml
    # Generate 30 M routes
    python /opt/sumo-0.25.0/tools/randomTrips.py -n vehDist.net.xml --min-distance=100000 -b 0 -e 30000 -i 200 -s 1 -r vehDist_tmp.rou.xml
        # The -s (seed) 1, for generate the same routes every time that run the command

## To test "is the same routes?"
    cat vehDist.rou.xml | grep -v generated | md5sum

## Wants TLS?
    # http://sumo.dlr.de/wiki/NETCONVERT
    netgenerate -g --grid.number=5 --grid.length=250 --default.lanenumber=1 --default.speed 85 -o vehDist.net.xml --tls.set "0/0, 0/1, 0/2, 0/3, 0/4, 1/0, 1/1, 1/2, 1/3, 1/4, 2/0, 2/1, 2/2, 2/3, 2/4, 3/0, 3/1, 3/2, 3/3, 3/4, 4/0, 4/1, 4/2, 4/3, 4/4"

## Test same start position
    cat [folder]/vehicle_position_initialize.r | grep -v number  | md5sum

## Change maxSpeed
    sed -i 's/maxSpeed=.* color/maxSpeed=\"15\" color/g' test_end.rou.xml
    sed -i 's/maxSpeed=.* color/maxSpeed=\"25\" color/g' test_end.rou.xml

## See iterations from ini file
    # cat General-*.sca | grep "attr iterationvars2"
    i=0; while [ $i -lt 32 ]; do cat General-$i.sca | grep "attr iterationvars2"; i=$((i+1)); done

## OMNeT run project on terminal
    -u = mode (com GUI - Tkenv, sem GUI - Cmdenv) -f = arquivo ini -r = nº de execucão -l = libveins -n = arquivos ned
    # sem GUI e todas as execuções
        opp_run -u Cmdenv -n ../../src/veins/ -l ../../src/libveins.so -f vehDist.ini > run.r
    # com GUI e todas as execuções
        opp_run -u Tkenv -n ../../src/veins/ -l ../../src/libveins.so -f vehDist.ini > run.r
    # Execuções de 0 até 3
        opp_run -r 0..3 -u Tkenv -n ../../src/veins/ -l ../../src/libveins.so -f vehDist.ini > run.r
        opp_run -r 0..3 -u Cmdenv -n ../../src/veins/ -l ../../src/libveins.so -f vehDist.ini > run.r

## Script in R ##
## Execute R < script_r.r or source('script_r.r', echo=TRUE)
    calculeDistance <- function() {
        ## copy in a=c("values") and b=c("values")
        a=c(1030.0,1300.0,0)
        b=c(520,520,3)
        sqrt((a[1]-b[1])^2 + (a[2] - b[2])^2)
    }

    calculeDistance()

## See if the selected vehicle to generate messages are the same
    kate results/*/*/*/*/VehRsu_Messages_Generated.r

## Error ASSERT: condition count == drivingVehicleCount false in function processVehicleSubscription, veins/modules/mobility/traci/TraCIScenarioManager.cc line 759
    Commenting out the ASSERT is totally fine.
    In SUMO any vehicle can have one of five states (according to statesvehicleStates_sm.uxf):
    first, it is loaded, transitions to running when it starts driving, then transitions to arrived when it arrived at its destination.
    In addition, running vehicles can temporarily become teleporting or parking.
    Veins subscribes to these state changes to keep track of the number of driving vehicles.
    To make sure that the bookkeeping is correct, it compares its own count against SUMO''s reported number of active vehicles.
    I do not know why the numbers do not match sometimes. It only seems to occur in large congested networks.
    # Link: http://stackoverflow.com/questions/31605511/assert-condition-for-driving-vehicles-in-veins-failed
