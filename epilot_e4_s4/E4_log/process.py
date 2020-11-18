from pylab import *
import numpy as np
import matplotlib.pyplot as plt

# extract data
data0 = np.loadtxt("02/ttyUSB0", delimiter=",")
data1 = np.loadtxt("02/ttyUSB1", delimiter=",")

# data from e4, usb 0
time_pc0 = data0[:, 0]
time_e4 = data0[:, 1]

vcc = data0[:, 2]
usb_in = data0[:, 3]
usb_out = data0[:, 4]
u_b1 = data0[:, 5]
u_b2 = data0[:, 6]
u_b3 = data0[:, 7]
u_b4 = data0[:, 8]
i_1 = data0[:, 9]
i_2 = data0[:, 10]

# reset timestamp to beginning of current
offset_0 = 0
for n in range(i_2.shape[0]):
    if i_2[n] > 600:
        offset_0 = n
        break

# time_pc0 = time_pc0[offset_0:-1]
# time_e4 = time_e4[offset_0:-1]
# vcc = vcc[offset_0:-1]
# usb_in = usb_in[offset_0:-1]
# usb_out = usb_out[offset_0:-1]
# u_b1 = u_b1[offset_0:-1]
# u_b2 = u_b2[offset_0:-1]
# u_b3 = u_b3[offset_0:-1]
# u_b4 = u_b4[offset_0:-1]
# i_1 = i_1[offset_0:-1]
# i_2 = i_2[offset_0:-1]

time_pc0 = time_pc0 - time_pc0[offset_0]  # get relative time stamp
time_pc0 = time_pc0 / 3600
time_e4 = time_e4 * 4 / 125 / 3600  # get the seconds/hour, timer e4 is 32ms multiplier


# data from power logger, usb 1
time_pc1 = data1[:, 0]
time_m32 = data1[:, 1]  # timer on mega32
u_load = data1[:, 2]
i_load = data1[:, 3]
e_load = data1[:, 4]

# reset timestamp to beginning of current
offset_1 = 0
for n in range(i_load.shape[0]):
    if i_load[n] > 600:
        offset_1 = n
        break

# time_pc1 = time_pc1[offset_1:-1]
# time_m32 = time_m32[offset_1:-1]
# u_load = u_load[offset_1:-1]
# i_load = i_load[offset_1:-1]
# e_load = e_load[offset_1:-1]


time_pc1 = time_pc1 - time_pc1[offset_1]  # get relative time stamp
time_pc1 = time_pc1 / 3600

time_m32 = time_m32 / 1000 / 3600  # get the seconds/hour, timer m32 is 1ms multiplier from arduino millis()

f1 = plt.figure(1)
l_vcc, = plt.plot(time_pc0, vcc, label='vcc')
l_usb_in, = plt.plot(time_pc0, usb_in, label='u_in')
l_usb_out, = plt.plot(time_pc0, usb_out, label='u_out')

l_u1, = plt.plot(time_pc0, u_b1, label='b1')
l_u2, = plt.plot(time_pc0, u_b2, label='b2')
l_u3, = plt.plot(time_pc0, u_b3, label='b3')
l_u4, = plt.plot(time_pc0, u_b4, label='b4')

l_u_load, = plt.plot(time_pc1, u_load, label='u_load')
plt.legend(handles=[l_vcc, l_usb_in, l_usb_out, l_u1, l_u2, l_u3, l_u4, l_u_load], loc='lower center')

plt.title('Eploit E4, mixed cells, discharge voltage plot at 0.7A')
plt.xlabel('time, H')
plt.ylabel('voltage, mV')

#plt.show()


f2 = plt.figure(2)
l_i1, = plt.plot(time_pc0, i_1, label='i1')
l_i2, = plt.plot(time_pc0, i_2, label='i2')

l_i_load, = plt.plot(time_pc1, i_load, label='i_load')
plt.legend(handles=[l_i1, l_i2, l_i_load], loc='center')

plt.title('Eploit E4, mixed cells, discharge current plot')
plt.xlabel('time, H')
plt.ylabel('current, mA')

# plt.show()


################################################################
#  re-calculate total power power_load using time_pc1, i_load and u_load
## energy = energy + loadvoltage * current_mA * 0.1 / 3600 / 1000

power_load = zeros(u_load.shape[0])

for n in range(1, u_load.shape[0]):
    power_load[n] = power_load[n - 1] + u_load[n] * i_load[n] * (time_pc1[n] - time_pc1[n - 1]) / 1000

#  re-calculate power_load_1 using time_pc0, i_1, power_load_2 using time_pc0 and i_2

# usb_out = usb_out - 300;

# power_load_1 = zeros(i_1.shape[0])
# power_load_2 = zeros(i_1.shape[0])
#
# for n in range(1, i_1.shape[0]):
#     power_load_1[n] = power_load_1[n - 1] + usb_out[n] * i_1[n] * (time_pc0[n] - time_pc0[n - 1]) / 3600000
#     power_load_2[n] = power_load_2[n - 1] + usb_out[n] * i_2[n] * (time_pc0[n] - time_pc0[n - 1]) / 3600000
#
# power_load_3 = power_load_1 + power_load_2

# f3 = plt.figure(3)
# plt.plot(time_pc0, i_1)
# plt.plot(time_e4, i_1)

# plt.plot(time_pc0, i_2)

# plt.plot(time_pc1, i_load)
# plt.plot(time_m32, i_load)

# plt.plot(time_pc1, e_load)
# plt.plot(time_pc1, power_load)
# plt.plot(time_pc0, power_load_1)
# plt.plot(time_pc0, power_load_2)
# plt.plot(time_pc0, power_load_3)

###########################################################
# use Exponential Filter to smooth battery readings

w = 0.1

u_exp = zeros(u_b4.shape[0])
u_exp[0] = u_b4[0]

for n in range(1, u_b4.shape[0]):
    u_exp[n] = w * u_b4[n] + ( 1 - w) * u_exp[n-1]

############################################################
# assign the battery readings according to the divided energy output

scale_size = 100

power_index = zeros(scale_size)
power_scale_time = zeros(scale_size)
power_scale_u_exp = zeros(scale_size)
power_scale_load = zeros(scale_size)

# get time and load for each scale
for i in range(scale_size):
    for n in range(power_load.shape[0]):
        if power_load[n] >= power_load[-1] * i / scale_size:
            power_index[i] = n
            power_scale_time[i] = time_pc1[n]
            power_scale_load[i] = power_load[n]
            break

# get voltage level for each scale
for i in range(scale_size):
    for n in range(time_pc0.shape[0]):
        if time_pc0[n] >= power_scale_time[i]:
            power_scale_u_exp[i] = u_exp[n]
            break

f4 = plt.figure(4)
l_u_b4, = plt.plot(time_pc0, u_b4, label='raw')
l_u_exp, = plt.plot(time_pc0, u_exp, label='exp')
plt.legend(handles=[l_u_b4, l_u_exp], loc='upper right')

# plt.plot(power_scale_time, power_scale_u_exp, 'o')

for i in range(power_scale_time.shape[0]):
    plt.plot(power_scale_time[i], power_scale_u_exp[i], 'bo')
    # plt.text(power_scale_time[i], power_scale_u_exp[i]+40, str((scale_size-i)*100/scale_size)+"%, "+str(int(power_scale_u_exp[i])))

plt.title('Eploit E4, voltage at each capacity level')
plt.xlabel('time, H')
plt.ylabel('voltage, mV')

# f5 = plt.figure(5)
# plt.plot(time_pc1, power_load)
# plt.plot(power_scale_time, power_scale_load, '*')
# plt.plot(power_scale_u_b1, 'o')


############################################################
# export battery level to txt file
np.savetxt('battery_scale.txt', power_scale_u_exp, delimiter='\n', fmt='%4d')

# export battery level to eeprom binary
power_scale_eeprom = power_scale_u_exp.astype(int16)
OutputFile = open("eeprom.bin", 'wb')
BlockArray = np.array(power_scale_eeprom).astype(np.uint16)
BlockArray.tofile(OutputFile)
OutputFile.close()

# print(type(power_scale_eeprom))
# print(power_scale_eeprom.shape)


plt.show()


