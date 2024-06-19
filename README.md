# ESP32-CAM-Semantic-Search
I am a messy person who forgets where things are located in my house so I built this camera wearable to take pictures throughout my day. I can then search for objects I've seen so I don't lose track of where I put things by vectorizing images and text descriptions using OpenAI's Contrastive Language-Image Pretraining (CLIP) model.

## Video Demo
[<img src="https://i.ytimg.com/vi/QoPYoWWtxFI/oardefault.jpg?sqp=-oaymwEdCJUDENAFSFWQAgHyq4qpAwwIARUAAIhCcAHAAQY=&rs=AOn4CLDlh2RQlAirwlc8tS3Wi4lZBgS-eA" height="500">](https://www.youtube.com/shorts/QoPYoWWtxFI)

## How It Works:
1. ESP32-CAM takes a picture every 10 seconds
2. The picture is stored on the mounted SD card with the current local epoch time as the title
3. The picture is also sent to the backend server and its vector embedding is generated using OpenAI CLIP
4. This vector embedding is then appended to the vectors.bin file along with the corresponding picture title
5. When the website hosted on the ESP32-CAM is opened the user can enter a text description they want to search
6. The text description is sent to the backend server and its vector embedding is generated using OpenAI CLIP
7. The text vector and all picture vectors are compared using the cosine similarity function
8. Pictures are shown and sorted by the highest cosine similarity (aka pictures that match the text description the most)
9. Pictures can also be filtered before a certain date and time and users can clear all images and corresponding vector embeddings if space is running out on the SD card


## Setup Instructions:
1. Get an ESP32-CAM and an SD card
2. Clone the repo
3. Copy the files in the SD-Card-Files folder onto the SD card and mount the SD card on the ESP32-CAM
4. Install the required libraries for main.py
5. In a terminal type ```uvicorn main:app --host 0.0.0.0``` to start up the backend server
6. Copy the IP address link to the backend server
7. In the ESP32-CAM-Code.ino file enter your WiFi credentials, password, and the backend server link into the corresponding variables
8. Upload the ESP32-CAM-Code.ino file to the ESP32-CAM
9. Open the Serial Monitor and copy and paste the IP address shown into your browser to access the frontend hosted by the ESP32-CAM

## Current Issues and Limitations (and Potential Fixes?):
- Images sometimes take a long time to load due to the SD card having to open multiple image files. Currently, the short-term solution is to press the search button again if an image doesn't load properly. The long-term solution is to maybe use an MJPEG file to store all images similar to how all the vectors are stored in one binary file so only one file needs to be opened
- Blurry and dim photos from the ESP32-CAM if the user is moving around or the lighting is bad, which kind of confuses the CLIP model. The solution is to optimize the camera settings or just get a better camera board (the ESP32-CAM was CAD 10)
- The CLIP model is general and not optimized to the user's unique objects so it probably can't pick out a very specific cup or object you have
- If the picture has too many objects in it the semantic search for a specific object may not work well

## Potential Future Ideas:
- Picture-to-picture similarity - using images as an input to search for similar images you've seen
- Using a LiPo battery and making a proper 3D-printed headset to better house the battery and ESP32-CAM
- Activity and habit tracking
- Actually integrating the MUSE 2 headset somehow

## Additional Notes:
I mainly built this for fun and to see where this goes in the future cause I haven't built a comprehensive project like this in a long time. I also learned a lot of things such as:
- How the OpenAI CLIP model works
- Setting up and connecting to a Python backend server with FastAPI
- Sending picture and text data to the FastAPI Python backend
- Writing data from the ESP32-CAM to binary files
