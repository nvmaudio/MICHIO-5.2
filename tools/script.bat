
set SWITCH=off  :: Chuyển đổi giá trị của biến này giữa "on" và "off"

if "%SWITCH%"=="on" (

    cd ..
    if exist mp2 (rd /s /q mp2)
    md mp2
    cd remind_res
    for %%i in (16K//*.mp3) do (..\tools\remind_script\ffmpeg.exe -i 16K//%%i -ab 24000 -ac 1 -ar 16000 ..\mp2//%%i.mp2 >> info.txt 2>&1)
    del info.txt
    cd ..
    for %%i in (remind_res//*.mp3) do (tools\remind_script\ffmpeg.exe -i remind_res//%%i -ab 32000 -ac 1 mp2//%%i.mp2 >> info.txt 2>&1)
    del info.txt
    cd mp2
    ren *.mp2 *.
    cd ..
    tools\remind_script\MergeAudio2BinNew.exe -a 0x0 -i ..\..\mp2
    rd /s /q mp2
    fc tools\remind_script\sound_remind_item.h app_framework\audio_engine\remind_sound_item.h
    if %errorlevel%==1 (copy tools\remind_script\sound_remind_item.h app_framework\audio_engine\remind_sound_item.h)

    rem cd tools\sys_parameter_script
    rem copy ..\..\app_src\system_config\parameter.ini parameter.ini
    rem sys_parameter parameter.ini sys_parameter.h sys_parameter.bin
    rem del parameter.ini
    rem cd ..\..
    rem fc tools\sys_parameter_script\sys_parameter.h app_src\components\flash_manage\sys_param_typedef.h
    rem if %errorlevel%==1 (copy tools\sys_parameter_script\sys_parameter.h app_src\components\flash_manage\sys_param_typedef.h)

    
) else (
    echo Skipping all operations due to SWITCH being off.
)
