# AFTONAPI - ESP32-CAM Image Receiver

This project includes a Python Flask server designed to receive JPEG images from an ESP32-S3-CAM module and save them to a specified directory. The ESP32-S3-CAM captures an image every 2 seconds and sends it over WiFi to this server.

## Server Setup

### Requirements

- Python 3
- Flask

Before running the server, ensure you have Flask installed. If you don't have Flask, you can install it using pip:

```bash
pip install Flask

## Running the Server

To get the Flask server up and running, follow these steps:

1. **Start the Server:**
   
   Navigate to the directory containing your Flask application. Then, start the server by executing:

   ```bash
   python app.py

