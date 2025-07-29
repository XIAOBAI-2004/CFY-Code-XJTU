from keras.preprocessing.image import load_img, img_to_array
import numpy as np

def preprocess_image(image_path):
    """
    加载并预处理图像以适应模型。
    
    参数:
    - image_path: 图像文件的路径。
    
    返回:
    - 处理后的图像数组，形状为 (1, 56, 56, 1)。
    """
    img = load_img(image_path, target_size=(56, 56), color_mode='grayscale')
    img_array = img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0)
    img_array = img_array / 255.0
    
    return img_array

def predict_image(model, image_path):
    """
    使用模型对图像进行推理。
    
    参数:
    - model: 已编译的 Keras 模型。
    - image_path: 图像文件的路径。
    
    返回:
    - 预测结果。
    """
    processed_image = preprocess_image(image_path)
    predictions = model.predict(processed_image)
    
    return predictions

# # 获取每个类别的名称
# class_indices = train_generator.class_indices
# # 反转字典以获取标签名称
# class_names = {v: k for k, v in class_indices.items()}
# print("Class names:", class_names)

import matplotlib.pyplot as plt
from PIL import Image

def display_image_from_path(image_path):
    """
    从文件路径加载并展示单张图片

    参数:
    image_path: 图片文件的路径
    """
    # 加载图像
    image = Image.open(image_path)
    
    # 显示图像
    plt.figure(figsize=(5, 5))
    plt.imshow(image, cmap='gray')  # 如果是灰度图像，可以使用 cmap='gray'
    plt.axis('off')  # 不显示坐标轴
    plt.show()

def load_model_from_file(weights_path):
    """
    从本地权重文件加载 Keras 模型。

    参数:
    - weights_path: 模型权重的文件路径。

    返回:
    - 加载的 Keras 模型。
    """
    model = tf.keras.models.load_model(weights_path)
    return model

display_image_from_path('./zym.png')

import tensorflow as tf
model=load_model_from_file("./model_weights.h5")
image_path = "./zym.png"  # 要进行推理的图像路径
predictions = predict_image(model, image_path)
# print(predictions)  # 输出预测结果
label_index=np.argmax(predictions)

emotion_dict = {
    0: 'angry',
    1: 'disgust',
    2: 'fear',
    3: 'happy',
    4: 'neutral',
    5: 'sad',
    6: 'surprise'
}

print("现在的心情是{}".format(emotion_dict[label_index]))
