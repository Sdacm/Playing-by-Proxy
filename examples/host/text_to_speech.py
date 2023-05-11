from gtts import gTTS
import os

testText = "hello everybody"
language = 'en'

output = gTTS(text = testText, lang = language, slow = False)

output.save("output.wav")

os.system("sart output.wav")
