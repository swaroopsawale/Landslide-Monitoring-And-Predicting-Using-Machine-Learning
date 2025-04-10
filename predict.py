import numpy as np
import tensorflow as tf
import pickle

# 1. Load saved model, scaler, and label encoder
model = tf.keras.models.load_model('model/lstm_model.h5')

with open('model/label_encoder.pkl', 'rb') as f:
    label_encoder = pickle.load(f)

with open('model/scaler.pkl', 'rb') as f:
    scaler = pickle.load(f)

# 2. Take user input
print("Enter the sensor values:")
accel_x = float(input("Accel X: "))
accel_y = float(input("Accel Y: "))
accel_z = float(input("Accel Z: "))
moisture_value = float(input("Moisture Value: "))
temperature = float(input("Temperature: "))
humidity = float(input("Humidity: "))
vibration = int(input("Vibration (0 = No, 1 = Yes): "))

# 3. Prepare the data
input_data = np.array([[accel_x, accel_y, accel_z, moisture_value, temperature, humidity, vibration]])

# 4. Scale input_data using same scaler
input_data_scaled = scaler.transform(input_data)

# 5. Reshape for LSTM
input_data_scaled = input_data_scaled.reshape((input_data_scaled.shape[0], 1, input_data_scaled.shape[1]))

# 6. Predict
prediction = model.predict(input_data_scaled)
predicted_class = np.argmax(prediction, axis=1)
predicted_label = label_encoder.inverse_transform(predicted_class)

# 7. Show output
print(f"\n🧠 Model Prediction: {predicted_label[0]}")

# 8. If landslide → give alert
if predicted_label[0].lower() == "landslide":
    print("🚨🚨 ALERT: LANDSLIDE DETECTED! 🚨🚨")
else:
    print("✅ Area Safe.")

 