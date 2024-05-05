# ESP32-CAM-Semantic-Search
I am a messy person who forgets where things are located in my house so I built this camera wearable to take pictures throughout my day. I can then search for objects I've seen so I don't lose track of where I put things by vectorizing images and text descriptions using OpenAI's Contrastive Language-Image Pretraining (CLIP) model.

Essentially:
1. ESP32-CAM takes a picture every 10 seconds
2. The picture is stored on the mounted SD card with the current local epoch time as the title
3. The picture is also sent to the backend server and its vector embedding is generated using OpenAI CLIP
4. This vector embedding is then appended to the vectors.bin file along with the corresponding picture title
5. When the website hosted on the ESP32-CAM is opened the user can enter a text description they want to search
6. The text description is sent to the backend server and its vector embedding is generated using OpenAI CLIP
7. The text vector and all picture vectors are compared using the cosine similarity function
8. Pictures are shown and sorted by the highest cosine similarity (aka pictures that match the text description the most)
9. Pictures can also be filtered before a certain date and time and users can clear all images and corresponding vector embeddings if space is running out on the SD card


# Installation Instructions:
1. Get an ESP32-CAM and an SD card
2. Clone the repo
3. Install the 

# Current Issues and Limitations (and Potential Fixes?):
- Images sometimes take a long time to load due to the SD card having to open multiple image files. Currently, the short-term solution is to press the search button again if an image doesn't load properly. The long-term solution is to maybe use an MJPEG file to store all images similar to how all the vectors are stored in one binary file so only one file needs to be opened
- Blurry and dim photos from the ESP32-CAM if the user is moving around or the lighting is bad, which kind of confuses the CLIP model. The solution is to optimize the camera settings or just get a better camera board (the ESP32-CAM was CAD 10)
- The CLIP model is general and not optimized to the user's unique objects so it probably can't pick out a very specific cup or object you have
- If the picture has too many objects in it the semantic search for a specific object may not work well

# Potential Future Ideas:
- Picture-to-picture similarity - using images as an input to search for similar images you've seen
- Using a LiPo battery and making a proper 3D-printed headset to better house the battery and ESP32-CAM
- Activity and habit tracking

# Additional Notes:
I mainly built this for fun and to see where this goes in the future cause I haven't built a comprehensive project like this in a long time. I also learned a lot of things such as:
- How the OpenAI CLIP model works
- Setting up and connecting to a Python backend server with FastAPI
- Sending picture and text data to the FastAPI Python backend
- Writing data from the ESP32-CAM to binary files
