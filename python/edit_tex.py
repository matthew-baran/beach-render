import matplotlib.pyplot as plt
import numpy as np

img = plt.imread(
    "gl-test/ocean_foam_texture_3_by_compasslogicstock_dar9asv-fullview.jpg"
)

mask = np.sum(np.greater(img, 140).astype(float), axis=2)
mask = (
    np.logical_and(np.equal(mask, 3), np.greater(img[:, :, 2], img[:, :, 0])).astype(
        np.uint8
    )
    * 255
)

out = np.stack((img[:, :, 0], img[:, :, 1], img[:, :, 2], mask), axis=2)

plt.imsave("foam.png", out)
