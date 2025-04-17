import speech_recognition as sr # type: ignore
import json

# Initialize recognizer
r = sr.Recognizer()

# Keywords to listen for
keyWords = ["shoot", "snapshot", "left", "right", "up", "down"]

# Capture speech
with sr.Microphone() as source:
    print("Listening...")  # This is just for your own terminal view
    audio = r.listen(source)
    print("Done listening.")

try:
    # Recognize speech with Google Speech Recognition
    text = r.recognize_google(audio)
    print(f"You said: {text}")

    # Check for keywords
    found = [word for word in text.lower().split() if word in keyWords]

    # Save output to file
    result = {"text": text, "keywords": found}
    with open("speech_output.json", "w") as f:
        json.dump(result, f)

except Exception as e:
    # Save error to file
    with open("speech_output.json", "w") as f:
        json.dump({"error": str(e)}, f)
