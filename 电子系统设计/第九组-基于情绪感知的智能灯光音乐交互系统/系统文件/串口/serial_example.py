from periphery import Serial
import time
from collections import Counter

ser = Serial("/dev/ttyAMA0", 115200)

emotion_commands = {
    "neutral": b"@zanting%#",
    "fear": b"@SONG:4%#",
    "surprise": b"@SONG:6%#",
    "happy": b"@SONG:2%#",
    "digust": b"@SONG:3%#",
    "angry": b"@SONG:1%#",
    "sad": b"@SONG:5%#"
}

last_emotion = None

try:
    while True:
        with open('emotion_results.txt', 'r') as file:
            lines = file.read().splitlines()
            if len(lines) > 0:
                last_lines = lines[-10:]
                emotions = []
                for line in last_lines:
                    parts = line.strip().split(',')
                    if len(parts) >= 2:
                        emotion = parts[1]
                        emotions.append(emotion)

                if emotions:
                    most_common_emotion = Counter(emotions).most_common(1)[0][0]
                    if most_common_emotion != last_emotion:
                        command = emotion_commands.get(most_common_emotion, b"@zanting%#")
                        ser.write(command)
                        print(f"Sent command for most common emotion {most_common_emotion}: {command.decode()}")
                        last_emotion = most_common_emotion

        time.sleep(1)

except Exception as e:
    print(f"An error occurred: {e}")

finally:
    ser.close()