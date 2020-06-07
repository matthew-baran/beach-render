import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import scipy
import os

from skimage.transform import resize

vec1 = np.linspace(-3, 3, 256)
vec2 = np.ones(shape=(256,1))
gradient = vec1 * vec2
gradient = np.stack((gradient, gradient, gradient, np.zeros(shape=(256,256))), axis=2)

img = np.random.normal(0.0, 1.0, size=(256, 256))
img = scipy.ndimage.gaussian_filter(img, 10)
plt.figure()
plt.imshow(img)

img = (img - np.min(img)) / (np.max(img) - np.min(img))
img = (img - 0.5) * 2

img = np.stack((img, img, img, np.zeros(shape=(256,256))), axis=2)

img = img + gradient

[dx, dy] = np.gradient(img[:,:,1])
norms = np.stack((-dx, -dy, np.ones_like(dx)), axis=2)
mag = np.sum(np.square(norms), axis=2)
mag = np.stack((mag,mag,mag), axis=2)
norms = np.divide(norms, mag)
norms = (norms + 1) / 2
plt.imsave("bathy_norms.png", norms)

img = (img - np.min(img)) / (np.max(img)-np.min(img))
plt.imsave("bathy.png", img)

plt.figure()
plt.imshow(img)

plt.show()