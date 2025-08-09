@echo off
cd ..
if exist mp2 (rd /s /q mp2)
md mp2
cd remind_res
for %%i in (16K//*.mp3) do (..\tools\remind_script\ffmpeg.exe -i 16K//%%i -ab 24000 -ac 1 -ar 16000 ..\mp2//%%i.mp2)
cd ..
for %%i in (remind_res//*.mp3) do (tools\remind_script\ffmpeg.exe -i remind_res//%%i -ab 48000 -ac 1 mp2//%%i.mp2)
cd mp2
ren *.mp2 *.
cd ..
tools\remind_script\MergeAudio2BinNew.exe -a 0x0 -i ..\..\mp2
rd /s /q mp2


rem cd tools\sys_parameter_script
rem copy ..\..\app_src\system_config\parameter.ini parameter.ini
rem sys_parameter parameter.ini sys_parameter.h sys_parameter.bin
rem del parameter.ini
rem cd ..


del ..\Release\output\*.img
del ..\Release\output\main_merge.mva
grep '^#define CFG_EFFECT_PARAM_IN_FLASH_EN' ..\app_src\system_config\app_config.h | grep '//' && set config1 =-d 0 || set config1 = -d 1
grep '^#define CFG_FUNC_REMIND_SOUND_EN' ..\app_src\system_config\app_config.h | grep '//' && set config2 = -s 0 || set config2 = -s 1
..\tools\merge_script\merge.exe %config1 % %config2 %
..\tools\merge_script\Andes_MVAGenerate.exe
if not exist ..\Release\output\BT_Audio_APP.bin (del ..\Release\output\main_merge.mva)
del ..\Release\output\noSDKData.bin
pause
