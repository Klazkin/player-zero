call .\venv\Scripts\activate.bat
call .\venv\Scripts\python.exe -m tf2onnx.convert --saved-model "./dueler_model/" --output dueler_model.onnx
call .\venv\Scripts\deactivate.bat
