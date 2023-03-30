@echo off
SET cur_path=%CD%
FOR %%A IN ("%~dp0.") DO SET project_path=%%~dpA
SET esp_idf_path=%project_path%sdk\esp-idf

%esp_idf_path%\install.bat

cd %cur_path%