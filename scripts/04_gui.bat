call .venv\Scripts\activate.bat

pip install paho-mqtt
pip install PyQt5
pip install pyqtgraph
python %~dp0..\manager\GUI\main.py --mqtt_host "10.21.136.107"

deactivate