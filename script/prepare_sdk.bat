@echo off
SET cur_path=%CD%
FOR %%A IN ("%~dp0.") DO SET project_path=%%~dpA
SET esp_idf_path=%project_path%sdk\esp-idf

%esp_idf_path%\export.bat

@echo on
echo "[esp-idf]"
cd %esp_idf_path%
git rev-parse HEAD
git describe --tags

cd %cur_path%