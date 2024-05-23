import serial
import time
import threading
import requests
import pygame
from config import AUDIO_TRACKS, SERIAL_PORT, DATA_URL

pygame.mixer.init()

tracks = {}
for number, path in AUDIO_TRACKS.items():
    tracks[number] = {
        "track": path,
        "channel": pygame.mixer.Channel(int(number) - 1),
        "is_playing": False
    }

def read_from_port(ser):
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            print(line)
            process_command(line)

def process_command(line):
    parts = line.split()
    if len(parts) == 2:
        command, number = parts
        if command == "ON" and number in tracks:
            track_info = tracks[number]
            if not track_info["channel"].get_busy():  
                sound = pygame.mixer.Sound(track_info["track"])
                track_info["channel"].play(sound)
                track_info["is_playing"] = True
        elif command == "OFF" and number in tracks:
            track_info = tracks[number]
            if track_info["is_playing"]:  
                track_info["channel"].stop()
                track_info["is_playing"] = False

def do_call():
    url = DATA_URL
    try:
        with requests.Session() as session:
            response = session.get(url)
            response.raise_for_status()  
            data = response.json()

        result = ','.join([
            str(data.get("Current World Population", 'N/A')),
            str(data.get("Deaths today", 'N/A')),
            str(data.get("People who died of hunger today", 'N/A')),
            str(data.get("Undernourished people in the world", 'N/A')),
            str(data.get("Deaths of mothers during birth today", 'N/A')),
            str(data.get("Deaths caused by water related diseases today", 'N/A')),
            str(data.get("Suicides today", 'N/A'))
        ])
        return result + '\n'
    except requests.RequestException as e:
        print(f"Failed to fetch data: {str(e)}\n")

if __name__ == '__main__':
    ser = serial.Serial(SERIAL_PORT, 9600, timeout=1)
    ser.reset_input_buffer()
    thread = threading.Thread(target=read_from_port, args=(ser,))
    thread.daemon = True  
    thread.start()

    while True:
        result = do_call()
        ser.write(result.encode())
        time.sleep(1) 
