call .\venv\Scripts\activate.bat
call .\venv\Scripts\python.exe -m tf2onnx.convert --saved-model "./saved_model/" --output model.onnx
call .\venv\Scripts\deactivate.bat
