cur_path=${PWD}
if [[ "$OSTYPE" == "darwin"* ]]; then
    project_path=$(dirname $(dirname $(realpath $0)))
else 
    project_path=$(dirname $(dirname $(realpath $BASH_SOURCE)))
fi
esp_idf_path=${project_path}/sdk/esp-idf

if [ -z "$IDF_PATH" ]; then
  source ${esp_idf_path}/export.sh
fi
cd ${cur_path}
