import numpy as np
import matplotlib.pyplot as plt
import math

depth = np.linspace(-3, 3, 100);
wavelength = 35;

v = np.sqrt(9.8*2*math.pi/wavelength * np.tanh(2*math.pi*depth/wavelength))

plt.figure()
plt.plot(depth,v)
plt.show()