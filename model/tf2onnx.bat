call .\venv\Scripts\activate.bat
call .\venv
call .\venv\Scripts\python.exe -m tf2onnx.convert --saved-model "./player_zero_model_gen1/" --output "../gaf6/player_zero_r1.onnx"
call .\venv\Scripts\deactivate.bat
