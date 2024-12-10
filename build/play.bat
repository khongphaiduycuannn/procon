@echo off
set QUESTION_ID=70

set WORKING_PATH=%~dp0
set WORKING_PATH=%WORKING_PATH:\=/%
set INPUT_PATH=%WORKING_PATH%input.json
set OUTPUT_PATH=%WORKING_PATH%output.json
set GET_INPUT_PATH=%WORKING_PATH%get_input.py
@REM set SUBMIT_PATH=%WORKING_PATH%submit.py
set SOLVER_PATH=%WORKING_PATH%solver_faster.exe
set TOKEN=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6MjcsIm5hbWUiOiJIQVVJIiwiaXNfYWRtaW4iOmZhbHNlLCJpYXQiOjE3MzM2NzI4NzMsImV4cCI6MTczMzg0NTY3M30.CF97B-Jeoi7evFygEeZevCe29lTQxQlGAJyAyJS5aD8

@REM py %GET_INPUT_PATH% %TOKEN% %QUESTION_ID% %INPUT_PATH%
set startTime=%time%
%SOLVER_PATH% %INPUT_PATH% %OUTPUT_PATH%
set endTime=%time%
@REM py %SUBMIT_PATH% %TOKEN% %QUESTION_ID% %OUTPUT_PATH%

for /F "tokens=1-4 delims=:.," %%a in ("%startTime%") do (
    set /A "startMs=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
for /F "tokens=1-4 delims=:.," %%a in ("%endTime%") do (
    set /A "endMs=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /A elapsedMs=endMs-startMs
set /A elapsedSec=elapsedMs/100
set /A ms=elapsedMs %% 100
echo Elapsed time: %elapsedSec%.%ms% second