import speech_recognition as sr

# Initialize recognizer class (for recognizing the speech)
r = sr.Recognizer()

#list of key words
keyWords = ["fire","shoot", "left", "right", "up", "down"]

# Reading Microphone as source
# listening the speech and store in audio_text variable
with sr.Microphone() as source:
    audio_text = r.listen(source)
    # recoginze_() method will throw a request
    # error if the API is unreachable,
    # hence using exception handling
    
    try:
        with open('voice_output.txt','w') as f:
        # using google speech recognition
            speechText = r.recognize_google(audio_text)
        #speechText = r.recognize_faster_whisper(audio_text)
            currentWord = speechText.split()
            for i in currentWord: 
                if i in keyWords:
                    if i == "left":
                        f.write(str(1))
                        exit()
                    elif i == "right":
                        f.write(str(-1))
                        exit()
                    elif i == "up":
                        f.write(str(2))
                        exit()
                    elif i == "down":
                        f.write(str(-2))
                        exit()
                    elif i == "shoot" or "fire":
                        f.write(str(3))
                        exit()
                    
                
                    
                
    except Exception as e:
        exit()
        e = 0
