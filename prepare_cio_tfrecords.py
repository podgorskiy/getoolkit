import tqdm
import numpy as np
import random
import os
import torch.nn.functional as F
import torch
import tensorflow as tf
from PIL import Image


def rand3f(a, b):
    return ((b - a) * np.random.random_sample(3) + a).reshape(3, 1, 1)


def rand3i(a, b):
    return ((b - a) * np.random.random_sample(3) + a).reshape(3, 1, 1).astype(np.int)


def prepare_celeba():
    directory = 'dataset'
    os.makedirs(directory, exist_ok=True)

    images = []
    source_path = 'realign1024x1024'
    for filename in tqdm.tqdm(os.listdir(source_path)):
        images.append((filename[:-4], filename))

    images = images * 8
    print("Total count: %d" % len(images))
    count = len(images)
    print("Count: %d" % count)


    random.seed(0)
    random.shuffle(images)

    folds = 16
    celeba_folds = [[] for _ in range(folds)]

    count_per_fold = count // folds
    for i in range(folds):
        celeba_folds[i] += images[i * count_per_fold: (i + 1) * count_per_fold]

    path = "dataset/cio-r%02d.tfrecords.%03d"

    for i in range(folds):
        writers = {}
        for lod in range(8, 1, -1):
            tfr_opt = tf.python_io.TFRecordOptions(tf.python_io.TFRecordCompressionType.NONE)
            part_path = path % (lod, i)
            os.makedirs(os.path.dirname(part_path), exist_ok=True)
            tfr_writer = tf.python_io.TFRecordWriter(part_path, tfr_opt)
            writers[lod] = tfr_writer

        for label, filename in tqdm.tqdm(celeba_folds[i]):
            img = np.asarray(Image.open(os.path.join(source_path, filename)))
            img = img.transpose((2, 0, 1))
            a = rand3f(0.95, 1.05)
            b = rand3f(-3, 3)
            img = np.clip(img.astype(np.float32) * a + b, 0, 255).astype(np.uint8)

            for lod in range(8, 1, -1):
                ex = tf.train.Example(features=tf.train.Features(feature={
                    'shape': tf.train.Feature(int64_list=tf.train.Int64List(value=img.shape)),
                    #'label': tf.train.Feature(int64_list=tf.train.Int64List(value=[label])),
                    'data': tf.train.Feature(bytes_list=tf.train.BytesList(value=[img.tostring()]))}))
                writers[lod].write(ex.SerializeToString())

                image = torch.tensor(np.asarray(img, dtype=np.float32)).view(1, 3, img.shape[1], img.shape[2])
                image_down = F.avg_pool2d(image, 2, 2).clamp_(0, 255).to('cpu', torch.uint8).view(3, image.shape[2] // 2, image.shape[3] // 2).numpy()

                img = image_down


def run():
    output_dir = "dataset"
    os.makedirs(output_dir, exist_ok=True)

    prepare_celeba()


if __name__ == '__main__':
    run()

