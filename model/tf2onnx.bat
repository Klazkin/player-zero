call .\venv\Scripts\activate.bat
call .\venv
call .\venv\Scripts\python.exe -m tf2onnx.convert --saved-model "./latest_test_model/" --output "../gaf6/player_zero_nd.onnx"
call .\venv\Scripts\deactivate.bat
