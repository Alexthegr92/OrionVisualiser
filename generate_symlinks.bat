@echo off

set REPLICA_SOURCE=PhysX-3.4\PhysX_3.4\Samples\Replicas\Source
set JUNCTION_DESTINATION=Plugins\Replicas\Source\Replicas\Source

set arg0=%0

:: Does the submodule exist?
if not exist %REPLICA_SOURCE%/*.cpp goto no_submodule

:: Create a junction to the Replica sources folder
if exist %JUNCTION_DESTINATION% goto junction_already_exists
mklink /j %JUNCTION_DESTINATION% %REPLICA_SOURCE%
goto ok_exit

:no_submodule
echo Couldn't find sources in %REPLICA_SOURCE% - did you checkout the submodule with 'git submodule update --init?'
goto error_exit

:junction_already_exists
echo %JUNCTION_DESTINATION% already exists, or this script has been run already.
goto error_exit

:error_exit
:: Pause only if we were launched with a double click
if [%arg0:~2,1%]==[:] pause
exit /b 1

:ok_exit
if [%arg0:~2,1%]==[:] pause
exit /b 0
