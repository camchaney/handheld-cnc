## Debugging
To produce a debug logFile to use with `debugViewer.ipynb`, make sure the variable `outputMode` in your firmware is set to `1`. Then run the following code in a *platformIO terminal*:
```
pio device monitor > /logFiles/logFile_XX.txt
```