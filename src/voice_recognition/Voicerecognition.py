import speech_recognition as sr

# Initialize recognizer class (for recognizing the speech)
r = sr.Recognizer()

#list of key words
keyWords = ["fire","shoot", "left", "right", "up", "down"]

# Reading Microphone as source
# listening the speech and store in audio_text variable
with sr.Microphone() as source:
    print("Talk")
    audio_text = r.listen(source)
    print("Time over, thanks")
    # recoginze_() method will throw a request
    # error if the API is unreachable,
    # hence using exception handling
    
    try:
        # using google speech recognition
        speechText = r.recognize_google(audio_text)
        #speechText = r.recognize_faster_whisper(audio_text)
        print("you said: " + speechText)
        currentWord = speechText.split()
        for i in currentWord: 
            if i in keyWords:
                if i == "left":
                    print(int(-1))
                elif i == "right":
                    print(int(1))
                elif i == "up":
                    print(int(2))
                elif i == "down":
                    print(int(-2))
                elif i == "shoot" or "fire":
                    print(int(3))
                else:
                    print(int(0))
                
    except Exception as e:
         print(e)
         print("Sorry, I did not get that")
