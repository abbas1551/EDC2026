import serial
import time
import csv

# --- CONFIGURATION ---
# --- CONFIGURATION ---
COM_PORT = '/dev/ttyUSB0'  # <--- Updated for Ubuntu
BAUD_RATE = 115200
FILE_NAME = 'circle_gestures.csv'
# FILE_NAME = 'idle.csv'
# FILE_NAME = 'random.csv'
# ---------------------
# ---------------------

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE)
    print(f"Connected to {COM_PORT}.")
    print("Recording data... Press Ctrl+C to stop and save.")
    
    with open(FILE_NAME, mode='w', newline='') as file:
        writer = csv.writer(file)
        
        while True:
            if ser.in_waiting > 0:
                # Read the line from serial, decode it, and strip whitespace
                line = ser.readline().decode('utf-8').strip()
                
                # Split the comma-separated string into a list
                data = line.split(',')
                
                # Write to the CSV file
                writer.writerow(data)
                
                # Optional: Print to console just so you know it's working
                print(data)

except KeyboardInterrupt:
    print(f"\nRecording stopped. Data saved to {FILE_NAME}")
    ser.close()
except Exception as e:
    print(f"Error: {e}")