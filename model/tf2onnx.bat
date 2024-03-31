call .\venv\Scripts\activate.bat
call .\venv
call .\venv\Scripts\python.exe -m tf2onnx.convert --saved-model "./player_zero_model/" --output player_zero.onnx
call .\venv\Scripts\deactivate.bat
