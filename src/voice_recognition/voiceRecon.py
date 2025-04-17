import speech_recognition as sr # type: ignore
import json

r = sr.Recognizer()
keyWords = ["shoot", "snapshot", "left", "right", "up", "down"]

with sr.Microphone() as source:
    print("Listening...")
    audio = r.listen(source)
    print("Done listening.")

    try:
        text = r.recognize_google(audio)
        found = [word for word in text.split() if word in keyWords]
        result = {"text": text, "keywords": found}
        print(json.dumps(result))  # send as JSON to C++
    except Exception as e:
        print(json.dumps({"error": str(e)}))
