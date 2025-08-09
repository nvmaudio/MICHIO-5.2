@echo off
del ..\Release\output\*.img
del ..\Release\output\main_merge.mva
grep '^#define CFG_EFFECT_PARAM_IN_FLASH_EN' ..\app_src\system_config\app_config.h | grep '//' && set config1 =-d 0 || set config1 = -d 1
grep '^#define CFG_FUNC_REMIND_SOUND_EN' ..\app_src\system_config\app_config.h | grep '//' && set config2 = -s 0 || set config2 = -s 1
..\tools\merge_script\merge.exe %config1 % %config2 %
..\tools\merge_script\Andes_MVAGenerate.exe
if not exist ..\Release\output\BT_Audio_APP.bin (del ..\Release\output\main_merge.mva)
del ..\Release\output\noSDKData.bin