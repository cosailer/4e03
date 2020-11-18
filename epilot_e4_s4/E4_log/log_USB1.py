import time
import serial

sport_1 = 'ttyUSB1'
serial_1 = serial.Serial("/dev/"+sport_1, 115200)
#file_1 = open( sport_1, 'a' )

while 1:
  line_1 = serial_1.readline()
  line_w = line_1.decode('utf8')
    
  if len(line_1) > 10 and line_1[0]==91 and line_1[-3]==93 :
    time_seconds = time.time();
    print("1, len:" + str(len(line_1)) + ", time:" + str(time_seconds) + ", " + str(line_1))
    line_w = str(time_seconds) + ", "+ str(line_w[1:-3]) + "\n"
    file_1 = open( sport_1, 'a' )
    file_1.write(line_w)
    file_1.close()
    

serial_1.close()


