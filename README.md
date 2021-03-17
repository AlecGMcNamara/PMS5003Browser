# PMS5003 Browser using 8266 esp12e board
Based on code from 'Build Web Servers' by Rui Santos. Great eBook, enjoyed reading it - thanks Rui. And thanks to anyone else that spots their code snippet! 

Redeveloped project using Web Sockets. For me this method is very fast, reliable and is the cleanest and easiest way to understand and debug IOT devices communicating with Browsers.

Connection details at the top of the main.cpp file. (PMS5003 TX --> ESP RX)

Next steps :- 
  Save Data to onboard flash (littleFS) as JSON object.
  Allow user to save data some how? (email?, webpage?)
  
Problems discovered:-

    PMS5003 needs +5V to power the internal fan, a 3.7v battery doesn't work
  
    ElegantOTA crashed the web sockets when you try to read serial port
  
    **PMS5003 TX connection to ESP RX pin** blocks the program uploading through the USB/serial port, disconnect it while programming.
  
  
