import pandas as pd
import numpy as np
import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, Dropout
from sklearn.preprocessing import LabelEncoder, MinMaxScaler
import os
import pickle

# 1. Load the data
mpu = pd.read_csv('Datasets/mpu6500.csv')
soil = pd.read_csv('Datasets/soil_moisture.csv')
temp = pd.read_csv('Datasets/Temperature.csv')
vib = pd.read_csv('Datasets/vibration_sensor.csv')

# 2. Preprocess the data
# Merge data properly
min_len = min(len(mpu), len(soil), len(temp), len(vib))

mpu = mpu.iloc[:min_len]
soil = soil.iloc[:min_len]
temp = temp.iloc[:min_len]
vib = vib.iloc[:min_len]

# Create final dataset
data = pd.DataFrame({
    'accel_x': mpu['accel_x'].values,
    'accel_y': mpu['accel_y'].values,
    'accel_z': mpu['accel_z'].values,
    'moisture_value': soil['moisture_value'].values,
    'temperature': temp['temperature'].values,
    'humidity': temp['humidity'].values,
    'vibration': vib['vibration'].values,
    'label': mpu['label'].values
})

# 3. Encode labels
label_encoder = LabelEncoder()
data['label'] = label_encoder.fit_transform(data['label'])

# 4. Scale features
feature_columns = ['accel_x', 'accel_y', 'accel_z', 'moisture_value', 'temperature', 'humidity', 'vibration']
X = data[feature_columns].values
y = data['label'].values

scaler = MinMaxScaler()
X_scaled = scaler.fit_transform(X)

# 5. Save scaler
os.makedirs('model', exist_ok=True)
with open('model/scaler.pkl', 'wb') as f:
    pickle.dump(scaler, f)

# 6. Reshape for LSTM: (samples, timesteps, features)
X_scaled = X_scaled.reshape((X_scaled.shape[0], 1, X_scaled.shape[1]))

# 7. Build LSTM Model
model = Sequential()
model.add(LSTM(64, input_shape=(X_scaled.shape[1], X_scaled.shape[2]), return_sequences=True))
model.add(Dropout(0.2))
model.add(LSTM(32))
model.add(Dropout(0.2))
model.add(Dense(32, activation='relu'))
model.add(Dense(len(np.unique(y)), activation='softmax'))  # output neurons = number of unique labels

# 8. Compile Model
model.compile(loss='sparse_categorical_crossentropy', optimizer='adam', metrics=['accuracy'])

# 9. Train Model
model.fit(X_scaled, y, epochs=50, batch_size=32)

# 10. Save Model
model.save('model/lstm_model.h5')

# 11. Save Label Encoder
with open('model/label_encoder.pkl', 'wb') as f:
    pickle.dump(label_encoder, f)

print("✅ Model trained and saved successfully!")
