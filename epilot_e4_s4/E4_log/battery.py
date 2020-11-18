from pylab import *

# UR18650ZY
data_10 = np.loadtxt("battery_10.txt", delimiter=",")

time_m32 = data_10[:, 0]
time_m32 = (time_m32 - time_m32[0])/1000/3600

u_load = data_10[:, 1]
i_load = data_10[:, 2]
e_load = data_10[:, 3]

# u_load = u_load / 1000

# averaging u_load with surrounding 17 values [8,x,8]

avg_size = 8
u_load_avg = zeros(u_load.shape[0])

u_load_avg[0:avg_size] = u_load[0:avg_size]
u_load_avg[-avg_size:-1] = u_load[-avg_size:-1]
u_load_avg[-1] = u_load[-1]

for n in range(avg_size, u_load.shape[0]-avg_size):
    for i in range(-avg_size, avg_size+1, 1):
        u_load_avg[n] = u_load_avg[n] + u_load[n+i]
    u_load_avg[n] = u_load_avg[n] / (2 * avg_size + 1)


# Exponential Filter

w = 0.05

u_load_exp = zeros(u_load.shape[0])
u_load_exp[0] = u_load[0]

for n in range(1, u_load.shape[0]):
    u_load_exp[n] = w * u_load[n] + ( 1 - w)* u_load_exp[n-1]

f1 = plt.figure(1)

l_u1, = plt.plot(time_m32, u_load, label='u')
plt.plot(time_m32[0], u_load[0], '*')
plt.text(time_m32[0], u_load[0], str(u_load[0]))
plt.plot(time_m32[-1], u_load[-1], '*')
plt.text(time_m32[-1], u_load[-1], str(u_load[-1]))

plt.title('UR18650ZY, discharge curve at 0.3A')
plt.xlabel('time, H')
plt.ylabel('voltage, mV')

plt.grid()


# plt.legend(handles=[l_u1, l_u_avg, l_u_exp], loc='upper right')

# f2 = plt.figure(2)
# l_i1, = plt.plot(time_m32, i_load, label='i')
# plt.legend(handles=[l_i1], loc='upper right')

plt.show()

