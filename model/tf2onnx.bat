call .\venv\Scripts\activate.bat
call .\venv\Scripts\python.exe -m tf2onnx.convert --saved-model "./winner_predictor_model/" --output wpred.onnx
call .\venv\Scripts\deactivate.bat
