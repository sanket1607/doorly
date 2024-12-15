from flask import Flask, request, jsonify
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
import os

app = Flask(__name__)

# Email credentials
sender_email = "iotpr123@gmail.com"
receiver_email = "ssannell@uci.edu"
app_password = "xgwf weio xxdk xqnm"  # Replace with the 16-character App Password


def send_email(image_path):
    """
    Sends an email with the provided image attachment.
    """
    subject = "Alert: Someone is at your door!"
    body = "This is a notification from your smart doorbell system. Please check the attached image."

    # Create the email message
    msg = MIMEMultipart()
    msg['From'] = sender_email
    msg['To'] = receiver_email
    msg['Subject'] = subject
    msg.attach(MIMEText(body, 'plain'))

    # Attach the image
    try:
        with open(image_path, "rb") as attachment:
            mime_base = MIMEBase('application', 'octet-stream')
            mime_base.set_payload(attachment.read())
        encoders.encode_base64(mime_base)
        mime_base.add_header(
            'Content-Disposition',
            f'attachment; filename="{os.path.basename(image_path)}"'
        )
        msg.attach(mime_base)
    except FileNotFoundError:
        print("Error: Image file not found. Check the file path.")
        return False

    # Send the email
    try:
        with smtplib.SMTP('smtp.gmail.com', 587) as server:
            server.starttls()  # Start TLS encryption
            server.login(sender_email, app_password)  # Login with app password
            server.sendmail(sender_email, receiver_email, msg.as_string())  # Send email
        print("Email with image attachment sent successfully!")
        return True
    except Exception as e:
        print(f"Failed to send email: {e}")
        return False


@app.route("/", methods=["POST"])
def handle_button_press():
    """
    Handles POST requests to the root route.
    Extracts the image from the request and sends it in an email.
    """
    # Check if files are included in the request
    if 'file' not in request.files:
        return jsonify({"error": "No file part in the request"}), 400

    # Retrieve the uploaded file
    file = request.files['file']

    if file.filename == '':
        return jsonify({"error": "No file selected for uploading"}), 400

    # Save the file temporarily
    temp_path = os.path.join("/tmp", file.filename)
    file.save(temp_path)

    print("Someone is at your door!! Image received.")

    # Send the email with the uploaded image
    email_status = send_email(temp_path)

    # Remove the temporary file
    os.remove(temp_path)

    if email_status:
        return jsonify({"message": "Image received and email sent successfully"}), 200
    else:
        return jsonify({"error": "Image received but failed to send email"}), 500


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)

