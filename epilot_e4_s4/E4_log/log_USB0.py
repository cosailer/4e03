import time
import serial

sport_0 = 'ttyUSB0'
serial_0 = serial.Serial("/dev/"+sport_0, 500000)
#file_0 = open( sport_0, 'a' )

while 1:
  line_0 = serial_0.readline()
  line_w = line_0.decode('utf8')
  
  #print(str(len(line_1))+" # "+str(line_1[0])+" # "+str(line_1))
  if len(line_0) > 20 and line_0[0]==91 and line_0[-2]==93 :
    time_seconds = time.time();
    print("0, len:" + str(len(line_0)) + ", time:" + str(time_seconds) + ", " + str(line_0))
    line_w = str(time_seconds) + ","+ str(line_w[1:-2]) + "\n"
    file_0 = open( sport_0, 'a' )
    file_0.write(line_w)
    file_0.close()
    
serial_0.close()



