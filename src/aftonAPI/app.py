from flask import Flask, request
import os
from datetime import datetime

app = Flask(__name__)

# Folder where the images will be saved
UPLOAD_FOLDER = 'uploaded_images'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route('/upload', methods=['POST'])
def upload_image():
    if 'image' not in request.files:
        return "No image part in the request", 400

    file = request.files['image']

    if file.filename == '':
        return "No image selected for uploading", 400

    if file and allowed_file(file.filename):
        # You can add a timestamp or an incremental count to filenames to avoid overwrites
        filename = datetime.now().strftime('%Y-%m-%d_%H-%M-%S') + '.jpg'
        file_path = os.path.join(UPLOAD_FOLDER, filename)
        file.save(file_path)
        return f"Image successfully uploaded and saved to {file_path}", 201
    else:
        return "Invalid file format", 400

def allowed_file(filename):
    # For simplicity, we only allow files with a .jpg extension
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ['jpg']

if __name__ == '__main__':
    app.run(host='192.168.1.12', port=5000, debug=True)
